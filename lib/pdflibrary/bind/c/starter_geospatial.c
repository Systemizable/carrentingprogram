/* $Id: starter_geospatial.c,v 1.3.2.5 2016/09/15 08:52:47 rjs Exp $
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

#include <string.h>

#include "pdflib.h"

int
main(void)
{
    /* This is where the data files are. Adjust if necessary. */
    const char* searchpath = "../data";
    const char* outfile = "starter_geospatial.pdf";
    const char* imagefile = "munich.png";
    double mapwidth, mapheight;

    char pageoptlist[2048], *target;
    PDF *p;
    int image;

    /* WKT for plain latitude/longitude values in WGS84 */
    const char *georefsystem =
    "worldsystem={type=geographic wkt={"
        "GEOGCS[\"WGS 84\","
        "  DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\", 6378137,298.257223563]],"
        "  PRIMEM[\"Greenwich\", 0.0],"
        "  UNIT[\"Degree\", 0.01745329251994328]]"
    "}} linearunit=M areaunit=SQM angularunit=degree";

    /* world coordinates of the image (in degrees) */
    double worldpoints[] =
    {
        48.145, /* latitude of top edge */
        11.565, /* longitude of left edge */
        11.59,  /* longitude of right edge */
        48.13   /* latitude of bottom edge */
    };

    /* create a new PDFlib object */
    if ((p = PDF_new()) == (PDF *) 0) {
        printf("Couldn't create PDFlib object (out of memory)!\n");
        return(2);
    }

    PDF_TRY(p) {
        char optlist[256];

        sprintf(optlist, "SearchPath={{%s}}", searchpath);
        PDF_set_option(p, optlist);

        /* This means we must check return values */
        PDF_set_option(p, "errorpolicy=return");

        /* Start the document */
        if (PDF_begin_document(p, outfile, 0, "compatibility=1.7ext3") == -1)
        {
            printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
            return(2);
        }

        PDF_set_info(p, "Creator", "PDFlib starter sample");
        PDF_set_info(p, "Title", "starter_geospatial");

        /* Load the map image */
        image = PDF_load_image(p, "auto", imagefile, 0, "");
        if (image == -1)
        {
            printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
            return(2);
        }

        /* Retrieve image width and height */
        mapwidth = PDF_info_image(p, image, "imagewidth", "");
        mapheight = PDF_info_image(p, image, "imageheight", "");
       
        /* Generate georeference option list */
        target=pageoptlist;

        sprintf(pageoptlist, "viewports={{ georeference={%s ", georefsystem);
        target += strlen(target);

        /* Use the four corners as reference points; (0,0)=lower left etc. */
        sprintf(target, "mappoints={0 0  1 0  1 1  0 1} ");
        target += strlen(target);

#ifdef AVENZA_WORKAROUND
        /* The following can be used as a workaround for a problem with the
         * Avenza PDF Maps app on Android; otherwise the (almost) default
         * bounds values can be skipped.
         */
        sprintf(target, "bounds={0.000001 0 0 1 1 1 1 0} ");
        target += strlen(target);
#endif

        sprintf(target, "worldpoints={");
        target += strlen(target);

        /* lower left corner */
        sprintf(target, "%g %g ", worldpoints[3], worldpoints[1]);
        target += strlen(target);
        /* lower right corner */
        sprintf(target, "%g %g ", worldpoints[3], worldpoints[2]);
        target += strlen(target);
        /* upper right corner */
        sprintf(target, "%g %g ", worldpoints[0], worldpoints[2]);
        target += strlen(target);
        /* upper left corner */
        sprintf(target, "%g %g ", worldpoints[0], worldpoints[1]);
        target += strlen(target);

        sprintf(target, "} } ");
        target += strlen(target);

        sprintf(target, "boundingbox={0 0 %f %f}", mapwidth, mapheight);
        target += strlen(target);

        sprintf(target, "} } ");
        target += strlen(target);

        /* Create a page with geospatial reference information. */
        PDF_begin_page_ext(p, mapwidth, mapheight, pageoptlist);

        /* Place the map on the lower left corner of the page */
        sprintf(optlist, "adjustpage boxsize={%f %f}", mapwidth, mapheight);
        PDF_fit_image(p, image, 0, 0, optlist);

        PDF_end_page_ext(p, "");

        PDF_end_document(p, "");

    }

    PDF_CATCH(p) {
        printf("PDFlib exception occurred:\n");
        printf("[%d] %s: %s\n",
            PDF_get_errnum(p), PDF_get_apiname(p), PDF_get_errmsg(p));
        PDF_delete(p);
        return(2);
    }

    PDF_delete(p);

    return 0;
}
