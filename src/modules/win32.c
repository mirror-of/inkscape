#define __SP_MODULE_WIN32_C__

/*
 * Windows stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define USE_TIMER

#include <config.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>

#include "display/nr-arena-item.h"
#include "display/nr-arena.h"
#include "document.h"

#include "win32.h"

/* Initialization */

static unsigned int SPWin32Modal = FALSE;

static void
sp_win32_gdk_event_handler (GdkEvent *event)
{
	if (SPWin32Modal) {
		/* Win32 widget is modal, filter events */
		switch (event->type) {
		case GDK_NOTHING:
		case GDK_DELETE:
		case GDK_SCROLL:
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
		case GDK_DRAG_STATUS:
		case GDK_DRAG_ENTER:
		case GDK_DRAG_LEAVE:
		case GDK_DRAG_MOTION:
		case GDK_DROP_START:
		case GDK_DROP_FINISHED:
			return;
			break;
		default:
			break;
		}
	}
	gtk_main_do_event (event);
}

void
sp_win32_init (int argc, char **argv, const char *name)
{
	gdk_event_handler_set ((GdkEventFunc) sp_win32_gdk_event_handler, NULL, NULL);
}

void
sp_win32_finish (void)
{
}

#define SP_FOREIGN_MAX_ITER 10

VOID CALLBACK
sp_win32_timer (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	int cdown = 0;
	while ((cdown++ < SP_FOREIGN_MAX_ITER) && gdk_events_pending ()) {
		gtk_main_iteration_do (FALSE);
	}
	gtk_main_iteration_do (FALSE);
}

/* Printing */

static void sp_module_print_win32_class_init (SPModulePrintWin32Class *klass);
static void sp_module_print_win32_init (SPModulePrintWin32 *pmod);
static void sp_module_print_win32_finalize (GObject *object);

static unsigned int sp_module_print_win32_setup (SPModulePrint *mod);
static unsigned int sp_module_print_win32_begin (SPModulePrint *mod, SPDocument *doc);
static unsigned int sp_module_print_win32_finish (SPModulePrint *mod);

static SPModulePrintClass *print_win32_parent_class;

GType
sp_module_print_win32_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModulePrintWin32Class),
			NULL, NULL,
			(GClassInitFunc) sp_module_print_win32_class_init,
			NULL, NULL,
			sizeof (SPModulePrintWin32),
			16,
			(GInstanceInitFunc) sp_module_print_win32_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE_PRINT, "SPModulePrintWin32", &info, 0);
	}
	return type;
}

static void
sp_module_print_win32_class_init (SPModulePrintWin32Class *klass)
{
	GObjectClass *g_object_class;
	SPModulePrintClass *module_print_class;

	g_object_class = (GObjectClass *)klass;
	module_print_class = (SPModulePrintClass *) klass;

	print_win32_parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_print_win32_finalize;

	module_print_class->setup = sp_module_print_win32_setup;
	module_print_class->begin = sp_module_print_win32_begin;
	module_print_class->finish = sp_module_print_win32_finish;

}

static void
sp_module_print_win32_init (SPModulePrintWin32 *pmod)
{
	/* Nothing here */
}

static void
sp_module_print_win32_finalize (GObject *object)
{
	SPModulePrintWin32 *w32mod;

	w32mod = (SPModulePrintWin32 *) object;
	
	DeleteDC (w32mod->hDC);

	G_OBJECT_CLASS (print_win32_parent_class)->finalize (object);
}

#ifdef USE_TIMER

UINT_PTR CALLBACK
sp_w32_print_hook (HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
#if 0
	int cdown = 0;
	while ((cdown++ < SP_FOREIGN_MAX_ITER) && gdk_events_pending ()) {
		gtk_main_iteration_do (FALSE);
	}
	gtk_main_iteration_do (FALSE);
#endif
	return 0;
}
#endif

static unsigned int
sp_module_print_win32_setup (SPModulePrint *mod)
{
	SPModulePrintWin32 *w32mod;
	HRESULT res;
	PRINTDLG pd = {
		sizeof (PRINTDLG),
		NULL, /* hwndOwner */
		NULL, /* hDevMode */
		NULL, /* hDevNames */
		NULL, /* hDC */
		PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE, /* Flags */
		1, 1, 1, 1, /* nFromPage, nToPage, nMinPage, nMaxPage */
		1, /* nCoies */
		NULL, /* hInstance */
		0, /* lCustData */
		NULL, NULL, NULL, NULL, NULL, NULL
	};
#ifdef USE_TIMER
	UINT_PTR timer;
#endif
	int caps;

	w32mod = (SPModulePrintWin32 *) mod;

	SPWin32Modal = TRUE;
#ifdef USE_TIMER
	pd.Flags |= PD_ENABLEPRINTHOOK;
	pd.lpfnPrintHook = sp_w32_print_hook;
	timer = SetTimer (NULL, 0, 40, sp_win32_timer);
#endif

	res = PrintDlg (&pd);

#ifdef USE_TIMER
	KillTimer (NULL, timer);
#endif
	SPWin32Modal = FALSE;

	if (!res) return FALSE;

	w32mod->hDC = pd.hDC;

#if 0
	caps = GetDeviceCaps (w32mod->hDC, RASTERCAPS);
	if (caps & RC_BANDING) {
		printf ("needs banding\n");
	}
	if (caps & RC_BITBLT) {
		printf ("does bitblt\n");
	}
	if (caps & RC_DIBTODEV) {
		printf ("does dibtodev\n");
	}
	if (caps & RC_STRETCHDIB) {
		printf ("does stretchdib\n");
	}
#endif
	if (pd.hDevMode) {
		DEVMODE *devmodep;
		devmodep = pd.hDevMode;
		if (devmodep->dmFields & DM_ORIENTATION) {
			w32mod->landscape = (devmodep->dmOrientation == DMORIENT_LANDSCAPE);
		}
	}

	return TRUE;
}

