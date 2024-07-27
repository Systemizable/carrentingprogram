/* $Id: quickreference.c,v 1.1.2.2 2013/07/09 13:16:44 rp Exp $
 *
 * PDFlib+PDI client: mini imposition demo
 */

#include <stdio.h>
#include <stdlib.h>

#include "pdflib.h"

int
main(void)
{
    const double width = 500, height = 770;

    PDF		*p;
    int		manual, page;
    int		font, row, col;
    const	int maxrow = 2;
    const	int maxcol = 2;
    int		endpage=0;
    int		pageno;

    /* This is where font/image/PDF input files live. Adjust as necessary. */
    const char* searchpath = "../data";

    const char *infile = "reference.pdf";

    /* create a new PDFlib object */
    if ((p = PDF_new()) == (PDF *) 0)
    {
        printf("Couldn't create PDFlib object (out of memory)!\n");
        return(2);
    }

    PDF_TRY(p)
    {
	char optlist[256];

	/* This means we must check return values of load_font() etc. */
	PDF_set_option(p, "errorpolicy=return");

	if (PDF_begin_document(p, "quickreference.pdf", 0, "") == -1)
	{
	    printf("Error: %s\n", PDF_get_errmsg(p));
	    return(2);
	}

        sprintf(optlist, "SearchPath={{%s}}", searchpath);

	PDF_set_option(p, optlist);

	/* This line is required to avoid problems on Japanese systems */
	PDF_set_option(p, "hypertextencoding=host");

	PDF_set_info(p, "Creator", "quickreference.c");
	PDF_set_info(p, "Author", "Thomas Merz");
	PDF_set_info(p, "Title", "mini imposition demo (C)");

	manual = PDF_open_pdi_document(p, infile, 0, "");
	if (manual == -1)
	{
	    printf("Error: %s\n", PDF_get_errmsg(p));
	    return(2);
	}

	row = 0;
	col = 0;

	/* Get the number of pages in the input document */
	endpage = (int) PDF_pcos_get_number(p, manual, "length:pages");

	for (pageno = 1; pageno <= endpage; pageno++)
	{
	    if (row == 0 && col == 0)
	    {
		PDF_begin_page_ext(p, width, height, "topdown");
		font = PDF_load_font(p, "Helvetica-Bold", 0, "host", "");
		if (font == -1) {
		    printf("Error: %s\n", PDF_get_errmsg(p));
		    PDF_delete(p);
		    return(2);
		}
		PDF_setfont(p, font, 18);
		PDF_set_text_pos(p, 24, 24);
		PDF_show(p, "PDFlib Quick Reference");
	    }

	    page = PDF_open_pdi_page(p, manual, pageno, "");

	    if (page == -1)
	    {
		printf("Error: %s\n", PDF_get_errmsg(p));
		return(2);
	    }

	    sprintf(optlist, "scale %f", 1.0 / maxrow);
	    PDF_fit_pdi_page(p, page,
		width/maxcol*col, (row + 1) * height/maxrow, optlist);
	    PDF_close_pdi_page(p, page);

	    col++;
	    if (col == maxcol)
	    {
		col = 0;
		row++;
	    }
	    if (row == maxrow)
	    {
		row = 0;
		PDF_end_page_ext(p, "");
	    }
	}

	/* finish the last partial page */
	if (row != 0 || col != 0)
	    PDF_end_page_ext(p, "");

	PDF_end_document(p, "");
	PDF_close_pdi_document(p, manual);
    }

    PDF_CATCH(p)
    {
        printf("PDFlib exception occurred in quickreference sample:\n");
        printf("[%d] %s: %s\n",
	    PDF_get_errnum(p), PDF_get_apiname(p), PDF_get_errmsg(p));
        PDF_delete(p);
        return(2);
    }

    PDF_delete(p);

    return 0;
}
