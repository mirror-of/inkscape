#ifndef SEEN_SNAPPER_H
#define SEEN_SNAPPER_H

/**
 *    \file src/snapper.h
 *    \brief Snapper class.
 *
 *    Authors:
 *      Carl Hetherington <inkscape@carlh.net>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <map>
#include <list>
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"

struct SPNamedView;
struct SPItem;

namespace Inkscape
{

/// Parent for classes that can snap points to something
class Snapper
{
public:
    Snapper(SPNamedView const *nv, NR::Coord const d);
    virtual ~Snapper() {}

    /// Point types to snap.
    enum PointType {
        SNAP_POINT,
        BBOX_POINT
    };

    typedef std::pair<PointType, NR::Point> PointWithType;

    void setSnapTo(PointType t, bool s);
    void setDistance(NR::Coord d);

    bool getSnapTo(PointType t) const;
    NR::Coord getDistance() const;

    bool will_snap_something() const;

    NR::Coord free_snap(PointType t,
                        NR::Point &req,
                        std::list<SPItem const *> const &it) const;

    NR::Coord vector_snap(PointType t,
                          NR::Point &req,
                          NR::Point const &d,
                          std::list<SPItem const *> const &it) const;
protected:
    SPNamedView const *_named_view;
    
private:

    /**
     *  Try to perform a "free snap" of a point.  Given a point, this method will
     *  try to snap it to whatever the snapper is interested in (grid, guides, whatever).
     *
     *  \param req Point to snap (desktop coordinates).  Filled in with the snapped point,
     *  if one is found.
     *  \param it Items that should not be snapped to.
     *  \return Distance (desktop coordinates) from the original to the snapped point, or NR_HUGE.
     */
    virtual NR::Coord do_free_snap(NR::Point &req,
                                   std::list<SPItem const *> const &it) const = 0;

    /**
     *  Try to perform a "vector snap" of a point.  Given a point, this method will
     *  try to snap it to whatever the snapper is interested in (grid, guides, whatever).
     *  FIXME: I am not convinced that this vector snapping is necessary, and it may soon
     *  be removed.
     *
     *  \param req Point to snap (desktop coordinates).  Filled in with the snapped point,
     *  if one is found.
     *  \param it Items that should not be snapped to.
     *  \return Distance (desktop coordinates) from the original to the snapped point, or NR_HUGE.
     */    
    virtual NR::Coord do_vector_snap(NR::Point &req,
                                     NR::Point const &d,
                                     std::list<SPItem const *> const &it) const = 0;
    
    NR::Coord _distance; ///< snap distance (desktop coordinates)
    std::map<PointType, bool> _snap_to; ///< point types that we will snap to
};

}

#endif /* !SEEN_SNAPPER_H */

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
