#define __SP_DESKTOP_SNAP_C__

/*
 * Various snapping methods
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* TODO:
 * Sort out 1e18, 1e15 mess (handle large numbers gracefully
 * Circular snap, path snap?
 */

#include <math.h>
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-snap.h"
#include "desktop.h"
#include "geom.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-values.h>

/* Minimal distance to norm before point is considered for snap. */
#define MIN_DIST_NORM 1.0

#define SNAP_ON(d) (((d)->namedview->grid_snapper.getDistance() > 0.0) || ((d)->namedview->guide_snapper.getDistance() > 0.0))

/**
 *    Try to snap `req' in one dimension.
 *
 *    \param dt Desktop to use.
 *    \param req Point to snap; updated to the snapped point if a snap occurred.
 *    \param dim Dimension to snap in.
 *    \return Distance to the snap point along the `dim' axis, or NR_HUGE
 *    if no snap occurred.
 */

NR::Coord sp_desktop_dim_snap(SPDesktop const *dt, NR::Point &req, NR::Dim2 const dim)
{
    return sp_desktop_vector_snap (dt, req, component_vectors[dim]);
}

/**
 *    Try to snap `req' in both dimensions.
 *
 *    \param dt Desktop to use.
 *    \param req Point to snap; updated to the snapped point if a snap occurred.
 *    \return Distance to the snap point, or NR_HUGE if no snap occurred.
 */

NR::Coord sp_desktop_free_snap (SPDesktop const *desktop, NR::Point& req)
{
    /* fixme: If allowing arbitrary snap targets, free snap is not the sum of h and v */
    NR::Point result = req;
	
    NR::Coord dh = sp_desktop_dim_snap (desktop, result, NR::X);
    result[NR::Y] = req[NR::Y];
    NR::Coord dv = sp_desktop_dim_snap (desktop, result, NR::Y);
    req = result;
	
    if (dh < NR_HUGE && dv < NR_HUGE) {
        return hypot (dh, dv);
    }

    if (dh < NR_HUGE) {
        return dh;
    }

    if (dv < NR_HUGE) {
	return dv;
    }
    
    return NR_HUGE;
}

/** Add some multiple of \a mv to \a req to make it line on the line {p : dot(n, p) == d} (within
    rounding error); unless that isn't possible (e.g. \a mv and \a n are orthogonal, or \a mv or \a
    n is zero-length), in which case \a req remains unchanged, and a big number is returned.

    Returns a badness measure of snapping to the specified line: if snapping was possible then
    L2(req - req0) (i.e. the distance moved); otherwise returns a large number.
**/
static double sp_intersector_a_vector_snap(NR::Point &req, NR::Point const &mv,
					   NR::Point const &n, NR::Coord const d) {
    NR::Point const req0(req);
    /* Implement "move from req0 by some multiple of mv" as "dot product with something
       orthogonal to mv remains unchanged". */
    NR::Point const n2(rot90(mv));
    NR::Coord const d2 = dot(n2, req);
    if (sp_intersector_line_intersection(n2, d2, n, d, req) == intersects) {
        return L2(req - req0);
    } else {
        return 1e300;
    }
}

static double round_to_nearest_multiple_plus(double x, double const c1, double const c0);

/**
 * Look for snap point along the line described by the point \a req
 * and the direction vector \a d.
 * Modifies req to the snap point, if one is found.
 * \return The distance from \a req to the snap point along the vector \a d,
 * or NR_HUGE if no snap point was found.
 *
 *
 * Requires: d != (0, 0).
 */

NR::Coord sp_desktop_vector_snap (SPDesktop const *desktop, NR::Point &req, NR::Point const &d)
{
    NR::Coord best = NR_HUGE;
    NR::Coord upper = NR_HUGE;

    g_assert (desktop != NULL);
    g_assert (SP_IS_DESKTOP (desktop));

    NR::Coord len = L2(d);
    if (len < 1e-18) {
        return sp_desktop_free_snap (desktop, req);
    }
    
    NR::Point const v = NR::unit_vector(d);

    SPNamedView const &nv = *desktop->namedview;
    NR::Point snapped = req;

    if (nv.snaptoguides) {
        upper = nv.guide_snapper.getDistance();
        for (GSList const *l = nv.guides; l != NULL; l = l->next) {
            SPGuide const &g = *SP_GUIDE(l->data);
            NR::Point trial(req);
            NR::Coord const dist = sp_intersector_a_vector_snap(trial,
                                                                v,
                                                                g.normal,
                                                                g.position);
            
            if (dist < upper) {
                upper = best = dist;
                snapped = trial;
            }
        }
    }

    if (nv.snaptogrid) {
        /*  find nearest grid line (either H or V whatever is closer) along
         *  the vector to the requested point.  If the distance along the
         *  vector is less than the snap distance then snap.
         */
        upper = MIN(best, nv.grid_snapper.getDistance());
        
        for (unsigned int i = 0; i < 2; ++i) {
            NR::Point trial(req);
            NR::Coord const rounded = round_to_nearest_multiple_plus(req[i],
                                                                     nv.gridspacing[i],
                                                                     nv.gridorigin[i]);
            
            NR::Coord const dist = sp_intersector_a_vector_snap (trial,
                                                                 v,
                                                                 component_vectors[i],
                                                                 rounded);

            if (dist < upper) {
                upper = best = dist;
                snapped = trial;
            }
        }
    }
    
    req = snapped;
    return best;
}

