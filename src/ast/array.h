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

#ifndef SEEN_INKSCAPE_AST_ARRAY_H
#define SEEN_INKSCAPE_AST_ARRAY_H

#include <sys/types.h>
#include "ast/c-array.h"

namespace Inkscape {
namespace AST {

template <typename T>
struct Array {
public:
    static Array<T> const &create(CArray<T> const &array, size_t length)
    throw(std::)
    {
    }

    size_t length() const throw() { return _length; }

    T const &operator[](int i) const throw() { return _array[i]; }

    operator CArray const &() const throw() { return _array; }
    operator T const *() const throw() { return _array; }

private:
    String(CArray<T> const &array, size_t length)
    : _array(array), _length(length) {}

    CArray<T> const &_array;
    size_t _length;
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
