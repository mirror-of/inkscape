#define __SPSVGVIEW_C__

/*
 * Inkscape - an ambitious vector drawing program
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
 * Inkscape authors:
 *   Johan Ceuppens
 * 
 * Copyright (C) 2004 Inkscape authors
 * 
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <config.h>

#include <string.h>
#include <sys/stat.h>
#include <locale.h>

#include <libnr/nr-macros.h>

#include <libxml/tree.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbutton.h>

#include "helper/sp-intl.h"
#include "document.h"
#include "svg-view.h"

#ifdef WITH_INKJAR
#include "inkjar/jar.h"
#endif

#include <iostream>

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif

struct _SPSlideShow {
    char **slides;
    int size;
    int length;
    int current;
    SPDocument *doc;
    GtkWidget *view;
    bool fullscreen;
};

static GtkWidget *sp_svgview_control_show (struct _SPSlideShow *ss);
static void sp_svgview_show_next (struct _SPSlideShow *ss);
static void sp_svgview_show_prev (struct _SPSlideShow *ss);
static void sp_svgview_goto_first (struct _SPSlideShow *ss);
static void sp_svgview_goto_last (struct _SPSlideShow *ss);

static int sp_svgview_show_next_cb (GtkWidget *widget, void *data);
static int sp_svgview_show_prev_cb (GtkWidget *widget, void *data);
static int sp_svgview_goto_first_cb (GtkWidget *widget, void *data);
static int sp_svgview_goto_last_cb (GtkWidget *widget, void *data);
#ifdef WITH_INKJAR
static bool is_jar(const gchar *filename);
#endif
static void usage();

static GtkWidget *ctrlwin = NULL;

/* TODO !!! make this temporary stub unnecessary */
Inkscape::Application *inkscape_get_instance() { return NULL; }

static int
sp_svgview_main_delete (GtkWidget *widget, GdkEvent *event, struct _SPSlideShow *ss)
{
    gtk_main_quit ();
    return FALSE;
}

static int
sp_svgview_main_key_press (GtkWidget *widget, GdkEventKey *event, struct _SPSlideShow *ss)
{
    switch (event->keyval) {
    case GDK_Up:
	sp_svgview_goto_first(ss);
	break;
    case GDK_Down:
	sp_svgview_goto_last(ss);
	break;
    case GDK_F11:
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
	if (ss->fullscreen) {
	    gtk_window_unfullscreen ((GtkWindow *) widget);
	    ss->fullscreen = false;
	} else {
	    gtk_window_fullscreen ((GtkWindow *) widget);
	    ss->fullscreen = true;
	}
#else
	std::cout<<"Your GTK+ does not support fullscreen mode. Upgrade to 2.2."<<std::endl;
#endif
	break;
    case GDK_Return:
	sp_svgview_control_show (ss);
	break;
    case GDK_Right:
	sp_svgview_show_next (ss);
	break;
    case GDK_Left:
	sp_svgview_show_prev (ss);
	break;
    case GDK_Escape:
	gtk_main_quit();
	break;
    default:
	break;
    }
    return FALSE;
}

