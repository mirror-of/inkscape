/*
 * Inkscape::Render::Polygon - simple polygon class
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

#ifndef SEEN_INKSCAPE_RENDER_POLYGON_H
#define SEEN_INKSCAPE_RENDER_POLYGON_H

#include "gc-managed.h"


namespace NR { class Point; }
namespace Inkscape { namespace Util { template <typename T> class List; } }


namespace Inkscape {

namespace Render {

class Polygon : public Inkscape::GC::Managed<> {
public:
    typedef Inkscape::Util::List<NR::Point> VertexList;

    Polygon() : _vertices(NULL), _is_closed(false) {}
    Polygon(VertexList const *vertices, bool closed=false)
    : _vertices(vertices), _is_closed(false) {}

    VertexList const *vertices() const { return _vertices; }
    VertexList const *setVertices(VertexList const *vertices) {
        _vertices = vertices;
    }

    bool isClosed() const { return _is_closed; }
    void setClosed(bool closed=true) { _is_closed = closed; }

private:
    VertexList const *_vertices;
    bool _is_closed;
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
