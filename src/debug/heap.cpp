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

#include <malloc.h>
#include "gc-alloc.h"
#include "debug/heap.h"
#include <vector>

namespace Inkscape {

namespace Debug {

namespace {

class MallocHeap : public Heap {
public:
    MallocHeap() {}
    
    int features() const;

    Util::SharedCStringPtr name() const {
        return Util::SharedCStringPtr::coerce("standard malloc()");
    }
    std::size_t size() const;
    std::size_t bytes_free() const;
    void force_collect() {}
};

int MallocHeap::features() const {
#ifdef HAS_MALLINFO
    return SIZE_AVAILABLE | FREE_AVAILABLE;
#else
    return 0;
#endif
}

std::size_t MallocHeap::size() const {
    std::size_t total=0;

#ifdef HAS_MALLINFO
    struct mallinfo info=mallinfo();

#ifdef HAS_MALLINFO_HBLKHD
    total += info.hblkhd;
#endif

#ifdef HAS_MALLINFO_USMBLKS
    total += info.usmblks;
#endif

#ifdef HAS_MALLINFO_FSMBLKS
    total += info.fsmblks;
#endif

#ifdef HAS_MALLINFO_UORDBLKS
    total += info.uordblks;
#endif

#ifdef HAS_MALLINFO_FORDBLKS
    total += info.fordblks;
#endif

#endif

    return total;
}

std::size_t MallocHeap::bytes_free() const {
    std::size_t total=0;

#ifdef HAS_MALLINFO
    struct mallinfo info=mallinfo();

#ifdef HAS_MALLINFO_FSMBLKS
    total += info.fsmblks;
#endif

#ifdef HAS_MALLINFO_FORDBLKS
    total += info.fordblks;
#endif

#endif

    return total;
}

typedef std::vector<Heap *, GC::Alloc<Heap *, GC::MANUAL> > Heaps;

Heaps &heaps() {
    static Heaps heaps;
    return heaps;
}

}

Heap &get_malloc_heap() {
    static MallocHeap heap;
    return heap;
}

unsigned heap_count() {
    return heaps().size() + 1;
}

Heap *get_heap(unsigned i) {
    if (i) {
        i--;
        if ( i < heaps().size() ) {
            return heaps()[i];
        } else {
            return NULL;
        }
    } else {
        return &get_malloc_heap();
    }
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
