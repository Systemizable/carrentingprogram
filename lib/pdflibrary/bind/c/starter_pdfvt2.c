/* $Id: starter_pdfvt2.c,v 1.7.2.6 2016/07/07 09:58:43 tm Exp $
 *
 * Starter sample for PDF/VT-2
 * Create a large number of invoices in a single PDF and make use of
 * the following PDF/VT-2 features:
 * - create a document part (DPart) hierarchy
 * - assign PDF/VT scope attributes to images and imported PDF pages
 * - add document part metadata (DPM) to the DPart root node and all page nodes
 * - use proxy/reference pairs for imported PDF pages for the letterhead
 *   and photographic images. The referenced PDFs are PDF/X-4p themselves.
 * - Since transparency is used (for the dashed rectangles in the proxies)
 *   we supply the required options to achieve GTS_Encapsulated status:
 *   - "transparencygroup" for the proxy templates
 *   - "mask" for the barcode image (use "renderingintent" for color images)
 *
 * Required software: PDFlib+PDI/PPS 9
 * Required data: PDF/X-4p input documents, fonts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pdflib.h"

static int make_proxy(PDF *p, const char *targetname, const char *description);
static int get_random(int n);
static void create_datamatrix(unsigned char *, int record);

int
main(void)
{
    const int MAXRECORD = 100;

    PDF *	p;
    size_t	i;
    int         record;
    int         barcodeimage;
    const char *stationeryfilename = "stationery_pdfx4p.pdf";
    const char *salesrepfilename = "sales_rep%d.pdf";
    const char *fontname= "LinLibertine_R";

    /* This is where font/image/PDF input files live. Adjust as necessary. */
    const char* searchpath = "../data";
    const char* outfile = "starter_pdfvt2.pdf";

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
        int proxy_stationery;
        int proxy_salesrepimage[SALESREPS];

        /* This means we must check return values of load_font() etc. */
	PDF_set_option(p, "errorpolicy=return");

	if (PDF_begin_document(p, outfile, 0,
	        "pdfvt=PDF/VT-2 pdfx=PDF/X-5pg usestransparency=true "
	        "nodenamelist={root recipient} recordlevel=1") == -1)
	{
	    printf("Error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
	    return(2);
	}

	sprintf(optlist, "SearchPath={{%s}}", searchpath);
	PDF_set_option(p, optlist);

	PDF_set_info(p, "Creator", "PDFlib starter sample");
	PDF_set_info(p, "Title", "starter_pdfvt2");

        sprintf(fontoptions,
                    "fontname=%s fontsize=%f embedding "
                    "textformat=utf8 "
                    "encoding=unicode",
                    fontname, fontsize);
	            
	/* Define output intent profile */
        if (PDF_load_iccprofile(p, "ISOcoated.icc", 0,
                "usage=outputintent urls={http://www.color.org}") == -1)
        {
            printf("Error: %s\n", PDF_get_errmsg(p));
            printf("Please install the ICC profile package from "
                   "www.pdflib.com to run the PDF/VT-2 starter sample.\n");
            PDF_delete(p);
            return(2);
        }

        /* -----------------------------------
         * Load company stationery as background (used on first page
         * for each recipient) by reference and construct proxy for it
         * -----------------------------------
         */
        proxy_stationery = make_proxy(p, stationeryfilename,
                "Proxy for stationery");
        if (proxy_stationery == -1)
        {
            printf("Proxy error: %s\n", PDF_get_errmsg(p));
            PDF_delete(p);
            return(2);
        }

        /* -----------------------------------
         * Preload PDF images of all local sales reps (used on first page
         * for each recipient) by reference and construct proxy for it
         * -----------------------------------
         */
        for (i=0; i < SALESREPS; i++)
        {
            char description[1024];

            sprintf(buf, salesrepfilename, i);
            sprintf(description, "Proxy for sales rep image %u",
                                    (unsigned int) i);
            proxy_salesrepimage[i] = make_proxy(p, buf, description);
            if (proxy_salesrepimage[i] == -1)
            {
                printf("Proxy error: %s\n", PDF_get_errmsg(p));
                PDF_delete(p);
                return(2);
            }
        }

        /* -----------------------------------
         * Construct DPM metadata for the DPart root node
         * -----------------------------------
         */
        dpm           = PDF_poca_new(p, "containertype=dict usage=dpm");
        cip4_root     = PDF_poca_new(p, "containertype=dict usage=dpm");
        cip4_metadata = PDF_poca_new(p, "containertype=dict usage=dpm");

        sprintf(optlist, "type=dict key=CIP4_Root value=%d", cip4_root);
        PDF_poca_insert(p, dpm, optlist);

        sprintf(optlist, "type=dict key=CIP4_Metadata value=%d",
                                                         cip4_metadata);
        PDF_poca_insert(p, cip4_root, optlist);

        PDF_poca_insert(p, cip4_metadata,
                "type=string key=CIP4_Conformance value=base");
        PDF_poca_insert(p, cip4_metadata,
                "type=string key=CIP4_Creator value=starter_pdfvt2");
        PDF_poca_insert(p, cip4_metadata,
                "type=string key=CIP4_JobID value={Kraxi Systems invoice}");

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
                     * Place company stationery / proxy (template) as background
                     * on the page
                     * -----------------------------------
                     */
                    PDF_fit_image(p, proxy_stationery, 0, 842, "");

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
                    /* Place the proxy on the page */
                    PDF_fit_image(p, proxy_salesrepimage[record % SALESREPS],
                            x, y, "boxsize={90 90} fitmethod=meet");


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
                     * Individual barcode image for each recipient
                     * -----------------------------------
                     */
                    create_datamatrix(datamatrix, record);
                    PDF_create_pvf(p, "barcode", 0, datamatrix, MATRIXDATASIZE,
                                    "");

                    /*
                     * The "mask" option helps us achieve GTS_Encapsulated
                     * status
                     */
                    barcodeimage = PDF_load_image(p, "raw", "barcode", 0,
                            "bpc=1 components=1 width=32 height=32 invert "
                            "pdfvt={scope=singleuse} mask");
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

/* -------------------------------
 * Load page 1 of the specified PDF and use it as reference for
 * a proxy which consists of a transparent crossed-out rectangle
 * of the same size.
 */
static int
make_proxy(PDF *p, const char *targetname, const char *description)
{
    char optlist[1024];
    int proxy;
    double linewidth = 2;
    double width, height;
    double x1, y1, x2, y2, x3, y3, x4, y4;
    int gstate;

    /* Create the template which will serve as proxy. The referenced
     * page (the target) is attached to the proxy.
     * The width and height parameters will be set in PDF_end_template_ext()
     * after we queried the size of the target page.
     * The "transparencygroup" option is provided to achieve GTS_Encapsulated
     * status.
     * You can add "pdfvt={xid={uuid:...} }" to optlist if you can
     * generate unique IDs.
     */

    sprintf(optlist,
            "reference={filename=%s pagenumber=1} pdfvt={scope=file} "
            "transparencygroup={colorspace=devicecmyk isolated=true}",
            targetname);
    proxy = PDF_begin_template_ext(p, 0, 0, optlist);

    if (proxy == -1)
    {
        return proxy;
    }

    /* Determine the coordinates of the target; we use it for
     * dimensioning the proxy appropriately.
     */
    x1 = PDF_info_image(p, proxy, "targetx1", "");
    y1 = PDF_info_image(p, proxy, "targety1", "");
    x2 = PDF_info_image(p, proxy, "targetx2", "");
    y2 = PDF_info_image(p, proxy, "targety2", "");
    x3 = PDF_info_image(p, proxy, "targetx3", "");
    y3 = PDF_info_image(p, proxy, "targety3", "");
    x4 = PDF_info_image(p, proxy, "targetx4", "");
    y4 = PDF_info_image(p, proxy, "targety4", "");

    width = x2 - x1;
    height = y4 - y1;

    /* Draw a transparent crossed-out rectangle to visualize the proxy.
     * Attention: if we use the exact corner points, one half of the
     * linewidth would end up outside the template, and therefore be
     * clipped.
     */
    PDF_setlinewidth(p, linewidth);
    PDF_set_graphics_option(p, "dasharray={10 5}");

    /* Make the dashed crossed-out rectangle transparent so that the proxy
     * does not obscure the underlying page contents.
     */
    gstate = PDF_create_gstate(p, "opacitystroke=0.25 opacityfill=0.25");
    PDF_set_gstate(p, gstate);

    PDF_moveto(p, x1 + linewidth / 2, y1 + linewidth / 2);
    PDF_lineto(p, x2 - linewidth / 2, y2 + linewidth / 2);
    PDF_lineto(p, x3 - linewidth / 2, y3 - linewidth / 2);
    PDF_lineto(p, x4 + linewidth / 2, y4 - linewidth / 2);
    PDF_lineto(p, x1 + linewidth / 2, y1 + linewidth / 2);
    PDF_lineto(p, x3 - linewidth / 2, y3 - linewidth / 2);
    PDF_moveto(p, x2 - linewidth / 2, y2 + linewidth / 2);
    PDF_lineto(p, x4 + linewidth / 2, y4 - linewidth / 2);
    PDF_stroke(p);

    sprintf(optlist,
            "fontname=LuciduxSans-Oblique "
            "encoding=unicode "
            "textformat=utf8 "
            "embedding "
            "fontsize=%f fitmethod=auto position=center boxsize={%f %f}",
            width>550 ? 24.0 : 48.0, width, height);
    PDF_fit_textline(p, description, 0, 0, 0, optlist);

    /* Make the proxy template the same size as the target page */
    PDF_end_template_ext(p, width, height);

    return proxy;
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
