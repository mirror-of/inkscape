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
unsigned LengthList<T>::length(LengthList<T> const *list)
{
    return list ? list->length() : 0;
}

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

template <typename T>
LengthList<T> const *reorder(LengthList<T> const *list, unsigned old_pos, unsigned new_pos)
throw(std::bad_alloc)
{
    // TODO this is obviously very inefficient
    LengthList<T>::pointer_type value=value_at(list, old_pos);
    if (value) {
        return insert(remove(list, old_pos), ( new_pos > old_pos ) ? new_pos - 1 : new_pos, value);
    } else {
        return list;
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
