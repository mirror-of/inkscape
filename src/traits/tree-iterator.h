/*
 * Inkscape::Traits::List - traits class for list-like types
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_TRAITS_TREE_ITERATOR_H
#define SEEN_INKSCAPE_TRAITS_TREE_ITERATOR_H

namespace Inkscape {

namespace Traits {

template <typename T> struct TreeIterator;

class TreeIteratorSample {};
class TreeIteratorSampleNode;

template <>
struct TreeIterator<TreeIteratorSample> {
    typedef TreeIteratorSampleNode Node;

    static bool is_null(TreeIteratorSample);
    static TreeIteratorSample null();

    static TreeIteratorSampleNode node(TreeIteratorSample);

    static TreeIteratorSample first_child(TreeIteratorSample);
    static TreeIteratorSample parent(TreeIteratorSample);
    static TreeIteratorSample next(TreeIteratorSample);
};

}

}

#endif
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
