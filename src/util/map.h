/*
 * Inkscape::Util::map - apply a function over a list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_MAP_H
#define SEEN_INKSCAPE_UTIL_MAP_H

#include "traits/function.h"
#include "util/list.h"

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Util {

template <typename F, typename L>
struct MapTraits {
    typedef Traits::List<L> ListT;
    typedef List<typename Traits::Function<F>::Result> ListType;
};

template <typename F, typename L>
typename MapTraits<F, L>::ListType *map(F &f, L list) {
    typedef typename MapTraits<F, L>::ListType ListType;
    typedef typename MapTraits<F, L>::ListT ListT;

    if (ListT::is_null(list)) { return NULL; }

    ListType *head=new ListType(f(ListT::first(list)), NULL);

    ListType *tail=head;
    for ( L iter=ListT::rest(list) ;
          !ListT::is_null(iter) ;
          iter = ListT::rest(iter) )
    {
        tail = tail->setNext(new ListType(f(ListT::first(iter)), NULL));
    }

    return head;
}

template <typename F, typename T>
inline List<T> *map_in_place(F &f, List<T> *list) {
    for ( List<T> *iter=list ; iter != NULL ; iter = iter->next() ) {
        iter->setData(f(iter->data()));
    }
    return list;
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
