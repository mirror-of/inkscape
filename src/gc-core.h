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

#include <new>
#include <cstdlib>
#include <gc/gc.h>
#include <glib/gmain.h>

//#define SUPPRESS_LIBGC

namespace Inkscape {

namespace GC {

inline void init() {
#ifndef SUPPRESS_LIBGC
    GC_finalize_on_demand = 1;
    GC_no_dls = 1;
    GC_INIT();
    // ensure that finalizers are called at sane times
    g_idle_add(GSourceFunc(GC_invoke_finalizers), NULL);
#endif
}

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

}

}

inline void *operator new(size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CollectionPolicy collect,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
#ifndef SUPPRESS_LIBGC
    void *mem;
    if ( collect == Inkscape::GC::AUTO ) {
        if ( scan == Inkscape::GC::SCANNED ) {
            mem = GC_MALLOC(size);
        } else {
            mem = GC_MALLOC_ATOMIC(size);
        }
    } else {
        if ( scan == Inkscape::GC::SCANNED ) {
            mem = GC_MALLOC_UNCOLLECTABLE(size);
        } else {
            abort(); // can't use g_assert as g++ doesn't like to inline it
        }
    }
    if (!mem) {
        throw std::bad_alloc();
    }
    if ( collect == Inkscape::GC::AUTO && cleanup ) {
        GC_REGISTER_FINALIZER_IGNORE_SELF(mem, cleanup, data, NULL, NULL);
    }
    return mem;
#else
    return ::operator new(size);
#endif
}

inline void *operator new(size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    return operator new(size, scan, Inkscape::GC::AUTO, cleanup, data);
}

inline void *operator new[](size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CollectionPolicy collect,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    return operator new(size, scan, collect, cleanup, data);
}

inline void *operator new[](size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    return operator new[](size, scan, Inkscape::GC::AUTO, cleanup, data);
}

inline void operator delete(void *mem, Inkscape::GC::Delete) {
#ifndef SUPPRESS_LIBGC
    GC_FREE(mem);
#else
    ::operator delete(mem);
#endif
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
