/*
 * Inkscape::Refcounted - base class for refcounted (refcounted) objects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_REFCOUNTED_H
#define SEEN_INKSCAPE_REFCOUNTED_H

#include <glib/gmessages.h>
#include "gc-object.h"

namespace Inkscape {

/**
 * A base class for refcounted objects.  Objects are managed
 * by our mark-and-sweep collector, but are anchored as GC roots
 * so long as their reference count is nonzero.
 *
 * New instances of refcounted objects should be created using the C++ new
 * operator.  Under normal circumstances they should not be created on
 * the stack.
 *
 * A newly created refcounted object begins with a refcount of one, and
 * will not be collected unless the refcount is zero.
 *
 * @see Inkscape::Refcounted::claim
 * @see Inkscape::Refcounted::release
 */

class Refcounted : public GC::FinalizedObject<> {
public:
    /**
     * @brief Increments the reference count of a refcounted object.
     *
     * This function template generates functions which take
     * a reference to a refcounted object of a given type, increment
     * that object's reference count, and return a reference to the
     * object of the same type as the function's parameter.
     *
     * @param m a reference to a refcounted object
     *
     * @return the reference to the object
     */
    template <typename R>
    static R &claim(R &r) {
        static_cast<Refcounted const &>(const_cast<R const &>(r))._claim();
        return r;
    }

    /**
     * @brief Increments the reference count of a refcounted object.
     *
     * This function template generates functions which take
     * a pointer to a refcounted object of a given type, increment
     * that object's reference count, and return a pointer to the
     * object of the same type as the function's parameter.
     *
     * @param m a pointer to refcounted object
     *
     * @return the pointer to the object
     */
    template <typename R>
    static R *claim(R *r) {
        static_cast<Refcounted const *>(const_cast<R const *>(r))->_claim();
        return r;
    }

    /**
     * @brief Decrements the reference count of a refcounted object.
     *
     * This function template generates functions which take
     * a reference to a refcounted object of a given type, increment
     * that object's reference count, and return a reference to the
     * object of the same type as the function's parameter.
     *
     * The return value is safe to use since the object, even if
     * its refcount has reached zero, will not actually be collected
     * until there are no references to it in local variables or
     * parameters.
     *
     * @param m a reference to a refcounted object
     *
     * @return the reference to the object
     */
    template <typename R>
    static R &release(R &r) {
        static_cast<Refcounted const &>(const_cast<R const &>(r))._release();
        return r;
    }

    /**
     * @brief Decrements the reference count of a refcounted object.
     *
     * This function template generates functions which take
     * a pointer to a refcounted object of a given type, increment
     * that object's reference count, and return a pointer to the
     * object of the same type as the function's parameter.
     *
     * The return value is safe to use since the object, even if
     * its refcount has reached zero, will not actually be collected
     * until there are no references to it in local variables or
     * parameters.
     *
     * @param m a pointer to a refcounted object
     *
     * @return the pointer to the object
     */
    template <typename R>
    static R *release(R *r) {
        static_cast<Refcounted const *>(const_cast<R const *>(r))->_release();
        return r;
    }

protected:
    Refcounted() : _anchor(NULL) { _claim(); }

private:
    struct Anchor : public GC::Object<GC::SCANNED, GC::MANUAL> {
        Anchor(Refcounted const *obj) : refcount(0), object(obj) {}
        int refcount;
        Refcounted const *object;
    };

    Refcounted(Refcounted const &); // no copy
    void operator=(Refcounted const &); // no assign

    void _claim() const {
        if (!_anchor) {
            _anchor = new Anchor(this);
        }
        _anchor->refcount++;
    }
    void _release() const {
        if (!--_anchor->refcount) {
            delete _anchor;
            _anchor = NULL;
        }
    }

    mutable Anchor *_anchor;
};

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
