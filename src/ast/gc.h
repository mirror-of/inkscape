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

#ifndef SEEN_INKSCAPE_AST_GC_H
#define SEEN_INKSCAPE_AST_GC_H

#include <new>
#include <gc/gc_cpp.h>

namespace Inkscape {
namespace AST {

template <class gc_type, GCPlacement default_placement>
class GCObject : public gc_type {
public:
    void *operator new(size_t size) throw(std::bad_alloc) {
        return operator new(size, default_placement);
    }
    void *operator new(size_t size, GCPlacement placement)
    throw(std::bad_alloc)
    {
        void *mem=gc_type::operator new(size, placement);
        if (mem) {
            return mem;
        } else {
            throw std::bad_alloc();
        }
    }

    void *operator new[](size_t size) throw(std::bad_alloc) {
        return operator new(size, default_placement);
    }
    void *operator new[](size_t size, GCPlacement placement)
    throw(std::bad_alloc)
    {
        return operator new(size, p);
    }
};

template <GCPlacement default_placement=::GC>
class SimpleGCObject : public GCObject<::gc, default_placement> {
};

template <GCPlacement default_placement=::GC>
class FinalizedGCObject : public GCObject<::gc_cleanup, default_placement> {
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
