#define SP_SVG_DOC_FACTORY_C

#include <config.h>
#include "embeddable-document.h"
#include "svg-doc-factory.h"

static BonoboObject *
sp_svg_factory (BonoboGenericFactory *factory,
        const char           *component_id,
        gpointer              closure)
{
    if (! strcmp ( component_id, "OAFIID:GNOME_Inkscape_Embeddable" ) ) {
        
        return sp_embeddable_document_new ();

    } else if (! strcmp ( component_id, "OAFIID:GNOME_Inkscape_CanvasItem" ) ) {
    
        g_warning ( "SodiPodi's canvas item code is dysfunctional" );

    } else {
        
        g_warning ( "Inkscape cannot activate unknown id '%s'",
                    component_id );

    return NULL;
}

void
sp_svg_doc_factory_init (void)
{
    static BonoboGenericFactory *doc_factory = NULL;

    doc_factory = bonobo_generic_factory_new_multi (
        "OAFIID:GNOME_Inkscape_ComponentFactory",
        sp_svg_factory, NULL);

    if (doc_factory == NULL) {
        g_error (_("Could not create sodipodi-svg-doc factory"));
    }
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
