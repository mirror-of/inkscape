#ifndef SEEN_NR_RECT_OPS_H
#define SEEN_NR_RECT_OPS_H

/*
 * Rect operators
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <libnr/nr-forward.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-convex-hull.h>
#include <libnr/nr-matrix-ops.h>

namespace NR {

inline Rect expand(Rect const &r, double by) {
    NR::Point const p(by, by);
    return Rect(r.min() + p, r.max() - p);
}

inline Rect expand(Rect const &r, NR::Point by) {
    return Rect(r.min() + by, r.max() - by);
}

#if 0
inline ConvexHull operator*(Rect const &r, Matrix const &m) {
    /* FIXME: no mention of m.  Should probably be made non-inline. */
    ConvexHull points(r.corner(0));
    for ( unsigned i = 1 ; i < 4 ; i++ ) {
        points.add(r.corner(i));
    }
    return points;
}
#endif

inline Rect transform_rect(Rect const &r, NR::Matrix const &m) {
    NR::Point p1 = r.corner(1) * m;
    NR::Point p2 = r.corner(2) * m;
    NR::Point p3 = r.corner(3) * m;
    NR::Point p4 = r.corner(4) * m;
    return Rect (
        NR::Point(
            std::min(std::min(p1[NR::X], p2[NR::X]), std::min(p3[NR::X], p4[NR::X])), 
            std::min(std::min(p1[NR::Y], p2[NR::Y]), std::min(p3[NR::Y], p4[NR::Y]))), 
        NR::Point(
            std::max(std::max(p1[NR::X], p2[NR::X]), std::max(p3[NR::X], p4[NR::X])), 
            std::max(std::max(p1[NR::Y], p2[NR::Y]), std::max(p3[NR::Y], p4[NR::Y]))));
}

} /* namespace NR */


#endif /* !SEEN_NR_RECT_OPS_H */

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
