/*
 * Inkscape::Util::reverse_list - generate a reversed list from iterator range
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_REVERSE_LIST_H
#define SEEN_INKSCAPE_UTIL_REVERSE_LIST_H

#include "util/list.h"

namespace Inkscape {

namespace Util {

template <typename T, typename InputIterator>
inline MutableList<T> reverse_list(InputIterator start, InputIterator end) {
    MutableList<T> head;
    while ( start != end ) {
        head = cons(*start, head);
        ++start;
    }
    return head;
}

template <typename T1, typename T2>
inline MutableList<T1> reverse_list(List<T2> const &list) {
    return reverse_list(list, List<T2>());
}

template <typename T>
inline MutableList<T>
reverse_list_in_place(MutableList<T> start,
                      MutableList<T> end=MutableList<T>())
{
    MutableList<T> reversed(end); 
    while ( start != end ) {
        MutableList<T> temp(start);
        ++start;
        temp.setNext(reversed);
        reversed = temp;
    }
    return reversed;
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
