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
#include "sp-item-group.h"
#include "xml/repr.h"
#include "algorithms/find-last-if.h"

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

namespace {

bool is_layer(SPObject &object) {
    return SP_IS_GROUP(&object) &&
           SP_GROUP(&object)->layerMode() == SPGroup::LAYER;
}

}

SPObject *next_layer(SPObject *root, SPObject *layer) {
    if ( layer == root ) {
        return NULL;
    }

    // TODO: look to cousins, children, and ancestors also (depth-last order)
    return std::find_if<SPObject::SiblingIterator>(
        SP_OBJECT_NEXT(layer),
        NULL,
        &is_layer
    );
}

SPObject *previous_layer(SPObject *root, SPObject *layer) {
    using Inkscape::Algorithms::find_last_if;

    if ( layer == root ) {
        return NULL;
    }

    // TODO: look to cousins, children, and ancestors also (depth-last order)
    return find_last_if<SPObject::SiblingIterator>(
        SP_OBJECT_PARENT(layer)->firstChild(),
        layer,
        &is_layer
    );
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
