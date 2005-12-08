#define __SP_DESKTOP_SNAP_C__

/**
 * \file snap.cpp
 *
 * \brief Various snapping methods
 * \todo Circular snap, path snap?
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>
#include <math.h>
#include <list>
#include <utility>
#include "sp-guide.h"
#include "sp-namedview.h"
#include "snap.h"
#include "geom.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-values.h>

static std::list<const Inkscape::Snapper*> namedview_get_snappers(SPNamedView const *nv);
static bool namedview_will_snap_something(SPNamedView const *nv);

/// Minimal distance to norm before point is considered for snap.
static const double MIN_DIST_NORM = 1.0;

/**
 * Try to snap \a req in one dimension.
 *
 * \param nv NamedView to use.
 * \param req Point to snap; updated to the snapped point if a snap occurred.
 * \param dim Dimension to snap in.
 * \return Distance to the snap point along the \a dim axis, or \c NR_HUGE
 *    if no snap occurred.
 */
NR::Coord namedview_dim_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point &req,
                             NR::Dim2 const dim, SPItem const *it)
{
    return namedview_vector_snap(nv, t, req, component_vectors[dim], it);
}

NR::Coord namedview_dim_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point &req,
                             NR::Dim2 const dim, std::list<SPItem const *> const &it)
{
    return namedview_vector_snap(nv, t, req, component_vectors[dim], it);
}

/**
 * Try to snap \a req in both dimensions.
 *
 * \param nv NamedView to use.
 * \param req Point to snap; updated to the snapped point if a snap occurred.
 * \return Distance to the snap point, or \c NR_HUGE if no snap occurred.
 */
NR::Coord namedview_free_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                              NR::Point& req, SPItem const *it)
{
    g_assert(nv != NULL);
    g_assert(SP_IS_NAMEDVIEW(nv));

    std::list<const Inkscape::Snapper*> snappers = namedview_get_snappers(nv);

    NR::Coord best = NR_HUGE;
    for (std::list<const Inkscape::Snapper*>::const_iterator i = snappers.begin(); i != snappers.end(); i++) {

        Inkscape::SnappedPoint const s = (*i)->freeSnap(t, req, it);
        if (s.second < best) {
            req = s.first;
            best = s.second;
        }
    }

    return best;
}


NR::Coord namedview_vector_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                NR::Point &req, NR::Point const &d,
                                SPItem const *it)
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return namedview_vector_snap(nv, t, req, d, lit);
}

/**
 * Look for snap point along the line described by the point \a req
 * and the direction vector \a d.
 * Modifies req to the snap point, if one is found.
 * \return The distance from \a req to the snap point along the vector \a d,
 * or \c NR_HUGE if no snap point was found.
 *
 * \pre d Å‚âÅ† (0, 0).
 */
NR::Coord namedview_vector_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                NR::Point &req, NR::Point const &d,
                                std::list<SPItem const *> const &it)
{
    g_assert(nv != NULL);
    g_assert(SP_IS_NAMEDVIEW(nv));

    std::list<const Inkscape::Snapper*> snappers = namedview_get_snappers(nv);

    NR::Coord best = NR_HUGE;
    for (std::list<const Inkscape::Snapper*>::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        Inkscape::SnappedPoint const s = (*i)->constrainedSnap(t, req, d, it);
        if (s.second < best) {
            req = s.first;
            best = s.second;
        }
    }

    return best;
}


/*
 * functions for lists of points
 *
 * All functions take a list of NR::Point and parameter indicating the proposed transformation.
 * They return the updated transformation parameter. 
 */

/**
 * Snap list of points in one dimension.
 * \return Coordinate difference.
 */
std::pair<NR::Coord, bool> namedview_dim_snap_list(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                                   const std::vector<NR::Point> &p,
                                                   NR::Coord const dx, NR::Dim2 const dim,
                                                   std::list<SPItem const *> const &it
                                                   )
{
    NR::Coord dist = NR_HUGE;
    NR::Coord xdist = dx;
    
    if (namedview_will_snap_something(nv)) {
        for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
            NR::Point q = *i;
            NR::Coord const pre = q[dim];
            q[dim] += dx;
            NR::Coord const d = namedview_dim_snap(nv, t, q, dim, it);
            if (d < dist) {
                xdist = q[dim] - pre;
                dist = d;
            }
        }
    }

    return std::make_pair(xdist, dist < NR_HUGE);
}

/**
 * Snap list of points in two dimensions.
 */