int
main (int argc, const char **argv)
{
    if (argc == 1) {
	usage();
    }
    
    struct _SPSlideShow ss; 
	
    GtkWidget *w;
    int i;

    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    LIBXML_TEST_VERSION

	gtk_init (&argc, (char ***) &argv);

#ifdef lalaWITH_MODULES
    g_warning ("Have to autoinit modules (lauris)");
    sp_modulesys_init();
#endif /* WITH_MODULES */

    /* We must set LC_NUMERIC to default, or otherwise */
    /* we'll end with localised SVG files :-( */

    setlocale (LC_NUMERIC, "C");

    ss.size = 32;
    ss.length = 0;
    ss.current = 0;
    ss.slides = nr_new (char *, ss.size);
    ss.current = 0;
    ss.doc = NULL;
    ss.view = NULL;
    ss.fullscreen = false;

    for (i = 1; i < argc; i++) {
	struct stat st;
	if (!stat (argv[i], &st) 
	    && S_ISREG (st.st_mode) 
	    && (st.st_size > 64)) {
	    

#ifdef WITH_INKJAR
	    if (is_jar(argv[i])) {
		Inkjar::JarFileReader jar_file_reader(argv[i]);
		for (;;) {
		    GByteArray *gba = jar_file_reader.get_next_file();
		    if (gba == NULL) {
			char *c_ptr;
			gchar *last_filename = jar_file_reader.get_last_filename();
			if (last_filename == NULL)
			    break;
			if ((c_ptr = std::strrchr(last_filename, '/')) != NULL) {
			    if (*(++c_ptr) == '\0') {
				g_free(last_filename);
				continue;
			    }
			}
		    } else if (gba->len > 0) {
			//::write(1, gba->data, gba->len);
			/* Append to list */
			if (ss.length >= ss.size) {
			    /* Expand */
			    ss.size <<= 1;
			    ss.slides = nr_renew (ss.slides, char *, ss.size);
			}
			
			ss.doc = sp_document_new_from_mem ((const gchar *)gba->data, 
							   gba->len,
							   TRUE, 
							   TRUE);
			gchar *last_filename = jar_file_reader.get_last_filename();
			if (ss.doc) {
			    ss.slides[ss.length++] = strdup (last_filename);
			    sp_document_set_uri (ss.doc, strdup(last_filename));
			}
			g_byte_array_free(gba, TRUE);
			g_free(last_filename);
		    } else
			break;
		}
	    } else {
#endif /* WITH_INKJAR */
		/* Append to list */
		if (ss.length >= ss.size) {
		    /* Expand */
		    ss.size <<= 1;
		    ss.slides = nr_renew (ss.slides, char *, ss.size);
		    
		}
		
		ss.slides[ss.length++] = strdup (argv[i]);
		ss.doc = sp_document_new (ss.slides[ss.current], TRUE, TRUE);
		
		if (!ss.doc && ++ss.current >= ss.length) {
		    /* No loadable documents */
		    return 1;
		}
#ifdef WITH_INKJAR
	    }
#endif
	}
		
    }
    w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (w), SP_DOCUMENT_NAME (ss.doc));
    gtk_window_set_default_size (GTK_WINDOW (w),
				 MIN ((int)sp_document_width (ss.doc), (int)gdk_screen_width () - 64),
				 MIN ((int)sp_document_height (ss.doc), (int)gdk_screen_height () - 64));
    gtk_window_set_policy (GTK_WINDOW (w), TRUE, TRUE, FALSE);

    g_signal_connect (G_OBJECT (w), "delete_event", (GCallback) sp_svgview_main_delete, &ss);
    g_signal_connect (G_OBJECT (w), "key_press_event", (GCallback) sp_svgview_main_key_press, &ss);

    ss.view = sp_svg_view_widget_new (ss.doc);
    sp_svg_view_widget_set_resize (SP_SVG_VIEW_WIDGET (ss.view), FALSE, sp_document_width (ss.doc), sp_document_height (ss.doc));
    sp_document_ensure_up_to_date (ss.doc);
    sp_document_unref (ss.doc);
    gtk_widget_show (ss.view);
    gtk_container_add (GTK_CONTAINER (w), ss.view);

    gtk_widget_show (w);

    gtk_main ();

    return 0;
}

static int
sp_svgview_ctrlwin_delete (GtkWidget *widget, GdkEvent *event, void *data)
{
    ctrlwin = NULL;
    return FALSE;
}

static GtkWidget *
sp_svgview_control_show (struct _SPSlideShow *ss)
{
    if (!ctrlwin) {
	GtkWidget *t, *b;
	ctrlwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (ctrlwin), "delete_event", (GCallback) sp_svgview_ctrlwin_delete, NULL);
	t = gtk_table_new (1, 4, TRUE);
	gtk_container_add ((GtkContainer *) ctrlwin, t);
	b = gtk_button_new_from_stock (GTK_STOCK_GOTO_FIRST);
	gtk_table_attach ((GtkTable *) t, b, 0, 1, 0, 1, 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  0, 0);
	g_signal_connect ((GObject *) b, "clicked", (GCallback) sp_svgview_goto_first_cb, ss); 
	b = gtk_button_new_from_stock (GTK_STOCK_GO_BACK);
	gtk_table_attach ((GtkTable *) t, b, 1, 2, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  0, 0);
	g_signal_connect (G_OBJECT(b), "clicked", (GCallback) sp_svgview_show_prev_cb, ss); 
	b = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	gtk_table_attach ((GtkTable *) t, b, 2, 3, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  0, 0);
	g_signal_connect (G_OBJECT(b), "clicked", (GCallback) sp_svgview_show_next_cb, ss); 
	b = gtk_button_new_from_stock (GTK_STOCK_GOTO_LAST);
	gtk_table_attach ((GtkTable *) t, b, 3, 4, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  0, 0);
	g_signal_connect (G_OBJECT(b), "clicked", (GCallback) sp_svgview_goto_last_cb, ss); 
	gtk_widget_show_all (ctrlwin);
    } else {
	gtk_window_present ((GtkWindow *) ctrlwin);
    }

    return NULL;
}

