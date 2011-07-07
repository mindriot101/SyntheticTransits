#include "AlterTransit.h"
#include "SortedIndex.h"
#include <vector>

using namespace std;



Lightcurve RemoveTransit(Lightcurve &data, Lightcurve &model)
{
    /** 
     * \bug Code does not take into account nans 
     * */

    /*  set up an output file for debugging */
    ofstream debugoutfile("debug.dat");
    debugoutfile.precision(15);

    /*  get the phase values */
    /*  the model SHOULD NOT contain nans */
    vector<double> modelPhase = model.phase();

    /*  must sort the input to the interpolator */
    vector<pair<double, int> > SortedModelPhase = SortedIndex(modelPhase);
    vector<double> SortedPhaseOnly(modelPhase.size()), SortedModelOnly(modelPhase.size());
    for (int i=0; i<modelPhase.size(); ++i)
    {
        SortedPhaseOnly[i] = SortedModelPhase[i].first;
        SortedModelOnly[i] = model.flux[SortedModelPhase[i].second];
    }


    
    /*  set up the output values initially as a copy of the input data */
    Lightcurve output = data;
    
    Linear_interp interpolator(SortedPhaseOnly, SortedModelOnly);

    
    /*  calculate the average of the data */
    double dataAv = 0;
    int dataCounter = 0;
    for (vector<double>::const_iterator i=data.flux.begin();
         i!=data.flux.end();
         ++i)
    {
        if (!isnan(*i))
        {
            /*  not nan */
            dataAv += *i;
            ++dataCounter;
        }
    }

    dataAv /= (double)dataCounter;



    /*  now interpolate onto the data's phase grid */
    vector<double> dataPhase = data.phase();
    /*  create the output array */
    //vector<double> LCWithFluxRemoved(data.size());
    for (size_t i=0; i<dataPhase.size(); ++i)
    {
        double InterpolatedModelValue = interpolator.interp(dataPhase[i]);

        /*  get the data phase value */
        double dataPhaseValue = dataPhase[i];
        if (isnan(dataPhaseValue))
        {
            /*  phase is null so ignore for now */
            output.flux[i] = dataPhaseValue;
        }
        else
        {
            /*  get the data flux value */
            double dataFluxValue = data.flux[i];
            if (isnan(dataFluxValue))
            {
                /*  flux is null so put a null in the output array */
                output.flux[i] = dataFluxValue;
            }
            else
            {
                /*  both data and phase are well behaved */
                double NormalisedFluxValue = dataFluxValue / dataAv;
                output.flux[i] = ((NormalisedFluxValue - InterpolatedModelValue) + 1.0) * dataAv;
            }
        }


    }
    

    debugoutfile.close();
    
        
    
    return output;
        
}



Lightcurve AddTransit(Lightcurve &data, Lightcurve &model)
{
    /** 
     * \bug Code does not take into account nans 
     * */

    /*  set up an output file for debugging */
    ofstream debugoutfile("debug.dat");
    debugoutfile.precision(15);

    /*  get the phase values */
    /*  the model SHOULD NOT contain nans */
    vector<double> modelPhase = model.phase();

    /*  must sort the input to the interpolator */
    vector<pair<double, int> > SortedModelPhase = SortedIndex(modelPhase);
    vector<double> SortedPhaseOnly(modelPhase.size()), SortedModelOnly(modelPhase.size());
    for (int i=0; i<modelPhase.size(); ++i)
    {
        SortedPhaseOnly[i] = SortedModelPhase[i].first;
        SortedModelOnly[i] = model.flux[SortedModelPhase[i].second];
    }


    
    /*  set up the output values initially as a copy of the input data */
    Lightcurve output = data;
    
    Linear_interp interpolator(SortedPhaseOnly, SortedModelOnly);

    
    /*  calculate the average of the data */
    double dataAv = 0;
    int dataCounter = 0;
    for (vector<double>::const_iterator i=data.flux.begin();
         i!=data.flux.end();
         ++i)
    {
        if (!isnan(*i))
        {
            /*  not nan */
            dataAv += *i;
            ++dataCounter;
        }
    }

    dataAv /= (double)dataCounter;



    /*  now interpolate onto the data's phase grid */
    vector<double> dataPhase = data.phase();
    /*  create the output array */
    //vector<double> LCWithFluxRemoved(data.size());
    for (size_t i=0; i<dataPhase.size(); ++i)
    {
        double InterpolatedModelValue = interpolator.interp(dataPhase[i]);

        /*  get the data phase value */
        double dataPhaseValue = dataPhase[i];
        if (isnan(dataPhaseValue))
        {
            /*  phase is null so ignore for now */
            output.flux[i] = dataPhaseValue;
        }
        else
        {
            /*  get the data flux value */
            double dataFluxValue = data.flux[i];
            if (isnan(dataFluxValue))
            {
                /*  flux is null so put a null in the output array */
                output.flux[i] = dataFluxValue;
            }
            else
            {
                /*  both data and phase are well behaved */
                double NormalisedFluxValue = dataFluxValue / dataAv;
                output.flux[i] = ((NormalisedFluxValue + InterpolatedModelValue) - 1.0) * dataAv;
            }
        }


    }
    

    debugoutfile.close();
    
        
    
    return output;
        
}

Lightcurve AlterTransit(Lightcurve &data, Lightcurve &subModel, Lightcurve &addModel)
{
    /*  first subtract... */
    Lightcurve LCRemoved = RemoveTransit(data, subModel);

    /*  then add */
    return AddTransit(LCRemoved, addModel);

}