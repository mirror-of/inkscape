// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 *
 * Authors:
 *   Johan Engelen
 *   Diederik van Lierop
 *
 * Copyright (C) Johan Engelen 2009 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Diederik van Lierop 2010 - 2012 <mail@diedenrezi.nl>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glibmm/i18n.h>
#include <string>
#include <iomanip>

#include "snap-indicator.h"

#include "desktop.h"
#include "enums.h"
#include "preferences.h"
#include "util/units.h"
#include "document.h"


#include "canvas-item-ctrl.h"
#include "canvas-item-rect.h"
#include "canvas-item-text.h"
#include "canvas-item-curve.h"

#include "ui/tools/measure-tool.h"

#define ALIGNMENT_GUIDE_MEASURE_OFFSET 15

namespace Inkscape {
namespace Display {

SnapIndicator::SnapIndicator(SPDesktop * desktop)
    :   _snaptarget(nullptr),
        _snaptarget_tooltip(nullptr),
        _snaptarget_bbox(nullptr),
        _snapsource(nullptr),
        _snaptarget_is_presnap(false),
        _desktop(desktop)
{
}

SnapIndicator::~SnapIndicator()
{
    // remove item that might be present
    remove_snaptarget();
    remove_snapsource();
}

void
SnapIndicator::set_new_snaptarget(Inkscape::SnappedPoint const &p, bool pre_snap)
{
    remove_snaptarget(); //only display one snaptarget at a time

    g_assert(_desktop != nullptr);

    if (!p.getSnapped()) {
        return; // If we haven't snapped, then it is of no use to draw a snapindicator
    }

    if (p.getTarget() == SNAPTARGET_CONSTRAINT) {
        // This is not a real snap, although moving along the constraint did affect the mouse pointer's position.
        // Maybe we should only show a snap indicator when the user explicitly asked for a constraint by pressing ctrl?
        // We should not show a snap indicator when stretching a selection box, which is also constrained. That would be
        // too much information.
        return;
    }

    bool is_alignment = p.getAlignmentTarget().has_value();
    bool is_distribution = p.getTarget() & SNAPTARGET_DISTRIBUTION_CATEGORY;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double scale = prefs->getDouble("/tools/measure/scale", 100.0) / 100.0;

    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        Glib::ustring target_name = _("UNDEFINED");
        Glib::ustring source_name = _("UNDEFINED");

        if (!is_alignment && !is_distribution) {
            // TRANSLATORS: undefined target for snapping
            switch (p.getTarget()) {
                case SNAPTARGET_UNDEFINED:
                    target_name = _("UNDEFINED");
                    g_warning("Snap target has not been specified");
                    break;
                case SNAPTARGET_GRID:
                    target_name = _("grid line");
                    break;
                case SNAPTARGET_GRID_INTERSECTION:
                    target_name = _("grid intersection");
                    break;
                case SNAPTARGET_GRID_PERPENDICULAR:
                    target_name = _("grid line (perpendicular)");
                    break;
                case SNAPTARGET_GUIDE:
                    target_name = _("guide");
                    break;
                case SNAPTARGET_GUIDE_INTERSECTION:
                    target_name = _("guide intersection");
                    break;
                case SNAPTARGET_GUIDE_ORIGIN:
                    target_name = _("guide origin");
                    break;
                case SNAPTARGET_GUIDE_PERPENDICULAR:
                    target_name = _("guide (perpendicular)");
                    break;
                case SNAPTARGET_GRID_GUIDE_INTERSECTION:
                    target_name = _("grid-guide intersection");
                    break;
                case SNAPTARGET_NODE_CUSP:
                    target_name = _("cusp node");
                    break;
                case SNAPTARGET_NODE_SMOOTH:
                    target_name = _("smooth node");
                    break;
                case SNAPTARGET_PATH:
                    target_name = _("path");
                    break;
                case SNAPTARGET_PATH_PERPENDICULAR:
                    target_name = _("path (perpendicular)");
                    break;
                case SNAPTARGET_PATH_TANGENTIAL:
                    target_name = _("path (tangential)");
                    break;
                case SNAPTARGET_PATH_INTERSECTION:
                    target_name = _("path intersection");
                    break;
                case SNAPTARGET_PATH_GUIDE_INTERSECTION:
                    target_name = _("guide-path intersection");
                    break;
                case SNAPTARGET_PATH_CLIP:
                    target_name = _("clip-path");
                    break;
                case SNAPTARGET_PATH_MASK:
                    target_name = _("mask-path");
                    break;
                case SNAPTARGET_BBOX_CORNER:
                    target_name = _("bounding box corner");
                    break;
                case SNAPTARGET_BBOX_EDGE:
                    target_name = _("bounding box side");
                    break;
                case SNAPTARGET_PAGE_BORDER:
                    target_name = _("page border");
                    break;
                case SNAPTARGET_LINE_MIDPOINT:
                    target_name = _("line midpoint");
                    break;
                case SNAPTARGET_OBJECT_MIDPOINT:
                    target_name = _("object midpoint");
                    break;
                case SNAPTARGET_ROTATION_CENTER:
                    target_name = _("object rotation center");
                    break;
                case SNAPTARGET_BBOX_EDGE_MIDPOINT:
                    target_name = _("bounding box side midpoint");
                    break;
                case SNAPTARGET_BBOX_MIDPOINT:
                    target_name = _("bounding box midpoint");
                    break;
                case SNAPTARGET_PAGE_CORNER:
                    target_name = _("page corner");
                    break;
                case SNAPTARGET_ELLIPSE_QUADRANT_POINT:
                    target_name = _("quadrant point");
                    break;
                case SNAPTARGET_RECT_CORNER:
                case SNAPTARGET_IMG_CORNER:
                    target_name = _("corner");
                    break;
                case SNAPTARGET_TEXT_ANCHOR:
                    target_name = _("text anchor");
                    break;
                case SNAPTARGET_TEXT_BASELINE:
                    target_name = _("text baseline");
                    break;
                case SNAPTARGET_CONSTRAINED_ANGLE:
                    target_name = _("constrained angle");
                    break;
                case SNAPTARGET_CONSTRAINT:
                    target_name = _("constraint");
                    break;
                default:
                    g_warning("Snap target not in SnapTargetType enum");
                    break;
            }

            switch (p.getSource()) {
                case SNAPSOURCE_UNDEFINED:
                    source_name = _("UNDEFINED");
                    g_warning("Snap source has not been specified");
                    break;
                case SNAPSOURCE_BBOX_CORNER:
                    source_name = _("Bounding box corner");
                    break;
                case SNAPSOURCE_BBOX_MIDPOINT:
                    source_name = _("Bounding box midpoint");
                    break;
                case SNAPSOURCE_BBOX_EDGE_MIDPOINT:
                    source_name = _("Bounding box side midpoint");
                    break;
                case SNAPSOURCE_NODE_SMOOTH:
                    source_name = _("Smooth node");
                    break;
                case SNAPSOURCE_NODE_CUSP:
                    source_name = _("Cusp node");
                    break;
                case SNAPSOURCE_LINE_MIDPOINT:
                    source_name = _("Line midpoint");
                    break;
                case SNAPSOURCE_OBJECT_MIDPOINT:
                    source_name = _("Object midpoint");
                    break;
                case SNAPSOURCE_ROTATION_CENTER:
                    source_name = _("Object rotation center");
                    break;
                case SNAPSOURCE_NODE_HANDLE:
                case SNAPSOURCE_OTHER_HANDLE:
                    source_name = _("Handle");
                    break;
                case SNAPSOURCE_PATH_INTERSECTION:
                    source_name = _("Path intersection");
                    break;
                case SNAPSOURCE_GUIDE:
                    source_name = _("Guide");
                    break;
                case SNAPSOURCE_GUIDE_ORIGIN:
                    source_name = _("Guide origin");
                    break;
                case SNAPSOURCE_CONVEX_HULL_CORNER:
                    source_name = _("Convex hull corner");
                    break;
                case SNAPSOURCE_ELLIPSE_QUADRANT_POINT:
                    source_name = _("Quadrant point");
                    break;
                case SNAPSOURCE_RECT_CORNER:
                case SNAPSOURCE_IMG_CORNER:
                    source_name = _("Corner");
                    break;
                case SNAPSOURCE_TEXT_ANCHOR:
                    source_name = _("Text anchor");
                    break;
                case SNAPSOURCE_GRID_PITCH:
                    source_name = _("Multiple of grid spacing");
                    break;
                default:
                    g_warning("Snap source not in SnapSourceType enum");
                    break;
            }
        }
        //std::cout << "Snapped " << source_name << " to " << target_name << std::endl;

        remove_snapsource(); // Don't set both the source and target indicators, as these will overlap

        double timeout_val = prefs->getDouble("/options/snapindicatorpersistence/value", 2.0);
        if (timeout_val < 0.1) {
            timeout_val = 0.1; // a zero value would mean infinite persistence (i.e. until new snap occurs)
            // Besides, negatives values would ....?
        }

        double fontsize = prefs->getDouble("/tools/measure/fontsize", 10.0);

        if (is_distribution) {
            make_distribution_indicators(p, fontsize, scale);
        }

        if (is_alignment) {
            auto color = pre_snap ? 0x7f7f7fff : get_guide_color(p.getAlignmentTargetType());
            make_alignment_indicator(p.getPoint(), *p.getAlignmentTarget(), color, fontsize, scale);
            if (p.getAlignmentTargetType() == SNAPTARGET_ALIGNMENT_INTERSECTION) {
                make_alignment_indicator(p.getPoint(), *p.getAlignmentTarget2(), color, fontsize, scale);
            }
        } 

        _snaptarget_is_presnap = pre_snap;
 
        // Display the snap indicator (i.e. the cross)
        Inkscape::CanvasItemCtrl *ctrl;

        if (!is_alignment && !is_distribution) {
            // Display snap indicator at snap target
            ctrl = new Inkscape::CanvasItemCtrl(_desktop->getCanvasTemp(), Inkscape::CANVAS_ITEM_CTRL_SHAPE_CROSS);
            ctrl->set_size(11);
            ctrl->set_stroke( pre_snap ? 0x7f7f7fff : 0xff0000ff);
            ctrl->set_position(p.getPoint());

            _snaptarget = _desktop->add_temporary_canvasitem(ctrl, timeout_val*1000.0);
            // The snap indicator will be deleted after some time-out, and sp_canvas_item_dispose
            // will be called. This will set canvas->current_item to NULL if the snap indicator was
            // the current item, after which any events will go to the root handler instead of any
            // item handler. Dragging an object which has just snapped might therefore not be possible
            // without selecting / repicking it again. To avoid this, we make sure here that the
            // snap indicator will never be picked, and will therefore never be the current item.
            // Reported bugs:
            //   - scrolling when hovering above a pre-snap indicator won't work (for example)
            //     (https://bugs.launchpad.net/inkscape/+bug/522335/comments/8)
            //   - dragging doesn't work without repicking
            //     (https://bugs.launchpad.net/inkscape/+bug/1420301/comments/15)
            ctrl->set_pickable(false);

            // Display the tooltip, which reveals the type of snap source and the type of snap target
            Glib::ustring tooltip_str;
            if ( (p.getSource() != SNAPSOURCE_GRID_PITCH) && (p.getTarget() != SNAPTARGET_UNDEFINED) ) {
                tooltip_str = source_name + _(" to ") + target_name;
            } else if (p.getSource() != SNAPSOURCE_UNDEFINED) {
                tooltip_str = source_name;
            }


            if (!tooltip_str.empty()) {
                Geom::Point tooltip_pos = p.getPoint();
                if (dynamic_cast<Inkscape::UI::Tools::MeasureTool *>(_desktop->event_context)) {
                    // Make sure that the snap tooltips do not overlap the ones from the measure tool
                    tooltip_pos += _desktop->w2d(Geom::Point(0, -3*fontsize));
                } else {
                    tooltip_pos += _desktop->w2d(Geom::Point(0, -2*fontsize));
                }

                auto canvas_tooltip = new Inkscape::CanvasItemText(_desktop->getCanvasTemp(), tooltip_pos, tooltip_str);
                canvas_tooltip->set_fontsize(fontsize);
                canvas_tooltip->set_fill(0xffffffff);
                canvas_tooltip->set_background(pre_snap ? 0x33337f40 : 0x33337f7f);

                _snaptarget_tooltip = _desktop->add_temporary_canvasitem(canvas_tooltip, timeout_val*1000.0);
            }

            // Display the bounding box, if we snapped to one
            Geom::OptRect const bbox = p.getTargetBBox();
            if (bbox) {
                auto box = new Inkscape::CanvasItemRect(_desktop->getCanvasTemp(), *bbox);
                box->set_stroke(pre_snap ? 0x7f7f7fff : 0xff0000ff);
                box->set_dashed(true);
                box->set_pickable(false); // Is false by default.
                box->set_z_position(0);
                _snaptarget_bbox = _desktop->add_temporary_canvasitem(box, timeout_val*1000.0);
            }
        }
    }
}

void
SnapIndicator::remove_snaptarget(bool only_if_presnap)
{
    if (only_if_presnap && !_snaptarget_is_presnap) {
        return;
    }

    if (_snaptarget) {
        _desktop->remove_temporary_canvasitem(_snaptarget);
        _snaptarget = nullptr;
        _snaptarget_is_presnap = false;
    }

    if (_snaptarget_tooltip) {
        _desktop->remove_temporary_canvasitem(_snaptarget_tooltip);
        _snaptarget_tooltip = nullptr;
    }

    if (_snaptarget_bbox) {
        _desktop->remove_temporary_canvasitem(_snaptarget_bbox);
        _snaptarget_bbox = nullptr;
    }

    for (auto *item : _alignment_snap_indicators) {
        _desktop->remove_temporary_canvasitem(item);
    }
    _alignment_snap_indicators.clear();

    for (auto *item : _distribution_snap_indicators) {
        _desktop->remove_temporary_canvasitem(item);
    }
    _distribution_snap_indicators.clear();
}

void
SnapIndicator::set_new_snapsource(Inkscape::SnapCandidatePoint const &p)
{
    remove_snapsource();

    g_assert(_desktop != nullptr); // If this fails, then likely setup() has not been called on the snap manager (see snap.cpp -> setup())

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        auto ctrl = new Inkscape::CanvasItemCtrl(_desktop->getCanvasTemp(), Inkscape::CANVAS_ITEM_CTRL_SHAPE_CIRCLE);
        ctrl->set_size(7);
        ctrl->set_stroke(0xff0000ff);
        ctrl->set_position(p.getPoint());
        _snapsource = _desktop->add_temporary_canvasitem(ctrl, 1000);
    }
}

