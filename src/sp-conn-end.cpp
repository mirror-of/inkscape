#include <glib/gmessages.h>
#include <glib/gstrfuncs.h>

#include "libnr/nr-matrix-div.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-point.h"
#include "libnr/nr-rect.h"
#include "xml/repr.h"
#include "sp-conn-end.h"
#include "sp-path.h"

static void change_endpts(SPCurve *const curve, NR::Point const h2endPt[2]);
static NR::Point calc_bbox_conn_pt(NR::Rect const &bbox, NR::Point const &p);
static double signed_one(double const x);

SPConnEnd::SPConnEnd(SPObject *const owner) :
    ref(owner),
    href(NULL),
    _changed_connection(),
    _delete_connection(),
    _transformed_connection()
{
}

/* In principle, SPItem is const, but invoke_bbox is currently non-const. */
static NR::Rect
get_bbox(SPItem *const item, NR::Matrix const &m)
{
    NRRect bbox;
    sp_item_invoke_bbox(item, &bbox, m, true);
    return NR::Rect(bbox);
}

static SPObject const *
get_nearest_common_ancestor(SPObject const *const obj, SPItem const *const objs[2]) {
    SPObject const *anc_sofar = obj;
    for (unsigned i = 0; i < 2; ++i) {
        if ( objs[i] != NULL ) {
            anc_sofar = anc_sofar->nearestCommonAncestor(objs[i]);
        }
    }
    return anc_sofar;
}

