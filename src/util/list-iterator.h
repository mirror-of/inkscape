/*
 * Inkscape::Util::ListIterator - an STL forward iterator for generic lists
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_LIST_ITERATOR_H
#define SEEN_INKSCAPE_UTIL_LIST_ITERATOR_H

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Util {

template <typename T>
class ListIterator {
    typedef Traits::List<T> ListT;

    ListIterator(T const &list) : _list(list) {}
    operator T const &() const { return _list; }

    void operator ++(int) { _list = ListT::rest(_list); }
    void operator ++() { _list = ListT::rest(_list); }
    ListT::Data operator*() const { return ListT::first(_list); }

    bool operator==(ListIterator<T> const &other) {
        return ( *this == other ) ||
               ( ListT::is_null(*this) && ListT::is_null(other) );
    }
    bool operator!=(ListIterator<T> const &other) {
        return !( *this == other );
    }

private:
    T _list;
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
