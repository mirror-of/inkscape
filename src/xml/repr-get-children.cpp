#include <glib/gmessages.h>

#include <xml/repr-get-children.h>
#include <xml/repr-private.h>


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
 *  first of those children.
 *
 *  Useful in combination with sp_repr_add_child, when you want to insert a new child _before_ a
 *  given existing child.
 */
SPRepr *
sp_repr_prev_sibling(SPRepr const * const child)
{
    g_return_val_if_fail(child != NULL, NULL);
    SPRepr const &parent = *sp_repr_parent(child);

    SPRepr *prev = NULL;
    for (SPRepr *curr = parent.children;;) {
        if ( curr == child ) {
            return prev;
        }
        if (!curr) {
            g_warning("child repr not found in its parent's list of children");
            return NULL;
        }

        prev = curr;
        curr = curr->next;
    }
    /* never reached */
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
