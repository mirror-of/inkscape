/*
 * Inkscape::Util::copy_list - select a subset of the items in a list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_COPY_LIST_H
#define SEEN_INKSCAPE_UTIL_COPY_LIST_H

#include "util/list.h"

namespace Inkscape {

namespace Util {

template <typename T, typename InputIterator>
inline MutableList<T>
copy_list(InputIterator start, InputIterator end) {
    MutableList<T> head;
    MutableList<T> tail;
    if ( start != end ) {
        head = tail = MutableList<T>(*start);
        ++start;
    }
    while ( start != end ) {
        tail.setNext(MutableList<T>(*start));
        ++tail;
        ++start;
    }
    return head;
}

template <typename T1, typename T2>
inline MutableList<T1>
copy_list(List<T2> const &list) {
    return copy_list(list, List<T2>());
}

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