void
SnapIndicator::set_new_debugging_point(Geom::Point const &p)
{
    g_assert(_desktop != nullptr);
    auto ctrl = new Inkscape::CanvasItemCtrl(_desktop->getCanvasTemp(), Inkscape::CANVAS_ITEM_CTRL_SHAPE_DIAMOND);
    ctrl->set_size(11);
    ctrl->set_stroke(0x00ff00ff);
    ctrl->set_position(p);
    _debugging_points.push_back(_desktop->add_temporary_canvasitem(ctrl, 5000));
}

void
SnapIndicator::remove_snapsource()
{
    if (_snapsource) {
        _desktop->remove_temporary_canvasitem(_snapsource);
        _snapsource = nullptr;
    }
}

void
SnapIndicator::remove_debugging_points()
{
    for (std::list<TemporaryItem *>::const_iterator i = _debugging_points.begin(); i != _debugging_points.end(); ++i) {
        _desktop->remove_temporary_canvasitem(*i);
    }
    _debugging_points.clear();
}

guint32 SnapIndicator::get_guide_color(SnapTargetType t)
{
    switch(t) {
        case SNAPTARGET_ALIGNMENT_BBOX_CORNER:
        case SNAPTARGET_ALIGNMENT_BBOX_MIDPOINT:
        case SNAPTARGET_ALIGNMENT_BBOX_EDGE_MIDPOINT:
            return 0xff0000ff;
        case SNAPTARGET_ALIGNMENT_PAGE_CENTER:
        case SNAPTARGET_ALIGNMENT_PAGE_CORNER:
            return 0x00ff00ff;
        case SNAPTARGET_ALIGNMENT_HANDLE:
            return 0x0000ffff;
        case SNAPTARGET_ALIGNMENT_INTERSECTION:
            return 0xd13bd1ff;
        default:
            g_warning("Alignment guide color not handled %i", t);
            return 0x000000ff;
    }
}

