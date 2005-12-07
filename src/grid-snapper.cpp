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

/**
 * \return x rounded to the nearest multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimention".  We're currently discussing "good" semantics
 * for guide/grid snapping.
 */

/* FIXME: move this somewhere else, perhaps */
static double round_to_nearest_multiple_plus(double x, double const c1, double const c0)
{
    return floor((x - c0) / c1 + .5) * c1 + c0;
}

Inkscape::GridSnapper::GridSnapper(SPNamedView const *nv, NR::Coord const d) : LineSnapper(nv, d)
{

}

Inkscape::LineSnapper::LineList Inkscape::GridSnapper::get_snap_lines(NR::Point const &p) const
{
    LineList s;
    
    for (unsigned int i = 0; i < 2; ++i) {
        NR::Coord const rounded = round_to_nearest_multiple_plus(p[i],
                                                                 _named_view->gridspacing[i],
                                                                 _named_view->gridorigin[i]);
        
        s.push_back(std::make_pair(component_vectors[i], rounded));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
