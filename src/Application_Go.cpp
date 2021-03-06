#include <iostream>
#include <cmath>
#include <CCfits/CCfits>
#include <vector>
#include <tclap/CmdLine.h>
#include <sstream>
#include <fstream>
#include <pugixml.hpp>

//#include <glog/logging.h>

/*  boost includes */
#include <boost/filesystem.hpp>



/* local includes */
#include "Lightcurve.h"
#include "Application.h"
#include "Exceptions.h"
#include "AlterTransit.h"
#include "GetSystemMemory.h"
#include "CopyFileEfficiently.h"
#include "ValidXML.h"
#include "ObjectSkipDefs.h"
#include "CopyParameters.h"
#include "timer.h"



using namespace std;
using namespace CCfits;
namespace bf = boost::filesystem;
namespace ad = AlterDetrending;





typedef list<string> StringList;


/* steps
 *
 * Read in the xml file and generate a model lightcurve
 *
 * Open data file
 * - if new file is selected then directly copy the file to a
 *   new location
 *
 * Select the object in the file
 *
 * Subtract the subtracting model
 *
 * (optionally) Add the adding model 
 *
 * Close the file */


namespace 
{
    /** Function to get the number of objects in a fits file
     *
     * Static funciton as it's only required in the Application::go() method */
    int getNObjects(const string &filename)
    {
        int nObjects = -1;
        /*  assume the number of objects is just the number of entries in the
         *  catalogue hdu */
        auto_ptr<FITS> pInfile(new FITS(filename, Read));

        ExtHDU &CatalogueHDU = pInfile->extension("CATALOGUE");
        nObjects = CatalogueHDU.rows();

        return nObjects;
    }

    /** Function to get the object name from an xml file */
    string ObjectFromXML(const string &xmlfilename)
    {
        using namespace pugi;
        xml_document doc;
        xml_parse_result result = doc.load_file(xmlfilename.c_str());

        /* Move down the tree until the info -> star -> obj_id -> value is retrieved */
        string ObjectName = doc.child("info").child("star").child("obj_id").attribute("val").value();
        return ObjectName;

    }


}








/**	The main function, abstracted from the main function for good separation
 *
 * Runs all of the later code and garruntees RAII behaviour */
