#ifndef SEEN_XML_SP_REPR_ATTR_H
#define SEEN_XML_SP_REPR_ATTR_H

#include <glib/gquark.h>
#include <glib/gtypes.h>
#include "gc-managed.h"
#include "xml/xml-forward.h"
#include "util/shared-c-string.h"


#define SP_REPR_ATTRIBUTE_KEY(a) g_quark_to_string((a)->key)
#define SP_REPR_ATTRIBUTE_VALUE(a) ((a)->value)


struct SPReprAttr : public Inkscape::GC::Managed<> {
    SPReprAttr(GQuark k,
               Inkscape::Util::SharedCString v,
               SPReprAttr *n=NULL)
    : next(n), key(k), value(v) {}

    SPReprAttr(SPReprAttr const &attr, SPReprAttr *n=NULL)
    : next(n), key(attr.key), value(attr.value) {}

    SPReprAttr *next;
    GQuark key;
    Inkscape::Util::SharedCString value;
};


#endif /* !SEEN_XML_SP_REPR_ATTR_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
