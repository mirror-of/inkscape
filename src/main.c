#define __MAIN_C__

/*
 * Sodipodi - an ambitious vector drawing program
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Davide Puricelli <evo@debian.org>
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Masatake YAMATO  <jet@gyve.org>
 *   F.J.Franklin <F.J.Franklin@sheffield.ac.uk>
 *   Michael Meeks <michael@helixcode.com>
 *   Chema Celorio <chema@celorio.com>
 *   Pawel Palucha
 * ... and various people who have worked with various projects
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <config.h>

#ifdef __FreeBSD__
#include <floatingpoint.h>
#endif
#include <string.h>
#include <locale.h>

#ifdef WITH_POPT
#include <popt.h>
#ifndef POPT_TABLEEND
#define POPT_TABLEEND { NULL, '\0', 0, 0, 0, NULL, NULL }
#endif /* Not def: POPT_TABLEEND */
#endif

#include <libxml/tree.h>
#include <glib-object.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkbox.h>

#include "macros.h"
#include "file.h"
#include "document.h"
#include "desktop.h"
#include "sp-object.h"
#include "toolbox.h"
#include "interface.h"
#include "print.h"
#include "slideshow.h"

#include "svg/svg.h"

#include "sodipodi-private.h"

#include "sp-namedview.h"
#include "sp-guide.h"
#include "sp-object-repr.h"

#ifdef WITH_MODULES
#include "modules/sp-module-sys.h"
#endif /* WITH_MODULES */

#ifdef WITH_KDE
#include "modules/kde.h"
#endif
#ifdef WIN32
#include "modules/win32.h"
#endif

#include "helper/sp-intl.h"

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif
#ifndef HAVE_GTK_WINDOW_SET_DEFAULT_ICON_FROM_FILE
#define gtk_window_set_default_icon_from_file(f,v)
#endif

#ifdef WITH_POPT
enum {
	SP_ARG_NONE,
	SP_ARG_NOGUI,
	SP_ARG_GUI,
	SP_ARG_FILE,
	SP_ARG_PRINT,
	SP_ARG_EXPORT_PNG,
	SP_ARG_EXPORT_DPI,
	SP_ARG_EXPORT_AREA,
	SP_ARG_EXPORT_WIDTH,
	SP_ARG_EXPORT_HEIGHT,
	SP_ARG_EXPORT_BACKGROUND,
	SP_ARG_EXPORT_SVG,
	SP_ARG_SLIDESHOW,
	SP_ARG_BITMAP_ICONS,
	SP_ARG_LAST
};
#endif

int sp_main_gui (int argc, const char **argv);
int sp_main_console (int argc, const char **argv);
static void sp_do_export_png (SPDocument *doc);

/* fixme: We need this non-static, but better arrange it another way (Lauris) */
gboolean sp_bitmap_icons = FALSE;

static guchar *sp_global_printer = NULL;
static gboolean sp_global_slideshow = FALSE;
static guchar *sp_export_png = NULL;
static guchar *sp_export_dpi = NULL;
static guchar *sp_export_area = NULL;
static guchar *sp_export_width = NULL;
static guchar *sp_export_height = NULL;
static guchar *sp_export_background = NULL;
static guchar *sp_export_svg = NULL;

