#ifndef SEEN_REPR_GET_CHILDREN_H
#define SEEN_REPR_GET_CHILDREN_H

#include <xml/xml-forward.h>

SPRepr *sp_repr_last_child(SPRepr const *parent);
SPRepr *sp_repr_prev_sibling(SPRepr const *child);


#endif /* !SEEN_REPR_GET_CHILDREN_H */

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
