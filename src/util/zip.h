/*
 * Inkscape::Util::zip - combine multiple lists into a list of tuples
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_FOLD_H
#define SEEN_INKSCAPE_UTIL_FOLD_H

#include "util/tuple.h"
#include "util/list.h"

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Util {

template <typename L0, typename L1>
struct ZipTraits {
    typedef Traits::List<L0> ListT0;
    typedef Traits::List<L1> ListT1;
    typedef Tuple<ListT0::Data, ListT1::Data> TupleType;
};

template <typename L0, typename L1>
List<typename ZipTraits<L0, L1>::TupleType> *zip(L0 list0, L1 list1) {
    typedef ZipTraits<L0, L1>::ListT0 ListT0;
    typedef ZipTraits<L0, L1>::ListT1 ListT1;
    typedef ZipTraits<L0, L1>::TupleType TupleType;

    if ( ListT0::is_null(list0) || ListT1::is_null(list1) ) {
        return NULL;
    }

    List<TupleType> *head=new List<TupleType>(TupleType(ListT0::first(list0), ListT1::first(list1)), NULL);
    List<TupleType> *tail=head;
    list0 = ListT0::rest(list0); 
    list1 = ListT1::rest(list1); 

    while ( !ListT0::is_null(list0) && !ListT1::is_null(list1) ) {
        tail = tail->setNext(new List<TupleType>(TupleType(ListT0::first(list0), ListT1::first(list1)), NULL));
        list0 = ListT0::rest(list0); 
        list1 = ListT1::rest(list1); 
    }

    return head;
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
