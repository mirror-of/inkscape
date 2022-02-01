// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Marker edit mode - onCanvas marker editing of marker orientation, position, scale
 *//*
 * Authors:
 * see git history
 * Rachana Podaralla <rpodaralla3@gatech.edu>
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef __SP_MARKER_CONTEXT_H__
#define __SP_MARKER_CONTEXT_H__

#include <cstddef>
#include <sigc++/sigc++.h>
#include <2geom/point.h>

#include "object/sp-marker.h"
#include "object/sp-marker-loc.h"

#include "ui/tools/tool-base.h"
#include "ui/tool/shape-record.h"

namespace Inkscape {
class Selection;
namespace UI {
namespace Tools {

class MarkerTool : public ToolBase {
public:
    MarkerTool(SPDesktop *desktop);
    ~MarkerTool() override;

    void selection_changed(Inkscape::Selection *selection);

    bool root_handler(GdkEvent *event) override;
    std::map<SPItem *, std::unique_ptr<ShapeEditor>> _shape_editors;

    int editMarkerMode = -1;

private:
    sigc::connection sel_changed_connection;
    ShapeRecord get_marker_transform(SPShape *shape, SPItem *parent_item, SPMarker *sp_marker, SPMarkerLoc marker_type);
};

}}}

#endif
