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

#ifndef SEEN_INKSCAPE_RENDER_BEZIER_H
#define SEEN_INKSCAPE_RENDER_BEZIER_H

#include "libnr/nr-point.h"
#include "util/list.h"

namespace Inkscape {

namespace Render {


namespace Bezier { typedef Inkscape::Util::List<NR::Point> VertexList; }


Bezier::VertexList *approx_cubic(NR::Point const &to,
                                 NR::Point const &cp1,
                                 NR::Point const &cp0,
                                 Bezier::VertexList &from,
                                 double smoothness);

inline
Bezier::VertexList const *approx_cubic(NR::Point const &to,
                                       NR::Point const &cp1,
                                       NR::Point const &cp0,
                                       Bezier::VertexList const &from,
                                       double smoothness)
{
    return approx_cubic(to, cp1, cp0, const_cast<Bezier::VertexList &>(from), smoothness);
}

inline
Bezier::VertexList *approx_cubic(NR::Point const &to,
                                 NR::Point const &cp1,
                                 NR::Point const &cp0,
                                 NR::Point const &from,
                                 double smoothness)
{
    return approx_cubic(to, cp1, cp0, *(new Bezier::VertexList(from, NULL)), smoothness);
}


Bezier::VertexList *approx_quad(NR::Point const &to,
                                NR::Point const &cp,
                                Bezier::VertexList &from,
                                double smoothness);

inline
Bezier::VertexList const *approx_quad(NR::Point const &to,
                                      NR::Point const &cp,
                                      Bezier::VertexList const &from,
                                      double smoothness)
{
    return approx_quad(to, cp, const_cast<Bezier::VertexList &>(from), smoothness);
}

inline
Bezier::VertexList *approx_quad(NR::Point const &to,
                                NR::Point const &cp,
                                NR::Point const &from,
                                double smoothness)
{
    return approx_quad(to, cp, *(new Bezier::VertexList(from, NULL)), smoothness);
}
    

}

}

#endif
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
