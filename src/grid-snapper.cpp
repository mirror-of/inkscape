/**
 *  \file grid-snapper.cpp
 *  \brief Snapping things to grids.
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

#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "sp-namedview.h"
#include "grid-snapper.h"
#include "snap.h"

/**
 * \return x rounded to the nearest multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimention".  We're currently discussing "good" semantics
 * for guide/grid snapping.
 */
static double round_to_nearest_multiple_plus(double x, double const c1, double const c0)
{
    return floor((x - c0) / c1 + .5) * c1 + c0;
}

GridSnapper::GridSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

/**
 * Try to snap point.
 * \return Movement vector or NR_HUGE.
 */
NR::Coord GridSnapper::vector_snap(PointType t, NR::Point &req, NR::Point const &d) const
{
    if (getSnapTo(t) == false) {
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
            
        NR::Coord const dist = intersector_a_vector_snap(trial,
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
