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

#ifndef SEEN_INKSCAPE_AST_LENGTH_LIST_H
#define SEEN_INKSCAPE_AST_LENGTH_LIST_H

#include "ast/list.h"

namespace Inkscape {
namespace AST {

template <typename T>
class LengthList : public List<T> {
public:
    LengthList(LengthList<T> const *prev, reference_type value)
    : List<T>(prev, value), _pos(prev?prev->length():0) {}

    unsigned length() const { return _pos+1; }
    unsigned pos() const { return _pos; }

    LengthList<T> const *prev() const {
        return reinterpret_cast<LengthList<T> const *>(List<T>::prev());
    }

private:
    unsigned _pos;
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
