#include <glib/gmessages.h>

#include "xml/node.h"
#include "xml/node-iterators.h"
#include "xml/node-fns.h"
#include "algorithms/find-if-before.h"

namespace Inkscape {
namespace XML {

struct node_matches {
    node_matches(Node const &n) : node(n) {}
    bool operator()(Node const &other) { return &other == &node; }
    Node const &node;
};

/** Returns the sibling before \a node in \a node's parent's children,
 *  or NULL if \a node is the first of those children (or if child is
 *  NULL or has no parent).
 *
 *  Useful in combination with Node::addChild, when you want to insert
 *  a new child _before_ a given existing child.
 *
 *  Note: Involves a linear search (unlike next_node).
 *
 * \pre Links are correct, i.e. \a node isin its parent's children.
 *
 * \post (ret == NULL
 *        ? node == NULL || node->parent() == NULL || node->parent()->firstChild() == node
 *        : ret->next() == node).
 */
Node *previous_node(Node *node) {
    using Inkscape::Algorithms::find_if_before;

    if ( !node || !node->parent() ) {
        return NULL;
    }

    Node *previous=find_if_before<NodeSiblingIterator>(
        node->parent()->firstChild(), NULL, node_matches(*node)
    );

    g_assert(previous == NULL
             ? node->parent()->firstChild() == node
             : previous->next() == node);

    return previous;
}

}
}


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
