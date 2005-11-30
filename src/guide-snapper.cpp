/**
 *  \file guide-snapper.cpp
 *  \brief Snapping things to guides.
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
#include "sp-guide.h"
#include "snap.h"
#include "guide-snapper.h"

GuideSnapper::GuideSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

/**
 * Try to snap point.
 * \return Movement vector or NR_HUGE.
 */
NR::Coord GuideSnapper::vector_snap(PointType t, NR::Point &req, NR::Point const &d) const
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

    upper = _named_view->guide_snapper.getDistance();
    for (GSList const *l = _named_view->guides; l != NULL; l = l->next) {
        SPGuide const &g = *SP_GUIDE(l->data);
        NR::Point trial(req);
        NR::Coord const dist = intersector_a_vector_snap(trial,
                                                         v,
                                                         g.normal,
                                                         g.position);
        
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
