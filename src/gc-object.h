/*
 * Inkscape::GC::Base - base class for GC-able objects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_GC_BASE_H
#define SEEN_INKSCAPE_GC_BASE_H

#include <new>
#include <glib/gmessages.h>
#include <gc/gc_cpp.h>
#include "gc-core.h"

namespace Inkscape {

namespace GC {

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
            g_assert_not_reached();
        }
    }
}

template <typename B, ScanPolicy default_scan, CollectionPolicy default_collect>
class Base : public B {
public:
    void *operator new(size_t size,
                       ScanPolicy scan=default_scan,
                       CollectionPolicy collect=default_collect)
    throw(std::bad_alloc)
    {
        void *mem=B::operator new(size, to_placement(scan, collect));
        if (!mem) {
            throw std::bad_alloc();
        }
        return mem;
    }

    void *operator new[](size_t size,
                         ScanPolicy scan=default_scan,
                         CollectionPolicy collect=default_collect)
    throw(std::bad_alloc)
    {
        return operator new(size, scan, collect);
    }
};

template <ScanPolicy default_scan=SCANNED,
          CollectionPolicy default_collect=AUTO>
class Object : Base<::gc_object, default_scan, default_collect> {}

template <ScanPolicy default_scan=SCANNED,
          CollectionPolicy default_collect=AUTO>
class FinalizedObject : Base<::gc_cleanup, default_scan, default_collect> {}

}

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
