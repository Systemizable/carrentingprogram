/* $Id: starter_pdfvt1.c,v 1.11.2.4 2014/05/08 13:17:40 rp Exp $
 *
 * Starter sample for PDF/VT-1
 * Create a large number of invoices in a single PDF and make use of
 * the following PDF/VT-1 features:
 * - create a document part (DPart) hierarchy
 * - assign PDF/VT scope attributes to images and imported PDF pages
 * - add document part metadata (DPM) to the DPart root node and all page nodes
 *
 * Required software: PDFlib+PDI/PPS 9
 * Required data: PDF background, fonts, several raster images
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdflib.h"

static int get_random(int n);
static void create_datamatrix(unsigned char *, int record);

int
main(void)
{
    const int MAXRECORD = 100;

    PDF *	p;
    size_t	i;
    int         stationery, page;
    int         record;
    int         barcodeimage;
    const char *stationeryfilename = "stationery_pdfx4p.pdf";
    const char *salesrepfilename = "sales_rep%d.jpg";
    const char *fontname= "LinLibertine_R";

    /* This is where font/image/PDF input files live. Adjust as necessary. */
    const char* searchpath = "../data";
    const char* outfile = "starter_pdfvt1.pdf";
    const double left = 55;
    const double right = 530;
    const double bottom = 822;

    time_t	timer;
    struct tm	ltime;
    double	fontsize = 12, leading, x, y;
    double	total;
    char	buf[256];
    char	optlist[256];
    char        fontoptions[256];

    const char	*closingtext =
	"Terms of payment: <save fillcolor={cmyk 0 1 1 0}>"
        "30 days net<restore>. "
	"90 days warranty starting at the day of sale. "
	"This warranty covers defects in workmanship only. "
	"Kraxi Systems, Inc. will, at its option, repair or replace the "
	"product under the warranty. This warranty is not transferable. "
	"No returns or exchanges will be accepted for wet products.";

    typedef struct { const char *name; double price; } articledata_s;

    const articledata_s articledata[] = {
	{ "Super Kite",		20 },
	{ "Turbo Flyer",	40 },
	{ "Giga Trash",		180 },
	{ "Bare Bone Kit",	50 },
	{ "Nitty Gritty",	20 },
	{ "Pretty Dark Flyer",	75 },
        { "Large Hadron Glider",85 },
        { "Flying Bat",         25 },
        { "Simple Dimple",      40 },
        { "Mega Sail",          95 },
        { "Tiny Tin",           25 },
        { "Monster Duck",       275 },
	{ "Free Gift",		0 }
    };

#define ARTICLECOUNT (sizeof(articledata)/sizeof(articledata[0]))

    typedef struct {
        const char *firstname;
        const char *lastname;
        const char *flat;
        const char *street;
        const char *city;

    } addressdata_s;

    const addressdata_s addressdata[] = {
        { "Edith", "Poulard", "Suite C", "Main Street", "New York"},
        { "Max", "Huber", "", "Lipton Avenue", "Albuquerque"},
        { "Herbert", "Pakard", "App. 29", "Easel", "Duckberg" },
        { "Charles", "Fever", "Office 3", "Scenic Drive", "Los Angeles" },
        { "D.", "Milliband", "", "Old Harbour", "Westland" },
        { "Lizzy", "Tin", "Workshop", "Ford", "Detroit" },
        { "Patrick", "Black", "Backside", "Woolworth Street", "Clover" },
    };
#define ADDRESSCOUNT (sizeof(addressdata)/sizeof(addressdata[0]))

    static const char *salesrepnames[] = {
        "Charles Ragner",
        "Hugo Baldwin",
        "Katie Blomock",
        "Ernie Bastel",
        "Lucy Irwin",
        "Bob Montagnier",
        "Chuck Hope",
        "Pierre Richard"
    };

#define SALESREPS (sizeof(salesrepnames)/sizeof(salesrepnames[0]))

    int salesrepimage[SALESREPS];

    static const char *headers[] = {
        "ITEM", "DESCRIPTION", "QUANTITY", "PRICE", "AMOUNT"
    };
