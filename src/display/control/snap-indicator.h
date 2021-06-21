// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef INKSCAPE_DISPLAY_SNAP_INDICATOR_H
#define INKSCAPE_DISPLAY_SNAP_INDICATOR_H

/**
 * @file
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 */
/*
 * Authors:
 *   Johan Engelen
 *   Diederik van Lierop
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Diederik van Lierop 2010 <mail@diedenrezi.nl>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "snap-enums.h"
#include "snapped-point.h"
#include "display/control/canvas-item-curve.h"

#include <glib.h>
#include <glibmm/i18n.h>
#include <unordered_map>

class SPDesktop;

namespace Inkscape {
namespace Display {

class TemporaryItem;

class SnapIndicator  {
public:
    SnapIndicator(SPDesktop *desktop);
    virtual ~SnapIndicator();

    void set_new_snaptarget(Inkscape::SnappedPoint const &p, bool pre_snap = false);
    void remove_snaptarget(bool only_if_presnap = false);

    void set_new_snapsource(Inkscape::SnapCandidatePoint const &p);
    void remove_snapsource();

    void set_new_debugging_point(Geom::Point const &p);
    void remove_debugging_points();

protected:
    TemporaryItem *_snaptarget;
    TemporaryItem *_snaptarget_tooltip;
    TemporaryItem *_snaptarget_bbox;
    TemporaryItem *_snapsource;

    std::list<TemporaryItem *> _alignment_snap_indicators;
    std::list<TemporaryItem *> _distribution_snap_indicators;
    std::list<TemporaryItem *> _debugging_points;
    bool _snaptarget_is_presnap;
    SPDesktop *_desktop;

private:
    SnapIndicator(const SnapIndicator&) = delete;
    SnapIndicator& operator=(const SnapIndicator&) = delete;

    void make_distribution_indicators(SnappedPoint const &p, double fontsize, double scale);
    void make_alignment_indicator(Geom::Point const &p1, Geom::Point const &p2, guint32 color, double fontsize, double scale);
    guint32 get_guide_color(SnapTargetType t);
    Inkscape::CanvasItemCurve* make_stub_line_h(Geom::Point const &p);
    Inkscape::CanvasItemCurve* make_stub_line_v(Geom::Point const &p);
    
    std::unordered_map<SnapSourceType, Glib::ustring> source2string = {
        {SNAPSOURCE_UNDEFINED, _("UNDEFINED")},
        {SNAPSOURCE_BBOX_CORNER, _("Bounding box corner")},
        {SNAPSOURCE_BBOX_MIDPOINT, _("Bounding box midpoint")},
        {SNAPSOURCE_BBOX_EDGE_MIDPOINT, _("Bounding box side midpoint")},
        {SNAPSOURCE_NODE_SMOOTH, _("Smooth node")},
        {SNAPSOURCE_NODE_CUSP, _("Cusp node")},
        {SNAPSOURCE_LINE_MIDPOINT, _("Line midpoint")},
        {SNAPSOURCE_PATH_INTERSECTION, _("Path intersection")},
        {SNAPSOURCE_RECT_CORNER, _("Corner")},
        {SNAPSOURCE_CONVEX_HULL_CORNER, _("Convex hull corner")},
        {SNAPSOURCE_ELLIPSE_QUADRANT_POINT, _("Quadrant point")},
        {SNAPSOURCE_NODE_HANDLE, _("Handle")},
        {SNAPSOURCE_GUIDE, _("Guide")},
        {SNAPSOURCE_GUIDE_ORIGIN, _("Guide origin")},
        {SNAPSOURCE_ROTATION_CENTER, _("Object rotation center")},
        {SNAPSOURCE_OBJECT_MIDPOINT, _("Object midpoint")},
        {SNAPSOURCE_IMG_CORNER, _("Corner")},
        {SNAPSOURCE_TEXT_ANCHOR, _("Text anchor")},
        {SNAPSOURCE_OTHER_HANDLE, _("Handle")},
        {SNAPSOURCE_GRID_PITCH, _("Multiple of grid spacing")},
    };

    std::unordered_map<SnapTargetType, Glib::ustring> target2string = {
        {SNAPTARGET_UNDEFINED, _("UNDEFINED")},
        {SNAPTARGET_BBOX_CORNER, _("bounding box corner")},
        {SNAPTARGET_BBOX_EDGE, _("bounding box side")},
        {SNAPTARGET_BBOX_EDGE_MIDPOINT, _("bounding box side midpoint")},
        {SNAPTARGET_BBOX_MIDPOINT, _("bounding box midpoint")},
        {SNAPTARGET_NODE_SMOOTH, _("smooth node")},
        {SNAPTARGET_NODE_CUSP, _("cusp node")},
        {SNAPTARGET_LINE_MIDPOINT, _("line midpoint")},
        {SNAPTARGET_PATH, _("path")},
        {SNAPTARGET_PATH_PERPENDICULAR, _("path (perpendicular)")},
        {SNAPTARGET_PATH_TANGENTIAL, _("path (tangential)")},
        {SNAPTARGET_PATH_INTERSECTION, _("path intersection")},
        {SNAPTARGET_PATH_GUIDE_INTERSECTION, _("guide-path intersection")},
        {SNAPTARGET_PATH_CLIP, _("clip-path")},
        {SNAPTARGET_PATH_MASK, _("mask-path")},
        {SNAPTARGET_ELLIPSE_QUADRANT_POINT, _("quadrant point")},
        {SNAPTARGET_RECT_CORNER, _("corner")},
        {SNAPTARGET_GRID, _("grid line")},
        {SNAPTARGET_GRID_INTERSECTION, _("grid intersection")},
        {SNAPTARGET_GRID_PERPENDICULAR, _("grid line (perpendicular)")},
        {SNAPTARGET_GUIDE, _("guide")},
        {SNAPTARGET_GUIDE_INTERSECTION, _("guide intersection")},
        {SNAPTARGET_GUIDE_ORIGIN, _("guide origin")},
        {SNAPTARGET_GUIDE_PERPENDICULAR, _("guide (perpendicular)")},
        {SNAPTARGET_GRID_GUIDE_INTERSECTION, _("grid-guide intersection")},
        {SNAPTARGET_PAGE_BORDER, _("page border")},
        {SNAPTARGET_PAGE_CORNER, _("page corner")},
        {SNAPTARGET_OBJECT_MIDPOINT, _("object midpoint")},
        {SNAPTARGET_IMG_CORNER, _("corner")},
        {SNAPTARGET_ROTATION_CENTER, _("object rotation center")},
        {SNAPTARGET_TEXT_ANCHOR, _("text anchor")},
        {SNAPTARGET_TEXT_BASELINE, _("text baseline")},
        {SNAPTARGET_CONSTRAINED_ANGLE, _("constrained angle")},
        {SNAPTARGET_CONSTRAINT, _("constraint")},
    };
};

} //namespace Display
} //namespace Inkscape

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
