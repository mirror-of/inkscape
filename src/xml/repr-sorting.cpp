#include "xml/repr-sorting.h"

#include "algorithms/longest-common-suffix.h"
#include "xml/repr.h"
#include "xml/sp-repr.h"
#include "xml/sp-repr-iterators.h"

static bool
same_repr(SPRepr &a, SPRepr &b)
{
  /* todo: I'm not certain that it's legal to take the address of a reference.  Check the exact wording of the spec on this matter. */
    return &a == &b;
}

SPRepr *
LCA(SPRepr *a, SPRepr *b)
{
    using Inkscape::Algorithms::longest_common_suffix;
    SPRepr *ancestor = longest_common_suffix<SPReprParentIterator>(
        a, b, NULL, &same_repr
    );
    if ( ancestor && ancestor->type() != SP_XML_DOCUMENT_NODE ) {
        return ancestor;
    } else {
        return NULL;
    }
}

/**
 * Returns a child of \a ancestor such that ret is itself an ancestor of \a descendent.
 *
 * The current version returns NULL if ancestor or descendent is NULL, though future versions may
 * call g_log.  Please update this comment if you rely on the current behaviour.
 */
SPRepr *
AncetreFils(SPRepr *descendent, SPRepr *ancestor)
{
    if (descendent == NULL || ancestor == NULL)
        return NULL;
    if (sp_repr_parent(descendent) == ancestor)
        return descendent;
    return AncetreFils(sp_repr_parent(descendent), ancestor);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