#define COLUMNCOUNT (sizeof(headers)/sizeof(headers[0]))

    static const char *alignments[] = {
        "right", "left", "right", "right", "right"
    };

    static const char *months[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
    };

    int dpm=0, cip4_root, cip4_metadata;

    leading = fontsize + 2;

    /* create a new PDFlib object */
    if ((p = PDF_new()) == (PDF *) 0)
    {
        printf("Couldn't create PDFlib object (out of memory)!\n");
        return(2);
    }

    PDF_TRY(p) {
        int     tbl, tf;
        unsigned int row, col;

	/* This means we must check return values of load_font() etc. */
	PDF_set_option(p, "errorpolicy=return");

	if (PDF_begin_document(p, outfile, 0,
	        "pdfx=PDF/X-4 pdfvt=PDF/VT-1 usestransparency=false "
	        "nodenamelist={root recipient} recordlevel=1") == -1)
	{
	    printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
	    return(2);
	}

	sprintf(optlist, "SearchPath={{%s}}", searchpath);
	PDF_set_option(p, optlist);

	PDF_set_info(p, "Creator", "PDFlib starter sample");
	PDF_set_info(p, "Title", "starter_pdfvt1");

	sprintf(fontoptions,
	            "fontname=%s fontsize=%f embedding "
	            "textformat=utf8 "
	            "encoding=unicode",
	            fontname, fontsize);

	/* Define output intent profile */
        if (PDF_load_iccprofile(p, "ISOcoated.icc", 0,
                "usage=outputintent") == -1)
        {
            printf("Error: %s\n", PDF_get_errmsg(p));
            printf("Please install the ICC profile package from "
                   "www.pdflib.com to run the PDF/VT-1 starter sample.\n");
            PDF_delete(p);
            return(2);
        }

        /* -----------------------------------
         * Load company stationery as background (used on first page
         * for each recipient)
         * -----------------------------------
         */
	stationery = PDF_open_pdi_document(p, stationeryfilename, 0, "");
	if (stationery == -1) {
	    printf("Error: %s\n", PDF_get_errmsg(p));
	    PDF_delete(p);
	    return(2);
	}

	page = PDF_open_pdi_page(p, stationery, 1,
	        "pdfvt={scope=global environment={Kraxi Systems}}");
	if (page == -1) {
	    printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
	    return(2);
	}

        /* -----------------------------------
         * Preload images of all local sales reps (used on first page
         * for each recipient). To get encapsulated image XObjects,
         * the renderingintent option is used.
         * -----------------------------------
         */
        for (i=0; i < SALESREPS; i++)
        {
            sprintf(buf, salesrepfilename, i);
            salesrepimage[i] = PDF_load_image(p, "auto", buf, 0,
                    "pdfvt={scope=file} renderingintent=Perceptual");

            if (salesrepimage[i] == -1) {
                printf("Error: %s\n", PDF_get_errmsg(p));
                PDF_delete(p);
                return(3);
            }
        }

        /* -----------------------------------
         * Construct DPM metadata for the DPart root node
         * -----------------------------------
         */
        cip4_metadata = PDF_poca_new(p, "containertype=dict usage=dpm");
        PDF_poca_insert(p, cip4_metadata,
                "type=string key=CIP4_Conformance value=base");
        PDF_poca_insert(p, cip4_metadata,
                "type=string key=CIP4_Creator value=starter_pdfvt1");
        PDF_poca_insert(p, cip4_metadata,
                "type=string key=CIP4_JobID value={Kraxi Systems invoice}");

        sprintf(optlist,
            "containertype=dict usage=dpm type=dict key=CIP4_Metadata value=%d",
            cip4_metadata);
        cip4_root = PDF_poca_new(p, optlist);

        sprintf(optlist,
            "containertype=dict usage=dpm type=dict key=CIP4_Root value=%d",
            cip4_root);
        dpm = PDF_poca_new(p, optlist);

        /* Create root node in the DPart hierarchy and add DPM metadata  */
        sprintf(optlist, "dpm=%d", dpm);
        PDF_begin_dpart(p, optlist);

        PDF_poca_delete(p, dpm, "recursive=true");

	for (record=0; record < MAXRECORD; record++)
	{
#define MATRIXROWS          32
#define MATRIXDATASIZE      (4*MATRIXROWS)
            unsigned char datamatrix[MATRIXDATASIZE];
            int cip4_recipient, cip4_contact, cip4_person;
            const char *firstname, *lastname, *result;
            int         pagecount=0;
            unsigned    int item;

            firstname = addressdata[get_random(ADDRESSCOUNT)].firstname;
            lastname = addressdata[get_random(ADDRESSCOUNT)].lastname;

            /* -----------------------------------
             * Construct DPM metadata for the next DPart node (i.e. the page)
             * -----------------------------------
             */
            dpm            = PDF_poca_new(p, "containertype=dict usage=dpm");
            cip4_root      = PDF_poca_new(p, "containertype=dict usage=dpm");
            cip4_recipient = PDF_poca_new(p, "containertype=dict usage=dpm");
            cip4_contact   = PDF_poca_new(p, "containertype=dict usage=dpm");
            cip4_person    = PDF_poca_new(p, "containertype=dict usage=dpm");

            sprintf(optlist, "type=dict key=CIP4_Root value=%d", cip4_root);
            PDF_poca_insert(p, dpm, optlist);

            sprintf(optlist, "type=dict key=CIP4_Recipient value=%d",
                                                           cip4_recipient);
            PDF_poca_insert(p, cip4_root, optlist);

            sprintf(optlist, "type=string key=CIP4_UniqueID value={ID_%d}",
                                record);
            PDF_poca_insert(p, cip4_recipient, optlist);

            sprintf(optlist, "type=dict key=CIP4_Contact value=%d",
                                                           cip4_contact);
            PDF_poca_insert(p, cip4_recipient, optlist);

            sprintf(optlist, "type=dict key=CIP4_Person value=%d",
                                                           cip4_person);
            PDF_poca_insert(p, cip4_contact, optlist);

            sprintf(optlist, "type=string key=CIP4_Firstname value={%s}",
                                firstname);
            PDF_poca_insert(p, cip4_person, optlist);

            sprintf(optlist, "type=string key=CIP4_Lastname value={%s}",
                                lastname);
            PDF_poca_insert(p, cip4_person, optlist);

	    /* Create a new node in the document part hierarchy and
             * add DPM metadata
             */
            sprintf(optlist, "dpm=%d", dpm);
	    PDF_begin_dpart(p, optlist);

            PDF_poca_delete(p, dpm, "recursive=true");

            /* -----------------------------------
             * Create and place table with article list
             * -----------------------------------
             */
            /* ---------- Header row */
            row = 1;
            tbl = -1;

            for (col=1; col <= COLUMNCOUNT; col++)
            {
                sprintf(optlist,
                    "fittextline={position={%s center} %s} margin=2",
                    alignments[col-1], fontoptions);
                tbl = PDF_add_table_cell(p, tbl, col, row, headers[col-1], 0,
                    optlist);
            }
            row++;

            /* ---------- Data rows: one for each article */
            total = 0;

            /* -----------------------------------
             * Print variable-length article list
             * -----------------------------------
             */
            for (i = 0, item = 0; i < ARTICLECOUNT; i++) {
                int quantity = get_random(9) + 1;
                double sum;

                if (get_random(2) % 2)
                    continue;

                col = 1;

                item++;
                sum = articledata[i].price * quantity;

                /* column 1: ITEM */
                sprintf(buf, "%u", item);
                sprintf(optlist,
                    "fittextline={position={%s center} %s} colwidth=5%% "
                    "margin=2",
                    alignments[col-1], fontoptions);
                tbl = PDF_add_table_cell(p, tbl, col++, row, buf, 0, optlist);

                /* column 2: DESCRIPTION */
                sprintf(optlist,
                    "fittextline={position={%s center} %s} colwidth=50%% "
                    "margin=2",
                    alignments[col-1], fontoptions);
                tbl = PDF_add_table_cell(p, tbl, col++, row,
                        articledata[i].name, 0, optlist);

                /* column 3: QUANTITY */
                sprintf(buf, "%d", quantity);
                sprintf(optlist,
                    "fittextline={position={%s center} %s} margin=2",
                    alignments[col-1], fontoptions);
                tbl = PDF_add_table_cell(p, tbl, col++, row, buf, 0, optlist);

                /* column 4: PRICE */
                sprintf(buf, "%.2f", articledata[i].price);
                sprintf(optlist,
                    "fittextline={position={%s center} %s} margin=2",
                    alignments[col-1], fontoptions);
                tbl = PDF_add_table_cell(p, tbl, col++, row, buf, 0, optlist);

                /* column 5: AMOUNT */
                sprintf(buf, "%.2f", sum);
                sprintf(optlist,
                    "fittextline={position={%s center} %s} margin=2",
                    alignments[col-1], fontoptions);
                tbl = PDF_add_table_cell(p, tbl, col++, row, buf, 0, optlist);

                total += sum;
                row++;
            }

            /* ---------- Print total in the rightmost column */
            sprintf(buf, "%.2f", total);
            sprintf(optlist,
                "fittextline={position={%s center} %s} margin=2",
                alignments[COLUMNCOUNT-1], fontoptions);
            tbl = PDF_add_table_cell(p, tbl, COLUMNCOUNT, row++, buf, 0,
                    optlist);


            /* ---------- Footer row with terms of payment */
            sprintf(optlist, "%s alignment=justify leading=120%%", fontoptions);
            tf = PDF_create_textflow(p, closingtext, 0, optlist);

            sprintf(optlist,
                    "rowheight=1 margin=2 margintop=%f textflow=%d colspan=%d",
                    2*fontsize, tf, (int) COLUMNCOUNT);
            tbl = PDF_add_table_cell(p, tbl, 1, row++, "", 0, optlist);


            /* ----- Place the table instance(s), creating pages as required */
            do {
                double top;

                PDF_begin_page_ext(p, 0, 0,
                        "topdown=true width=a4.width height=a4.height");

                if (++pagecount == 1)
                {
                    /* -----------------------------------
                     * Place company stationery as background on first page
                     * for each recipient
                     * -----------------------------------
                     */
                    PDF_fit_pdi_page(p, page, 0, 842, "");

                    /* -----------------------------------
                     * Place name and image of local sales rep on first page
                     * for each recipient
                     * -----------------------------------
                     */
                    y = 177;
                    x = 455;

                    sprintf(optlist, "%s fontsize=9", fontoptions);
                    PDF_fit_textline(p, "Local sales rep:", 0, x, y, optlist);
                    PDF_fit_textline(p, salesrepnames[record % SALESREPS], 0,
                            x, y+9, optlist);

                    y = 280;
                    PDF_fit_image(p, salesrepimage[record % SALESREPS], x, y,
                            "boxsize={90 90} fitmethod=meet");


                    /* -----------------------------------
                     * Address of recipient
                     * -----------------------------------
                     */
                    y = 170;

                    sprintf(buf, "%s %s", firstname, lastname);
                    PDF_fit_textline(p, buf, 0, left, y, fontoptions);

                    y += leading;
                    PDF_fit_textline(p,
                            addressdata[get_random(ADDRESSCOUNT)].flat,
                            0, left, y, fontoptions);

                    y += leading;
                    sprintf(buf, "%d %s",
                            get_random(999),
                            addressdata[get_random(ADDRESSCOUNT)].street);
                    PDF_fit_textline(p, buf, 0, left, y, fontoptions);

                    y += leading;
                    sprintf(buf, "%05d %s",
                            get_random(99999),
                            addressdata[get_random(ADDRESSCOUNT)].city);
                    PDF_fit_textline(p, buf, 0, left, y, fontoptions);


                    /* -----------------------------------
                     * Individual barcode image for each recipient. To get
                     * encapsulated image XObjects the renderingintent option
                     *  is used.
                     * -----------------------------------
                     */
                    create_datamatrix(datamatrix, record);
                    PDF_create_pvf(p, "barcode", 0, datamatrix, MATRIXDATASIZE,
                                    "");

                    barcodeimage = PDF_load_image(p, "raw", "barcode", 0,
                        "bpc=1 components=1 width=32 height=32 invert "
                        "pdfvt={scope=singleuse} renderingintent=Saturation");
                    if (barcodeimage == -1) {
                        printf("Error: %s\n", PDF_get_errmsg(p));
                        PDF_delete(p);
                        return(3);
                    }

                    PDF_fit_image(p, barcodeimage, 280.0, 200.0, "scale=1.5");
                    PDF_close_image(p, barcodeimage);
                    PDF_delete_pvf(p, "barcode", 0);


                    /* -----------------------------------
                     * Print header and date
                     * -----------------------------------
                     */
                    time(&timer);
                    ltime = *localtime(&timer);

                    y = 300;
                    sprintf(buf, "INVOICE %d-%d", ltime.tm_year + 1900,
                                                    record+1);
                    PDF_fit_textline(p, buf, 0, left, y, fontoptions);

                    sprintf(buf, "%s %d, %d",
                            months[ltime.tm_mon], ltime.tm_mday,
                            ltime.tm_year + 1900);
                    sprintf(optlist, "%s position {100 0}", fontoptions);
                    PDF_fit_textline(p, buf, 0, right, y, optlist);

                    top = y + 2*leading;
                }
                else
                {
                    top = 50;
                }

                /*
                 * Place the table on the page.
                 * Shade every other row, except the footer row.
                 */
                result = PDF_fit_table(p, tbl,
                        left, bottom, right, top,
                        "header=1 "
                        "fill={{area=rowodd fillcolor={gray 0.9}} "
                            "{area=rowlast fillcolor={gray 1}}} "
			"rowheightdefault=auto colwidthdefault=auto");

                if (!strcmp(result, "_error"))
                {
                    printf("Couldn't place table: %s\n", PDF_get_errmsg(p));
                    PDF_delete(p);
                    return(2);
                }

                PDF_end_page_ext(p, "");
            } while (!strcmp(result, "_boxfull"));

            PDF_delete_table(p, tbl, "");

            /* Close node in the document part hierarchy */
            PDF_end_dpart(p, "");
	}

        PDF_close_pdi_page(p, page);
	PDF_close_pdi_document(p, stationery);

	for (i=0; i<SALESREPS; i++)
	{
            PDF_close_image(p, salesrepimage[i]);
	}

        /* Close root node in the document part hierarchy */
        PDF_end_dpart(p, "");

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

/* Get a pseudo random number between 0 and n-1 */
static int
get_random(int n)
{
    return (int) ((double) n * rand()/(RAND_MAX+1.0));
}

/* Simulate a datamatrix barcode */
static void
create_datamatrix(unsigned char *datamatrix, int record)
{
    int i;

    for (i=0; i<MATRIXROWS; i++)
    {
        datamatrix[4*i+0] = ((0xA3 + 1*record + 17*i) % 0xFF);
        datamatrix[4*i+1] = ((0xA2 + 3*record + 11*i) % 0xFF);
        datamatrix[4*i+2] = ((0xA0 + 5*record +  7*i) % 0xFF);
        datamatrix[4*i+3] = ((0x71 + 7*record +  9*i) % 0xFF);
    }
    for (i=0; i<MATRIXROWS; i++)
    {
        datamatrix[4*i+0] |= 0x80;
        datamatrix[4*i+2] |= 0x80;
        if (i%2)
            datamatrix[4*i+3] |= 0x01;
        else
            datamatrix[4*i+3] &= 0xFE;
    }
    for (i=0; i<4; i++)
    {
        datamatrix[4*(MATRIXROWS/2-1)+i] = 0xFF;
        datamatrix[4*(MATRIXROWS-1)+i] = 0xFF;
    }
}
