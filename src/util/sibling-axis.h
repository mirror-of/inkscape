/*
 * Inkscape::Util::SiblingAxis - adaptor to treat tree iterator as ancestor list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_SIBLING_AXIS_H
#define SEEN_INKSCAPE_UTIL_SIBLING_AXIS_H

namespace Inkscape {

namespace Traits { template <typename T> struct TreeIterator; }

namespace Util {

template <typename T>
class SiblingAxis {
public:
    SiblingAxis(T const &iter) : _iter(iter) {
        // assert that T implements TreeIterator
        (void)Traits::TreeIterator<T>::null();
    }
    static SiblingAxis<T> const &coerce(T const &iter) {
        return reinterpret_cast<SiblingAxis<T> const &>(iter);
    }
    operator T const &() const { return _iter; }

    bool operator==(SiblingAxis<T> const &i) { return _iter == i._iter; }
    bool operator!=(SiblingAxis<T> const &i) { return _iter != i._iter; }

private:
    T _iter;
};

}

namespace Traits {

template <typename T> struct List;

template <typename T>
struct List<Inkscape::Util::SiblingAxis<T> > {
    typedef Inkscape::Util::SiblingAxis<T> ListType;
    typedef typename TreeIterator<T>::Node Data;

    static bool is_null(ListType list) {
        return TreeIterator<T>::is_null(list);
    }
    static ListType null() {
        return ListType::coerce(TreeIterator<T>::null());
    }

    static Data first(ListType list) {
        return ListType::coerce(TreeIterator<T>::node(list));
    }
    static ListType rest(ListType list) {
        return ListType::coerce(TreeIterator<T>::next(list));
    }
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
