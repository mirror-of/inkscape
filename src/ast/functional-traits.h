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

#ifndef SEEN_INKSCAPE_AST_FUNCTIONAL_TRAITS_H
#define SEEN_INKSCAPE_AST_FUNCTIONAL_TRAITS_H

namespace Inkscape {
namespace AST {

template <typename T>
struct FunctionalTraits {
    typedef T const value_type;
    typedef T const *pointer_type;
    typedef T const &reference_type;
};

template <typename T>
struct FunctionalTraits<T &> {
    typedef T const &value_type;
    typedef T const *pointer_type;
    typedef T const &reference_type;
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
