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

#ifndef SEEN_INKSCAPE_AST_COPY_H
#define SEEN_INKSCAPE_AST_COPY_H

// TODO: autoconf this
#define HAS_TYPE_TRAITS_H 1

#include <sys/types.h>
#include <cstring>
#ifdef HAS_TYPE_TRAITS_H
#include <type_traits.h>
#endif

namespace Inkscape {
namespace AST {


template <typename T>
T *genericInitVector(T *to, T const *from, size_t count) {
    for ( size_t i=0 ; i < count ; i++ ) {
        new (reinterpret_cast<void *>(to+i)) T(from[i]);
    }
    return to;
}

template <typename T>
T *genericCopyVector(T *to, T const *from, size_t count) {
    for ( size_t i=0 ; i < count ; i++ ) {
        to[i] = from[i];
    }
    return to;
}


template <typename T>
T *initVector(T *to, T const *from, size_t count) {
#ifdef HAS_TYPE_TRAITS_H
    return initVector(to, from, count, typename __type_traits<T>::has_trivial_copy_constructor());
#else
    return genericInitVector(to, from, count);
#endif
}

template <typename T>
T *copyVector(T *to, T const *from, size_t count) {
#ifdef HAS_TYPE_TRAITS_H
    return copyVector(to, from, count, typename __type_traits<T>::has_trivial_assignment_operator());
#else
    return genericCopyVector(to, from, count);
#endif
}


#ifdef HAS_TYPE_TRAITS_H

template <typename T>
T *initVector(T *to, T const *from, size_t count, __true_type) {
    return std::memcpy(to, from, count*sizeof(T));
}

template <typename T>
T *initVector(T *to, T const *from, size_t count, __false_type) {
    return genericInitVector(to, from, count);
}

template <typename T>
T *copyVector(T *to, T const *from, size_t count, __true_type) {
    return std::memcpy(to, from, count*sizeof(T));
}

template <typename T>
T *copyVector(T *to, T const *from, size_t count, __false_type) {
    return genericCopyVector(to, from, count);
}

#endif

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
