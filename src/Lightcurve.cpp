#include "Lightcurve.h"
#include "WaspDateConverter.h"
#include <cmath>

using namespace std;

Lightcurve::Lightcurve(size_t n)
: npts(n), period(0), epoch(0)
{
    this->jd = vector<double>(n);
    this->flux = vector<double>(n);
    this->fluxerr = vector<double>(n);
}

size_t Lightcurve::size()
{
    return npts;
}

void Lightcurve::clear()
{
    jd.clear();
    flux.clear();
    fluxerr.clear();
    npts = flux.size();
}

vector<double> Lightcurve::phase()
{
    size_t N = this->size();
    
    
    
    vector<double> ReturnPhaseVal(N);
    
    
    for (unsigned int i=0; i<N; ++i)
    {
        double currentJDValue = this->jd[i];
        
        /*  need to convert from wd if this->asWASP is true */
        if (asWASP)
            currentJDValue = wd2jd(currentJDValue);

        /*  if the jd value is nan then put nan into the array */
        if (isnan(currentJDValue))
        {
            ReturnPhaseVal[i] = currentJDValue;
        }
        else
        {
            double phaseval = (currentJDValue - this->epoch) / double(this->period / secondsInDay);
            double nperiods = 0;
            double remainder = fabs(modf(phaseval, &nperiods));
            
            if (remainder > 0.5)
            {
                ReturnPhaseVal[i] = remainder - 1.0;
            }
            else
            {
                ReturnPhaseVal[i] = remainder;
            }
        
        }
    }
    
    
    
    return ReturnPhaseVal;
    
}