#ifndef SEEN_XML_SP_REPR_ATTR_H
#define SEEN_XML_SP_REPR_ATTR_H

#include <glib/gquark.h>
#include <glib/gtypes.h>

#include <xml/xml-forward.h>


#define SP_REPR_ATTRIBUTE_KEY(a) g_quark_to_string((a)->key)
#define SP_REPR_ATTRIBUTE_VALUE(a) ((a)->value)


struct SPReprAttr {
    SPReprAttr *next;
    GQuark key;
    gchar *value;
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