static unsigned int
sp_module_print_win32_begin (SPModulePrint *mod, SPDocument *doc)
{
	SPModulePrintWin32 *w32mod;
	DOCINFO di = {
		sizeof (DOCINFO),
		NULL, /* lpszDocName */
		NULL, /* lpszOutput */
		NULL, /* lpszDatatype */
		0 /* DI_APPBANDING */ /* fwType */
	};
	int res;

	w32mod = (SPModulePrintWin32 *) mod;

	w32mod->PageWidth = sp_document_width (doc);
	w32mod->PageHeight = sp_document_height (doc);

	di.lpszDocName = SP_DOCUMENT_NAME (doc);

	SPWin32Modal = TRUE;

	res = StartDoc (w32mod->hDC, &di);
	res = StartPage (w32mod->hDC);

	SPWin32Modal = FALSE;

	return 0;
}

static unsigned int
sp_module_print_win32_finish (SPModulePrint *mod)
{
	SPModulePrintWin32 *w32mod;
	int dpiX, dpiY;
	int pPhysicalWidth, pPhysicalHeight;
	int pPhysicalOffsetX, pPhysicalOffsetY;
	int pPrintableWidth, pPrintableHeight;
	float scalex, scaley;
	int x0, y0, x1, y1;
	int width, height;
	NRMatrixF affine;
	unsigned char *px;
	int sheight, row;
	BITMAPINFO bmInfo = {
		{
			sizeof (BITMAPINFOHEADER), // bV4Size 
			64, /* biWidth */
			64, /* biHeight */
			1, /* biPlanes */
			32, /* biBitCount */
			BI_RGB, /* biCompression */
			0, /* biSizeImage */
			2835, /* biXPelsPerMeter */
			2835, /* biYPelsPerMeter */
			0, /* biClrUsed */
			0, /* biClrImportant */
		},
		{0, 0, 0, 0} /* bmiColors */
	};
	RECT wrect;
	int res;

	w32mod = (SPModulePrintWin32 *) mod;

	SPWin32Modal = TRUE;

	// Number of pixels per logical inch
	dpiX = (float) GetDeviceCaps (w32mod->hDC, LOGPIXELSX);
	dpiY = (float) GetDeviceCaps (w32mod->hDC, LOGPIXELSY);
	// Size in pixels of the printable area
	pPhysicalWidth = GetDeviceCaps (w32mod->hDC, PHYSICALWIDTH); 
	pPhysicalHeight = GetDeviceCaps (w32mod->hDC, PHYSICALHEIGHT); 
	// Top left corner of prontable area
	pPhysicalOffsetX = GetDeviceCaps (w32mod->hDC, PHYSICALOFFSETX); 
	pPhysicalOffsetY = GetDeviceCaps (w32mod->hDC, PHYSICALOFFSETY); 
	// Size in pixels of the printable area
	pPrintableWidth = GetDeviceCaps (w32mod->hDC, HORZRES); 
	pPrintableHeight = GetDeviceCaps (w32mod->hDC, VERTRES); 

	// Scaling from document to device
	scalex = dpiX / 72.0;
	scaley = dpiY / 72.0;

	// We simply map document 0,0 to physical page 0,0
	affine.c[0] = scalex / 1.25;
	affine.c[1] = 0.0;
	affine.c[2] = 0.0;
	affine.c[3] = scaley / 1.25;
	affine.c[4] = 0.0;
	affine.c[5] = 0.0;

	nr_arena_item_set_transform (mod->root, &affine);

	// Calculate printable area in device coordinates
	x0 = pPhysicalOffsetX;
	y0 = pPhysicalOffsetY;
	x1 = x0 + pPrintableWidth;
	y1 = y0 + pPrintableHeight;
	x1 = MIN (x1, (int) (w32mod->PageWidth * scalex));
	y1 = MIN (y1, (int) (w32mod->PageHeight * scaley));

	width = x1 - x0;
	height = y1 - y0;

	px = nr_new (unsigned char, 4 * 64 * width);
	sheight = 64;

	/* Printing goes here */
	for (row = 0; row < height; row += 64) {
		NRPixBlock pb;
		NRRectL bbox;
		NRGC gc;
		int num_rows;
		int i;

		num_rows = sheight;
		if ((row + num_rows) > height) num_rows = height - row;

		/* Set area of interest */
		bbox.x0 = x0;
		bbox.y0 = y0 + row;
		bbox.x1 = bbox.x0 + width;
		bbox.y1 = bbox.y0 + num_rows;
		/* Update to renderable state */
		nr_matrix_d_set_identity (&gc.transform);
		nr_arena_item_invoke_update (mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

		nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N, bbox.x0, bbox.y0, bbox.x1, bbox.y1, px, 4 * (bbox.x1 - bbox.x0), FALSE, FALSE);

		/* Blitter goes here */
		bmInfo.bmiHeader.biWidth = bbox.x1 - bbox.x0;
		bmInfo.bmiHeader.biHeight = -(bbox.y1 - bbox.y0);

		memset (px, 0xff, 4 * num_rows * width);
		/* Render */
		nr_arena_item_invoke_render (mod->root, &bbox, &pb, 0);

		SetStretchBltMode(w32mod->hDC, COLORONCOLOR);
		res = StretchDIBits (w32mod->hDC,
						bbox.x0 - x0, bbox.y0 - y0, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0,
						0, 0, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0,
						px,
						&bmInfo,
                        DIB_RGB_COLORS,
                        SRCCOPY);

		/* Blitter ends here */

		nr_pixblock_release (&pb);
	}

	nr_free (px);

	res = EndPage (w32mod->hDC);
	res = EndDoc (w32mod->hDC);

	SPWin32Modal = FALSE;

	return 0;
}

