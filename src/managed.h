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

namespace Inkscape {

class Managed {
public:
    static Managed &claim(Managed &m) { return m._claim(); }
    static Managed *claim(Managed *m) { return &m->_claim(); }
    static void release(Managed &m) { m._release(); }
    static void release(Managed *m) { m->_release(); }

protected:
    Managed() : _refcount(1) {}
    virtual ~Managed() {}

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
