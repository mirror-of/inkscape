#ifndef SEEN_DESKTOP_SNAP_H
#define SEEN_DESKTOP_SNAP_H

/*
 * Snap distance calculation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include <map>

#include <forward.h>
#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-forward.h>


/* Consider moving Snapper etc. to separate files if this reduces the number of translation units
 * that #include these class definitions.  Doing so reduces the amount of work of the compiler, and
 * reduces the number of recompilations necessary.
 * -- pjrm
 */

///< Parent for classes that can snap points to something
class Snapper
{
public:
    Snapper(SPNamedView const *nv, NR::Coord const d);
    virtual ~Snapper() {}

    enum PointType {
        SNAP_POINT,
        BBOX_POINT
    };

    void setSnapTo(PointType t, bool s);
    void setEnabled(bool s);
    void setDistance(NR::Coord d);

    bool getSnapTo(PointType t) const;
    bool getSnapToBBox() const;
    bool getEnabled() const;
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
    bool _enabled;
    std::map<PointType, bool> _snap_to;
};


///< Snap to grid
class GridSnapper : public Snapper
{
public:
    GridSnapper(SPNamedView const *nv, NR::Coord const d);

    NR::Coord vector_snap(PointType t,
                          NR::Point &req,
                          NR::Point const &d) const;
    
};


///< Snap to guides
class GuideSnapper : public Snapper
{
public:
    GuideSnapper(SPNamedView const *nv, NR::Coord const d);

    NR::Coord vector_snap(PointType t,
                          NR::Point &req,
                          NR::Point const &d) const;
};


/* Single point methods */
NR::Coord namedview_free_snap(SPNamedView const *nv, Snapper::PointType t, NR::Point &req);
NR::Coord namedview_vector_snap(SPNamedView const *nv, Snapper::PointType t, NR::Point &req, NR::Point const &d);
NR::Coord namedview_dim_snap(SPNamedView const *nv, Snapper::PointType t, NR::Point& req, NR::Dim2 const dim);

/* List of points methods */

NR::Coord namedview_vector_snap_list(SPNamedView const *nv, Snapper::PointType t, const std::vector<NR::Point> &p,
                                     NR::Point const &norm, NR::scale const &s);

std::pair<NR::Coord, bool> namedview_dim_snap_list(SPNamedView const *nv,
                                                   Snapper::PointType t, const std::vector<NR::Point> &p,
                                                   double const dx, NR::Dim2 const dim);

std::pair<double, bool> namedview_dim_snap_list_scale(SPNamedView const *nv,
                                                      Snapper::PointType t, const std::vector<NR::Point> &p,
                                                      NR::Point const &norm, double const sx, NR::Dim2 const dim);

NR::Coord namedview_dim_snap_list_skew(SPNamedView const *nv, Snapper::PointType t, const std::vector<NR::Point> &p,
                                       NR::Point const &norm, double const sx, NR::Dim2 const dim);


/* FIXME:
** Temporary methods.  These snap a single point as if it were both a snap point
** and a bbox point.  Any callers of these methods should instead be calling
** namedview_*_snap twice; once with a snap point and once with the corresponding
** bbox point (taking, e.g., stroke width into account).  However in some cases
** working out the bbox point is hard, so these methods are a hack for the
** 0.39 release.  They should be removed.
*/
NR::Coord namedview_free_snap_all_types(SPNamedView const *nv, NR::Point &req);
NR::Coord namedview_vector_snap_all_types(SPNamedView const *nv, NR::Point &req, NR::Point const &d);


#endif /* !SEEN_DESKTOP_SNAP_H */

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
