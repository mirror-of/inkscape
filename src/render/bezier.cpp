/*
 * Bezier curve approximation functions
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

#include "render/bezier.h"
#include "libnr/nr-point.h"
#include "libnr/nr-point-fns.h"
#include "util/list.h"

namespace Inkscape {

namespace Render {

using NR::Lerp;
using Bezier::VertexList;

namespace {

bool check_smoothness(NR::Point const &a, NR::Point const &b,
                      NR::Point const &c, double smoothness)
{
    // TODO
    return true;
}

}


VertexList *approx_cubic(NR::Point const &to,
                         NR::Point const &cp1, NR::Point const &cp0,
                         VertexList &from, double smoothness)
{
    if ( check_smoothness(to, cp0, from.data(), smoothness) &&
         check_smoothness(to, cp1, from.data(), smoothness) )
    {
        return cons_mutable(to, cons_mutable(cp1, cons_mutable(cp0, &from)));
    } else {
        NR::Point m0=Lerp(0.5, to, cp1);
        NR::Point m1=Lerp(0.5, cp1, cp0);
        NR::Point m2=Lerp(0.5, cp0, from.data());

        NR::Point m3=Lerp(0.5, m0, m1);
        NR::Point m4=Lerp(0.5, m1, m2);

        NR::Point m5=Lerp(0.5, m3, m4);

        return approx_cubic(to, m0, m3, approx_cubic(m5, m4, m2, from, smoothness), smoothness);
    }
}


VertexList *approx_quad(NR::Point const &to, NR::Point const &cp,
                        VertexList &from, double smoothness)
{
    if (check_smoothness(to, cp, from.data(), smoothness)) {
        return cons_mutable(to, cons_mutable(cp, &from));
    } else {
        NR::Point m0=Lerp(0.5, to, cp);
        NR::Point m1=Lerp(0.5, cp, from.data());

        NR::Point m2=Lerp(0.5, m0, m1);

        return approx_quad(to, m0, approx_quad(m2, m1, from, smoothness), smoothness);
    }
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
