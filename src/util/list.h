/*
 * Inkscape::Util::List - managed linked list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_LIST_H
#define SEEN_INKSCAPE_UTIL_LIST_H

#include "gc-object.h"
#include "traits/reference.h"

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Util {

template <typename T>
class List : public GC::Object<> {
public:
    typedef T Data;

    List() {}
    List(typename Traits::Reference<T>::RValue data, List<T> *next)
    : _data(data), _next(next) {}

    typename Traits::Reference<T>::LValue data() { return _data; }
    typename Traits::Reference<T>::RValue data() const { return _data; }
    typename Traits::Reference<T>::RValue setData(typename Traits::Reference<T>::RValue data) {
        return _data = data;
    }

    List<T> *&next() { return _next; }
    List<T> const *next() const { return _next; }
    List<T> *setNext(List<T> *next) {
        return _next = next;
    }

private:
    T _data;
    List<T> *_next;
};

template <typename T>
inline List<T> *empty() { return NULL; }

template <typename T>
inline List<T> const *cons(typename Traits::Reference<T>::RValue data,
                           List<T> const *next)
{
    return new List<T>(data, const_cast<List<T> *>(next));
}

template <typename T>
inline List<T> *cons_mutable(typename Traits::Reference<T>::RValue data,
                             List<T> *next)
{
    return new List<T>(data, next);
}

}

namespace Traits {

template <typename T>
struct List<Util::List<T> *> {
    typedef Util::List<T> *ListType;
    typedef T Data;

    static bool is_null(ListType l) { return ( l == NULL ); }
    static ListType null() { return NULL; }

    static typename Reference<T>::RValue first(ListType l) { return l->data(); }
    static ListType rest(ListType l) { return l->next(); }
};

template <typename T>
struct List<Util::List<T> const *> {
    typedef Util::List<T> const *ListType;
    typedef T const Data;

    static bool is_null(ListType l) { return ( l == NULL ) ; }
    static ListType null() { return NULL; }

    static typename Reference<T>::RValue first(ListType l) { return l->data(); }
    static ListType rest(ListType l) { return l->next(); }
};

template <typename T>
struct List<Util::List<T &> const *> {
    typedef Util::List<T &> const *ListType;
    typedef T &Data;

    static bool is_null(ListType l) { return ( l == NULL ) ; }
    static ListType null() { return NULL; }

    static T &first(ListType l) { return l->data(); }
    static ListType rest(ListType l) { return l->next(); }
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
