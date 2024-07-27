/* $Id: starter_3d.c,v 1.7.2.2 2013/07/09 13:16:44 rp Exp $
 * 3D Starter:
 * Load a 3D model and create a 3D annotation from it.
 *
 * Define a 3D view and load some 3D data with the view defined. Then create
 * an annotation containing the loaded 3D data with the defined 3D view as the
 * initial view.
 *
 * Required software: PDFlib/PDFlib+PDI/PPS 9
 * Required data: PRC data file
 */

#include <stdio.h>
#include <stdlib.h>

#include "pdflib.h"


int
main(void)
{
    /* This is where the data files are. Adjust if necessary. */
    const char* searchpath = "../data";
    const char* outfile = "starter_3d.pdf";

    char buf[1024];
    int view, data;
    PDF *p;

    /* create a new PDFlib object */
    if ((p = PDF_new()) == (PDF *) 0) {
        printf("Couldn't create PDFlib object (out of memory)!\n");
        return(2);
    }


    PDF_TRY(p) {
        char optlist[256];

        sprintf(optlist, "SearchPath={{%s}}", searchpath);
        PDF_set_option(p, optlist);

        /* This means we must check return values of begin_document() etc. */
        PDF_set_option(p, "errorpolicy=return");

        /* Start the document */
        if (PDF_begin_document(p, outfile, 0, "compatibility=1.7ext3") == -1) {
            printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
            return(2);
	}

        PDF_set_info(p, "Creator", "PDFlib starter sample");
        PDF_set_info(p, "Title", "starter_3d");

        PDF_begin_page_ext(p, 0, 0, "width=a4.width height=a4.height");

        /* Define a 3D view which shows the model from the top */
        sprintf(optlist,
           "type=PRC name=FirstView background={fillcolor=Lavender} "
            "camera2world={-1 0 0 0 1 0 0 0 -1 0.5 0 300}");
        if ((view = PDF_create_3dview(p, "First view", 0, optlist)) == -1) {
            printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
            return(2);
        }

        /* Load some 3D data with the view defined above */
	sprintf(buf, "type=PRC views={%d}", view);
        if ((data = PDF_load_3ddata(p, "riemann.prc", 0, buf)) == -1) {
            printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
            return(2);
        }

        /* Create an annotation containing the loaded 3D data with the
         * defined 3D view as the initial view
         */
	sprintf(buf, "name=annot usercoordinates contents=PRC 3Ddata= %d "
            "3Dactivate={enable=open} 3Dinitialview=%d", data, view);
        PDF_create_annotation(p, 116, 200, 447, 580, "3D", buf);

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
