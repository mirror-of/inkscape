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
#include <new>
#include <stdexcept>
#include <cstring>
#include <gc/gc_cpp.h>
#include "ast/c-array.h"

namespace Inkscape {
namespace AST {

template <typename T>
struct Array : public gc {
public:
    Array(CArray<T> const &array, size_t length)
    : _array(length ? array : CArray<T>::create_unsafe(NULL)),
      _length(length)
    throw() {}

    Array(Array<T> const &array, size_t length)
    : _array(array), _length(length)
    throw(std::range_error)
    {
        if ( length > array.length() ) {
            throw std::range_error("array cannot be larger than original");
        }
    }

    size_t length() const throw() { return _length; }

    T const &operator[](int i) const throw() { return _array[i]; }

    operator CArray<T> const &() const throw() { return _array; }
    operator T const *() const throw() { return _array.toPointer(); }

    Array<T> const &truncate(size_t length) const
    throw(std::bad_alloc, std::range_error)
    {
        return *(new Array<T>(_array, length));
    }

private:
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
