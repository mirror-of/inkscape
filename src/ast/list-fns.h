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

#ifndef SEEN_INKSCAPE_AST_LIST_FNS_H
#define SEEN_INKSCAPE_AST_LIST_FNS_H

#include "ast/list.h"

namespace Inkscape {
namespace AST {

template <typename T, typename P>
T const *findLast(T const *list, P predicate)
throw()
{
    for ( ; list ; list = list->prev() ) {
        if (predicate(*list)) {
            return list;
        }
    }
    return NULL;
}

template <typename T, typename P>
typename T::pointer_type findLastValue(T const *list, P predicate)
throw()
{
    T const *found=findLast(list, predicate);
    return found ? &found->value() : NULL;
}

namespace {

struct NotFound {};

template <typename T, typename P>
T const *doRemoveLast(T const *list, P predicate)
throw(NotFound(), std::bad_alloc)
{
    if (!list) {
        // we bottomed out without finding the element we were supposed to
        // remove -- bail here to avoid duplicating the whole list
        throw NotFound();
    } else if (predicate(*list)) {
        return list->prev();
    } else {
        return new T(removeLast(list->prev, predicate), value());
    }
}

template <typename T, typename P>
T const *AST::removeLast(T const *list, P predicate)
throw(std::bad_alloc)
{
    try {
        return doRemoveLast(list, predicate);
    } catch (NotFound &e) {
        return list;
    }
}

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
