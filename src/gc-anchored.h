/*
 * Inkscape::GC::Anchored - base class for anchored GC-managed objects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_GC_ANCHORED_H
#define SEEN_INKSCAPE_GC_ANCHORED_H

#include <glib/gmessages.h>
#include "gc-managed.h"

namespace Inkscape {

namespace GC {

/**
 * A base class for anchored objects.  Objects are managed
 * by our mark-and-sweep collector, but are anchored against garbage
 * collection so long as their reference count is nonzero.
 *
 * Object and member destructors will not be called on destruction
 * unless a subclass also inherits from Inkscape::GC::Finalized.
 *
 * New instances of anchored objects should be created using the C++ new
 * operator.  Under normal circumstances they should not be created on
 * the stack.
 *
 * A newly created anchored object begins with a refcount of one, and
 * will not be collected unless the refcount is zero.
 *
 * @see Inkscape::GC::Managed
 * @see Inkscape::GC::Finalized
 * @see Inkscape::GC::anchor
 * @see Inkscape::GC::release
 */

class Anchored {
public:
    void anchor() const {
        if (!_anchor) {
            _anchor = new Anchor(GC_base(const_cast<Anchored *>(this)));
        }
        _anchor->refcount++;
    }

    void release() const {
        if (!--_anchor->refcount) {
            delete _anchor;
            _anchor = NULL;
        }
    }

protected:
    Anchored() : _anchor(NULL) { anchor(); } // initial refcount of one

private:
    struct Anchor : public Managed<SCANNED, MANUAL> {
        Anchor(void const *obj) : refcount(0), object(obj) {}
        int refcount;
        void const *object;
    };

    Anchored(Anchored const &); // no copy
    void operator=(Anchored const &); // no assign

    mutable Anchor *_anchor;
};

/**
 * @brief Increments the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a reference to a anchored object of a given type, increment
 * that object's reference count, and return a reference to the
 * object of the same type as the function's parameter.
 *
 * @param m a reference to a anchored object
 *
 * @return the reference to the object
 */
template <typename R>
static R &anchor(R &r) {
    static_cast<Anchored const &>(const_cast<R const &>(r)).anchor();
    return r;
}

/**
 * @brief Increments the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a pointer to a anchored object of a given type, increment
 * that object's reference count, and return a pointer to the
 * object of the same type as the function's parameter.
 *
 * @param m a pointer to anchored object
 *
 * @return the pointer to the object
 */
template <typename R>
static R *anchor(R *r) {
    static_cast<Anchored const *>(const_cast<R const *>(r))->anchor();
    return r;
}

/**
 * @brief Decrements the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a reference to a anchored object of a given type, increment
 * that object's reference count, and return a reference to the
 * object of the same type as the function's parameter.
 *
 * The return value is safe to use since the object, even if
 * its refcount has reached zero, will not actually be collected
 * until there are no references to it in local variables or
 * parameters.
 *
 * @param m a reference to a anchored object
 *
 * @return the reference to the object
 */
template <typename R>
static R &release(R &r) {
    static_cast<Anchored const &>(const_cast<R const &>(r)).release();
    return r;
}

/**
 * @brief Decrements the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a pointer to a anchored object of a given type, increment
 * that object's reference count, and return a pointer to the
 * object of the same type as the function's parameter.
 *
 * The return value is safe to use since the object, even if
 * its refcount has reached zero, will not actually be collected
 * until there are no references to it in local variables or
 * parameters.
 *
 * @param m a pointer to a anchored object
 *
 * @return the pointer to the object
 */
template <typename R>
static R *release(R *r) {
    static_cast<Anchored const *>(const_cast<R const *>(r))->release();
    return r;
}

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
