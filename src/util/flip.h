/*
 * Inkscape::Util::Flip - reverse the order of a function's arguments
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_FLIP_H
#define SEEN_INKSCAPE_UTIL_FLIP_H

#include "traits/function.h"

namespace Inkscape {

namespace Util {

template <typename F>
class Flip {
public:
    typedef typename Traits::Function<F>::Result Result;
    typedef typename Traits::Function<F>::Arg0 First;
    typedef typename Traits::Function<F>::Arg1 Second;

    Flip(F &f) : _f(f) {}

    Result operator()(Second second, First first) { return _f(first, second); }

private:
    F &_f;
};

template <typename F>
Flip<F> flip(F &f) { return Flip<F>(f); }

}

namespace Traits {

template <typename F>
struct Function<Util::Flip<F> > {
    typedef typename Util::Flip<F>::Result Result;

    typedef typename Util::Flip<F>::Second Arg0;
    typedef typename Util::Flip<F>::First Arg1;
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
