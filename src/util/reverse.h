/*
 * Inkscape::Util::reverse - reverse a list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_REVERSE_H
#define SEEN_INKSCAPE_UTIL_REVERSE_H

#include "traits/function.h"
#include "traits/reference.h"

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Util {

template <typename L>
inline List<typename Traits::List<L>::Data> *reverse(L list) {
    typedef Traits::List<L> ListT;
    typedef typename List<ListT::Data> ListType;

    ListType *head=NULL;
    while (!ListT::is_null(list)) {
        head = new ListType(ListT::first(list), head);
        list = ListT::rest(list);
    }

    return head;
}

template <typename T>
inline List<T> *reverse_in_place(List<T> *list) {
    List<T> *reversed=NULL;
    while (list) {
        List<T> *temp=list;
        list = list->next();
        temp->setNext(reversed);
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
