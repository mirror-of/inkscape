/*
 * SPRepr iterators
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_XML_SP_REPR_ITERATORS_H
#define SEEN_INKSCAPE_XML_SP_REPR_ITERATORS_H

#include "util/forward-pointer-iterator.h"
#include "xml/repr.h"

struct SPReprSiblingIteratorStrategy {
    static SPRepr const *next(SPRepr const *repr) {
        return sp_repr_next(const_cast<SPRepr *>(repr));
    }
};
struct SPReprParentIteratorStrategy {
    static SPRepr const *next(SPRepr const *repr) {
        return sp_repr_parent(const_cast<SPRepr *>(repr));
    }
};

typedef Inkscape::Util::ForwardPointerIterator<SPRepr,
                                               SPReprSiblingIteratorStrategy>
        SPReprSiblingIterator;

typedef Inkscape::Util::ForwardPointerIterator<SPRepr const,
                                               SPReprSiblingIteratorStrategy>
        SPReprConstSiblingIterator;

typedef Inkscape::Util::ForwardPointerIterator<SPRepr,
                                               SPReprParentIteratorStrategy>
        SPReprParentIterator;

typedef Inkscape::Util::ForwardPointerIterator<SPRepr const,
                                               SPReprParentIteratorStrategy>
        SPReprConstParentIterator;

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
