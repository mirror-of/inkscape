#define __SP_HELP_C__

/*
 * Help/About window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "path-prefix.h"

#include <gtk/gtk.h>
#include "inkscape.h"
#include "document.h"
#include "sp-text.h"
#include "text-editing.h"
#include "svg-view-widget.h"
#include "help.h"
#include "file.h"
#include <glibmm/i18n.h>
#include "libnr/nr-macros.h"
#include "inkscape_version.h"

#include "ui/dialog/aboutbox.h"

static Inkscape::UI::Dialog::AboutBox * aboutbox = NULL;

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

    if (!aboutbox) {
        /* TRANSLATORS: This is the filename of the `About Inkscape' picture in
           the `screens' directory.  Thus the translation of "about.svg" should be
           the filename of its translated version, e.g. about.zh.svg for Chinese.

           N.B. about.svg changes once per release.  (We should probably rename
           the original to about-0.40.svg etc. as soon as we have a translation.
           If we do so, then add an item to release-checklist saying that the
           string here should be changed.) */
	char *about = g_build_filename(INKSCAPE_SCREENSDIR, _("about.svg"), NULL);
        doc = sp_document_new (about, TRUE);
        g_free(about);
        g_return_if_fail (doc != NULL);
        version = doc->getObjectById("version");
        
        if (version && SP_IS_TEXT (version)) {
            sp_te_set_repr_text_multiline ( SP_TEXT (version), 
                                              INKSCAPE_VERSION);
        }

        sp_document_ensure_up_to_date (doc);


        width  = static_cast< gint > ( CLAMP( sp_document_width(doc), 
                                              WINDOW_MIN, WINDOW_MAX ));
        
        height = static_cast< gint > ( CLAMP( sp_document_height(doc), 
                                              WINDOW_MIN, WINDOW_MAX ));

        /*
        gtk_window_set_position( GTK_WINDOW(w), GTK_WIN_POS_CENTER);

        gtk_window_set_policy ( GTK_WINDOW (w), TRUE, TRUE, FALSE);
        */

        v = sp_svg_view_widget_new (doc);

        sp_svg_view_widget_set_resize ( SP_SVG_VIEW_WIDGET (v), FALSE, 
                                        sp_document_width (doc), 
                                        sp_document_height (doc));

        sp_document_unref (doc);
        gtk_widget_show (v);

        Gtk::Widget * vmm = Glib::wrap(v);
        aboutbox = new Inkscape::UI::Dialog::AboutBox(*vmm,width,height);

    } // close if (!aboutbox)

    aboutbox->show();

} // close sp_help_about()

void
sp_help_open_tutorial(GtkMenuItem *, gpointer data)
{
    gchar const *name = static_cast<gchar const *>(data);
    gchar *c = g_build_filename(INKSCAPE_TUTORIALSDIR, name, NULL);
    sp_file_open(c, NULL, false, false);
    g_free(c);
}

void
sp_help_open_screen(gchar const *name)
{
    gchar *c = g_build_filename(INKSCAPE_SCREENSDIR, name, NULL);
    sp_file_open(c, NULL, false, false);
    g_free(c);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
