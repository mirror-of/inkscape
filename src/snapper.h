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
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"

struct SPNamedView;

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

    void setSnapTo(PointType t, bool s);
    void setDistance(NR::Coord d);

    bool getSnapTo(PointType t) const;
    NR::Coord getDistance() const;

    bool will_snap_something() const;

    virtual NR::Coord vector_snap(PointType t,
                                  NR::Point &req,
                                  NR::Point const &d) const = 0;
protected:

    NR::Coord intersector_a_vector_snap(NR::Point &req, NR::Point const &mv,
                                        NR::Point const &n, NR::Coord const d) const;

    SPNamedView const *_named_view;
    
private:
    NR::Coord _distance;
    std::map<PointType, bool> _snap_to;
};


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