#ifdef WITH_POPT
static GSList *sp_process_args (poptContext ctx);
struct poptOption options[] = {
	{"without-gui", 'z', POPT_ARG_NONE, NULL, SP_ARG_NOGUI,
	 N_("Do not use X server (only process files from console)"),
	 NULL},
	{"with-gui", 'x', POPT_ARG_NONE, NULL, SP_ARG_GUI,
	 N_("Try to use X server even if $DISPLAY is not set)"),
	 NULL},
	{"file", 'f', POPT_ARG_STRING, NULL, SP_ARG_FILE,
	 N_("Open specified document(s) (option string may be excluded)"),
	 N_("FILENAME")},
	{"print", 'p', POPT_ARG_STRING, &sp_global_printer, SP_ARG_PRINT,
	 N_("Print document(s) to specified output file (use '| program' for pipe)"),
	 N_("FILENAME")},
	{"export-png", 'e', POPT_ARG_STRING, &sp_export_png, SP_ARG_EXPORT_PNG,
	 N_("Export document to png file"),
	 N_("FILENAME")},
	{"export-dpi", 'd', POPT_ARG_STRING, &sp_export_dpi, SP_ARG_EXPORT_DPI,
	 N_("The resolution used for converting SVG into bitmap (default 72.0)"),
	 N_("DPI")},
	{"export-area", 'a', POPT_ARG_STRING, &sp_export_area, SP_ARG_EXPORT_AREA,
	 N_("Exported area in millimeters (default is full document, 0,0 is lower-left corner)"),
	 N_("x0:y0:x1:y1")},
	{"export-width", 'w', POPT_ARG_STRING, &sp_export_width, SP_ARG_EXPORT_WIDTH,
	 N_("The width of generated bitmap in pixels (overwrites dpi)"), N_("WIDTH")},
	{"export-height", 'h', POPT_ARG_STRING, &sp_export_height, SP_ARG_EXPORT_HEIGHT,
	 N_("The height of generated bitmap in pixels (overwrites dpi)"), N_("HEIGHT")},
	{"export-background", 'b', POPT_ARG_STRING, &sp_export_background, SP_ARG_EXPORT_HEIGHT,
	 N_("Background color of exported bitmap (any SVG supported color string)"), N_("COLOR")},
	{"export-svg", 0, POPT_ARG_STRING, &sp_export_svg, SP_ARG_EXPORT_SVG,
	 N_("Export document to plain SVG file (no \"xmlns:sodipodi\" namespace)"), N_("FILENAME")},
	{"slideshow", 's', POPT_ARG_NONE, &sp_global_slideshow, SP_ARG_SLIDESHOW,
	 N_("Show given files one-by-one, switch to next on any key/mouse event"), NULL},
	{"bitmap-icons", 'i', POPT_ARG_NONE, &sp_bitmap_icons, SP_ARG_BITMAP_ICONS,
	 N_("Prefer bitmap (xpm) icons to SVG ones"),
	 NULL},
	POPT_AUTOHELP POPT_TABLEEND
};
#endif

int
main (int argc, const char **argv)
{
	gboolean use_gui;
	gint result, i;

#ifdef __FreeBSD__
	fpsetmask (fpgetmask() & ~(FP_X_DZ|FP_X_INV));
#endif

	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	LIBXML_TEST_VERSION

#ifndef WIN32
	use_gui = (getenv ("DISPLAY") != NULL);
#else
	use_gui = TRUE;
#endif
	/* Test whether with/without GUI is forced */
	for (i = 1; i < argc; i++) {
		if (!strcmp (argv[i], "-z") ||
		    !strcmp (argv[i], "--without-gui") ||
		    !strcmp (argv[i], "-p") ||
		    !strncmp (argv[i], "--print", 7) ||
		    !strcmp (argv[i], "-e") ||
		    !strncmp (argv[i], "--export-png", 12) ||
		    !strncmp (argv[i], "--export-svg", 12)) {
			use_gui = FALSE;
			break;
		} else if (!strcmp (argv[i], "-x") || !strcmp (argv[i], "--with-gui")) {
			use_gui = TRUE;
			break;
		}
	}

	if (use_gui) {
		result = sp_main_gui (argc, argv);
	} else {
		result = sp_main_console (argc, argv);
	}

#ifdef __FreeBSD__
	fpresetsticky(FP_X_DZ|FP_X_INV);
	fpsetmask(FP_X_DZ|FP_X_INV);
#endif
	return result;
}

