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
#ifndef SUPPRESS_LIBGC
        void *base=GC_base(this);
        if (base) { // only if we are managed by the collector
            CleanupFunc old_cleanup;
            void *old_data;

            GC_register_finalizer_ignore_self(base, _invoke_dtor,
                                                    _offset(base, this),
                                                    &old_cleanup, &old_data);

            if (old_cleanup) { // Whoops, already one registered?  Put it back.
                GC_register_finalizer_ignore_self(base, old_cleanup, old_data,
                                                        NULL, NULL);
            }
        }
#endif
    }

    virtual ~Finalized() {
#ifndef SUPPRESS_LIBGC
        // make sure the destructor won't get invoked twice
        GC_register_finalizer_ignore_self(GC_base(this), NULL, NULL, NULL, NULL);
#endif
    }

private:
    static void _invoke_dtor(void *base, void *offset) {
        _unoffset(base, offset)->~Finalized();
    }

    static void *_offset(void *base, Finalized *self) {
        return reinterpret_cast<void *>(
            reinterpret_cast<char *>(self) - reinterpret_cast<char *>(base)
        );
    }
    static Finalized *_unoffset(void *base, void *offset) {
        return reinterpret_cast<Finalized *>(
            reinterpret_cast<char *>(base) + reinterpret_cast<int>(offset)
        );
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
