/*
 * Inkscape::GC - Wrapper for Boehm GC
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_GC_CORE_H
#define SEEN_INKSCAPE_GC_CORE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <new>
#include <cstdlib>
#include <cstddef>
#ifdef HAVE_GC_GC_H
# include <gc/gc.h>
#else
# include <gc.h>
#endif
#include <glib/gmain.h>

namespace Inkscape {
namespace GC {

void init();

enum ScanPolicy {
    SCANNED,
    ATOMIC
};

enum CollectionPolicy {
    AUTO,
    MANUAL
};

enum Delete {
    GC
};

typedef void (*CleanupFunc)(void *mem, void *data);

struct Ops {
    void *(*malloc)(std::size_t size);
    void *(*malloc_atomic)(std::size_t size);
    void *(*malloc_uncollectable)(std::size_t size);
    void *(*base)(void *ptr);
    void (*register_finalizer_ignore_self)(void *base,
                                           CleanupFunc func, void *data,
                                           CleanupFunc *old_func,
                                           void **old_data);
    int (*general_register_disappearing_link)(void **p_ptr,
                                              void *base);
    int (*unregister_disappearing_link)(void **p_ptr);
    void (*free)(void *ptr);
};

extern Ops ops;

}
}

inline void *operator new(std::size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CollectionPolicy collect,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    using Inkscape::GC::ops;

    void *mem;
    if ( collect == Inkscape::GC::AUTO ) {
        if ( scan == Inkscape::GC::SCANNED ) {
            mem = ops.malloc(size);
        } else {
            mem = ops.malloc_atomic(size);
        }
    } else {
        if ( scan == Inkscape::GC::SCANNED ) {
            mem = ops.malloc_uncollectable(size);
        } else {
            abort(); // can't use g_assert as g++ doesn't like to inline it
        }
    }
    if (!mem) {
        throw std::bad_alloc();
    }
    if (cleanup) {
        ops.register_finalizer_ignore_self(mem, cleanup, data, NULL, NULL);
    }
    return mem;
}

inline void *operator new(std::size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    return operator new(size, scan, Inkscape::GC::AUTO, cleanup, data);
}

inline void *operator new[](std::size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CollectionPolicy collect,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    return operator new(size, scan, collect, cleanup, data);
}

inline void *operator new[](std::size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    return operator new[](size, scan, Inkscape::GC::AUTO, cleanup, data);
}

inline void operator delete(void *mem, Inkscape::GC::Delete) {
    Inkscape::GC::ops.free(mem);
}

inline void operator delete[](void *mem, Inkscape::GC::Delete) {
    operator delete(mem, Inkscape::GC::GC);
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