static int 
sp_svgview_show_next_cb (GtkWidget *widget, void *data)
{
    sp_svgview_show_next(static_cast<_SPSlideShow *>(data));
    return FALSE;
}

static int 
sp_svgview_show_prev_cb (GtkWidget *widget, void *data)
{
    sp_svgview_show_prev(static_cast<struct _SPSlideShow *>(data));
    return FALSE;
}

static int 
sp_svgview_goto_first_cb (GtkWidget *widget, void *data)
{
    sp_svgview_goto_first(static_cast<struct _SPSlideShow *>(data));
    return FALSE;
}

static int 
sp_svgview_goto_last_cb (GtkWidget *widget, void *data)
{
    sp_svgview_goto_last(static_cast<struct _SPSlideShow *>(data));
    return FALSE;
}

static void
sp_svgview_show_next (struct _SPSlideShow *ss)
{
    SPDocument *doc;
    int current;
    doc = NULL;
    current = ss->current;
    while (!doc && (current < ss->length - 1)) {
	doc = sp_document_new (ss->slides[++current], TRUE, TRUE);
    }
    if (doc) {
	sp_view_set_document (SP_VIEW_WIDGET_VIEW (ss->view), doc);
	sp_document_ensure_up_to_date (doc);
	ss->doc = doc;
	ss->current = current;
    }
}

static void
sp_svgview_show_prev (struct _SPSlideShow *ss)
{
    SPDocument *doc;
    int current;
    doc = NULL;
    current = ss->current;
    while (!doc && (current > 0)) {
	doc = sp_document_new (ss->slides[--current], TRUE, TRUE);
    }
    if (doc) {
	sp_view_set_document (SP_VIEW_WIDGET_VIEW (ss->view), doc);
	sp_document_ensure_up_to_date (doc);
	ss->doc = doc;
	ss->current = current;
    }
}

static void
sp_svgview_goto_first (struct _SPSlideShow *ss)
{
    SPDocument *doc = NULL;
    int current = 0;
    for ( ; !doc && (current < ss->length); current++) {
	doc = sp_document_new (ss->slides[current], TRUE, TRUE);
    }
    if (doc) {
	sp_view_set_document (SP_VIEW_WIDGET_VIEW (ss->view), doc);
	sp_document_ensure_up_to_date (doc);
	ss->doc = doc;
	ss->current = current;
    }
}

static void
sp_svgview_goto_last (struct _SPSlideShow *ss)
{
    SPDocument *doc = NULL;
    int current = ss->length - 1;
    for ( ; !doc && (current >= 0); current--) {
	doc = sp_document_new (ss->slides[current], TRUE, TRUE);
    }
    if (doc) {
	sp_view_set_document (SP_VIEW_WIDGET_VIEW (ss->view), doc);
	sp_document_ensure_up_to_date (doc);
	ss->doc = doc;
	ss->current = current;
    }
}

#ifdef WITH_INKJAR
static bool
is_jar(const gchar *filename)
{
    //fixme: mime check or something
    char *extension;
    if ((extension = strrchr(filename, '.')) != NULL) {
	if (strcmp(extension, ".jar") == 0 || strcmp(extension, ".sxw") == 0)
	    return true;
    }
    return false;
}
#endif /* WITH_INKJAR */

static void usage()
{
    fprintf(stdout, 
	    "Usage: inkview [FILES ...]\n"
	    "\twhere FILES are SVG (.svg)"
#ifdef WITH_INKJAR
	    "or archives of svgs (.sxw, .jar)"
#endif
	    "\n");
    exit(1);
}

Inkscape::Application *inkscape;
void inkscape_ref (void) {}
void inkscape_unref (void) {}
void inkscape_add_document (SPDocument *document) {}
void inkscape_remove_document (SPDocument *document) {}
SPRepr *inkscape_get_repr (Inkscape::Application *inkscape, const gchar *key) {return NULL;}
#include "widgets/menu.h"
void sp_menu_append (SPMenu *menu, const gchar *name, const gchar *tip, const void *data) {}
