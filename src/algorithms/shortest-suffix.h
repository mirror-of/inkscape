/*
 * Inkscape::Algorithms::shortest_suffix
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ALGORITHMS_SHORTEST_SUFFIX_H
#define SEEN_INKSCAPE_ALGORITHMS_SHORTEST_SUFFIX_H

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Algorithms {

template <typename List, typename Predicate>
List shortest_suffix(Predicate const &p, List xs) {
    typedef Traits::List<List> ListT;
    List candidate=ListT::null();

    while (!ListT::is_null(xs)) {
        if (p(xs)) {
            candidate = xs;
        }
        xs = ListT::rest(xs);
    }

    return candidate;
}

}

}

#endif /* !SEEN_INKSCAPE_ALGORITHMS_SHORTEST_COMMON_SUFFIX_H */

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
