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

#include "util/list.h"
#include "render/polygon.h"
#include "render/shape-builder.h"

namespace Inkscape {

namespace Render {

using Inkscape::Util::cons;

ShapeBuilder::ShapeBuilder()
: _start(NULL), _vertices(NULL), _polygons(NULL)
{}

void ShapeBuilder::moveTo(NR::Point const &pt) {
    _endPolygon(false);
    _start = new VertexList(pt, NULL);
}

void ShapeBuilder::lineTo(NR::Point const &pt) {
    _startSegment();
    _vertices = cons_mutable(pt, _vertices);
}

void ShapeBuilder::quadTo(NR::Point const &c, NR::Point const &pt,
                          double smoothness)
{
    _startSegment();
    _vertices = cons_mutable(pt, cons_mutable(c, _vertices));
}

void ShapeBuilder::curveTo(NR::Point const &c0, NR::Point const &c1,
                           NR::Point const &pt, double smoothness)
{
    _startSegment();
    _vertices = cons_mutable(pt, cons_mutable(c1, cons_mutable(c0, _vertices)));
}

void ShapeBuilder::closePath() {
    _startSegment();
    _endPolygon(true);
}

ShapeBuilder::VertexList *ShapeBuilder::finishVertices() {
    _endPolygon(false);
    VertexList *vertices=_vertices;
    discard();
    return vertices;
}

ShapeBuilder::PolygonList *ShapeBuilder::finish() {
    _endPolygon(false);
    PolygonList *polygons=_polygons;
    discard();
    return polygons;
}

void ShapeBuilder::discard() {
    _start = NULL;
    _vertices = NULL;
    _polygons = NULL;
}

void ShapeBuilder::_startSegment() {
    if (!_vertices) {
        _vertices = _start;
    }
}

void ShapeBuilder::_endPolygon(bool close) {
    if (_vertices) {
        _polygons = cons_mutable(Polygon(_vertices, close), _polygons);
    }
    _vertices = NULL;
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
