#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <document.h>
#include <dir-util.h>
#include <extension/system.h>
#include "gdkpixbuf-input.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

SPDocument *
GdkpixbufInput::open (Inkscape::Extension::Input * mod, const char * uri)
{
    /* Try pixbuf */
    GdkPixbuf *pb;
    SPDocument * doc = sp_document_new(NULL, TRUE, TRUE);
    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    const gchar * docbase, * relname;
    SPRepr * rdoc;
    gchar* localFilename = g_filename_from_utf8 ( uri,
                                                  -1,
                                                  &bytesRead,
                                                  &bytesWritten,
                                                  &error);
    pb = gdk_pixbuf_new_from_file (localFilename, NULL);
    rdoc = sp_document_repr_root (doc);
    docbase = sp_repr_attr (rdoc, "sodipodi:docbase");
    relname = sp_relative_path_from_path (uri, docbase);

    if (pb) {
        SPRepr * repr;

        /* We are readable */
        repr = sp_repr_new ("image");
        sp_repr_set_attr (repr, "xlink:href", relname);
        sp_repr_set_attr (repr, "sodipodi:absref", uri);
        sp_repr_set_double (repr, "width", gdk_pixbuf_get_width (pb));
        sp_repr_set_double (repr, "height", gdk_pixbuf_get_height (pb));
        sp_document_add_repr (doc, repr);
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
    GSList * formatlist;

    /* \todo I'm not sure if I need to free this list */
    for (formatlist = gdk_pixbuf_get_formats ();
         formatlist != NULL;
         formatlist = g_slist_next(formatlist)) {
        GdkPixbufFormat * pixformat;
        gchar * xmlString;

        pixformat = (GdkPixbufFormat *)formatlist->data;

        /* thanks but no thanks, we'll handle SVG extensions... */        
        if (strcmp(gdk_pixbuf_format_get_extensions(pixformat)[0],"svg")==0) {
            continue;
        }

        xmlString = g_strdup_printf(
            "<spmodule>\n"
                "<name>%s GDK pixbuf Input</name>\n"
                "<id>modules.input.gdkpixbuf.%s</id>\n"
                "<input>\n"
                    "<extension>.%s</extension>\n"
                    "<mimetype>%s</mimetype>\n"
                    "<filetypename>%s (*.%s)</filetypename>\n"
                    "<filetypetooltip>%s</filetypetooltip>\n"
                "</input>\n"
            "</spmodule>",
            gdk_pixbuf_format_get_name(pixformat),
            gdk_pixbuf_format_get_extensions(pixformat)[0],
            gdk_pixbuf_format_get_extensions(pixformat)[0],
            gdk_pixbuf_format_get_mime_types(pixformat)[0],
            gdk_pixbuf_format_get_name(pixformat),
            gdk_pixbuf_format_get_extensions(pixformat)[0],
            gdk_pixbuf_format_get_description(pixformat)
            );

        Inkscape::Extension::build_from_mem(xmlString, new GdkpixbufInput());
        g_free(xmlString);
     }


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
