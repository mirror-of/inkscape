#define __SP_HELP_C__

/*
 * Help/About window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtksignal.h>
#include "inkscape.h"
#include "document.h"
#include "sp-text.h"
#include "svg-view.h"
#include "help.h"
#include "file.h"
#include "helper/sp-intl.h"
#include "libnr/nr-macros.h"

static GtkWidget *w = NULL;

static gint
sp_help_about_delete (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    w = NULL;
    return FALSE;
}

#define WINDOW_MIN 20
#define WINDOW_MAX INT_MAX

void
sp_help_about (void)
{
    SPDocument *doc;
    SPObject *version;
    GtkWidget *v;
    gint width, height;


    /* REJON: INKSCAPE_PIXMAPSDIR was changed to INKSCAPE_SCREENSDIR to 
     * coordinate with the directory reorganization.
     */

    if (!w) {
        doc = sp_document_new (INKSCAPE_SCREENSDIR "/about.svg", FALSE, TRUE);
        g_return_if_fail (doc != NULL);
        version = sp_document_lookup_id (doc, "version");
        
        if (version && SP_IS_TEXT (version)) {
            sp_text_set_repr_text_multiline ( SP_TEXT (version), 
                                              INKSCAPE_VERSION);
        }
		
        sp_document_ensure_up_to_date (doc);


        w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

        gtk_window_set_title (GTK_WINDOW (w), _("About Inkscape"));

        width  = static_cast< gint > ( CLAMP( sp_document_width(doc), 
                                              WINDOW_MIN, WINDOW_MAX ));
        
        height = static_cast< gint > ( CLAMP( sp_document_height(doc), 
                                              WINDOW_MIN, WINDOW_MAX ));

        gtk_window_set_default_size ( GTK_WINDOW (w), width, height );

        gtk_window_set_position( GTK_WINDOW(w), GTK_WIN_POS_CENTER);

        gtk_window_set_policy ( GTK_WINDOW (w), TRUE, TRUE, FALSE);

        gtk_signal_connect ( GTK_OBJECT (w), "delete_event", 
                             GTK_SIGNAL_FUNC (sp_help_about_delete), NULL);

        v = sp_svg_view_widget_new (doc);

        sp_svg_view_widget_set_resize ( SP_SVG_VIEW_WIDGET (v), FALSE, 
                                        sp_document_width (doc), 
                                        sp_document_height (doc));

        sp_document_unref (doc);
        gtk_widget_show (v);
        gtk_container_add (GTK_CONTAINER (w), v);

    } // close if (!w)

    gtk_window_present (GTK_WINDOW (w));

} // close sp_help_about()


void
sp_help_tutorial (void)
{
	gchar *c = g_strconcat (INKSCAPE_TUTORIALSDIR, _("/tutorial.svg"), 
                                NULL);
	sp_file_open (c, NULL);
	g_free (c);
}

void
sp_help_elementsofdesign (void)
{
        gchar *c = g_strconcat (INKSCAPE_TUTORIALSDIR, 
				_("/elementsofdesign.svg"),
                                NULL);
        sp_file_open (c, NULL);
        g_free (c);
}

void
sp_help_keys (void)
{
	gchar *c = g_strconcat (INKSCAPE_SCREENSDIR, _("/keys.svg"), NULL);
	sp_file_open (c, NULL);
	g_free (c);
}
