/*
 * Inkscape::Managed - base class for managed (refcounted) objects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_MANAGED_H
#define SEEN_INKSCAPE_MANAGED_H

#include <glib/gmessages.h>

namespace Inkscape {

/**
 * A base class for garbage-collected objects.  The current collection
 * scheme is based on simple refcounting, but may include other
 * strategies (e.g. mark-and-sweep) in the future.
 *
 * New instances of managed objects should be created using the C++ new
 * operator.  Under normal circumstances they should not be created on
 * the stack.
 *
 * A newly created managed object begins with a refcount of one, and
 * may only be collected unless the refcount is zero.  Currently
 * collection takes place as soon as the refcount reaches zero, but
 * immediate collection is not guaranteed in the future.  Do not rely
 * on it.
 *
 * @see Inkscape::Managed::claim
 * @see Inkscape::Managed::release
 */

class Managed {
public:
    /**
     * @brief Increments the reference count of a managed object.
     *
     * This function template generates functions which take
     * a reference to a managed object of a given type, increment
     * that object's reference count, and return a reference to the
     * object of the same type as the function's parameter.
     *
     * @param m a reference to a managed object
     *
     * @return the reference to the object
     */
    template <typename M>
    static M &claim(M &m) {
        static_cast<Managed &>(m)._claim();
        return m;
    }

    /**
     * @brief Increments the reference count of a managed object.
     *
     * This function template generates functions which take
     * a pointer to a managed object of a given type, increment
     * that object's reference count, and return a pointer to the
     * object of the same type as the function's parameter.
     *
     * @param m a pointer to managed object
     *
     * @return the pointer to the object
     */
    template <typename M>
    static M *claim(M *m) {
        static_cast<Managed *>(m)->_claim();
        return m;
    }

    /**
     * @brief Decrements the reference count of a managed object.
     *
     * This function template generates functions which take
     * a reference to a managed object of a given type, increment
     * that object's reference count, and return a reference to the
     * object of the same type as the function's parameter.
     *
     * The object will likely have been destroyed by the time this
     * function returns.  This function's return value is for future
     * use only.
     *
     * @param m a reference to a managed object
     *
     * @return the reference to the object
     */
    template <typename M>
    static M &release(M &m) {
        static_cast<Managed &>(m)._release();
        return m;
    }

    /**
     * @brief Decrements the reference count of a managed object.
     *
     * This function template generates functions which take
     * a pointer to a managed object of a given type, increment
     * that object's reference count, and return a pointer to the
     * object of the same type as the function's parameter.
     *
     * The object will likely have been destroyed by the time this
     * function returns.  This function's return value is for future
     * use only.
     *
     * @param m a pointer to a managed object
     *
     * @return the pointer to the object
     */
    template <typename M>
    static M *release(M *m) {
        static_cast<Managed *>(m)->_release();
        return m;
    }

protected:
    Managed() : _refcount(1) {}
    virtual ~Managed() {
        if (_refcount) {
            g_critical("Managed object destroyed with nonzero refcount");
        }
    }

private:
    Managed(Managed const &); // no copy
    void operator=(Managed const &); // no assign

    Managed &_claim() {
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
