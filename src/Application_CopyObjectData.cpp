#include "Application.h"
#include "Exceptions.h"

#include <CCfits/CCfits>

using namespace std;
using namespace CCfits;

typedef map<string, Column*> ColumnList;
typedef list<string> StringList;

/** Copies an object at given index to another location in the file
 *
 * @param LocationIndex Empty lightcurve location in the file */
void Application::CopyObject(const int LocationIndex)
{
    /*  things to retrieve:
     *      - Catalogue entries
     *      - Image data in a strip
     */
    ExtHDU &CatalogueHDU = mInfile->extension("CATALOGUE");
    ColumnList Columns = CatalogueHDU.column();

    for (ColumnList::const_iterator i=Columns.begin();
            i!=Columns.end();
            ++i)
    {

        /*  get the data type */
        string Format = i->second->format();

        if (Format == "1J")
        {
            /*  data type is long */
            vector<long> data;
            i->second->read(data, mObjectIndex + 1, mObjectIndex + 1);
            i->second->write(data, LocationIndex + 1);

        }
        else if (Format == "1I")
        {
            /*  data type is int */
            vector<int> data;
            i->second->read(data, mObjectIndex + 1, mObjectIndex + 1);
            i->second->write(data, LocationIndex + 1);

        }
        else if (Format == "1E")
        {
            /*  data type is float */
            vector<float> data;
            i->second->read(data, mObjectIndex + 1, mObjectIndex + 1);
            i->second->write(data, LocationIndex + 1);

        }
        else if (Format == "26A")
        {
            /*  data type is string */
            vector<string> data;
            i->second->read(data, mObjectIndex + 1, mObjectIndex + 1);
            i->second->write(data, LocationIndex + 1);

        }
        else if (Format == "1D")
        {
            /*  data type is double */
            vector<double> data;
            i->second->read(data, mObjectIndex + 1, mObjectIndex + 1);
            i->second->write(data, LocationIndex + 1);

        }
        else
        {
            cerr << "Unknown data type: " << Format << endl;
        }
    }

    StringList HDUList;
    HDUList.push_back("HJD");
    HDUList.push_back("FLUX");
    HDUList.push_back("FLUXERR");
    HDUList.push_back("CCDX");
    HDUList.push_back("CCDY");
    HDUList.push_back("QUALITY");
    HDUList.push_back("SKYBKG");

    for (StringList::const_iterator i=HDUList.begin();
            i!=HDUList.end();
            ++i)
    {
        ExtHDU &CurrentHDU = mInfile->extension(*i);
        /*  do not need to be efficient about this */

        const long bitpix = CurrentHDU.bitpix();
        switch (bitpix)
        {
            case BYTE_IMG:
                break;
            case SHORT_IMG:
                break;
            case LONG_IMG:
                break;
            case LONGLONG_IMG:
                break;
            case FLOAT_IMG:
                break;
            case DOUBLE_IMG:
                break;
            default:
                cerr << "Unknown HDU type encountered: " << bitpix << endl;
        }
    }


}
