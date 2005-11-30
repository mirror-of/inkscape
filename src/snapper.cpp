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
 *  Return a badness measure of snapping to the specified line.
 * 
 *  Add some multiple of \a mv to \a req to make it line on the line {p : dot(n, p) == d} (within
 *  rounding error); unless that isn't possible (e.g.\ \a mv and \a n are orthogonal, or \a mv or \a
 *  n is zero-length), in which case \a req remains unchanged, and a big number is returned.
 *
 *  \return a badness measure of snapping to the specified line: if snapping was possible then
 *  L2(req - req0) (i.e. the distance moved); otherwise returns NR_HUGE.
 */
NR::Coord Snapper::intersector_a_vector_snap(NR::Point &req, NR::Point const &mv,
                                             NR::Point const &n, NR::Coord const d) const
{
    NR::Point const req0(req);
    /* Implement "move from req0 by some multiple of mv" as "dot product with something
       orthogonal to mv remains unchanged". */
    NR::Point const n2(NR::rot90(mv));
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
    std::map<PointType, bool>::const_iterator i = _snap_to.begin();
    while (i != _snap_to.end() && i->second == false) {
        i++;
    }

    return (i != _snap_to.end());
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
