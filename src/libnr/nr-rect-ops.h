#ifndef SEEN_NR_RECT_OPS_H
#define SEEN_NR_RECT_OPS_H

/* ex:set et ts=4 sw=4: */

/*
 * Rect operators
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-convex-hull.h>

namespace NR {

ConvexHull operator*(Rect const &r, Matrix const &m) {
    ConvexHull points(r.corner(0));
    for ( unsigned i = 1 ; i < 4 ; i++ ) {
        points.add(r.corner(i));
    }
    return points;
}

} /* namespace NR */

#endif
