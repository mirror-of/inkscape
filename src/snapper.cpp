/**
 *  \file src/snapper.cpp
 *  \brief Snapper class.
 *
 *  Authors:
 *    Carl Hetherington <inkscape@carlh.net>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "libnr/nr-point-fns.h"
#include "libnr/nr-values.h"
#include "geom.h"
#include "sp-namedview.h"
#include "snapper.h"
#include "snap.h"


/**
 *  Construct new Snapper for named view.
 *  \param nv Named view.
 *  \param d Snap distance.
 */
Snapper::Snapper(SPNamedView const *nv, NR::Coord const d) : _named_view(nv), _distance(d)
{
    g_assert(_named_view != NULL);
    g_assert(SP_IS_NAMEDVIEW(_named_view));
    
    setSnapTo(BBOX_POINT, true);
}

/**
 *  Set snap distance.
 *  \param d New snap distance.
 */
void Snapper::setDistance(NR::Coord const d)
{
    _distance = d;
}

/**
 *  \return Snap distance.
 */
NR::Coord Snapper::getDistance() const
{
    return _distance;
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Snapper::setSnapTo(PointType t, bool s)
{
    _snap_to[t] = s;
}

/**
 *  \param t Point type.
 *  \return true if snapper will snap this type of point, otherwise false.
 */
bool Snapper::getSnapTo(PointType t) const
{
    std::map<PointType, bool>::const_iterator i = _snap_to.find(t);
    if (i == _snap_to.end()) {
        return false;
    }

    return i->second;
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Snapper::will_snap_something() const
{
    std::map<PointType, bool>::const_iterator i = _snap_to.begin();
    while (i != _snap_to.end() && i->second == false) {
        i++;
    }

    return (i != _snap_to.end());
}


/**
 *  Try to snap a point using this snapper along a vector.
 *  If a snap is made, req will be filled in with the snapped point.
 *  
 *  \param t Point type.
 *  \param req Point.
 *  \param d Vector to snap along.
 *  \return Distance along d from the original to the snapped point.
 */

NR::Coord Snapper::vector_snap(PointType t, NR::Point &req, NR::Point const &d) const
{
    if (getSnapTo(t) == false) {
        /* We're not interested in snapping this point type */
        return NR_HUGE;
    }

    /* Sanity check d */
    NR::Coord len = L2(d);
    if (len < NR_EPSILON) {
        return namedview_free_snap(_named_view, t, req);
    }

    NR::Point const v = NR::unit_vector(d);

    /* Set to the snapped point, if we snap */
    NR::Point snapped = req;
    /* Distance to best snap point */
    NR::Coord best = NR_HUGE;
    /* Current upper limit for an acceptable snap */
    NR::Coord upper = getDistance();

    /* Get the lines that we will try to snap to */
    const LineList s = get_snap_lines(req);

    for (LineList::const_iterator i = s.begin(); i != s.end(); i++) {

        NR::Point trial(req);

        /* Normal to the line we're trying to snap along */
        NR::Point const n2(NR::rot90(v));

        /* Hence constant term of the line we're trying to snap along */
        NR::Coord const d2 = dot(n2, req);

        /* Try to intersect this line with the target line */
        if (intersector_line_intersection(n2, d2, i->first, i->second, trial) == INTERSECTS) {
            const NR::Coord dist = L2(trial - req);
            if (dist < upper) {
                upper = best = dist;
                snapped = trial;
            }
        }
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
