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
#include <gc/gc_cpp.h>
#include <glib/gmain.h>

namespace Inkscape {

namespace GC {

inline void init() {
    GC_finalize_on_demand = 1;
    GC_INIT();
    // ensure that finalizers are called at sane times
    g_idle_add(GSourceFunc(GC_invoke_finalizers), NULL);
}

enum ScanPolicy {
    SCANNED,
    ATOMIC
};

enum CollectionPolicy {
    AUTO,
    MANUAL
};

inline ::GCPlacement to_placement(ScanPolicy scan, CollectionPolicy collect) {
    if ( scan == SCANNED ) {
        if ( collect == AUTO ) {
            return ::UseGC;
        } else {
            return ::NoGC;
        }
    } else {
        if ( collect == AUTO ) {
            return ::PointerFreeGC;
        } else {
            // g++ doesn't like inlining g_assert()s for some reason
            //g_assert_not_reached();
            std::abort();
            return ::NoGC;
        }
    }
}

typedef ::GCCleanUpFunc CleanupFunc;

}

}

inline void *operator new(size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CollectionPolicy collect,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    void *mem=::operator new(size, Inkscape::GC::to_placement(scan, collect), cleanup, data);
    if (!mem) {
        throw std::bad_alloc();
    }
    return mem;
}

inline void *operator new(size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    void *mem=::operator new(size, Inkscape::GC::to_placement(scan, Inkscape::GC::AUTO), cleanup, data);
    if (!mem) {
        throw std::bad_alloc();
    }
    return mem;
}

inline void *operator new[](size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CollectionPolicy collect,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    void *mem=::operator new[](size, Inkscape::GC::to_placement(scan, collect), cleanup, data);
    if (!mem) {
        throw std::bad_alloc();
    }
    return mem;
}

inline void *operator new[](size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    void *mem=::operator new[](size, Inkscape::GC::to_placement(scan, Inkscape::GC::AUTO), cleanup, data);
    if (!mem) {
        throw std::bad_alloc();
    }
    return mem;
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
