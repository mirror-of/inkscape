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

#include <gc/gc.h>

namespace Inkscape {

namespace GC {

static inline void init() {
    GC_INIT();
}

enum ScanPolicy {
    SCANNED,
    ATOMIC
};

enum CollectionPolicy {
    AUTO,
    MANUAL
};

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