/* File dialogs */

char *
sp_win32_get_open_filename (unsigned char *dir, unsigned char *filter, unsigned char *title)
{
	char fnbuf[4096] = {0};
	OPENFILENAME ofn = {
		sizeof (OPENFILENAME),
		NULL, /* hwndOwner */
		NULL, /* hInstance */
		filter, /* lpstrFilter */
		NULL, /* lpstrCustomFilter */
		0, /* nMaxCustFilter  */
		1, /* nFilterIndex */
		fnbuf, /* lpstrFile */
		sizeof (fnbuf), /* nMaxFile */
		NULL, /* lpstrFileTitle */
		0, /* nMaxFileTitle */
		dir, /* lpstrInitialDir */
		title, /* lpstrTitle */
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, /* Flags */
		0, /* nFileOffset */
		0, /* nFileExtension */
		NULL, /* lpstrDefExt */
		0, /* lCustData */
		NULL, /* lpfnHook */
		NULL /* lpTemplateName */
	};
	int retval;
#ifdef USE_TIMER
	UINT_PTR timer;
#endif

	SPWin32Modal = TRUE;
#ifdef USE_TIMER
	timer = SetTimer (NULL, 0, 40, sp_win32_timer);
#endif

	retval = GetOpenFileName (&ofn);

#ifdef USE_TIMER
	KillTimer (NULL, timer);
#endif
	SPWin32Modal = FALSE;

	if (!retval) {
		int errcode;
		errcode = CommDlgExtendedError();
		return NULL;
    }
	return g_strdup (fnbuf);
}

char *
sp_win32_get_write_filename (unsigned char *dir, unsigned char *filter, unsigned char *title)
{
	return NULL;
}

char *
sp_win32_get_save_filename (unsigned char *dir, unsigned int *spns)
{
	char fnbuf[4096] = {0};
	OPENFILENAME ofn = {
		sizeof (OPENFILENAME),
		NULL, /* hwndOwner */
		NULL, /* hInstance */
		"SVG with sodipodi namespace\0*\0Standard SVG\0*\0", /* lpstrFilter */
		NULL, /* lpstrCustomFilter */
		0, /* nMaxCustFilter  */
		1, /* nFilterIndex */
		fnbuf, /* lpstrFile */
		sizeof (fnbuf), /* nMaxFile */
		NULL, /* lpstrFileTitle */
		0, /* nMaxFileTitle */
		dir, /* lpstrInitialDir */
		"Save document to file", /* lpstrTitle */
		OFN_HIDEREADONLY, /* Flags */
		0, /* nFileOffset */
		0, /* nFileExtension */
		NULL, /* lpstrDefExt */
		0, /* lCustData */
		NULL, /* lpfnHook */
		NULL /* lpTemplateName */
	};
	int retval;
#ifdef USE_TIMER
	UINT_PTR timer;
#endif

	SPWin32Modal = TRUE;
#ifdef USE_TIMER
	timer = SetTimer (NULL, 0, 40, sp_win32_timer);
#endif

	retval = GetSaveFileName (&ofn);

#ifdef USE_TIMER
	KillTimer (NULL, timer);
#endif
	SPWin32Modal = FALSE;

	if (!retval) {
		int errcode;
		errcode = CommDlgExtendedError();
		return NULL;
    }
	*spns = (ofn.nFilterIndex != 2);
	return g_strdup (fnbuf);
}


