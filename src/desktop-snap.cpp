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
 * Circular snap, path snap?
 */

#include <math.h>
#include <list>
#include <utility>
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-snap.h"
#include "geom.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-values.h>

static std::list<const Snapper*> namedview_get_snappers(SPNamedView const *nv);
static bool namedview_will_snap_something(SPNamedView const *nv);

/* Minimal distance to norm before point is considered for snap. */
static const double MIN_DIST_NORM = 1.0;

/**
 *    Try to snap `req' in one dimension.
 *
 *    \param nv NamedView to use.
 *    \param req Point to snap; updated to the snapped point if a snap occurred.
 *    \param dim Dimension to snap in.
 *    \return Distance to the snap point along the `dim' axis, or NR_HUGE
 *    if no snap occurred.
 */

NR::Coord namedview_dim_snap(SPNamedView const *nv, Snapper::PointType t, NR::Point &req, NR::Dim2 const dim)
{
    return namedview_vector_snap (nv, t, req, component_vectors[dim]);
}

/**
 *    Try to snap `req' in both dimensions.
 *
 *    \param nv NamedView to use.
 *    \param req Point to snap; updated to the snapped point if a snap occurred.
 *    \return Distance to the snap point, or NR_HUGE if no snap occurred.
 */

NR::Coord namedview_free_snap(SPNamedView const *nv, Snapper::PointType t, NR::Point& req)
{
    /* fixme: If allowing arbitrary snap targets, free snap is not the sum of h and v */
    NR::Point result = req;
	
    NR::Coord dh = namedview_dim_snap(nv, t, result, NR::X);
    result[NR::Y] = req[NR::Y];
    NR::Coord dv = namedview_dim_snap(nv, t, result, NR::Y);
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



/**
 * Look for snap point along the line described by the point \a req
 * and the direction vector \a d.
 * Modifies req to the snap point, if one is found.
 * \return The distance from \a req to the snap point along the vector \a d,
 * or NR_HUGE if no snap point was found.
 *
 * Requires: d != (0, 0).
 */

NR::Coord namedview_vector_snap (SPNamedView const *nv, Snapper::PointType t, NR::Point &req, NR::Point const &d)
{
    g_assert(nv != NULL);
    g_assert(SP_IS_NAMEDVIEW(nv));

    std::list<const Snapper*> snappers = namedview_get_snappers(nv);

    NR::Coord best = NR_HUGE;
    for (std::list<const Snapper*>::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        NR::Point trial_req = req;
        NR::Coord dist = (*i)->vector_snap(t, trial_req, d);

        if (dist < best) {
            req = trial_req;
            best = dist;
        }
    }

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

std::pair<NR::Coord, bool> namedview_dim_snap_list(SPNamedView const *nv,
                                                    Snapper::PointType t, const std::vector<NR::Point> &p,
                                                    NR::Coord const dx, NR::Dim2 const dim)
{
    NR::Coord dist = NR_HUGE;
    NR::Coord xdist = dx;
    
    if (namedview_will_snap_something(nv)) {
        for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
            NR::Point q = *i;
            NR::Coord const pre = q[dim];
            q[dim] += dx;
            NR::Coord const d = namedview_dim_snap(nv, t, q, dim);
            if (d < dist) {
                xdist = q[dim] - pre;
                dist = d;
            }
        }
    }

    return std::make_pair(xdist, dist < NR_HUGE);
}

double namedview_vector_snap_list(SPNamedView const *nv, Snapper::PointType t, const std::vector<NR::Point> &p,
				   NR::Point const &norm, NR::scale const &s)
{
    using NR::X;
    using NR::Y;

    if (namedview_will_snap_something(nv) == false) {
        return s[X];
    }
    
    NR::Coord dist = NR_HUGE;
    double ratio = fabs(s[X]);
    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point const &q = *i;
        NR::Point check = ( q - norm ) * s + norm;
        if (NR::LInfty( q - norm ) > MIN_DIST_NORM) {
            NR::Coord d = namedview_vector_snap(nv, t, check, check - norm);
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

double namedview_dim_snap_list_scale(SPNamedView const *nv, Snapper::PointType t, const std::vector<NR::Point> &p,
				      NR::Point const &norm, double const sx, NR::Dim2 dim)
{
    if (namedview_will_snap_something(nv) == false) {
        return sx;
    }

    g_assert(dim < 2);

    NR::Coord dist = NR_HUGE;
    double scale = sx;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;
        check[dim] = (sx * (q - norm) + norm)[dim];
        if (fabs (q[dim] - norm[dim]) > MIN_DIST_NORM) {
            const gdouble d = namedview_dim_snap (nv, t, check, dim);
            if (d < NR_HUGE && d < dist) {
                dist = d;
                scale = (check[dim] - norm[dim]) / (q[dim] - norm[dim]);
            }
        }
    }

    return scale;
}

double namedview_dim_snap_list_skew(SPNamedView const *nv, Snapper::PointType t, const std::vector<NR::Point> &p,
				     NR::Point const &norm, double const sx, NR::Dim2 const dim)
{
    if (namedview_will_snap_something(nv) == false) {
        return sx;
    }

    g_assert(dim < 2);

    gdouble dist = NR_HUGE;
    gdouble skew = sx;
    
    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;
        // apply shear
        check[dim] += sx * (q[!dim] - norm[!dim]);
        if (fabs (q[!dim] - norm[!dim]) > MIN_DIST_NORM) {
            const gdouble d = namedview_dim_snap (nv, t, check, dim);
            if (d < NR_HUGE && d < fabs (dist)) {
                dist = d;
                skew = (check[dim] - q[dim]) / (q[!dim] - norm[!dim]);
            }
        }
    }

    return skew;
}


/* FIXME: this should probably be in SPNamedView */
static std::list<const Snapper*> namedview_get_snappers(SPNamedView const *nv)
{
    std::list<const Snapper*> s;
    s.push_back(&nv->grid_snapper);
    s.push_back(&nv->guide_snapper);
    return s;
}

bool namedview_will_snap_something(SPNamedView const *nv)
{
    std::list<const Snapper*> s = namedview_get_snappers(nv);
    std::list<const Snapper*>::iterator i = s.begin();
    while (i != s.end() && (*i)->will_snap_something() == false) {
        i++;
    }

    return (i != s.end());
}
            


Snapper::Snapper(SPNamedView const *nv, NR::Coord const d) : _named_view(nv), _distance(d), _enabled(false)
{
    g_assert (_named_view != NULL);
    g_assert (SP_IS_NAMEDVIEW (_named_view));
    setSnapTo(BBOX_POINT, true);
}

void Snapper::setEnabled(bool s)
{
    _enabled = s;
}

bool Snapper::getEnabled() const
{
    return _enabled;
}

void Snapper::setDistance(NR::Coord const d)
{
    _distance = d;
}

NR::Coord Snapper::getDistance() const
{
    return _distance;
}

void Snapper::setSnapTo(PointType t, bool s)
{
    _snap_to[t] = s;
}

bool Snapper::getSnapTo(PointType t) const
{
    std::map<PointType, bool>::const_iterator i = _snap_to.find(t);
    if (i == _snap_to.end()) {
        return false;
    }

    return i->second;
}



/**
 *  Add some multiple of \a mv to \a req to make it line on the line {p : dot(n, p) == d} (within
 *  rounding error); unless that isn't possible (e.g. \a mv and \a n are orthogonal, or \a mv or \a
 *  n is zero-length), in which case \a req remains unchanged, and a big number is returned.
 *
 *  Returns a badness measure of snapping to the specified line: if snapping was possible then
 *  L2(req - req0) (i.e. the distance moved); otherwise returns NR_HUGE;
 */

NR::Coord Snapper::intersector_a_vector_snap(NR::Point &req, NR::Point const &mv,
                                             NR::Point const &n, NR::Coord const d) const
{
    NR::Point const req0(req);
    /* Implement "move from req0 by some multiple of mv" as "dot product with something
       orthogonal to mv remains unchanged". */
    NR::Point const n2(rot90(mv));
    NR::Coord const d2 = dot(n2, req);
    if (sp_intersector_line_intersection(n2, d2, n, d, req) == intersects) {
        return L2(req - req0);
    } else {
        return NR_HUGE;
    }
}


/**
 *  \return true if this Snapper will snap at least one kind of point.
 */

bool Snapper::will_snap_something() const
{
    if (_enabled == false) {
        return false;
    }

    std::map<PointType, bool>::const_iterator i = _snap_to.begin();
    while (i != _snap_to.end() && i->second == false) {
        i++;
    }

    return (i != _snap_to.end());
}


GridSnapper::GridSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

NR::Coord GridSnapper::vector_snap(PointType t, NR::Point &req, NR::Point const &d) const
{
    if (getEnabled() == false || getSnapTo(t) == false) {
        return NR_HUGE;
    }
    
    NR::Coord len = L2(d);
    if (len < NR_EPSILON) {
        return namedview_free_snap(_named_view, t, req);
    }

    NR::Point const v = NR::unit_vector(d);

    NR::Point snapped = req;
    NR::Coord best = NR_HUGE;
    NR::Coord upper = NR_HUGE;

    /*  find nearest grid line (either H or V whatever is closer) along
     *  the vector to the requested point.  If the distance along the
     *  vector is less than the snap distance then snap.
     */
    upper = MIN(best, getDistance());
        
    for (unsigned int i = 0; i < 2; ++i) {
        NR::Point trial(req);
        NR::Coord const rounded = round_to_nearest_multiple_plus(req[i],
                                                                 _named_view->gridspacing[i],
                                                                 _named_view->gridorigin[i]);
            
        NR::Coord const dist = intersector_a_vector_snap (trial,
                                                          v,
                                                          component_vectors[i],
                                                          rounded);
        
        if (dist < upper) {
            upper = best = dist;
            snapped = trial;
        }
    }

    req = snapped;
    return best;
}

GuideSnapper::GuideSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

NR::Coord GuideSnapper::vector_snap(PointType t, NR::Point &req, NR::Point const &d) const
{
    if (getEnabled() == false || getSnapTo(t) == false) {
        return NR_HUGE;
    }
    
    NR::Coord len = L2(d);
    if (len < NR_EPSILON) {
        return namedview_free_snap (_named_view, t, req);
    }

    NR::Point const v = NR::unit_vector(d);

    NR::Point snapped = req;
    NR::Coord best = NR_HUGE;
    NR::Coord upper = NR_HUGE;

    upper = _named_view->guide_snapper.getDistance();
    for (GSList const *l = _named_view->guides; l != NULL; l = l->next) {
        SPGuide const &g = *SP_GUIDE(l->data);
        NR::Point trial(req);
        NR::Coord const dist = intersector_a_vector_snap(trial,
                                                         v,
                                                         g.normal,
                                                         g.position);
        
        if (dist < upper) {
            upper = best = dist;
            snapped = trial;
        }
    }

    req = snapped;
    return best;
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
