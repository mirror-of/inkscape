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
#include "algorithms/longest-suffix.h"
#include "algorithms/shortest-prefix.h"
#include "sp-object-tree-iterator.h"
#include "util/sibling-axis.h"

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

struct starts_with_any_layer {
    starts_with_any_layer() {}

    template <typename List>
    bool operator()(List os) const {
        SPObject *o=Inkscape::Traits::List<List>::first(os);
        return ( SP_IS_GROUP(o) && SP_GROUP(o)->layerMode() == SPGroup::LAYER );
    }
};

struct starts_with_object {
    SPObject *object;

    starts_with_object(SPObject *o) : object(o) {}

    template <typename List>
    bool operator()(List os) const {
        SPObject *o=Inkscape::Traits::List<List>::first(os);
        return ( o == object );
    }
};

}

SPObject *next_layer(SPObject *root, SPObject *layer) {
    using Inkscape::Util::SiblingAxis;
    using Inkscape::Algorithms::longest_suffix;

    if ( layer == root ) {
        return NULL;
    }

    // TODO: look to cousins, children, and ancestors also (depth-last order)
    return longest_suffix<SiblingAxis<SPObject *> >(starts_with_any_layer(), SP_OBJECT_NEXT(layer));
}

SPObject *previous_layer(SPObject *root, SPObject *layer) {
    using Inkscape::Util::SiblingAxis;
    using Inkscape::Util::List;
    using Inkscape::Algorithms::longest_suffix;
    using Inkscape::Algorithms::shortest_prefix;

    if ( layer == root ) {
        return NULL;
    }

    // TODO: look to cousins, children, and ancestors also (depth-last order)
    List<SPObject *> *found=longest_suffix(
        starts_with_any_layer(),
        shortest_prefix<SiblingAxis<SPObject *> >(
            starts_with_object(layer), SP_OBJECT_PARENT(layer)->firstChild()
        )->next()
    );
    return ( found ? found->data() : NULL );
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
