#ifndef SEEN_REPR_GET_CHILDREN_H
#define SEEN_REPR_GET_CHILDREN_H

namespace Inkscape {
namespace XML {
class Node;
}
}


Inkscape::XML::Node *sp_repr_last_child(Inkscape::XML::Node const *parent);
Inkscape::XML::Node *sp_repr_prev(Inkscape::XML::Node const *child);

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
