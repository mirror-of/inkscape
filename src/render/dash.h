/*
 * Inkscape::Render::dash - dash a polygon
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

#ifndef SEEN_INKSCAPE_RENDER_DASH_H
#define SEEN_INKSCAPE_RENDER_DASH_H

#include <vector>


namespace Inkscape { namespace Util { template <typename T> class List; } }


namespace Inkscape {

namespace Render {


class Polygon;


Inkscape::Util::List<Polygon> *dash(Polygon const &polygon, 
                                    double const intervals[],
                                    unsigned n_intervals,
                                    double offset);

template <typename Alloc>
inline
Inkscape::Util::List<Polygon> *dash(Polygon const &polygon,
                                    std::vector<double, Alloc> const &intervals,
                                    double offset)
{
    return dash(polygon, &intervals.front(), intervals.size(), offset);
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
