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

#include <forward.h>
#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-forward.h>


/* FIXME: these classes are work-in-progress */
/* Consider moving Snapper etc. to separate files if this reduces the number of translation units
 * that #include these class definitions.  Doing so reduces the amount of work of the compiler, and
 * reduces the number of recompilations necessary.
 *
 * Also, I'd suggest use of the standard modeline stuff (see end of this file) to make it easier to
 * conform to Inkscape's indentation standards.
 *
 * -- pjrm
 */

///< Parent for classes that can snap points to something
class Snapper
{
public:
    Snapper(NR::Coord const d);

    void setDistance(NR::Coord const d);

    NR::Coord getDistance() const;

private:
    NR::Coord _distance;
};

///< Snap to grid
class GridSnapper : public Snapper
{
public:
    GridSnapper(NR::Coord const d);
};

///< Snap to guides
class GuideSnapper : public Snapper
{
public:
    GuideSnapper(NR::Coord const d);
};


/* Single point methods */
double sp_desktop_free_snap(SPDesktop const *desktop, NR::Point &req);
double sp_desktop_vector_snap(SPDesktop const *desktop, NR::Point &req, NR::Point const &d);

gdouble sp_desktop_dim_snap(SPDesktop const *dt, NR::Point& req, NR::Dim2 const dim);

/* List of points methods */

NR::Coord sp_desktop_vector_snap_list(SPDesktop const *desktop, const std::vector<NR::Point> &p,
                                      NR::Point const &norm, NR::scale const &s);

NR::Coord sp_desktop_dim_snap_list(SPDesktop const *desktop, const std::vector<NR::Point> &p,
                                   double const dx, unsigned const dim);

NR::Coord sp_desktop_dim_snap_list_scale(SPDesktop const *desktop, const std::vector<NR::Point> &p,
                                         NR::Point const &norm, double const sx, NR::Dim2 const dim);

NR::Coord sp_desktop_dim_snap_list_skew(SPDesktop const *desktop, const std::vector<NR::Point> &p,
                                        NR::Point const &norm, double const sx, NR::Dim2 const dim);

// These little functions are only here to provide an edge between NR::Point-land and NR::Point-land.

NR::Coord sp_desktop_dim_snap_list(SPDesktop const *desktop, const std::vector<NR::Point> &p,
                                   double const dx, NR::Dim2 const dim);


/* Single point methods */
double sp_desktop_free_snap(SPDesktop const *desktop, NR::Point *req);
double sp_desktop_vector_snap(SPDesktop const *desktop, NR::Point *req, NR::Coord dx, NR::Coord dy);


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
