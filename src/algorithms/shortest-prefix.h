/*
 * Inkscape::Algorithms::shortest_prefix
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ALGORITHMS_SHORTEST_PREFIX_H
#define SEEN_INKSCAPE_ALGORITHMS_SHORTEST_PREFIX_H

#include "util/list.h"

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Algorithms {

template <typename List, typename Predicate>
Util::List<typename Traits::List<List>::Data> *
shortest_prefix(Predicate const &p, List xs) {
    typedef Traits::List<List> ListT;
    typedef Util::List<typename ListT::Data> ResultList;

    ResultList *reversed=NULL;

    while (!ListT::is_null(xs)) {
        reversed = cons_mutable(ListT::first(xs), reversed);
        if (p(reversed)) {
            return reversed;
        }
        xs = ListT::rest(xs);
    }

    return NULL;
}

}

}

#endif /* !SEEN_INKSCAPE_ALGORITHMS_SHORTEST_PREFIX_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
