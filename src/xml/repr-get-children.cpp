#include <glib/gmessages.h>

#include <xml/repr-get-children.h>
#include <xml/repr-private.h>


/** Returns the last child of \a parent, or NULL if parent has no children.
 *
 *  Suitable for use with sp_repr_add_child.
 */
SPRepr *
sp_repr_last_child(SPRepr const * const parent)
{
    SPRepr *last = NULL;
    SPRepr *child = parent->children;
    while (child) {
        last = child;
        child = child->next;
    }

    return last;
}

/** Returns the sibling before \a child in \a child's parent's children, or NULL if \a child is the
 *  first of those children (or if child is NULL or has no parent).
 *
 *  Useful in combination with sp_repr_add_child, when you want to insert a new child _before_ a
 *  given existing child.
 *
 *  Note: Involves a linear search (unlike sp_repr_next).

 * \pre Links are correct, i.e. \a child isin its parent's children.
 *
 * \post (ret == NULL
 *        ? child == NULL || child.parent == NULL || child.parent.children == child
 *        : sp_repr_next(ret) == child).
 */
SPRepr *
sp_repr_prev(SPRepr const *const child)
{
    if (!child || !child->parent) {
        return NULL;
    }

    SPRepr *prev = NULL;
    for (SPRepr *curr = child->parent->children; curr != child; curr = curr->next) {
        if (!curr) {
            g_warning("child repr not found in its parent's list of children");
            return NULL;
        }

        prev = curr;
    }

    g_assert(prev == NULL
             ? child->parent->children == child
             : prev->next == child);
    return prev;
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
