/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "../config.h"
#include <cstdlib>
#include "layer-fns.h"
#include "document.h"
#include "sp-object.h"
#include "xml/repr.h"

namespace Inkscape {

SPObject *create_layer(SPObject *root, SPObject *layer) {
    SPDocument *document=SP_OBJECT_DOCUMENT(root);

    static int layer_suffix=1;
    gchar *id=NULL;
    do {
        g_free(id);
        id = g_strdup_printf("layer%d", layer_suffix++);
    } while (document->getObjectById(id));

    SPRepr *repr=sp_repr_new("g");
    sp_repr_set_attr(repr, "inkscape:groupmode", "layer");
    sp_repr_set_attr(repr, "id", id);
    g_free(id);

    if ( root == layer ) {
        sp_repr_append_child(SP_OBJECT_REPR(root), repr);
    } else {
        sp_repr_add_child(SP_OBJECT_REPR(root), repr, SP_OBJECT_REPR(layer));
    }

    return document->getObjectByRepr(repr);
}

SPObject *next_layer(SPObject *root, SPObject *layer) {
    // TODO
    return layer;
}

SPObject *previous_layer(SPObject *root, SPObject *layer) {
    // TODO
    return layer;
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
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
