/*
 * Inkscape::GC::Finalized - mixin for GC-managed objects with non-trivial
 *                           destructors
 *
 * Copyright 2004 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_GC_FINALIZED_H
#define SEEN_INKSCAPE_GC_FINALIZED_H

#include <new>
#include "gc-core.h"

namespace Inkscape {

namespace GC {

class Finalized {
public:
    Finalized() {
        void *mem=GC_base(this);
        if (mem) { // only if we were allocated via the GC
            CleanupFunc old_cleanup;
            void *old_data;

            GC_register_finalizer_ignore_self(mem, _invoke_dtor, this,
                                                   &old_cleanup, &old_data);

            if (old_cleanup) { // Whoops, already one registered?  Put it back.
                GC_register_finalizer_ignore_self(mem, old_cleanup, old_data,
                                                       NULL, NULL);
            }
        }
    }

    virtual ~Finalized() {
        // make sure the destructor won't get invoked twice
        GC_register_finalizer_ignore_self(GC_base(this), NULL, NULL, NULL, NULL);
    }

private:
    static void _invoke_dtor(void *mem, void *data) {
        reinterpret_cast<Finalized *>(data)->~Finalized();
    }
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