Geom::Coord get_y(Geom::Rect const &source, Geom::Rect const &target)
{
    Geom::Coord y;

    if (source.max().y() < target.midpoint().y())
        y = target.min().y();
    else if(source.min().y() > target.midpoint().y())
        y = target.max().y();
    else
        y = target.midpoint().y();

    return y;
}

Geom::Coord get_x(Geom::Rect const &source, Geom::Rect const &target)
{
    Geom::Coord x;

    if (source.max().x() < target.midpoint().x())
        x = target.min().x();
    else if(source.min().x() > target.midpoint().x())
        x = target.max().x();
    else
        x = target.midpoint().x();

    return x;
}

void SnapIndicator::make_alignment_indicator(Geom::Point const &p1, Geom::Point const &p2, guint32 color, double fontsize, double scale)
{
    Preferences *prefs = Preferences::get();
    bool show_distance = prefs->getBool("/options/snapindicatordistance/value", true);

    Inkscape::CanvasItemCurve *line;

    if (show_distance) {
        auto dist = Geom::L2(p2 - p1);
        double offset = ALIGNMENT_GUIDE_MEASURE_OFFSET/_desktop->current_zoom();
        auto direction = Geom::unit_vector(p1 - p2);
        auto text_pos = (p1 + p2)/2;

        Glib::ustring unit_name = _desktop->doc()->getDisplayUnit()->abbr.c_str();
        if (!unit_name.compare("")) {
            unit_name = DEFAULT_UNIT_NAME;
        }

        dist = Inkscape::Util::Quantity::convert(dist, "px", unit_name);

        Glib::ustring distance = Glib::ustring::format(std::fixed, std::setprecision(1), std::noshowpoint, scale*dist);
        
        auto text = new Inkscape::CanvasItemText(_desktop->getCanvasTemp(), text_pos, distance);
        text->set_fontsize(fontsize);
        text->set_fill(color);
        _alignment_snap_indicators.push_back(_desktop->add_temporary_canvasitem(text, 0));
        text->set_background(0xffffff00);

        auto temp_point = text_pos + offset*direction;
        line = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p1, temp_point);
        line->set_stroke(color);
        _alignment_snap_indicators.push_back(_desktop->add_temporary_canvasitem(line, 0));

        temp_point = text_pos - offset*direction;
        line = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), temp_point, p2);
        line->set_stroke(color);
        _alignment_snap_indicators.push_back(_desktop->add_temporary_canvasitem(line, 0));
    } else {
        line = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p1, p2);
        line->set_stroke(color);
        _alignment_snap_indicators.push_back(_desktop->add_temporary_canvasitem(line, 0));
    }

    auto ctrl = new Inkscape::CanvasItemCtrl(_desktop->getCanvasTemp(), Inkscape::CANVAS_ITEM_CTRL_SHAPE_CIRCLE);
    ctrl->set_size(7);
    ctrl->set_stroke(color);
    ctrl->set_fill(color);
    ctrl->set_position(p1);
    ctrl->set_pickable(false);
    _alignment_snap_indicators.push_back(_desktop->add_temporary_canvasitem(ctrl, 0));

    ctrl = new Inkscape::CanvasItemCtrl(_desktop->getCanvasTemp(), Inkscape::CANVAS_ITEM_CTRL_SHAPE_CIRCLE);
    ctrl->set_size(7);
    ctrl->set_stroke(color);
    ctrl->set_fill(color);
    ctrl->set_position(p2);
    ctrl->set_pickable(false);
    _alignment_snap_indicators.push_back(_desktop->add_temporary_canvasitem(ctrl, 0));
}

