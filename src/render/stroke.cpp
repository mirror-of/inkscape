/*
 * Inkscape::Render::stroke - stroke a polygon
 *
 * Copyright 2004 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#include <math.h>
#include "render/bezier.h"
#include "render/stroke.h"
#include "libnr/nr-point.h"
#include "libnr/nr-point-fns.h"
#include "render/polygon.h"
#include "util/list.h"
#include "util/reverse.h"

namespace Inkscape {
namespace Render {

using Inkscape::Util::List;
using Inkscape::Util::reverse_in_place;
using NR::rot90;
using NR::cross;

namespace {


typedef void (*JoinFn)(Polygon::VertexList &from,
                       Polygon::VertexList &to,
                       NR::Point const &center,
                       double miter_limit,
                       double smoothness);

void join_bevel(Polygon::VertexList &from,
                Polygon::VertexList &to,
                NR::Point const &center,
                double miter_limit,
                double smoothness)
{
    to.setNext(&from);
}

void join_miter(Polygon::VertexList &from,
                Polygon::VertexList &to,
                NR::Point const &center,
                double miter_limit,
                double smoothness)
{
    join_bevel(from, to, center, miter_limit, smoothness);
}

void join_round(Polygon::VertexList &from,
                Polygon::VertexList &to,
                NR::Point const &center,
                double miter_limit,
                double smoothness)
{
    join_bevel(from, to, center, miter_limit, smoothness);
}

inline JoinFn join_type_to_fn(JoinType join) {
    switch (join) {
        case JOIN_MITER: {
            return &join_miter;
        }
        case JOIN_ROUND: {
            return &join_round;
        }
        case JOIN_BEVEL: {
            return &join_bevel;
        }
    }
}

typedef void (*CapFn)(Polygon::VertexList &from,
                      Polygon::VertexList &to,
                      NR::Point const &center,
                      double smoothness);

void cap_butt(Polygon::VertexList &from,
              Polygon::VertexList &to,
              NR::Point const &center,
              double smoothness)
{
    to.setNext(&from);
}

void cap_square(Polygon::VertexList &from,
                Polygon::VertexList &to,
                NR::Point const &center,
                double smoothness)
{
    NR::Point &from_pt=from.data();
    NR::Point &to_pt=to.data();

    // instead of moving the original points, should we instead 
    // add two new ones, suitably offset, to form the corners of
    // the cap?

    // the disadvantage to moving the original points is that
    // any errors in cap placement will affect the entire stroke
    // segment

    // the advantage is that (with e.g. dashed lines), we can
    // nearly halve the number of segments involved

    from_pt += rot90(from_pt - center);
    to_pt += rot90(center - to_pt);

    to.setNext(&from);
}

void cap_round(Polygon::VertexList &from,
               Polygon::VertexList &to,
               NR::Point const &center,
               double smoothness)
{
    NR::Point &from_pt=from.data();
    NR::Point &to_pt=to.data();

    // TODO this is not a good approximation at all,
    // but it should do for now

    to = *approx_quad(to_pt, to_pt + rot90(center - to_pt),
                      *approx_quad(center + rot90(center - to_pt),
                                   from_pt + rot90(from_pt - center),
                                   from, smoothness),
                      smoothness);
}

inline CapFn cap_type_to_fn(CapType cap) {
    switch (cap) {
        case CAP_BUTT: {
            return &cap_butt;
        }
        case CAP_SQUARE: {
            return &cap_square;
        }
        case CAP_ROUND: {
            return &cap_round;
        }
    }
}


struct Yoke {
    typedef Polygon::VertexList VertexList;

    Yoke() {}
    Yoke(VertexList *l, VertexList const *c, VertexList *r)
    : left(l), center(c), right(r) {}

    VertexList *left;
    VertexList const *center;
    VertexList *right;
};

/// break a circular vertex list and make a polygon from it
inline
Polygon to_polygon(Polygon::VertexList &first) {
    Polygon::VertexList *last=first.next();
    first.setNext(NULL);
    return Polygon(last, true);
}

Polygon::VertexList const *advance(Polygon::VertexList const &current) {
    Polygon::VertexList const *iter=current.next();
    while ( iter && iter->data() == current.data() ) {
        iter = iter->next();
    }
    return iter;
}

List<Polygon> *stroke_impl(Polygon const &polygon, JoinFn join, CapFn cap,
                           double width, double miter_limit, double smoothness)
{
    bool is_clockwise(true); // is the original polygon clockwise?
    NR::Point cusp(-HUGE_VAL, -HUGE_VAL);

    if (!polygon.vertices()) {
        return NULL;
    }
    
    Yoke start(NULL, NULL, NULL);
    Yoke end(NULL, polygon.vertices(), NULL);

    Polygon::VertexList const *next;
    for ( next=advance(*end.center) ; next ; next=advance(*next) ) {
        NR::Point const &a=end.center->data();
        NR::Point const &b=next->data();

        NR::Point direction=( b - a );
        direction.normalize();

        NR::Point offset=( width * rot90(direction) );

        Polygon::VertexList *left=cons_mutable(a - offset, end.left);
        Polygon::VertexList *right=cons_mutable(a + offset, end.right);

        if (start.center) {
            NR::Point perpendicular=( end.right->data() - end.left->data() );
            perpendicular.normalize();

            bool right_turn=( cross(perpendicular, direction) > 0 );

            // apply the join style to the outside of the turn
            if (right_turn) {
                join(*end.left, *left, end.center->data(), miter_limit, smoothness);
            } else {
                join(*end.right, *right, end.center->data(), miter_limit, smoothness);
            }

            // find the cusp of a convex area of the polygon; the turn
            // direction here indicates the winding direction of the polygon
            if ( a[NR::X] > cusp[NR::X] || 
                 a[NR::Y] > cusp[NR::Y] )
            {
                cusp = a;
                is_clockwise = right_turn;
            }
        } else {
            start.left = left;
            start.center = end.center;
            start.right = right;
        }

        end = Yoke(cons_mutable(b - offset, left),
                   next,
                   cons_mutable(b + offset, right));
    }

    if (!start.center) {
        if ( polygon.isClosed() && cap == cap_round ) {
            // TODO make a round dot
            return NULL;
        } else {
            return NULL;
        }
    }

    // ensure that the inner polygon is on the right-hand side of the yokes
    if (!is_clockwise) {
        start = Yoke(start.right, start.center, start.left);
        end = Yoke(end.right, end.center, end.left);
    }
    // reverse the direction of the inner polygon
    reverse_in_place(start.right);

    if (polygon.isClosed()) {
        if ( start.center->data() != end.center->data() ) {
            // TODO add a segment to complete the shape, making the
            // inside and outside vertex lists circular
        }

        // TODO perform a final join

        return new List<Polygon>(to_polygon(*start.left),
               new List<Polygon>(to_polygon(*end.right),
               NULL));
    } else {
        // bridge inner and outer polygons with end caps
        cap(*start.right, *start.left, start.center->data(), smoothness);
        cap(*end.left, *end.right, start.center->data(), smoothness);

        return new List<Polygon>(to_polygon(*start.left), NULL);
    }
}


}

List<Polygon> *stroke(Polygon const &polygon, JoinType join, CapType cap,
                      double width, double miter_limit, double smoothness)
{
    return stroke_impl(polygon, join_type_to_fn(join), cap_type_to_fn(cap),
                       width, miter_limit, smoothness);
}

}
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
