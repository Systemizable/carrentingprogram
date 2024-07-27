/* $Id: starter_pdfvt1.cpp,v 1.12.2.1 2013/07/05 10:01:54 rp Exp $
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

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined(WIN32) && !defined(MAC)
#include <unistd.h>
#endif

#include "pdflib.hpp"

using namespace std;
using namespace pdflib;

const int MATRIXROWS=32;
static int get_random(int n);
static void create_datamatrix(unsigned char *, int record);

int
main(void)
{
    try {
	const int MAXRECORD = 100;

	PDFlib  p;
	size_t	i;
	int     stationery, page;
	int     record;
	int     barcodeimage;
	const wstring stationeryfilename = L"stationery_pdfx4p.pdf";
	const wstring fontname= L"LinLibertine_R";

	/* This is where font/image/PDF input files live. Adjust as necessary.*/
	const wstring searchpath = L"../data";
	const wstring outfile = L"starter_pdfvt1.pdf";
	const double left = 55;
	const double right = 530;
	const double bottom = 822;

	time_t	timer;
	struct tm	ltime;
	double	fontsize = 12, leading, x, y;
	wostringstream  buf;
	wostringstream	optlist;
	wostringstream	fontoptions;

	const wstring closingtext =
	    L"Terms of payment: <save fillcolor={cmyk 0 1 1 0}>30 days "
            L"net<restore>. "
	    L"90 days warranty starting at the day of sale. "
	    L"This warranty covers defects in workmanship only. "
	    L"Kraxi Systems, Inc. will, at its option, repair or replace the "
	    L"product under the warranty. This warranty is not transferable. "
	    L"No returns or exchanges will be accepted for wet products.";

	struct articledata_s {
	    articledata_s(wstring n, double pr):
		name(n), price(pr) {}
	    wstring name;
	    double price;
	};

	const articledata_s articledata[] = {
            articledata_s(L"Super Kite",                20),
            articledata_s(L"Turbo Flyer",               40),
            articledata_s(L"Giga Trash",                180),
            articledata_s(L"Bare Bone Kit",             50),
            articledata_s(L"Nitty Gritty",              20),
            articledata_s(L"Pretty Dark Flyer",         75),
            articledata_s(L"Large Hadron Glider",       85),
	    articledata_s(L"Flying Bat",                25),
	    articledata_s(L"Simple Dimple",             40),
	    articledata_s(L"Mega Sail",                 95),
	    articledata_s(L"Tiny Tin",                  25),
	    articledata_s(L"Monster Duck",              275),
	    articledata_s(L"Free Gift",                 0),
	};

	const size_t ARTICLECOUNT =
	        sizeof(articledata) / sizeof(articledata[0]);

	struct addressdata_s {
	    addressdata_s(wstring fn, wstring ln, wstring f, wstring s,
	                    wstring c):
		firstname(fn), lastname(ln), flat(f), street(s), city(c) {}

	    wstring firstname;
	    wstring lastname;
	    wstring flat;
	    wstring street;
	    wstring city;
	};

	const addressdata_s addressdata[] = {
	    addressdata_s(L"Edith", L"Poulard", L"Suite C", L"Main Street",
	                                                L"New York"),
	    addressdata_s(L"Max", L"Huber", L"", L"Lipton Avenue",
	                                                L"Albuquerque"),
	    addressdata_s(L"Herbert", L"Pakard", L"App. 29", L"Easel",
	                                                L"Duckberg" ),
	    addressdata_s(L"Charles", L"Fever", L"Office 3", L"Scenic Drive",
	                                                L"Los Angeles" ),
	    addressdata_s(L"D.", L"Milliband", L"", L"Old Harbour",
	                                                L"Westland"),
	    addressdata_s(L"Lizzy", L"Tin", L"Workshop", L"Ford", L"Detroit" ),
	    addressdata_s(L"Patrick", L"Black", L"Backside",
	                                    L"Woolworth Street", L"Clover" ),
	};

	const size_t ADDRESSCOUNT =
	        sizeof(addressdata) / sizeof(addressdata[0]);

	static const wstring salesrepnames[] = {
	    L"Charles Ragner",
	    L"Hugo Baldwin",
	    L"Katie Blomock",
	    L"Ernie Bastel",
	    L"Lucy Irwin",
	    L"Bob Montagnier",
	    L"Chuck Hope",
	    L"Pierre Richard"
	};

	const size_t SALESREPS =
	        sizeof(salesrepnames) / sizeof(salesrepnames[0]);

	int salesrepimage[SALESREPS];

        static const wstring headers[] = {
            L"ITEM", L"DESCRIPTION", L"QUANTITY", L"PRICE", L"AMOUNT"
        };
	const size_t COLUMNCOUNT = sizeof(headers) / sizeof(headers[0]);

        static const wstring alignments[] = {
            L"right", L"left", L"right", L"right", L"right"
        };

	static const wstring months[] = {
	    L"January", L"February", L"March", L"April", L"May", L"June",
	    L"July", L"August", L"September", L"October", L"November",
	    L"December"
	};

	int dpm=0, cip4_root, cip4_metadata;

	leading = fontsize + 2;

	//  This means we must check return values of load_font() etc.
	p.set_option(L"errorpolicy=return");

	// Set the search path for fonts and PDF files
	optlist.str(L"");
	optlist << L"searchpath={{" << searchpath << L"}}";
	p.set_option(optlist.str());

	if (p.begin_document(outfile,
		L"pdfx=PDF/X-4 pdfvt=PDF/VT-1 usestransparency=false "
		L"nodenamelist={root recipient} recordlevel=1") == -1)
	{
            wcerr << L"Error: " << p.get_errmsg() << endl; return(2);
	}

	p.set_info(L"Creator", L"PDFlib starter sample");
	p.set_info(L"Title", L"starter_pdfvt1");

	fontoptions.str(L"");
        fontoptions << L"fontname=" << fontname << L" fontsize=" << fontsize
                << L" embedding encoding=unicode";

	/* Define output intent profile */
	if (p.load_iccprofile(L"ISOcoated.icc", L"usage=outputintent") == -1)
	{
            wcerr << L"Error: " << p.get_errmsg() << endl;
	    wcerr << L"Please install the ICC profile package from L"
		   L"www.pdflib.com to run the PDF/VT-1 starter sample." <<endl;
	    return(2);
	}

	/* -----------------------------------
	 * Load company stationery as background (used on first page
	 * for each recipient)
	 * -----------------------------------
	 */
	stationery = p.open_pdi_document(stationeryfilename, L"");
	if (stationery == -1) {
            wcerr << L"Error: " << p.get_errmsg() << endl; return(2);
	}

	page = p.open_pdi_page(stationery, 1,
		L"pdfvt={scope=global environment={Kraxi Systems}}");
	if (page == -1) {
            wcerr << L"Error: " << p.get_errmsg() << endl; return(2);
	}

	/* -----------------------------------
         * Preload images of all local sales reps (used on first page
         * for each recipient). To get encapsulated image XObjects,
         * the renderingintent option is used.
	 * -----------------------------------
	 */
	for (i=0; i < SALESREPS; i++)
	{
	    buf.str(L"");
	    buf << L"sales_rep" << i << L".jpg";
	    salesrepimage[i] = p.load_image(L"auto", buf.str(),
		    L"pdfvt={scope=file} renderingintent=Perceptual");

	    if (salesrepimage[i] == -1) {
		wcerr << L"Error: " << p.get_errmsg() << endl; return(2);
	    }
	}

	/* -----------------------------------
	 * Construct DPM metadata for the DPart root node
	 * -----------------------------------
	 */
	cip4_metadata = p.poca_new(L"containertype=dict usage=dpm");
	p.poca_insert(cip4_metadata,
		L"type=string key=CIP4_Conformance value=base");
	p.poca_insert(cip4_metadata,
		L"type=string key=CIP4_Creator value=starter_pdfvt1");
	p.poca_insert(cip4_metadata,
		L"type=string key=CIP4_JobID value={Kraxi Systems invoice}");

	optlist.str(L"");
	optlist << L"containertype=dict usage=dpm type=dict "
			L"key=CIP4_Metadata value=" << cip4_metadata;
	cip4_root = p.poca_new(optlist.str());

	optlist.str(L"");
	optlist << L"containertype=dict usage=dpm type=dict "
			L"key=CIP4_Root value=" << cip4_root;
	dpm = p.poca_new(optlist.str());

	optlist.str(L"");
	/* Create root node in the DPart hierarchy and add DPM metadata  */
	optlist << L"dpm=" << dpm;
	p.begin_dpart(optlist.str());

	p.poca_delete(dpm, L"recursive=true");

	for (record=0; record < MAXRECORD; record++)
	{
	    const int MATRIXDATASIZE = 4 * MATRIXROWS;

	    unsigned char datamatrix[MATRIXDATASIZE];
	    int cip4_recipient, cip4_contact, cip4_person;
	    wstring firstname, lastname, result;
	    int         pagecount=0;
            unsigned    int item;

	    firstname = addressdata[get_random(ADDRESSCOUNT)].firstname;
	    lastname = addressdata[get_random(ADDRESSCOUNT)].lastname;

	    /* -----------------------------------
	     * Construct DPM metadata for the next DPart node (i.e. the page)
	     * -----------------------------------
	     */
	    dpm            = p.poca_new(L"containertype=dict usage=dpm");
	    cip4_root      = p.poca_new(L"containertype=dict usage=dpm");
	    cip4_recipient = p.poca_new(L"containertype=dict usage=dpm");
	    cip4_contact   = p.poca_new(L"containertype=dict usage=dpm");
	    cip4_person    = p.poca_new(L"containertype=dict usage=dpm");

	    optlist.str(L"");
	    optlist << L"type=dict key=CIP4_Root value=" << cip4_root;
	    p.poca_insert(dpm, optlist.str());

	    optlist.str(L"");
	    optlist << L"type=dict key=CIP4_Recipient value=" << cip4_recipient;
	    p.poca_insert(cip4_root, optlist.str());

	    optlist.str(L"");
	    optlist << L"type=string key=CIP4_UniqueID value={ID_" 
						    << record << L"}";
	    p.poca_insert(cip4_recipient, optlist.str());

	    optlist.str(L"");
	    optlist << L"type=dict key=CIP4_Contact value=" << cip4_contact;
	    p.poca_insert(cip4_recipient, optlist.str());

	    optlist.str(L"");
	    optlist << L"type=dict key=CIP4_Person value=" << cip4_person;
	    p.poca_insert(cip4_contact, optlist.str());

	    optlist.str(L"");
	    optlist << L"type=string key=CIP4_Firstname value={"
	    						<< firstname << L"}";
	    p.poca_insert(cip4_person, optlist.str());

	    optlist.str(L"");
	    optlist << L"type=string key=CIP4_Lastname value={"
	    						<< lastname << L"}";
	    p.poca_insert(cip4_person, optlist.str());

	    /* Create a new node in the document part hierarchy and
	     * add DPM metadata
	     */
	    optlist.str(L"");
	    optlist << L"dpm=" << dpm;
	    p.begin_dpart(optlist.str());

	    p.poca_delete(dpm, L"recursive=true");

            /* -----------------------------------
             * Create and place table with article list
             * -----------------------------------
             */
            /* ---------- Header row */
            unsigned int row = 1, col;
            int tbl = -1;

            for (col=1; col <= COLUMNCOUNT; col++)
            {
                optlist.str(L"");
                optlist << L"fittextline={position={" << alignments[col-1]
                    << L" center} " << fontoptions.str() << L"} margin=2";
                tbl = p.add_table_cell(tbl, col, row, headers[col-1],
                                        optlist.str());
            }
            row++;

            /* ---------- Data rows: one for each article */
            double total = 0;

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
                buf.str(L"");
                buf << item;
                optlist.str(L"");
                optlist << L"fittextline={position={" << alignments[col-1]
                        << L" center} " << fontoptions.str()
                        << L"} colwidth=5% margin=2";
                tbl = p.add_table_cell(tbl, col++, row, buf.str(),
                                        optlist.str());

                /* column 2: DESCRIPTION */
                optlist.str(L"");
                optlist << L"fittextline={position={" << alignments[col-1]
                        << L" center} " << fontoptions.str()
                        << L"} colwidth=50% margin=2";
                tbl = p.add_table_cell(tbl, col++, row,
                        articledata[i].name, optlist.str());

                /* column 3: QUANTITY */
                buf.str(L"");
                buf << quantity;
                optlist.str(L"");
                optlist << L"fittextline={position={" << alignments[col-1]
                        << L" center} " << fontoptions.str() << L"} margin=2";
                tbl = p.add_table_cell(tbl, col++, row, buf.str(),
                                        optlist.str());

                /* column 4: PRICE */
                buf.str(L"");
                buf << fixed << setprecision(2) << articledata[i].price;
                optlist.str(L"");
                optlist << L"fittextline={position={" << alignments[col-1]
                        << L" center} " << fontoptions.str() << L"} margin=2";
                tbl = p.add_table_cell(tbl, col++, row, buf.str(),
                                        optlist.str());

                /* column 5: AMOUNT */
                buf.str(L"");
                buf << fixed << setprecision(2) << sum;
                optlist.str(L"");
                optlist << L"fittextline={position={" << alignments[col-1]
                        << L" center} " << fontoptions.str() << L"} margin=2";
                tbl = p.add_table_cell(tbl, col++, row, buf.str(),
                                        optlist.str());

                total += sum;
                row++;
            }

            /* ---------- Print total in the rightmost column */
            buf.str(L"");
            buf << fixed << setprecision(2) << total;
            optlist.str(L"");
            optlist << L"fittextline={position={" << alignments[COLUMNCOUNT-1]
                    << L" center} " << fontoptions.str() << L"} margin=2";
            tbl = p.add_table_cell(tbl, COLUMNCOUNT, row++, buf.str(),
                                    optlist.str());


            /* ---------- Footer row with terms of payment */
            optlist.str(L"");
            optlist << fontoptions.str() << " alignment=justify leading=120%";
            int tf = p.create_textflow(closingtext, optlist.str());

            optlist.str(L"");
            optlist << L"rowheight=1 margin=2 margintop=" << 2*fontsize
                    << L" textflow=" << tf << L" colspan=" << COLUMNCOUNT;
            tbl = p.add_table_cell(tbl, 1, row++, L"", optlist.str());


            /* ----- Place the table instance(s), creating pages as required */
            do {
                double top;

                p.begin_page_ext(0, 0,
                        L"topdown=true width=a4.width height=a4.height");

                if (++pagecount == 1)
                {
                    /* -----------------------------------
                     * Place company stationery as background on first page
                     * for each recipient
                     * -----------------------------------
                     */
                    p.fit_pdi_page(page, 0, 842, L"");

                    /* -----------------------------------
                     * Place name and image of local sales rep on first page
                     * for each recipient
                     * -----------------------------------
                     */
                    y = 177;
                    x = 455;

                    optlist.str(L"");
                    optlist << L"fontname=" << fontname
                            << L" encoding=winansi embedding fontsize=9";
                    p.fit_textline(L"Local sales rep:", x, y, optlist.str());
                    p.fit_textline(salesrepnames[record % SALESREPS],
                            x, y+9, optlist.str());

                    y = 280;
                    p.fit_image(salesrepimage[record % SALESREPS], x, y,
                            L"boxsize={90 90} fitmethod=meet");


                    /* -----------------------------------
                     * Address of recipient
                     * -----------------------------------
                     */
                    y = 170;

                    optlist.str(L"");
                    optlist << L"fontname=" << fontname
                            << L" encoding=winansi embedding fontsize="
                            << fontsize;
                    buf.str(L"");
                    buf << firstname << L" " << lastname;
                    p.fit_textline(buf.str(), left, y, optlist.str());

                    y += leading;
                    p.fit_textline(addressdata[get_random(ADDRESSCOUNT)].flat,
                            left, y, optlist.str());

                    y += leading;
                    buf.str(L"");
                    buf << get_random(999) << L" "
                        << addressdata[get_random(ADDRESSCOUNT)].street;
                    p.fit_textline(buf.str(), left, y, optlist.str());

                    y += leading;
                    buf.str(L"");
                    buf << setw(5) << setfill(L'0') << get_random(99999)
                        << L" " << addressdata[get_random(ADDRESSCOUNT)].city;
                    p.fit_textline(buf.str(), left, y, optlist.str());


                    /* -----------------------------------
                     * Individual barcode image for each recipient. To get
                     * encapsulated image XObjects the renderingintent option
                     *  is used.
                     * -----------------------------------
                     */
                    create_datamatrix(datamatrix, record);
                    p.create_pvf(L"barcode", datamatrix, MATRIXDATASIZE,
                                    L"");

                    barcodeimage = p.load_image(L"raw", L"barcode",
                        L"bpc=1 components=1 width=32 height=32 invert "
                        L"pdfvt={scope=singleuse} renderingintent=Saturation");
                    if (barcodeimage == -1) {
                        wcerr << L"Error: " << p.get_errmsg() << endl;
                        return(2);
                    }

                    p.fit_image(barcodeimage, 280.0, 200.0, L"scale=1.5");
                    p.close_image(barcodeimage);
                    p.delete_pvf(L"barcode");


                    /* -----------------------------------
                     * Print header and date
                     * -----------------------------------
                     */
                    time(&timer);
                    ltime = *localtime(&timer);

                    y = 300;
                    buf.str(L"");
                    buf << L"INVOICE " << ltime.tm_year + 1900
                            << L"-" << record+1;
                    optlist.str(L"");
                    optlist << L"fontname=" << fontname
                            << L" encoding=winansi embedding fontsize="
                            << fontsize;
                    p.fit_textline(buf.str(), left, y, optlist.str());

                    buf.str(L"");
                    buf << months[ltime.tm_mon] << L" " << ltime.tm_mday
                        << L", " << ltime.tm_year + 1900;
                    optlist.str(L"");
                    optlist << L"fontname=" << fontname
                            << L" encoding=winansi fontsize=" << fontsize
                            << L" embedding position {100 0}";
                    p.fit_textline(buf.str(), right, y, optlist.str());

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
                result = p.fit_table(tbl,
                        left, bottom, right, top,
                        L"header=1 "
                        L"fill={{area=rowodd fillcolor={gray 0.9}} "
                            L"{area=rowlast fillcolor={gray 1}}} "
                        L"rowheightdefault=auto colwidthdefault=auto");

                if (result == L"_error")
                {
                    wcerr << L"Couldn't place table: " << p.get_errmsg()
                          << endl;
                    return 2;
                }

                p.end_page_ext(L"");
            } while (result == L"_boxfull");

            p.delete_table(tbl, L"");

	    /* Close node in the document part hierarchy */
	    p.end_dpart(L"");
	}

	p.close_pdi_page(page);
	p.close_pdi_document(stationery);

	for (i=0; i<SALESREPS; i++)
	{
	    p.close_image(salesrepimage[i]);
	}

	/* Close root node in the document part hierarchy */
	p.end_dpart(L"");

	p.end_document(L"");
    }
    catch (PDFlib::Exception &ex) {
        wcerr << L"PDFlib exception occurred: " << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname()
              << L": " << ex.get_errmsg() << endl;
        return 2;
    }

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
