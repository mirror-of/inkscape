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

namespace Inkscape {

/**
 * A base class for refcounted objects.  The current collection
 * scheme is based on simple refcounting, but may include other
 * strategies (e.g. mark-and-sweep) in the future.
 *
 * New instances of refcounted objects should be created using the C++ new
 * operator.  Under normal circumstances they should not be created on
 * the stack.
 *
 * A newly created refcounted object begins with a refcount of one, and
 * may only be collected unless the refcount is zero.  Currently
 * collection takes place as soon as the refcount reaches zero, but
 * immediate collection is not guaranteed in the future.  Do not rely
 * on it.
 *
 * @see Inkscape::Refcounted::claim
 * @see Inkscape::Refcounted::release
 */

class Refcounted {
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
    template <typename M>
    static M &claim(M &m) {
        static_cast<Refcounted &>(m)._claim();
        return m;
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
        static_cast<Refcounted *>(r)->_claim();
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
     * The object will likely have been destroyed by the time this
     * function returns.  This function's return value is for future
     * use only.
     *
     * @param m a reference to a refcounted object
     *
     * @return the reference to the object
     */
    template <typename R>
    static R &release(R &r) {
        static_cast<Refcounted &>(r)._release();
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
     * The object will likely have been destroyed by the time this
     * function returns.  This function's return value is for future
     * use only.
     *
     * @param m a pointer to a refcounted object
     *
     * @return the pointer to the object
     */
    template <typename R>
    static R *release(R *r) {
        static_cast<Refcounted *>(r)->_release();
        return r;
    }

protected:
    Refcounted() : _refcount(1) {}
    virtual ~Refcounted() {
        if (_refcount) {
            g_critical("Refcounted object destroyed with nonzero refcount");
        }
    }

private:
    Refcounted(Refcounted const &); // no copy
    void operator=(Refcounted const &); // no assign

    Refcounted &_claim() {
        _refcount++;
        return *this;
    }
    void _release() {
        if (!--_refcount) {
            delete this;
        }
    }

    int _refcount;
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
