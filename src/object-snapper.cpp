/**
 *  \file object-snapper.cpp
 *  \brief Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2005 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-values.h"
#include "libnr/n-art-bpath.h"
#include "libnr/nr-rect-ops.h"
#include "libnr/nr-point-matrix-ops.h"
#include "object-snapper.h"
#include "document.h"
#include "sp-namedview.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "display/curve.h"
#include "snap.h"
#include "geom.h"
#include "desktop-affine.h"
#include "inkscape.h"
#include "splivarot.h"


Inkscape::ObjectSnapper::ObjectSnapper(SPNamedView const *nv, NR::Coord const d)
    : Snapper(nv, d), _snap_to_nodes(true), _snap_to_paths(true)
{
    
}


/**
 *  \param p Point we are trying to snap (desktop coordinates)
 */

void Inkscape::ObjectSnapper::_find_candidates(std::list<SPItem*>& c,
                                               SPObject* r,
                                               std::list<SPItem const *> const &it,
                                               NR::Point const &p) const
{
    for (SPObject* o = r->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o)) {

            /* See if this item is on the ignore list */
            std::list<SPItem const *>::const_iterator i = it.begin();
            while (i != it.end() && *i != o) {
                i++;
            }
            
            if (i == it.end()) {
                /* See if the item is within range */
                NR::Rect const b = NR::expand(sp_item_bbox_desktop(SP_ITEM(o)), -getDistance());
                if (b.contains(p)) {
                    c.push_back(SP_ITEM(o));
                }
            }
        }
        
        _find_candidates(c, o, it, p);
    }
}


void Inkscape::ObjectSnapper::_snap_nodes(NR::Point &snapped, NR::Coord &best, NR::Coord &upper,
                                          NR::Point const &req, std::list<SPItem*> const &cand) const
{
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;

    for (std::list<SPItem*>::const_iterator i = cand.begin(); i != cand.end(); i++) {
        if (SP_IS_SHAPE(*i)) {

            SPShape const *s = SP_SHAPE(*i);
            if (s->curve) {

                int j = 0;
                NR::Matrix const i2doc = sp_item_i2doc_affine(*i);
                
                while (s->curve->bpath[j].code != NR_END) {

                    /* Get this node in desktop coordinates */
                    NArtBpath const &bp = s->curve->bpath[j];
                    NR::Point const n = sp_desktop_doc2d_xy_point(desktop, bp.c(3) * i2doc);

                    /* Try to snap to this node of the path */
                    NR::Coord const dist = NR::L2(n - req);
                    if (dist < upper) {
                        upper = best = dist;
                        snapped = n;
                    }
                    
                    j++;
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::_snap_paths(NR::Point &snapped, NR::Coord &best, NR::Coord &upper,
                                          NR::Point const &req, std::list<SPItem*> const &cand) const
{
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;

    NR::Point const req_doc = sp_desktop_d2doc_xy_point(desktop, req);

    for (std::list<SPItem*>::const_iterator i = cand.begin(); i != cand.end(); i++) {

        /* Transform the requested snap point to this item's coordinates */
        NR::Matrix const i2doc = sp_item_i2doc_affine(*i);
        NR::Point const req_it = req_doc * i2doc.inverse();

        /* Look for the nearest position on this SPItem to our snap point */
        NR::Maybe<Path::cut_position> const p = get_nearest_position_on_Path(*i, req_it);
        if (p != NR::Nothing() && p.assume().t >= 0 && p.assume().t <= 1) {

            /* Convert the nearest point back to desktop coordinates */
            NR::Point const np_it = get_point_on_Path(*i, p.assume().piece, p.assume().t);
            NR::Point const d = sp_desktop_doc2d_xy_point(desktop, np_it * i2doc);
            
            NR::Coord const dist = NR::L2(d - req);
            if (dist < upper) {
                upper = best = dist;
                snapped = d;
            }
        }
    }

}


NR::Coord Inkscape::ObjectSnapper::do_vector_snap(NR::Point &req, NR::Point const &d,
                                                  std::list<SPItem const *> const &it) const
{
    return do_free_snap(req, it);
}


NR::Coord Inkscape::ObjectSnapper::do_free_snap(NR::Point &req, std::list<SPItem const *> const &it) const
{
    /* Get a list of all the SPItems that we will try to snap to */
    std::list<SPItem*> cand;
    _find_candidates(cand, sp_document_root(_named_view->document), it, req);

    /* Set to the snapped point, if we snap */
    NR::Point snapped = req;
    /* Distance to best snap point */
    NR::Coord best = NR_HUGE;
    /* Current upper limit for an acceptable snap */
    NR::Coord upper = getDistance();

    if (_snap_to_nodes) {
        _snap_nodes(snapped, best, upper, req, cand);
    }
    if (_snap_to_paths) {
        _snap_paths(snapped, best, upper, req, cand);
    }

    req = snapped;
    return best;
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
