/*
 * Inkscape::AST - Abstract Syntax Tree in a Database
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_AST_LIST_H
#define SEEN_INKSCAPE_AST_LIST_H

#include "ast/gc.h"
#include "ast/functional-traits.h"

namespace Inkscape {
namespace AST {

template <typename T>
class List : public SimpleGCObject<> {
public:
    typedef FunctionalTraits<T> value_traits;
    typedef typename value_traits::value_type value_type;
    typedef typename value_traits::reference_type reference_type;

    List(List<T> const *prev, reference_type value)
    : _prev(prev), _value(value) {}

    List<T> const *prev() const { return _prev; }
    reference_type value() const { return _value; }

private:
    List<T> const *_prev;
    value_type _value;
};

};
};

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
