/*
 * Inkscape::Algorithms::longest_common_suffix 
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ALGORITHMS_LONGEST_COMMON_SUFFIX_H
#define SEEN_INKSCAPE_ALGORITHMS_LONGEST_COMMON_SUFFIX_H

#include <list>

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Algorithms {

/**
 * Note that this implementation and the rest of this comment assume
 * that `List==List' runs in O(1) time.
 *
 * Time costs:
 *
 * The case of sharing a common successor is handled in O(1) time.
 *
 * If \a a is the longest common suffix, then runs in O(len(rest of b)) time.
 *
 * Otherwise, runs in O(len(a) + len(b)) time.
 */
template <typename List>
List longest_common_suffix(List a, List b) {
    typedef Traits::List<List> ListT;

    if ( ListT::is_null(a) || ListT::is_null(b) ) {
        return ListT::null();
    }

    /* Handle in O(1) time the common case of equal tails. */
    {
        List const tail = ListT::rest(a);
        if ( tail == ListT::rest(b) ) {
            return tail;
        }
    }

    /* Build parallel lists of suffixes, ordered by increasing length. */

    List lists[2] = { a, b };
    std::list<List> suffixes[2];

    for ( int i=0 ; i < 2 ; i++ ) {
        for ( List iter = lists[i] ;
              !ListT::is_null(iter) ;
              iter = ListT::rest(iter) )
        {
            // TODO: == is ambiguous; in scheme terms, I mean roughly:
            // (eqv? iter (vector-ref lists (- 1 i)))
            if ( iter == lists[1-i] ) {
                return lists[1-i]; // lists[i] contains lists[1-i];
            }

            suffixes[i].push_front(iter);
        }
    }

    /* iterate through the lists of suffix lists in parallel, stopping at
       the first pair of suffix lists that have different heads */

    List longest_common = ListT::null();

    typename std::list<List>::iterator iters[2] = {
        suffixes[0].begin(),
        suffixes[1].begin()
    };

    while ( iters[0] != suffixes[0].end() &&
            iters[1] != suffixes[1].end() &&
            // TODO: == is ambiguous here also; in scheme terms I mean roughly:
            // (equal? (car (vector-ref iters 0)) (car (vector-ref iters 1)))
            // maybe we should make the equality predicate the third parameter
            ListT::first(*iters[0]) == ListT::first(*iters[1]) )
    {
        longest_common = *iters[0];
        ++iters[0];
        ++iters[1];
    }

    return longest_common;
}

}

}

#endif /* !SEEN_INKSCAPE_ALGORITHMS_LONGEST_COMMON_SUFFIX_H */

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
