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

#ifndef SEEN_INKSCAPE_RENDER_STROKE_H
#define SEEN_INKSCAPE_RENDER_STROKE_H


namespace Inkscape { namespace Util { template <typename T> class List; } }


namespace Inkscape {

namespace Render {

class Polygon;

enum JoinType {
    JOIN_MITER,
    JOIN_ROUND,
    JOIN_BEVEL
};

enum CapType {
    CAP_BUTT,
    CAP_ROUND,
    CAP_SQUARE
};

Inkscape::Util::List<Polygon> *stroke(Polygon const &polygon,
                                      JoinType join, CapType cap,
                                      double width, double miter_limit,
                                      double smoothness);

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