/**
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimention".  We're currently discussing "good" semantics
 * for guide/grid snapping.
 */
static double
round_to_nearest_multiple_plus(double x, double const c1, double const c0)
{
    return floor((x - c0) / c1 + .5) * c1 + c0;
}


/* 
 * functions for lists of points
 *
 * All functions take a list of NR::Point and parameter indicating the proposed transformation.
 * They return the updated transformation parameter. 
 */

double sp_desktop_dim_snap_list(SPDesktop const *desktop, const std::vector<NR::Point> &p,
				double const dx, NR::Dim2 const dim)
{
    gdouble dist = NR_HUGE;
    gdouble xdist = dx;
    
    if (SNAP_ON (desktop)) {
        for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
            NR::Point q = *i;
            NR::Coord const pre = q[dim];
            q[dim] += dx;
            NR::Coord const d = sp_desktop_dim_snap (desktop, q, dim);
            if (d < dist) {
                xdist = q[dim] - pre;
                dist = d;
            }
        }
    }

    return xdist;
}

double sp_desktop_vector_snap_list(SPDesktop const *desktop, const std::vector<NR::Point> &p,
				   NR::Point const &norm, NR::scale const &s)
{
    using NR::X;
    using NR::Y;

    if (!SNAP_ON(desktop)) {
        return s[X];
    }
    
    NR::Coord dist = NR_HUGE;
    double ratio = fabs(s[X]);
    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point const &q = *i;
        NR::Point check = ( q - norm ) * s + norm;
        if (NR::LInfty( q - norm ) > MIN_DIST_NORM) {
            NR::Coord d = sp_desktop_vector_snap(desktop, check, check - norm);
            if ((d < NR_HUGE) && (d < dist)) {
                dist = d;
                NR::Dim2 const dominant = ( ( fabs( q[X] - norm[X] )  >
                                              fabs( q[Y] - norm[Y] ) )
                                            ? X
                                            : Y );
                ratio = ( ( check[dominant] - norm[dominant] )
                          / ( q[dominant] - norm[dominant] ) );
            }
        }
    }
    
    return ratio;
}

double sp_desktop_dim_snap_list_scale(SPDesktop const *desktop, const std::vector<NR::Point> &p,
				      NR::Point const &norm, double const sx, NR::Dim2 dim)
{
    g_assert( dim < 2 );
    if (!SNAP_ON (desktop)) {
        return sx;
    }

    NR::Coord dist = NR_HUGE;
    double scale = sx;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;
        check[dim] = (sx * (q - norm) + norm)[dim];
        if (fabs (q[dim] - norm[dim]) > MIN_DIST_NORM) {
            const gdouble d = sp_desktop_dim_snap (desktop, check, dim);
            if (d < NR_HUGE && d < dist) {
                dist = d;
                scale = (check[dim] - norm[dim]) / (q[dim] - norm[dim]);
            }
        }
    }

    return scale;
}

double sp_desktop_dim_snap_list_skew(SPDesktop const *desktop, const std::vector<NR::Point> &p,
				     NR::Point const &norm, double const sx, NR::Dim2 const dim)
{
    g_assert( dim < 2 );
    if (!SNAP_ON (desktop)) {
        return sx;
    }

    gdouble dist = NR_HUGE;
    gdouble skew = sx;
    
    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;
        // apply shear
        check[dim] += sx * (q[!dim] - norm[!dim]);
        if (fabs (q[!dim] - norm[!dim]) > MIN_DIST_NORM) {
            const gdouble d = sp_desktop_dim_snap (desktop, check, dim);
            if (d < NR_HUGE && d < fabs (dist)) {
                dist = d;
                skew = (check[dim] - q[dim]) / (q[!dim] - norm[!dim]);
            }
        }
    }

    return skew;
}


Snapper::Snapper(NR::Coord const d) : _distance(d)
{

}

void Snapper::setDistance(NR::Coord const d)
{
    _distance = d;
}

NR::Coord Snapper::getDistance() const
{
    return _distance;
}

GridSnapper::GridSnapper(NR::Coord const d) : Snapper(d)
{

}

GuideSnapper::GuideSnapper(NR::Coord const d) : Snapper(d)
{

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