int
sp_main_gui (int argc, const char **argv)
{
#if WITH_POPT
	poptContext ctx;
#endif
	GSList *fl = NULL;

	gtk_init (&argc, (char ***) &argv);

	sp_modulesys_init();

	/* We must set LC_NUMERIC to default, or otherwise */
	/* we'll end with localised SVG files :-( */

	setlocale (LC_NUMERIC, "C");

	/* fixme: Move these to some centralized location (Lauris) */
	sp_object_type_register ("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
	sp_object_type_register ("sodipodi:guide", SP_TYPE_GUIDE);

#if WITH_POPT
	ctx = poptGetContext (NULL, argc, argv, options, 0);
	g_return_val_if_fail (ctx != NULL, 1);
	/* Collect own arguments */
	fl = sp_process_args (ctx);
	poptFreeContext (ctx);
#endif

#ifdef WITH_KDE
	sp_kde_init (argc, (char **) argv, "Sodipodi");
#endif
#ifdef WIN32
	sp_win32_init (0, NULL, "Sodipodi");
#endif

	/* Set default icon */
	if (g_file_test (DATADIR "/pixmaps/sodipodi.png", G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK)) {
		gtk_window_set_default_icon_from_file (DATADIR "/pixmaps/sodipodi.png", NULL);
	}

	if (!sp_global_slideshow) {
		sodipodi = sodipodi_application_new ();
		sodipodi_load_preferences (sodipodi);
		sodipodi_load_extensions (sodipodi);
		sp_maintoolbox_create_toplevel ();
		sodipodi_unref ();

		while (fl) {
			SPDocument *doc;
			doc = sp_document_new ((const gchar *) fl->data, TRUE, TRUE);
			if (doc) {
				if (sp_export_png) {
					sp_do_export_png (doc);
				} else {
					SPViewWidget *dtw;
					dtw = sp_desktop_widget_new (sp_document_namedview (doc, NULL));
					if (dtw) sp_create_window (dtw, TRUE);
				}
				sp_document_unref (doc);
			}
			fl = g_slist_remove (fl, fl->data);
		}
	} else {
		if (fl) {
			GtkWidget *ss;
			sodipodi = sodipodi_application_new ();
			sodipodi_load_preferences (sodipodi);
			sodipodi_load_extensions (sodipodi);
			ss = sp_slideshow_new (fl);
			if (ss) gtk_widget_show (ss);
			sodipodi_unref ();
		} else {
			fprintf (stderr, "No slides to display\n");
			exit (0);
		}
	}

	gtk_main ();

#ifdef WITH_KDE
	sp_kde_finish ();
#endif
#ifdef WIN32
	sp_win32_finish ();
#endif

	return 0;
}

int
sp_main_console (int argc, const char **argv)
{
#ifdef WITH_POPT
	poptContext ctx = NULL;
#endif
	GSList * fl = NULL;
	guchar *printer;

	/* We are started in text mode */

#ifdef WITH_XFT
	/* Still have to init gdk, or Xft does not work */
	gdk_init (&argc, (char ***) &argv);
#endif

	/* We must set LC_NUMERIC to default, or otherwise */
	/* we'll end with localised SVG files :-( */

	setlocale (LC_NUMERIC, "C");

	/* fixme: Move these to some centralized location (Lauris) */
	sp_object_type_register ("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
	sp_object_type_register ("sodipodi:guide", SP_TYPE_GUIDE);

#ifdef WITH_POPT
	ctx = poptGetContext (NULL, argc, argv, options, 0);
	g_return_val_if_fail (ctx != NULL, 1);
	fl = sp_process_args (ctx);
	poptFreeContext (ctx);
#endif

	if (fl == NULL) {
		g_print ("Nothing to do!\n");
		exit (0);
	}

	/* Check for and set up printing path */
	printer = NULL;
	if (sp_global_printer != NULL) {
		if ((sp_global_printer[0] != '|') && (sp_global_printer[0] != '/')) {
			gchar *cwd;
			/* Gnome-print appends relative paths to $HOME by default */
			cwd = g_get_current_dir ();
			printer = g_build_filename (cwd, sp_global_printer, NULL);
			g_free (cwd);
		} else {
			printer = g_strdup (sp_global_printer);
		}
	}

	/* Start up g type system, without requiring X */
	g_type_init();
	sodipodi = sodipodi_application_new ();
	sodipodi_load_preferences (sodipodi);
	sodipodi_load_extensions (sodipodi);

	while (fl) {
		SPDocument *doc;
		doc = sp_document_new ((gchar *) fl->data, FALSE, TRUE);
		if (doc == NULL) {
			g_warning ("Specified document %s cannot be opened (is it valid SVG file?)", (gchar *) fl->data);
		} else {
			if (printer) {
				sp_print_document_to_file (doc, printer);
			}
			if (sp_export_png) {
				sp_do_export_png (doc);
			}
			if (sp_export_svg) {
				SPReprDoc *rdoc;
				SPRepr *repr;
				rdoc = sp_repr_document_new ("svg");
				repr = sp_repr_document_root (rdoc);
				repr = sp_object_invoke_write (sp_document_root (doc), repr, SP_OBJECT_WRITE_BUILD);
				sp_repr_save_file (sp_repr_document (repr), sp_export_svg);
			}
		}
		fl = g_slist_remove (fl, fl->data);
	}

	if (printer) g_free (printer);

	sodipodi_unref ();

	return 0;
}

static void
sp_do_export_png (SPDocument *doc)
{
	ArtDRect area;
	gdouble dpi;
	gboolean has_area;
	gint width, height;
	guint32 bgcolor;

	/* Check for and set up exporting path */
	has_area = FALSE;
	dpi = 72.0;

	if (sp_export_dpi) {
		dpi = atof (sp_export_dpi);
		if ((dpi < 0.1) || (dpi > 10000.0)) {
			g_warning ("DPI value %s out of range [0.1 - 10000.0]", sp_export_dpi);
			return;
		}
		g_print ("dpi is %g\n", dpi);
	}

	if (sp_export_area) {
		/* Try to parse area (given in mm) */
		if (sscanf (sp_export_area, "%lg:%lg:%lg:%lg", &area.x0, &area.y0, &area.x1, &area.y1) == 4) {
			area.x0 *= (72.0 / 25.4);
			area.y0 *= (72.0 / 25.4);
			area.x1 *= (72.0 / 25.4);
			area.y1 *= (72.0 / 25.4);
			has_area = TRUE;
		} else {
			g_warning ("Export area '%s' illegal (use 'x0:y0:x1:y1)", sp_export_area);
			return;
		}
		if ((area.x0 >= area.x1) || (area.y0 >= area.y1)) {
			g_warning ("Export area '%s' has invalid values", sp_export_area);
			return;
		}
	} else {
		/* Export the whole document */
		area.x0 = 0.0;
		area.y0 = 0.0;
		area.x1 = sp_document_width (doc);
		area.y1 = sp_document_height (doc);
	}

	/* Kill warning */
	width = 0;
	height = 0;

	if (sp_export_width) {
		width = atoi (sp_export_width);
		if ((width < 16) || (width > 65536)) {
			g_warning ("Export width %d out of range (16 - 65536)", width);
			return;
		}
		dpi = (gdouble) width * 72.0 / (area.x1 - area.x0);
	}

	if (sp_export_height) {
		height = atoi (sp_export_height);
		if ((height < 16) || (height > 65536)) {
			g_warning ("Export height %d out of range (16 - 65536)", width);
			return;
		}
		dpi = (gdouble) height * 72.0 / (area.y1 - area.y0);
	}

	if (!sp_export_width) {
		width = (gint) ((area.x1 - area.x0) * dpi / 72.0 + 0.5);
	}

	if (!sp_export_height) {
		height = (gint) ((area.y1 - area.y0) * dpi / 72.0 + 0.5);
	}

	bgcolor = 0x00000000;
	if (sp_export_background) {
		bgcolor = sp_svg_read_color (sp_export_background, 0xffffff00);
		bgcolor |= 0xff;
	}
	g_print ("Background is %x\n", bgcolor);

	g_print ("Exporting %g %g %g %g to %d x %d rectangle\n", area.x0, area.y0, area.x1, area.y1, width, height);

	if ((width >= 16) || (height >= 16) || (width < 65536) || (height < 65536)) {
		sp_export_png_file (doc, sp_export_png, area.x0, area.y0, area.x1, area.y1, width, height, bgcolor);
	} else {
		g_warning ("Calculated bitmap dimensions %d %d out of range (16 - 65535)", width, height);
	}
}

#ifdef WITH_POPT
static GSList *
sp_process_args (poptContext ctx)
{
	GSList * fl;
	const gchar ** args, *fn;
	gint i, a;

	fl = NULL;

	while ((a = poptGetNextOpt (ctx)) >= 0) {
		switch (a) {
		case SP_ARG_FILE:
			fn = poptGetOptArg (ctx);
			if (fn != NULL) {
				fl = g_slist_append (fl, g_strdup (fn));
			}
			break;
		default:
			break;
		}
	}
	args = poptGetArgs (ctx);
	if (args != NULL) {
		for (i = 0; args[i] != NULL; i++) {
			fl = g_slist_append (fl, (gpointer) args[i]);
		}
	}

	return fl;
}
#endif