int Application::go(int argc, char *argv[])
{
    /* new main function */
    TCLAP::CmdLine cmd("Synthetic lightcurves", ' ', "1.0");
    TCLAP::ValueArg<float> memlimit_arg("M", "memorylimit", "Fraction of system memory to use", false, 0.1, "0-1", cmd);
    TCLAP::SwitchArg wasptreatment_arg("w", "wasp", "Do not treat as WASP object", cmd, true);
    TCLAP::ValueArg<string> output_arg("o", "output", "Optional output file", false, "synthout.fits", "Fits filename", cmd);
    //    TCLAP::ValueArg<string> objectid_arg("O", "object", "Object to alter", true, "", "Object identifier", cmd);
    TCLAP::ValueArg<string> subModel_arg("s", "submodel", "Model to subtract", true, "", "Model xml file", cmd);
    TCLAP::ValueArg<string> addModelFilename_arg("a", "addmodels", "List of model files", true, "", "List of files", cmd);
    TCLAP::UnlabeledValueArg<string> filename_arg("file", "File", true, "", "Fits file", cmd);



    cmd.parse(argc, argv);

    /* Start a timer */
    Timer timer;
    timer.start("all");
    timer.start("config");

    /*  get the system memory */
    long SystemMemory = getTotalSystemMemory();

    cout  << "System memory: " << SystemMemory / 1024. / 1024. << " MB" << endl;
    float MemFraction = memlimit_arg.getValue();

    /*  make sure this is within the range 0-1 */
    if ((MemFraction <= 0) || (MemFraction > 1))
    {
        throw MemoryException("Allowed memory is within range 0-1");
    }



    string ObjectName = ObjectFromXML(subModel_arg.getValue());
    cout << "Object name: " << ObjectName << endl;


    /*  print if the object is from wasp or not */
    const bool asWASP = wasptreatment_arg.getValue();
    if (asWASP)
    {
        cout << "WASP object chosen" << endl;
    }
    else
    {
        cout << "Non-WASP object chosen" << endl;
    }

    /*  need to get the number of objects that were originally in the 
     *  file so we know which index to add the nExtra objects at */
    const int nObjects = getNObjects(filename_arg.getValue());

    /*  string for storing the filename */
    string DataFilename = output_arg.getValue();
    timer.stop("config");


    /*  need to get the list of extra models to add */
    int nExtra = 0;


    StringList AddModelFilenames;

    /*  need to get the path of the models list file */
    bf::path BasePath = bf::path(addModelFilename_arg.getValue()).parent_path();
    cout << "Using base path: " << BasePath << endl;




    ifstream ModelsListFile(addModelFilename_arg.getValue().c_str());
    if (!ModelsListFile.is_open())
    {
        throw FileNotOpen("Cannot open list of model files for reading");
    }

    string line;
    while (getline(ModelsListFile, line))
    {
        bf::path FullPath = BasePath / bf::path(line);


        AddModelFilenames.push_back(FullPath.string());
        nExtra++;
    }


    cout << nExtra << " lightcurves will be appended to the file" << endl;








    /*  now copy the file across */
    /*  exclamation mark ensures the file is overwritten if it exists */
    timer.start("filecopy");
    CopyFileEfficiently(filename_arg.getValue(), nExtra, "!" + DataFilename, MemFraction);
    timer.stop("filecopy");

    /*  open the fits file */
    timer.start("update");
    mInfile = auto_ptr<FITS>(new FITS(DataFilename, Write));
    fptr = mInfile->fitsPointer();

    /*  get the desired index */
    mObjectIndex = ObjectIndex(ObjectName);

    /*  need to update the object's skipdet column value */
    vector<unsigned int> SkipdetData(1, ad::skiptfa);
    mInfile->extension("CATALOGUE").column("SKIPDET").write(SkipdetData, mObjectIndex+1);

    /*  extract the flux */
    Lightcurve ChosenObject = getObject();

    if (asWASP)
    {
        ChosenObject.asWASP = true;
    }
    else
    {
        ChosenObject.asWASP = false;
    }

    /*  need a subtraction model whatever happens */
    Lightcurve SubModel = GenerateModel(subModel_arg.getValue(), ChosenObject);


    /*  update the period and epoch */
    ChosenObject.period = SubModel.period;
    ChosenObject.epoch = SubModel.epoch;


    SubModel.asWASP = false;

    Lightcurve LCRemoved = RemoveTransit(ChosenObject, SubModel);
    LCRemoved.asWASP = ChosenObject.asWASP;


    /*  now need to iterate through the list of filenames generating 
     *  a new object every time 
     *
     *  TODO: This will generate a lot of output if the code remains as it is 
     *  so this may need altering */

    int count = 0;

    ExtHDU &FluxHDU = mInfile->extension("FLUX");
    const int nFrames = FluxHDU.axis(0);
    for (StringList::iterator i=AddModelFilenames.begin();
            i!=AddModelFilenames.end();
            ++i, ++count)
    {
        const int InsertIndex = nObjects + count;
        cout << "Using model file: " << *i << endl;
        CopyObject(InsertIndex);

        Lightcurve AddModel = GenerateModel(*i, LCRemoved);

        AddModel.asWASP = false;
        //LCRemoved.period = AddModel.period;
        //LCRemoved.epoch = AddModel.epoch;
        CopyParameters(AddModel, LCRemoved);
        Lightcurve SyntheticLightcurve = AddTransit(LCRemoved, AddModel);
        CopyParameters(LCRemoved, SyntheticLightcurve);
        //SyntheticLightcurve.radius = AddModel.radius;

        /*  set the data to the new value */
        UpdateFile(SyntheticLightcurve, InsertIndex);






    }







    timer.stop("update");
    timer.stop("all");




    return 0;
}


int main(int argc, char *argv[])
{
    try
    {
        Application app;
        return app.go(argc, argv);
    }
    catch (FitsException &e)
    {
        cerr << "CCfits error: " << e.message() << endl;
    }
    catch (FitsioException &e)
    {
        cerr << "FITSIO error: " << e.what() << endl;
    }
    catch (TCLAP::ArgException &e)
    {
        cerr << "TCLAP error: " << e.error() << " for arg " << e.argId() << endl;
    }
    catch (BaseException &e)
    {
        cerr << "Error: " << e.type << ". " << e.what() << endl;
    }
    catch (exception &e)
    {
        cerr << "STD error: " << e.what() << endl;
    }
}
