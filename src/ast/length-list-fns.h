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

#ifndef SEEN_INKSCAPE_AST_LENGTH_LIST_FNS_H
#define SEEN_INKSCAPE_AST_LENGTH_LIST_FNS_H

#include "ast/length-list.h"

namespace Inkscape {
namespace AST {

template <typename T>
typename LengthList<T>::pointer_type value_at(LengthList<T> const *list, unsigned pos)
{
    while ( list && list->pos() > pos ) {
        list = list->prev();
    }
    if ( list && list->pos() == pos ) {
        return &list->value();
    } else {
        return NULL;
    }
}

template <typename T>
LengthList<T> const *insert(LengthList<T> const *list, unsigned pos,
                            typename LengthList<T>::reference_type value)
throw(std::bad_alloc)
{
    if ( list && list->pos() >= pos ) {
        return *(new LengthList<T>(insert(list->prev(), pos, value), list->value()));
    } else {
        return *(new LengthList<T>(list, value));
    }
}

template <typename T>
LengthList<T> const *replace(LengthList<T> const *list, unsigned pos,
                             typename LengthList<T>::reference_type value)
throw(std::bad_alloc)
{
    if (list) {
        if ( list->pos() > pos ) {
            return *(new LengthList<T>(replace(list->prev(), pos, value), list->value()));
        } else if ( list->pos == pos ) {
            return *(new LengthList<T>(list->prev(), value));
        } else {
            return *(new LengthList<T>(list, value));
        }
    } else {
        return *(new LengthList<T>(NULL, value));
    }
}

template <typename T>
LengthList<T> const *remove(LengthList<T> const *list, unsigned pos)
throw(std::bad_alloc)
{
    if (list) {
        if ( list->pos() > pos ) {
            return *(new LengthList<T>(remove(list->prev(), pos), list->value()));
        } else if ( list->pos() == pos ) {
            return list->prev();
        } else {
            return list;
        }
    } else {
        return NULL;
    }
}

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
