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

#include "gc-core.h"
#include <glib/gmessages.h>

namespace Inkscape {
namespace GC {

namespace {

void display_warning(char *msg, GC_word arg) {
    g_warning(msg, arg);
}

}

void init() {
#ifndef SUPPRESS_LIBGC

    GC_no_dls = 1;
    GC_all_interior_pointers = 1;
    GC_finalize_on_demand = 0;

    GC_INIT();

    GC_set_free_space_divisor(8);
    GC_set_warn_proc(&display_warning);

#endif
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
