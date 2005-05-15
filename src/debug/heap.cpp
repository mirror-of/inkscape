/*
 * Inkscape::Debug::Heap - interface for gathering heap statistics
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "gc-alloc.h"
#include "debug/heap.h"
#include <vector>

namespace Inkscape {

namespace Debug {

namespace {

class SysVHeap : public Heap {
public:
    SysVHeap() {}
    
    int features() const;

    Util::SharedCStringPtr name() const {
        return Util::SharedCStringPtr::coerce("standard malloc()");
    }
    Stats stats() const;
    void force_collect() {}
};

int SysVHeap::features() const {
#ifdef HAVE_MALLINFO
    return SIZE_AVAILABLE | FREE_AVAILABLE;
#else
    return 0;
#endif
}

Heap::Stats SysVHeap::stats() const {
    Stats stats;

    stats.size = 0;
    stats.bytes_free = 0;

#ifdef HAVE_MALLINFO
    struct mallinfo info=mallinfo();

#ifdef HAVE_MALLINFO_USMBLKS
    stats.size += info.usmblks;
#endif

#ifdef HAVE_MALLINFO_FSMBLKS
    stats.size += info.fsmblks;
    stats.bytes_free += info.fsmblks;
#endif

#ifdef HAVE_MALLINFO_UORDBLKS
    stats.size += info.uordblks;
#endif

#ifdef HAVE_MALLINFO_FORDBLKS
    stats.size += info.fordblks;
    stats.free += info.fordblks;
#endif

#ifdef HAVE_MALLINFO_HBLKHD
    stats.size += info.hblkhd;
#endif

#endif

    return stats;
}

typedef std::vector<Heap *, GC::Alloc<Heap *, GC::MANUAL> > HeapCollection;

HeapCollection &heaps() {
    static bool is_initialized=false;
    static HeapCollection heaps;
    if (!is_initialized) {
        heaps.push_back(new SysVHeap());
        is_initialized = true;
    }
    return heaps;
}

}

unsigned heap_count() {
    return heaps().size() + 1;
}

Heap *get_heap(unsigned i) {
    return heaps()[i];
}

void register_extra_heap(Heap &heap) {
    heaps().push_back(&heap);
}

}

}

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
