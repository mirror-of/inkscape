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

#ifndef SEEN_INKSCAPE_DEBUG_HEAP_H
#define SEEN_INKSCAPE_DEBUG_HEAP_H

#include <cstddef>
#include "util/shared-c-string-ptr.h"

namespace Inkscape {

namespace Debug {

class Heap {
public:
    virtual ~Heap() {}

    struct Stats {
        Stats() {}
        Stats(std::size_t s, std::size_t f) : size(s), bytes_free(f) {}

        std::size_t size;
        std::size_t bytes_free;
    };

    enum {
        SIZE_AVAILABLE    = ( 1 << 0 ),
        FREE_AVAILABLE    = ( 1 << 1 ),
        GARBAGE_COLLECTED = ( 1 << 2 )
    };

    virtual int features() const=0;

    virtual Util::SharedCStringPtr name() const=0;
    virtual Stats stats() const=0;
    virtual void force_collect()=0;
};

unsigned heap_count();
Heap *get_heap(unsigned i);

void register_extra_heap(Heap &heap);

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