Inkscape::CanvasItemCurve* SnapIndicator::make_stub_line_v(Geom::Point const & p) 
{
    Geom::Coord length = 20;
    auto line = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p + Geom::Point(0, length/2), p - Geom::Point(0, length/2));
    line->set_stroke(0xff5f1fff);
    return line;
}

Inkscape::CanvasItemCurve* SnapIndicator::make_stub_line_h(Geom::Point const & p) 
{
    Geom::Coord length = 20;
    auto line = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p + Geom::Point(length/2, 0), p - Geom::Point(length/2, 0));
    line->set_stroke(0xff5f1fff);
    return line;
}

void SnapIndicator::make_distribution_indicators(SnappedPoint const &p,
                                                double fontsize,
                                                double scale)
{
    Preferences *prefs = Preferences::get();
    bool show_distance = prefs->getBool("/options/snapindicatordistance/value", false);

    guint32 color = 0xff5f1fff;
    guint32 text_fill = 0xffffffff;
    guint32 text_bg = 0xff5f1fff; //0x33337f7f
    Geom::Point text_pos;

    Glib::ustring unit_name = _desktop->doc()->getDisplayUnit()->abbr.c_str();
    if (!unit_name.compare("")) {
        unit_name = DEFAULT_UNIT_NAME;
    }
    auto equal_dist = Inkscape::Util::Quantity::convert(p.getDistributionDistance(), "px", unit_name);
    Glib::ustring distance = Glib::ustring::format(std::fixed, std::setprecision(1), std::noshowpoint, scale*equal_dist);

    switch (p.getTarget()) {
        case SNAPTARGET_DISTRIBUTION_Y:
        case SNAPTARGET_DISTRIBUTION_X:
        case SNAPTARGET_DISTRIBUTION_RIGHT:
        case SNAPTARGET_DISTRIBUTION_LEFT:
        case SNAPTARGET_DISTRIBUTION_UP:
        case SNAPTARGET_DISTRIBUTION_DOWN: {
            Geom::Coord x, y; 
            Geom::Point p1, p2;
            Inkscape::CanvasItemCurve *point1, *point2;

            for (auto it = p.getBBoxes().begin(); it + 1 != p.getBBoxes().end(); it++) {
                switch (p.getTarget()) {
                    case SNAPTARGET_DISTRIBUTION_RIGHT: 
                    case SNAPTARGET_DISTRIBUTION_LEFT:
                    case SNAPTARGET_DISTRIBUTION_X:
                        y = get_y(*it,*std::next(it));
                        p1 = Geom::Point(it->max().x(), y);
                        p2 = Geom::Point(std::next(it)->min().x(), y);
                        text_pos = (p1 + p2)/2 + _desktop->w2d(Geom::Point(0, -2*fontsize));

                        point1 = make_stub_line_v(p1);
                        point2 = make_stub_line_v(p2);
                        break;

                    case SNAPTARGET_DISTRIBUTION_DOWN:
                    case SNAPTARGET_DISTRIBUTION_UP:
                    case SNAPTARGET_DISTRIBUTION_Y:
                        x = get_x(*it,*std::next(it));
                        p1 = Geom::Point(x, it->max().y());
                        p2 = Geom::Point(x, std::next(it)->min().y());
                        text_pos = (p1 + p2)/2 + _desktop->w2d(Geom::Point(-2*fontsize, 0));

                        point1 = make_stub_line_h(p1);
                        point2 = make_stub_line_h(p2);
                        break;
                }

                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(point1, 0));
                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(point2, 0));

                auto line1 = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p1, p2);
                line1->set_stroke(color);
                line1->set_width(2);
                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(line1, 0));

                if (show_distance) {
                    auto text = new Inkscape::CanvasItemText(_desktop->getCanvasTemp(), text_pos, distance);
                    text->set_fontsize(fontsize);
                    text->set_fill(text_fill);
                    text->set_background(text_bg);
                    _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(text, 0));
                }
            }
        break;
        }
        case SNAPTARGET_DISTRIBUTION_XY: {
            Geom::Coord x, y; 
            Geom::Point p1, p2;
            Inkscape::CanvasItemCurve *point1, *point2;

            auto equal_dist2 = Inkscape::Util::Quantity::convert(p.getDistributionDistance2(), "px", unit_name);
            Glib::ustring distance2 = Glib::ustring::format(std::fixed, std::setprecision(1), std::noshowpoint, scale*equal_dist2);

            for (auto it = p.getBBoxes().begin(); it + 1 != p.getBBoxes().end(); it++) {
                y = get_y(*it,*std::next(it));
                p1 = Geom::Point(it->max().x(), y);
                p2 = Geom::Point(std::next(it)->min().x(), y);
                text_pos = (p1 + p2)/2 + _desktop->w2d(Geom::Point(0, -2*fontsize));

                point1 = make_stub_line_v(p1);
                point2 = make_stub_line_v(p2);

                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(point1, 0));
                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(point2, 0));

                auto line1 = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p1, p2);
                line1->set_stroke(color);
                line1->set_width(2);
                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(line1, 0));

                if (show_distance) {
                    auto text = new Inkscape::CanvasItemText(_desktop->getCanvasTemp(), text_pos, distance);
                    text->set_fontsize(fontsize);
                    text->set_fill(text_fill);
                    text->set_background(text_bg);
                    _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(text, 0));
                }
            }

            for (auto it = p.getBBoxes2().begin(); it + 1 != p.getBBoxes2().end(); it++) {
                x = get_x(*it,*std::next(it));
                p1 = Geom::Point(x, it->max().y());
                p2 = Geom::Point(x, std::next(it)->min().y());
                text_pos = (p1 + p2)/2 + _desktop->w2d(Geom::Point(-2*fontsize, 0));

                point1 = make_stub_line_h(p1);
                point2 = make_stub_line_h(p2);

                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(point1, 0));
                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(point2, 0));

                auto line1 = new Inkscape::CanvasItemCurve(_desktop->getCanvasTemp(), p1, p2);
                line1->set_stroke(color);
                line1->set_width(2);
                _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(line1, 0));

                if (show_distance) {
                    auto text = new Inkscape::CanvasItemText(_desktop->getCanvasTemp(), text_pos, distance2);
                    text->set_fontsize(fontsize);
                    text->set_fill(text_fill);
                    text->set_background(text_bg);
                    _distribution_snap_indicators.push_back(_desktop->add_temporary_canvasitem(text, 0));
                }
            }

            break;
        }
    }
}

} //namespace Display
} /* namespace Inkscape */


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