std::pair<double, bool> namedview_vector_snap_list(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                                   const std::vector<NR::Point> &p, NR::Point const &norm, 
                                                   NR::scale const &s, std::list<SPItem const *> const &it)
{
    using NR::X;
    using NR::Y;

    if (namedview_will_snap_something(nv) == false) {
        return std::make_pair(s[X], false);
    }
    
    NR::Coord dist = NR_HUGE;
    double ratio = fabs(s[X]);
    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point const &q = *i;
        NR::Point check = ( q - norm ) * s + norm;
        if (NR::LInfty( q - norm ) > MIN_DIST_NORM) {
            NR::Coord d = namedview_vector_snap(nv, t, check, check - norm, it);
            if (d < dist) {
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
    
    return std::make_pair(ratio, dist < NR_HUGE);
}


/**
 * Try to snap points in \a p after they have been scaled by \a sx with respect to
 * the origin \a norm.  The best snap is the one that changes the scale least.
 *
 * \return Pair containing snapped scale and a flag which is true if a snap was made.
 */
std::pair<double, bool> namedview_dim_snap_list_scale(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                                      const std::vector<NR::Point> &p, NR::Point const &norm, 
                                                      double const sx, NR::Dim2 dim,
                                                      std::list<const SPItem *> const &it)
{
    if (namedview_will_snap_something(nv) == false) {
        return std::make_pair(sx, false);
    }

    g_assert(dim < 2);

    NR::Coord dist = NR_HUGE;
    double scale = sx;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;

        /* Scaled version of the point we are looking at */
        check[dim] = (sx * (q - norm) + norm)[dim];
        
        if (fabs (q[dim] - norm[dim]) > MIN_DIST_NORM) {
            /* Snap this point */
            const NR::Coord d = namedview_dim_snap (nv, t, check, dim, it);
            /* Work out the resulting scale factor */
            double snapped_scale = (check[dim] - norm[dim]) / (q[dim] - norm[dim]);
            
            if (dist == NR_HUGE || fabs(snapped_scale - sx) < fabs(scale - sx)) {
                /* This is either the first point, or the snapped scale
                ** is the closest yet to the original.
                */
                scale = snapped_scale;
                dist = d;
            }
        }
    }

    return std::make_pair(scale, dist < NR_HUGE);
}

/**
 * Try to snap points after they have been skewed.
 */
double namedview_dim_snap_list_skew(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                    const std::vector<NR::Point> &p, NR::Point const &norm, 
                                    double const sx, NR::Dim2 const dim)
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
            const gdouble d = namedview_dim_snap (nv, t, check, dim, NULL);
            if (d < fabs (dist)) {
                dist = d;
                skew = (check[dim] - q[dim]) / (q[!dim] - norm[!dim]);
            }
        }
    }

    return skew;
}

/**
 * Return list of all snappers.
 */
static std::list<const Inkscape::Snapper*> namedview_get_snappers(SPNamedView const *nv)
{
/// \todo FIXME: this should probably be in SPNamedView
    std::list<const Inkscape::Snapper*> s;
    s.push_back(&nv->grid_snapper);
    s.push_back(&nv->guide_snapper);
    s.push_back(&nv->object_snapper);
    return s;
}

/**
 * True if one of the snappers in the named view's list will snap something.
 */
bool namedview_will_snap_something(SPNamedView const *nv)
{
    std::list<const Inkscape::Snapper*> s = namedview_get_snappers(nv);
    std::list<const Inkscape::Snapper*>::iterator i = s.begin();
    while (i != s.end() && (*i)->willSnapSomething() == false) {
        i++;
    }

    return (i != s.end());
}



/**
 * Snap in two dimensions to nearest snapper regardless of point type.
 */
NR::Coord namedview_free_snap_all_types(SPNamedView const *nv, NR::Point &req, SPItem const *it)
{
    NR::Point snap_req = req;
    NR::Coord snap_dist = namedview_free_snap(nv, Inkscape::Snapper::SNAP_POINT, snap_req, it);
    NR::Point bbox_req = req;
    NR::Coord bbox_dist = namedview_free_snap(nv, Inkscape::Snapper::BBOX_POINT, bbox_req, it);
    
    req = snap_dist < bbox_dist ? snap_req : bbox_req;
    return std::min(snap_dist, bbox_dist);
}


/**
 * Snap in one direction to nearest snapper regardless of point type.
 */
NR::Coord namedview_vector_snap_all_types(SPNamedView const *nv,
                                          NR::Point &req,
                                          NR::Point const &d,
                                          SPItem const *it)
{
    NR::Point snap_req = req;
    NR::Coord snap_dist = namedview_vector_snap(nv, Inkscape::Snapper::SNAP_POINT, snap_req, d, it);
    NR::Point bbox_req = req;
    NR::Coord bbox_dist = namedview_vector_snap(nv, Inkscape::Snapper::BBOX_POINT, bbox_req, d, it);

    req = snap_dist < bbox_dist ? snap_req : bbox_req;
    return std::min(snap_dist, bbox_dist);
}

/**
 * Snap in one dimension to nearest snapper regardless of point type.
 */
NR::Coord namedview_dim_snap_all_types(SPNamedView const *nv,
                                       NR::Point &req,
                                       NR::Dim2 const dim,
                                       SPItem const *it)
{
    NR::Point snap_req = req;
    NR::Coord snap_dist = namedview_dim_snap(nv, Inkscape::Snapper::SNAP_POINT, snap_req, dim, it);
    NR::Point bbox_req = req;
    NR::Coord bbox_dist = namedview_dim_snap(nv, Inkscape::Snapper::BBOX_POINT, bbox_req, dim, it);

    req = snap_dist < bbox_dist ? snap_req : bbox_req;
    return std::min(snap_dist, bbox_dist);
}

std::pair<int, NR::Point> namedview_free_snap(SPNamedView const *nv,
                                              std::vector<Inkscape::Snapper::PointWithType> const &p,
                                              SPItem const *it)
{
    NR::Coord best = NR_HUGE;
    std::pair<int, NR::Point> s = std::make_pair(-1, NR::Point(0, 0));
    
    for (int i = 0; i < int(p.size()); i++) {
        NR::Point r = p[i].second;
        NR::Coord const d = namedview_free_snap(nv, p[i].first, r, it);
        if (d < best) {
            best = d;
            s = std::make_pair(i, r);
        }
    }

    return s;
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
