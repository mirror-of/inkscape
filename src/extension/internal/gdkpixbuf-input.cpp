#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <document.h>
#include <document-private.h>
#include <sp-object.h>
#include <dir-util.h>
#include <prefs-utils.h>
#include <extension/system.h>
#include "gdkpixbuf-input.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

SPDocument *
GdkpixbufInput::open (Inkscape::Extension::Input * mod, const char * uri)
{
    /* Try pixbuf */
    SPDocument * doc = sp_document_new(NULL, TRUE, TRUE);
    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    gchar* localFilename = g_filename_from_utf8 ( uri,
                                                  -1,
                                                  &bytesRead,
                                                  &bytesWritten,
                                                  &error);
    GdkPixbuf *pb = gdk_pixbuf_new_from_file (localFilename, NULL);
    SPRepr *rdoc = sp_document_repr_root (doc);
    const gchar *docbase = sp_repr_attr (rdoc, "sodipodi:docbase");
    const gchar *relname = sp_relative_path_from_path (uri, docbase);

    if (pb) {
        SPRepr *repr = NULL;

        /* We are readable */

        if (prefs_get_int_attribute("options.importbitmapsasimages", "value", 1) == 1) {
            // import as <image>
            repr = sp_repr_new ("image");
            sp_repr_set_attr (repr, "xlink:href", relname);
            sp_repr_set_attr (repr, "sodipodi:absref", uri);
            sp_repr_set_double (repr, "width", gdk_pixbuf_get_width (pb));
            sp_repr_set_double (repr, "height", gdk_pixbuf_get_height (pb));

        } else {
            // import as pattern-filled rect
            SPRepr *pat = sp_repr_new ("pattern");
            sp_repr_set_attr(pat, "inkscape:collect", "always");
            sp_repr_set_attr (pat, "patternUnits", "userSpaceOnUse");
            sp_repr_set_double (pat, "width", gdk_pixbuf_get_width (pb));
            sp_repr_set_double (pat, "height", gdk_pixbuf_get_height (pb));
            sp_repr_append_child (SP_OBJECT_REPR(SP_DOCUMENT_DEFS(doc)), pat);
            const gchar *pat_id = sp_repr_attr(pat, "id");
            SPObject *pat_object = doc->getObjectById(pat_id);

            SPRepr *im = sp_repr_new ("image");
            sp_repr_set_attr (im, "xlink:href", relname);
            sp_repr_set_attr (im, "sodipodi:absref", uri);
            sp_repr_set_double (im, "width", gdk_pixbuf_get_width (pb));
            sp_repr_set_double (im, "height", gdk_pixbuf_get_height (pb));
            sp_repr_add_child (SP_OBJECT_REPR(pat_object), im, NULL);

            repr = sp_repr_new ("rect");
            sp_repr_set_attr (repr, "style", g_strdup_printf("stroke:none;fill:url(#%s)", pat_id));
            sp_repr_set_double (repr, "width", gdk_pixbuf_get_width (pb));
            sp_repr_set_double (repr, "height", gdk_pixbuf_get_height (pb));
        }

        SP_DOCUMENT_ROOT(doc)->appendChildRepr(repr);
        sp_repr_unref (repr);
        sp_document_done (doc);
        gdk_pixbuf_unref (pb);
    } else {
        printf("GdkPixbuf loader failed\n");
    }

    if ( localFilename != NULL ) {
            g_free (localFilename);
    }

    return doc;
}


void
GdkpixbufInput::init (void)
{
    GSList * formatlist, * formatlisthead;

    /* \todo I'm not sure if I need to free this list */
    for (formatlist = formatlisthead = gdk_pixbuf_get_formats ();
         formatlist != NULL;
         formatlist = g_slist_next(formatlist)) {

        GdkPixbufFormat *pixformat = (GdkPixbufFormat *)formatlist->data;

        gchar *name =        gdk_pixbuf_format_get_name(pixformat);
        gchar *description = gdk_pixbuf_format_get_description(pixformat);
        gchar **extensions =  gdk_pixbuf_format_get_extensions(pixformat);
        gchar **mimetypes =   gdk_pixbuf_format_get_mime_types(pixformat);

        for (int i = 0; extensions[i] != NULL; i++) {
        for (int j = 0; mimetypes[j] != NULL; j++) {

            /* thanks but no thanks, we'll handle SVG extensions... */        
            if (strcmp(extensions[i], "svg") == 0) {
                continue;
            }

            gchar *xmlString = g_strdup_printf(
                "<inkscape-extension>\n"
                    "<name>%s GDK pixbuf Input</name>\n"
                    "<id>org.inkscape.input.gdkpixbuf.%s</id>\n"
                    "<input>\n"
                        "<extension>.%s</extension>\n"
                        "<mimetype>%s</mimetype>\n"
                        "<filetypename>%s (*.%s)</filetypename>\n"
                        "<filetypetooltip>%s</filetypetooltip>\n"
                    "</input>\n"
                "</inkscape-extension>",
                name,
                extensions[i],
                extensions[i],
                mimetypes[j],
                name,
                extensions[i],
                description
                );

            Inkscape::Extension::build_from_mem(xmlString, new GdkpixbufInput());
            g_free(xmlString);
        }}

        g_free(name);
        g_free(description);
        g_strfreev(mimetypes);
        g_strfreev(extensions);
     }

     g_slist_free (formatlisthead);

    return;
}

};};}; /* namespace Inkscape, Extension, Implementation */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
