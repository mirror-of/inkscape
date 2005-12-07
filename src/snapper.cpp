/**
 *  \file src/snapper.cpp
 *  \brief Snapper class.
 *
 *  Authors:
 *    Carl Hetherington <inkscape@carlh.net>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "libnr/nr-values.h"
#include "sp-namedview.h"
#include "snapper.h"


/**
 *  Construct new Snapper for named view.
 *  \param nv Named view.
 *  \param d Snap distance.
 */
Inkscape::Snapper::Snapper(SPNamedView const *nv, NR::Coord const d) : _named_view(nv), _distance(d)
{
    g_assert(_named_view != NULL);
    g_assert(SP_IS_NAMEDVIEW(_named_view));
    
    setSnapTo(BBOX_POINT, true);
}

/**
 *  Set snap distance.
 *  \param d New snap distance (desktop coordinates)
 */
void Inkscape::Snapper::setDistance(NR::Coord const d)
{
    _distance = d;
}

/**
 *  \return Snap distance (desktop coordinates)
 */
NR::Coord Inkscape::Snapper::getDistance() const
{
    return _distance;
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Inkscape::Snapper::setSnapTo(PointType t, bool s)
{
    _snap_to[t] = s;
}

/**
 *  \param t Point type.
 *  \return true if snapper will snap this type of point, otherwise false.
 */
bool Inkscape::Snapper::getSnapTo(PointType t) const
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
bool Inkscape::Snapper::will_snap_something() const
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
 *  \param req Point (desktop coordinates).
 *  \param d Vector to snap along.
 *  \param it Items to ignore while snapping.
 *  \return Distance along d from the original to the snapped point.
 */

NR::Coord Inkscape::Snapper::vector_snap(PointType t, NR::Point &req, NR::Point const &d,
                               std::list<SPItem const *> const &it) const
{
    if (getSnapTo(t) == false) {
        return NR_HUGE;
    }

    return do_vector_snap(req, d, it);
}


/**
 *  Try to snap a point using this snapper.
 *  If a snap is made, req will be filled in with the snapped point.
 *  
 *  \param t Point type.
 *  \param req Point (desktop coordinates).
 *  \param it Items to ignore while snapping.
 *  \return Distance along d from the original to the snapped point.
 */

NR::Coord Inkscape::Snapper::free_snap(PointType t, NR::Point &req, std::list<SPItem const *> const &it) const
{
    if (getSnapTo(t) == false) {
        return NR_HUGE;
    }

    return do_free_snap(req, it);
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
