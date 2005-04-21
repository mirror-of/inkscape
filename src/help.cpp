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
#include "svg-view.h"
#include "help.h"
#include "file.h"
#include <glibmm/i18n.h>
#include "libnr/nr-macros.h"
#include "inkscape_version.h"

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
sp_help_open_tutorial (GtkMenuItem *menuitem, gpointer data)
{
    gchar const *name = static_cast<gchar const *>(data);
    gchar *c = g_build_filename(INKSCAPE_TUTORIALSDIR, name, NULL);
    sp_file_open(c, NULL, false);
    g_free(c);
}

void
sp_help_open_screen(gchar const *name)
{
    gchar *c = g_build_filename(INKSCAPE_SCREENSDIR, name, NULL);
    sp_file_open(c, NULL, false);
    g_free(c);
}




static char* panicBuf = ""
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<!-- Created with Inkscape (http://www.inkscape.org/) -->\n"
    "<svg\n"
    "   xmlns:xml=\"http://www.w3.org/XML/1998/namespace\"\n"
    "   xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
    "   xmlns:cc=\"http://web.resource.org/cc/\"\n"
    "   xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
    "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
    "   xmlns=\"http://www.w3.org/2000/svg\"\n"
    "   xmlns:sodipodi=\"http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
    "   xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
    "   width=\"800.0px\"\n"
    "   height=\"600.0px\"\n"
    "   id=\"svg2\"\n"
    "   sodipodi:version=\"0.32\"\n"
    "   inkscape:version=\"0.41+cvs\"\n"
    "   sodipodi:docbase=\"/home/joncruz/devel/inkscape\"\n"
    "   sodipodi:docname=\"bsod.svg\">\n"
    "  <defs\n"
    "     id=\"defs3\" />\n"
    "  <sodipodi:namedview\n"
    "     id=\"base\"\n"
    "     pagecolor=\"#ffffff\"\n"
    "     bordercolor=\"#666666\"\n"
    "     borderopacity=\"1.0\"\n"
    "     inkscape:pageopacity=\"0.0\"\n"
    "     inkscape:pageshadow=\"2\"\n"
    "     inkscape:zoom=\"0.57500000\"\n"
    "     inkscape:cx=\"400.0\"\n"
    "     inkscape:cy=\"300.0\"\n"
    "     inkscape:document-units=\"px\"\n"
    "     inkscape:current-layer=\"layer1\"\n"
    "     inkscape:window-width=\"712\"\n"
    "     inkscape:window-height=\"515\"\n"
    "     inkscape:window-x=\"10\"\n"
    "     inkscape:window-y=\"44\" />\n"
    "  <metadata\n"
    "     id=\"metadata4\">\n"
    "    <rdf:RDF\n"
    "       id=\"RDF5\">\n"
    "      <cc:Work\n"
    "         rdf:about=\"\"\n"
    "         id=\"Work6\">\n"
    "        <dc:format\n"
    "           id=\"format7\">image/svg+xml</dc:format>\n"
    "        <dc:type\n"
    "           id=\"type9\"\n"
    "           rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\n"
    "      </cc:Work>\n"
    "    </rdf:RDF>\n"
    "  </metadata>\n"
    "  <g\n"
    "     inkscape:label=\"Layer 1\"\n"
    "     inkscape:groupmode=\"layer\"\n"
    "     id=\"layer1\">\n"
    "    <rect\n"
    "       style=\"stroke-opacity:1.0;stroke-dashoffset:0.0;stroke-dasharray:10.0 10.0;stroke-miterlimit:4.0;stroke-width:10.0;stroke:none;fill-opacity:1.0;fill:#0000ff\"\n"
    "       id=\"rect1274\"\n"
    "       x=\"0\"\n"
    "       y=\"0\"\n"
    "       height=\"600\"\n"
    "       width=\"800\" />\n"
    "    <g\n"
    "       transform=\"translate(1.144409e-5,10.43478)\"\n"
    "       id=\"g1291\">\n"
    "      <rect\n"
    "         y=\"173.0\"\n"
    "         x=\"348.0\"\n"
    "         height=\"22.574173\"\n"
    "         width=\"103.69102\"\n"
    "         id=\"rect2797\"\n"
    "         style=\"font-size:18.0;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:100.0%;writing-mode:lr-tb;text-anchor:start;fill:#ffffff;fill-opacity:1.0;stroke:none;stroke-width:10.0;stroke-miterlimit:4.0;stroke-dasharray:10.0 10.0 ;stroke-dashoffset:0.0;stroke-opacity:1.0;font-family:Bitstream Vera Sans\" />\n"
    "      <text\n"
    "         sodipodi:linespacing=\"100.0%\"\n"
    "         id=\"text2035\"\n"
    "         y=\"189.0\"\n"
    "         x=\"361.0\"\n"
    "         style=\"font-size:18.0;font-style:normal;font-variant:normal;font-weight:bold;font-stretch:normal;line-height:100.0%;writing-mode:lr-tb;text-anchor:start;fill:#0000ff;fill-opacity:1.0;stroke:none;stroke-width:1.0px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0;font-family:Courier\"\n"
    "         xml:space=\"preserve\"><tspan\n"
    "           y=\"190.0\"\n"
    "           x=\"361.0\"\n"
    "           id=\"tspan2811\"\n"
    "           sodipodi:role=\"line\">Windows</tspan></text>\n"
    "    </g>\n"
    "    <text\n"
    "       xml:space=\"preserve\"\n"
    "       style=\"font-size:18.0;font-style:normal;font-variant:normal;font-weight:bold;font-stretch:normal;line-height:100.0%;writing-mode:lr-tb;text-anchor:start;fill:#ffffff;fill-opacity:1.0;stroke:none;stroke-width:1.0px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0;font-family:Courier\"\n"
    "       x=\"41.0\"\n"
    "       y=\"249.0\"\n"
    "       id=\"text2818\"\n"
    "       sodipodi:linespacing=\"100.0%\"><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan3580\"\n"
    "         x=\"41.0\"\n"
    "         y=\"249.0\">An exeption  06 has occured at 0028:C11B3ADC in VxD DiskTSD(03) +</tspan><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan3582\"\n"
    "         x=\"41.0\"\n"
    "         y=\"267.0\">00001660.  This was called from 0028:C11B40C8 in VxD voltrack(04) +</tspan><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan3584\"\n"
    "         x=\"41.0\"\n"
    "         y=\"285.0\">00000000.  It may be possible to continue normally.</tspan><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan3586\"\n"
    "         x=\"41.0\"\n"
    "         y=\"303.0\" /><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan3588\"\n"
    "         x=\"41.0\"\n"
    "         y=\"321.0\">*  Press any key to attempt to continue.</tspan><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan3590\"\n"
    "         x=\"41.0\"\n"
    "         y=\"339.0\">*  Press CTRL+ALT+RESET to restart your computer.  You will</tspan><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         xml:space=\"preserve\"\n"
    "         id=\"tspan3592\"\n"
    "         x=\"41.0\"\n"
    "         y=\"357.0\">   lose any unsaved information in all applications.</tspan></text>\n"
    "    <text\n"
    "       xml:space=\"preserve\"\n"
    "       style=\"font-size:18.0;font-style:normal;font-variant:normal;font-weight:bold;font-stretch:normal;line-height:100.0%;writing-mode:lr-tb;text-anchor:start;fill:#ffffff;fill-opacity:1.0;stroke:none;stroke-width:1.0px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0;font-family:Courier\"\n"
    "       x=\"262.0\"\n"
    "       y=\"397.0\"\n"
    "       id=\"text4355\"\n"
    "       sodipodi:linespacing=\"100.0%\"><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan4371\"\n"
    "         x=\"262.0\"\n"
    "         y=\"397.0\">Press any key to continue</tspan></text>\n"
    "  </g>\n"
    "</svg>\n"
    "";

static GtkWidget *panic = NULL;

static gint
sp_help_panic_delete( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    panic = NULL;
    return FALSE;
}


void sp_help_panic(void)
{
    SPDocument *doc;
    GtkWidget *v;
    gint width, height;

    if (!panic) {
        doc = sp_document_new_from_mem( panicBuf, strlen(panicBuf), TRUE );
        g_return_if_fail (doc != NULL);

        sp_document_ensure_up_to_date (doc);


        panic = gtk_window_new( GTK_WINDOW_TOPLEVEL );

        gtk_window_set_title( GTK_WINDOW (panic), _("BSOD") );

        width  = static_cast<gint>( CLAMP( sp_document_width(doc),
                                              WINDOW_MIN, WINDOW_MAX ) );

        height = static_cast<gint>( CLAMP( sp_document_height(doc),
                                              WINDOW_MIN, WINDOW_MAX ) );

        gtk_window_set_default_size ( GTK_WINDOW(panic), width, height );

        gtk_window_set_position( GTK_WINDOW(panic), GTK_WIN_POS_CENTER);

        gtk_window_set_policy( GTK_WINDOW(panic), TRUE, TRUE, FALSE);

        gtk_signal_connect( GTK_OBJECT(panic), "delete_event",
                            GTK_SIGNAL_FUNC(sp_help_panic_delete), NULL);

        v = sp_svg_view_widget_new( doc );

        sp_svg_view_widget_set_resize( SP_SVG_VIEW_WIDGET( v ), FALSE,
                                       sp_document_width( doc ),
                                       sp_document_height( doc ) );

        sp_document_unref( doc );
        gtk_widget_show( v );
        gtk_container_add( GTK_CONTAINER( panic ), v );

    } // close if (!panic)

    gtk_window_present( GTK_WINDOW( panic ) );

} // close sp_help_panic()



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
