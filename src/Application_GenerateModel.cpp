#include "Application.h"
#include "XMLParserPugi.h"
#include "constants.h"
#include <cassert>
#include "FuncSquare.h"
#include "FuncIntensity.h"
#include "FuncOmega.h"
#include "WaspDateConverter.h"
#include "CopyParameters.h"

#define _USESTDVECTOR_
#include <nr/nr3.h>
#include <nr/ran.h>
#include <nr/gamma.h>
#include <nr/deviates.h>

namespace
{
    /** Local function only to this file
     
     does the hard number crunching part of the code 
     so that the other functions can just interface this one */
    Lightcurve GenerateSyntheticFromParams(const vector<double> &Time, double period, double midpoint, const vector<double> &coeffs, 
                                           double semi, double rPlan, double rStar, double inclination, double dr, double noise)
    {
        double normalisedDistance = semi / rStar;
        cout << "Normalisation constant: " << normalisedDistance << endl;
        double omega = calcOmega(coeffs);
        double angFreq = 2. * M_PI / period;
        cout << "Angular frequency: " << angFreq << " rad per sec" << endl;
        
        /* get the cosine of the inclination */
        double cosi = cos(inclination);
        
        const double p = rPlan / rStar;
        
        /* set up the random number generator */
        Normaldev_BM randGenerator(0., 1., time(NULL));


        
        /* Output lightcurve */
        Lightcurve lc(Time.size());
        lc.period = period;
        lc.epoch = midpoint;
        
        /* parallel process this part */
#pragma omp parallel for
        for (unsigned int i=0; i<Time.size(); ++i)
        {
            double t = Time.at(i);
            
            /* get the normalised device coordinates */
            double firstTerm = square(sin(angFreq * t));
            double secondTerm = square(cosi * cos(angFreq * t));
            
            
            
            
            double z = normalisedDistance * sqrt(firstTerm + secondTerm);
            
            double intpart;
            double phase = fabs(modf(t  / period , &intpart));
            phase = phase > 0.5 ? phase - 1.0 : phase;
            
            double F = 0;
            
            /* Hack to make sure the secondary eclipse is not created */
            if ((phase > -0.25) && (phase < 0.25))
            {
                
                
                if (z <= 1 - p)
                {
                    F = 0.;
                    //F = 1. - square(p);
                    double norm = 1. / (4. * z * p);
                    double integral = IntegratedI(dr, coeffs, z-p, z+p);
                    integral *= norm;
                    F = 1. - (square(p) * integral / 4. / omega);
                }
                else if (z > 1 + p)
                {
                    F = 1.;
                }
                else
                {
                    double startPoint = z - p;
                    double a = square(startPoint);
                    double norm = 1./(1 - a);
                    
                    /* Integrate the I*(z) function from startPoint to 1 */
                    double integral = IntegratedI(dr, coeffs, startPoint, 1.);
                    integral *= norm;
                    
                    double insideSqrt = square(p) - square(z - 1.);
                    double sqrtVal = sqrt(insideSqrt);
                    sqrtVal *= (z - 1.);
                    
                    double insideAcos = (z - 1.) / p;
                    double firstTerm = square(p) * acos(insideAcos);
                    
                    F = 1. - (integral * (firstTerm - sqrtVal) / (4. * M_PI * omega));
                }
                
            }
            else
            {
                F = 1.;
            }
            
            /* add the noise */
            F += noise * randGenerator.dev();
            
            /* append the data to the vectors */
            lc.jd[i] = t / secondsInDay + midpoint;
            lc.flux[i] = F;
            
        }
        //outfile.close();
        
        lc.radius = rPlan / rJup;
        return lc;




    }
    
    
    
}


/** Create a model from the xml config file
 *
 * Takes a std::string filename and returns a Lightcurve object 
 *
 * \param xmlfilename Name of the input filename
 */
