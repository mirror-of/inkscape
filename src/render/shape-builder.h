/*
 * Inkscape::Render::ShapeBuilder - build shapes (lists of polygons) using
 *                                  postscript-like path operations
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

#ifndef SEEN_INKSCAPE_SHAPE_BUILDER_H
#define SEEN_INKSCAPE_SHAPE_BUILDER_H

#include "gc-managed.h"
#include "libnr/nr-point.h"


namespace NR { class Point; }
namespace Inkscape { namespace Util { template <typename T> class List; } }


namespace Inkscape {

namespace Render {

class Polygon;

class ShapeBuilder : public Inkscape::GC::Managed<> {
public:
    typedef Inkscape::Util::List<Polygon> PolygonList;
    typedef Inkscape::Util::List<NR::Point> VertexList;

    ShapeBuilder();

    void moveTo(NR::Point const &pt);
    void lineTo(NR::Point const &pt);
    void quadTo(NR::Point const &c, NR::Point const &pt, double smoothness);
    void curveTo(NR::Point const &c0, NR::Point const &c1, NR::Point const &pt, double smoothness);
    void closePath();

    VertexList const *finishVertices();
    PolygonList const *finish();
    void discard();

private:
    inline void _startSegment();
    inline void _endPolygon(bool close);
    
    VertexList const *_start;
    VertexList const *_vertices;
    PolygonList const *_polygons;
};

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
