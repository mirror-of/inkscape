#ifndef SEEN_SP_OBJECT_TREE_ITERATOR_H
#define SEEN_SP_OBJECT_TREE_ITERATOR_H

#include "sp-object.h"


namespace Inkscape {
namespace Traits {

template <typename T> struct TreeIterator;

template <>
struct TreeIterator<SPObject *> {
    typedef SPObject *Node;

    static bool is_null(Node o) { return o == NULL; }
    static Node null() { return NULL; }

    static Node node(Node o) { return o; }

    static Node first_child(Node o) {
        return o->firstChild();
    }
    static Node parent(Node o) {
        return SP_OBJECT_PARENT(o);
    }
    static Node next(Node o) {
        return SP_OBJECT_NEXT(o);
    }
};

template <>
struct TreeIterator<SPObject const *> {
    typedef SPObject const *Node;

    static bool is_null(Node o) { return o == NULL; }
    static Node null() { return NULL; }

    static Node node(Node o) { return o; }

    static Node first_child(Node o) {
        return o->firstChild();
    }
    static Node parent(Node o) {
        return SP_OBJECT_PARENT(o);
    }
    static Node next(Node o) {
        return SP_OBJECT_NEXT(o);
    }
};

}
}


#endif /* !SEEN_SP_OBJECT_TREE_ITERATOR_H */

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