Lightcurve Application::GenerateModel(const string &xmlfilename)
{
    /* set up the conversion constants */
    Config::Config config;
    config.LoadFromFile(xmlfilename);


    const double rPlan = config.getPlanetRadius();
    const double rStar = config.getStarRadius();


    const double period = config.getPeriod();
    const double semi = config.getSemi(); 
    const double inclination = config.getInclination();
    const double maxtime = config.getMaxTime() / 2.;            // the 2 is a fudge factor
    const double dt = config.getDT();
    const double dr = config.getDR();
    const double midpoint = config.getMidpoint();
    const double noise = config.getNoise();

    cout << "Planet radius: " << rPlan << " m" << endl;
    cout << "Star radius: " << rStar << " m" << endl;
    cout << "Radius ratio: " << rPlan / rStar << "" << endl;
    cout << "Period: " << period << " seconds" << endl;
    cout << "Semi-axis: " << semi << " m " << endl;
    cout << "Inclination: " << inclination << " rad" << endl;
    cout << "Max simulation time: " << maxtime << " seconds" << endl;
    cout << "Time step: " << dt << " seconds" << endl;
    cout << "Integral step: " << dr * rSun << " m" << endl;
    cout << "Transit mid point: " << midpoint << endl;
    cout << "Simulated noise: " << noise * 100. << "%" << endl;




    //cout << "P: " << p << endl;
    //cout << "P^2: " << square(p) << endl;
    const vector<double> coeffs = config.getCoeffs();


    /* c0 must be greater than 0. */
    assert(coeffs[0] > 0.);








    /* generate the time array */
    vector<double> time;

    for (double t=0; t<maxtime; t+=dt)
    {
        time.push_back(t);
    }


    
    //ofstream outfile("TransitModel.txt");
    //outfile.precision(15);
    
    /* Now calculate the lightcurve */
    Lightcurve OutputLightcurve = GenerateSyntheticFromParams(time, period, midpoint, coeffs, semi, rPlan, rStar, inclination, dr, noise);
    
    /* Update the lightcurve's parameters */
    CopyParameters(OutputLightcurve, period, midpoint, rPlan, rStar, inclination, semi);
    
    return OutputLightcurve;
}

/** Function to generate a lightcurve given a data set
 
 The input data set is required to get the phase grid coverage from the data. This way 
 the model only needs to be exactly generated on the data's grid and no errors will be 
 induced into this process */
Lightcurve Application::GenerateModel(const std::string &xmlfilename, const Lightcurve &SourceData)
{
    
    /* set up the conversion constants */
    Config::Config config;
    config.LoadFromFile(xmlfilename);
    
    
    const double rPlan = config.getPlanetRadius();
    const double rStar = config.getStarRadius();
    
    
    const double period = config.getPeriod();
    const double semi = config.getSemi(); 
    const double inclination = config.getInclination();
    const double maxtime = config.getMaxTime() / 2.;            // the 2 is a fudge factor
    const double dt = config.getDT();
    const double dr = config.getDR();
    const double midpoint = config.getMidpoint();
    const double noise = config.getNoise();
    
    cout << "Planet radius: " << rPlan << " m" << endl;
    cout << "Star radius: " << rStar << " m" << endl;
    cout << "Radius ratio: " << rPlan / rStar << "" << endl;
    cout << "Period: " << period << " seconds" << endl;
    cout << "Semi-axis: " << semi << " m " << endl;
    cout << "Inclination: " << inclination << " rad" << endl;
    cout << "Max simulation time: " << maxtime << " seconds" << endl;
    cout << "Time step: " << dt << " seconds" << endl;
    cout << "Integral step: " << dr * rSun << " m" << endl;
    cout << "Transit mid point: " << midpoint << endl;
    cout << "Simulated noise: " << noise * 100. << "%" << endl;
    
    
    
    
    //cout << "P: " << p << endl;
    //cout << "P^2: " << square(p) << endl;
    const vector<double> coeffs = config.getCoeffs();
    
    
    /* c0 must be greater than 0. */
    assert(coeffs[0] > 0.);
    

    /* Need to get the data's time data */
    vector<double> TimeData = SourceData.jd;
    /* The time data has to be in seconds */
    
    /* Need to convert this to time since epoch */
    const double DataEpoch = midpoint;
    if (SourceData.asWASP)
    {
        /* Data already in seconds so ignore */
        for (vector<double>::iterator i=TimeData.begin();
             i!=TimeData.end();
             ++i)
        {
            *i = *i - jd2wd(DataEpoch);
            
        }
    
    }
    else
    {
        for (vector<double>::iterator i=TimeData.begin();
             i!=TimeData.end();
             ++i)
        {
            *i = (*i - DataEpoch) * secondsInDay;
            
        }
    }

    
    
    Lightcurve OutputLightcurve = GenerateSyntheticFromParams(TimeData, period, midpoint, coeffs, semi, rPlan, rStar, inclination, dr, noise);
    
    /* Update the lightcurve's parameters */
    CopyParameters(OutputLightcurve, period, midpoint, rPlan, rStar, inclination, semi);

    
    
    
    
    
    return OutputLightcurve;
}

