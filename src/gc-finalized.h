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
#include <cstddef>
#include "gc-core.h"

namespace Inkscape {

namespace GC {

// @brief a mix-in ensuring that a object's destructor will get called before
//        the garbage collector destroys it
//
// Normally, the garbage collector does not call destructors before destroying
// an object.  On construction, this "mix-in" will register a finalizer
// function to call destructors before derived objects are destroyed.
// 
// This works pretty well, with the following caveats:
//
//   1. The garbage collector uses strictly topologically-ordered
//      finalization; if objects with finalizers reference each other
//      directly or indirectly, the collector will refuse to finalize (and
//      therefor free) them.
//
//      The best way to limit this effect is to only make "leaf" objects
//      finalizable, or to register some of pointer members as
//      "disappearing links" which will be cleared to break cycles
//      before finalization.
//
//   2. Because there is no guarantee when the collector will destroy
//      objects, there is no guarantee when the destructor will get called.
//
//      It may not get called until the very end of the program, or never.
//
//   3. If allocated in arrays, only the first object in the array will
//      have its destructor called, unless you make other arrangements by
//      registering your own finalizer instead.
//
//   4. Similarly, making multiple GC::Finalized-derived objects members
//      of a non-finalized but garbage-collected object generally won't
//      work unless you take care of registering finalizers yourself.
//
// [n.b., by "member", I mean an actual by-value-member of a type that
//  derives from GC::Finalized, not simply a member that's a pointer or a
//  reference to such a type]
//
// Your best choice is to use either non-garbage-collected immediate values,
// (C++'s standard RAII idiom), and otherwise avoid the use of non-trivial
// destructors as much as is reasonably possible.  With the garbage collector,
// it's not as hard as it sounds.
//
class Finalized {
public:
    Finalized() {
#ifndef SUPPRESS_LIBGC
        void *base=GC_base(this);
        if (base) { // only if we are managed by the collector
            CleanupFunc old_cleanup;
            void *old_data;

            // the finalization callback needs to know the value of 'this'
            // to call the destructor, but registering a real pointer to
            // ourselves would pin us forever and prevent us from being
            // finalized; instead we use an offset-from-base-address

            GC_register_finalizer_ignore_self(base, _invoke_dtor,
                                                    _offset(base, this)
                                                    &old_cleanup, &old_data);

            if (old_cleanup) {
                // If there was already a finalizer registered for our
                // base address, there are two main possibilities:
                //
                // 1. one of our members is also a GC::Finalized and had
                //    already registered a finalizer -- keep ours, since
                //    it will call that member's destructor, too
                //
                // 2. someone else registered a finalizer and we will have
                //    to trust that they will call the destructor -- put
                //    the existing finalizer back
                //
                // It's also possible that a member's constructor was called
                // after ours (e.g. via placement new).  Don't do that.

                if ( old_cleanup != _invoke_dtor ) {
                    GC_register_finalizer_ignore_self(base,
                                                      old_cleanup, old_data,
                                                      NULL, NULL);
                }
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

    /// turn 'this' pointer into an offset-from-base-address (stored as void *)
    static void *_offset(void *base, Finalized *self) {
        return reinterpret_cast<void *>(
            reinterpret_cast<char *>(self) - reinterpret_cast<char *>(base)
        );
    }
    /// reconstitute 'this' given an offset-from-base-address in a void *
    static Finalized *_unoffset(void *base, void *offset) {
        return reinterpret_cast<Finalized *>(
            reinterpret_cast<char *>(base) +
            reinterpret_cast<std::ptrdiff_t>(offset)
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
