/*
 * Inkscape::Util::fold - fold functions over a list
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

#include "traits/function.h"
#include "util/flip.h"
#include "util/reverse.h"

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Util {

template <typename F, typename L>
inline typename Traits::Function<F>::Result foldl(
    F &f,
    typename Traits::Function<F>::Result seed,
    L list
) {
    typedef Traits::List<L> ListT;
    while (!ListT::is_null(list)) {
        seed = f(seed, ListT::first(list));
        list = ListT::rest(list);
    }
    return seed;
}

template <typename F, typename L>
typename Traits::Function<F>::Result foldr(
    F &f,
    typename Traits::Function<F>::Result seed,
    L list
) {
    return foldl(flip(f), seed, reverse(list));
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
