/* $Id: starter_geospatial.c,v 1.8.2.7 2016/06/16 09:33:09 tm Exp $
 * Starter for georeferenced PDF:
 * Import an image with a map and add geospatial reference information
 *
 * Sample map and coordinates:
 * We use a map from www.openstreetmap.com; the geospatial coordinates of the
 * image edges were also provided by that Web site.
 * The coordinate system is WGS84 which is also used for GPS.
 *
 * Required software: PDFlib/PDFlib+PDI/PPS 9
 * Required data: image file and associated geospatial reference information
 */

#include <iostream>
#include <string>

#include "pdflib.hpp"

using namespace std;
using namespace pdflib;

int main(void)
{
    /* This is where the data files are. Adjust if necessary. */
    const wstring searchpath = L"../data";
    const wstring outfile = L"starter_geospatial.pdf";
    const wstring imagefile = L"munich.png";
    double mapwidth, mapheight;

    PDFlib p;

    /* WKT for plain latitude/longitude values in WGS84 */
    const wstring georefsystem =
            L"worldsystem={type=geographic wkt={"
                    L"GEOGCS[\"WGS 84\","
                    L"  DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\", 6378137,298.257223563]],"
                    L"  PRIMEM[\"Greenwich\", 0.0],"
                    L"  UNIT[\"Degree\", 0.01745329251994328]]"
                    L"}} linearunit=M areaunit=SQM angularunit=degree";

    /* world coordinates of the image (in degrees) */
    double const worldpoints[] =
    {
        48.145, /* latitude of top edge */
        11.565, /* longitude of left edge */
        11.59,  /* longitude of right edge */
        48.13   /* latitude of bottom edge */
    };

    try
    {
        wostringstream optlist;

        optlist << L"SearchPath={{" << searchpath << L"}}";
        p.set_option(optlist.str());

        /* This means we must check return values */
        p.set_option(L"errorpolicy=return");

        /* Start the document */
        if (p.begin_document(outfile, L"compatibility=1.7ext3") == -1)
        {
            wcerr << L"Error: " << p.get_errmsg() << endl;
            return 2;
        }

        p.set_info(L"Creator", L"PDFlib starter sample");
        p.set_info(L"Title", L"starter_geospatial");

        /* Load the map image */
        int image = p.load_image(L"auto", imagefile, L"");
        if (image == -1)
        {
            wcerr << L"Error: " << p.get_errmsg() << endl;
            return 2;
        }

        /* Retrieve image width and height */
        mapwidth = p.info_image(image, L"imagewidth", L"");
        mapheight = p.info_image(image, L"imageheight", L"");

        /* Generate georeference option list */
        wostringstream pageoptlist;

        pageoptlist << L"viewports={{ georeference={" << georefsystem << L" ";

        /* Use the four corners as reference points; (0,0)=lower left etc. */
        pageoptlist << L"mappoints={0 0  1 0  1 1  0 1} ";

#ifdef AVENZA_WORKAROUND
        /* The following can be used as a workaround for a problem with the
         * Avenza PDF Maps app on Android; otherwise the (almost) default
         * bounds values can be skipped.
         */
        pageoptlist << L"bounds={0.000001 0 0 1 1 1 1 0} ";
#endif

        pageoptlist << L"worldpoints={";

        /* lower left corner */
        pageoptlist << worldpoints[3] << L" " << worldpoints[1] << L" ";
        /* lower right corner */
        pageoptlist << worldpoints[3] << L" " << worldpoints[2] << L" ";
        /* upper right corner */
        pageoptlist << worldpoints[0] << L" " << worldpoints[2] << L" ";
        /* upper left corner */
        pageoptlist << worldpoints[0] << L" " << worldpoints[1] << L" ";

        pageoptlist << L"} } ";

        pageoptlist << L"boundingbox={0 0 " << mapwidth << L" " << mapheight
                << L"}";

        pageoptlist << L"} } ";

        /* Create a page with geospatial reference information. */
        p.begin_page_ext(mapwidth, mapheight, pageoptlist.str());

        /* Place the map on the lower left corner of the page */
        optlist.str(L"");
        optlist << L"adjustpage boxsize={" << mapwidth << L" " << mapheight
                << L"}";
        p.fit_image(image, 0, 0, optlist.str());

        p.end_page_ext(L"");

        p.end_document(L"");
    }
    catch (PDFlib::Exception &ex)
    {
        wcerr << L"PDFlib exception occurred:" << endl << L"["
                << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
                << ex.get_errmsg() << endl;
        return 2;
    }

    return 0;
}