static void
sp_conn_end_move_compensate(NR::Matrix const *mp, SPItem *moved_item,
                            SPPath *const path)
{
    SPItem *h2attItem[2];
    path->connEndPair.getAttachedItems(h2attItem);
    g_return_if_fail( h2attItem[0] ||
                      h2attItem[1]   );

    SPItem const *const path_item = SP_ITEM(path);
    SPObject const *const ancestor = get_nearest_common_ancestor(path_item, h2attItem);
    NR::Matrix const path2anc(i2anc_affine(path_item, ancestor));

    if (h2attItem[0] != NULL && h2attItem[1] != NULL) {
        /* Initial end-points: centre of attached object. */
        NR::Point h2endPt_icoordsys[2];
        NR::Matrix h2i2anc[2];
        NR::Rect h2bbox_icoordsys[2] = {
            get_bbox(h2attItem[0], NR::identity()),
            get_bbox(h2attItem[1], NR::identity())
        };
        for (unsigned h = 0; h < 2; ++h) {
            h2i2anc[h] = i2anc_affine(h2attItem[h], ancestor);
            h2endPt_icoordsys[h] = h2bbox_icoordsys[h].midpoint();
        }

        /* For each attached object, change the corresponding point to be on the edge of
         * the bbox. */
        NR::Point h2endPt_pcoordsys[2];
        for (unsigned h = 0; h < 2; ++h) {
            h2endPt_icoordsys[h] = calc_bbox_conn_pt(h2bbox_icoordsys[h],
                                                     ( h2endPt_icoordsys[!h]
                                                       * h2i2anc[!h]
                                                       / h2i2anc[h] ));
            h2endPt_pcoordsys[h] = h2endPt_icoordsys[h] * h2i2anc[h] / path2anc;
        }
        change_endpts(path->shape.curve, h2endPt_pcoordsys);
    } else {
        /* Only one end attached.  Do translate. */
        unsigned const att_h = ( h2attItem[0] == NULL );
        SPCurve const *const curve = path->shape.curve;
        NR::Point const h2oldEndPt_pcoordsys[2] = { sp_curve_first_point(curve),
                                                    sp_curve_last_point(curve) };
        NR::Point const dirn_pcoordsys = ( h2oldEndPt_pcoordsys[!att_h] -
                                           h2oldEndPt_pcoordsys[att_h]   );
        NR::Matrix const i2anc(i2anc_affine(h2attItem[att_h], ancestor));
        NR::Point const dirn_icoordsys = ( dirn_pcoordsys
                                           * NR::transform(path2anc)
                                           / NR::transform(i2anc) );
        NR::Rect const bbox_icoordsys(get_bbox(h2attItem[att_h], NR::identity()));
        NR::Point const ctr_icoordsys = bbox_icoordsys.midpoint();
        NR::Point const connPt = calc_bbox_conn_pt(bbox_icoordsys,
                                                ctr_icoordsys + dirn_icoordsys);
        sp_curve_transform(path->shape.curve,
                           NR::translate(connPt - h2oldEndPt_pcoordsys[att_h]));
    }
    SP_OBJECT(path)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static NR::Point
calc_bbox_conn_pt(NR::Rect const &bbox, NR::Point const &p)
{
    using NR::X;
    using NR::Y;
    NR::Point const ctr(bbox.midpoint());
    NR::Point const lengths(bbox.dimensions());
    if ( ctr == p ) {
	/* Arbitrarily choose centre of right edge. */
	return NR::Point(ctr[X] + .5 * lengths[X],
			 ctr[Y]);
    }
    NR::Point const cp( p - ctr );
    NR::Dim2 const edgeDim = ( ( fabs(lengths[Y] * cp[X]) <
	   		         fabs(lengths[X] * cp[Y])  )
			       ? Y
			       : X );
    NR::Dim2 const otherDim = (NR::Dim2) !edgeDim;
    NR::Point offset;
    offset[edgeDim] = (signed_one(cp[edgeDim])
		       * lengths[edgeDim]);
    offset[otherDim] = (lengths[edgeDim]
			* cp[otherDim]
			/ fabs(cp[edgeDim]));
    g_assert((offset[otherDim] >= 0) == (cp[otherDim] >= 0));
#ifndef NDEBUG
    for (unsigned d = 0; d < 2; ++d) {
	g_assert(fabs(offset[d]) <= lengths[d] + .125);
    }
#endif
    return ctr + .5 * offset;
}

static double signed_one(double const x)
{
    return (x < 0
	    ? -1.
	    : 1.);
}

static void
change_endpts(SPCurve *const curve, NR::Point const h2endPt[2])
{
    /* TODO: Retain curves of existing curve. */
    sp_curve_reset(curve);
    sp_curve_moveto(curve, h2endPt[0]);
    sp_curve_lineto(curve, h2endPt[1]);
}

static void
sp_conn_end_deleted(SPObject *, SPObject *const owner, unsigned const handle_ix)
{
    /* todo: The first argument is the deleted object.  Perhaps use it for assertion. */
    g_return_if_fail(handle_ix < 2);
    char const *const attr_str[] = {"inkscape:connection-start",
                                    "inkscape:connection-end"};
    sp_repr_set_attr(SP_OBJECT_REPR(owner), attr_str[handle_ix], NULL);
    /* I believe this will trigger sp_conn_end_href_changed. */
}

void
SPConnEnd::setAttacherHref(gchar const *value)
{    
    if ( value && href && ( strcmp(value, href) == 0 ) ) {
        /* No change, do nothing. */
    } else {
        g_free(href);
        href = NULL;
        if (value) {
            // First, set the href field, because sp_conn_end_href_changed will need it.
            href = g_strdup(value);

            // Now do the attaching, which emits the changed signal.
            try {
                ref.attach(Inkscape::URI(value));
            } catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                ref.detach();
            }
        } else {
            ref.detach();
        }
    }
}

void
sp_conn_end_href_changed(SPObject *old_ref, SPObject *ref,
                         SPConnEnd *connEndPtr, SPPath *const path, unsigned const handle_ix)
{
    g_return_if_fail(connEndPtr != NULL);
    SPConnEnd &connEnd = *connEndPtr;
    connEnd._delete_connection.disconnect();
    connEnd._transformed_connection.disconnect();

    if (connEnd.href) {
        SPObject *refobj = connEnd.ref.getObject();
        if (refobj) {
            connEnd._delete_connection
                = SP_OBJECT(refobj)->connectDelete(sigc::bind(sigc::ptr_fun(&sp_conn_end_deleted),
                                                              SP_OBJECT(path), handle_ix));
            connEnd._transformed_connection
                = SP_ITEM(refobj)->connectTransformed(sigc::bind(sigc::ptr_fun(&sp_conn_end_move_compensate),
                                                                 path));
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
