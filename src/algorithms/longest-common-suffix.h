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

#ifndef SEEN_INKSCAPE_ALGORITMS_LONGEST_COMMON_SUFFIX_H
#define SEEN_INKSCAPE_ALGORITMS_LONGEST_COMMON_SUFFIX_H

#include <list>

namespace Inkscape {

namespace Traits { template <typename T> struct List; }

namespace Algorithms {

template <typename List>
List longest_common_suffix(List a, List b) {
    typedef Traits::List<List> ListT;

    if ( ListT::is_null(a) || ListT::is_null(b) ) {
        return ListT::null();
    }

    /* build parallel lists of suffixes, ordered by increasing length */

    List lists[2] = { a, b };
    std::list<List> suffixes[2];

    for ( int i=0 ; i < 2 ; i++ ) {
        for ( List iter=lists[i] ;
              !ListT::is_null(iter) ;
              iter=ListT::tail(iter) )
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

    List longest_common=ListT::null();

    typename std::list<List>::iterator iters[2] = {
        suffixes[0].begin(),
        suffixes[1].begin()
    };

    while ( iters[0] != suffixes[0].end() &&
            iters[1] != suffixes[1].end() &&
            // TODO: == is ambiguous here also; in scheme terms I mean roughly:
            // (equal? (car (vector-ref iters 0)) (car (vector-ref iters 1)))
            // maybe we should make the equality predicate the third parameter
            ListT::head(*iters[0]) == ListT::head(*iters[1]) )
    {
        longest_common = *iters[0];
        ++iters[0];
        ++iters[1];
    }

    return longest_common;
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
