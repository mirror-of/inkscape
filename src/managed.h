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

class Managed {
public:
    template <typename M>
    static M &claim(M &m) {
        static_cast<Managed &>(m)._claim();
        return m;
    }

    template <typename M>
    static M *claim(M *m) {
        static_cast<Managed *>(m)->_claim();
        return m;
    }

    static void release(Managed &m) { m._release(); }
    static void release(Managed *m) { m->_release(); }

protected:
    Managed() : _refcount(0) {}
    virtual ~Managed() {
        if (_refcount) {
            g_critical("Managed object destroyed with nonzero refcount");
        }
    }

private:
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
