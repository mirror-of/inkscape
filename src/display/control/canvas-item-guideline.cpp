// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * A class to represent a control guide line.
 */

/*
 * Authors:
 *   Tavmjong Bah       - Rewrite of SPGuideLine
 *   Rafael Siejakowski - Tweaks to handle appearance
 *
 * Copyright (C) 2020-2022 the Authors.
 *
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "canvas-item-guideline.h"

#include <2geom/line.h>

#include "canvas-item-ctrl.h"

#include "desktop.h" // Canvas orientation so label is orientated correctly.

#include "ui/widget/canvas.h"

namespace Inkscape {

/**
 * Create a control guide line. Points are in document units.
 */
CanvasItemGuideLine::CanvasItemGuideLine(CanvasItemGroup *group, Glib::ustring label,
                                         Geom::Point const &origin, Geom::Point const &normal)
    : CanvasItem(group)
    , _origin(origin)
    , _normal(normal)
    , _label(std::move(label))
{
    _name = "CanvasItemGuideLine:" + _label;
    _pickable = true; // For now, everybody gets events from this class!

    // Required when rotating canvas:
    _bounds = Geom::Rect(-Geom::infinity(), -Geom::infinity(), Geom::infinity(), Geom::infinity());

    // Control to move guide line.
    _origin_ctrl = std::make_unique<CanvasItemGuideHandle>(group, _origin, this);
    _origin_ctrl->set_name("CanvasItemGuideLine:Ctrl:" + _label);
    _origin_ctrl->set_size_default();
    _origin_ctrl->set_pickable(true); // The handle will also react to dragging
    set_locked(false); // Init _origin_ctrl shape and stroke.
}

CanvasItemGuideLine::~CanvasItemGuideLine() = default;

/**
 * Sets origin of guide line (place where handle is located).
 */
void CanvasItemGuideLine::set_origin(Geom::Point const &origin)
{
    if (_origin != origin) {
        _origin = origin;
        _origin_ctrl->set_position(_origin);
        request_update();
    }
}

/**
 * Sets orientation of guide line.
 */
void CanvasItemGuideLine::set_normal(Geom::Point const &normal)
{
    if (_normal != normal) {
        _normal = normal;
        request_update();
    }
}

/**
 * Sets the inverted nature of the line
 */
void CanvasItemGuideLine::set_inverted(bool inverted)
{
    if (_inverted != inverted) {
        _inverted = inverted;
        request_update();
    }
}


/**
 * Returns distance between point in canvas units and nearest point on guideLine.
 */
double CanvasItemGuideLine::closest_distance_to(Geom::Point const &p)
{
    // Maybe store guide as a Geom::Line?
    Geom::Line guide =
        Geom::Line::from_origin_and_vector(_origin, Geom::rot90(_normal));
    guide *= _affine;
    return Geom::distance(p, guide);
}

/**
 * Returns true if point p (in canvas units) is within tolerance (canvas units) distance of guideLine (or 1 if tolerance is zero).
 */
bool CanvasItemGuideLine::contains(Geom::Point const &p, double tolerance)
{
    if (tolerance == 0) {
        tolerance = 1; // Can't pick of zero!
    }

    return closest_distance_to(p) < tolerance;
}

/**
 * Returns the pointer to the origin control (the "dot")
 */
CanvasItemGuideHandle* CanvasItemGuideLine::dot() const
{
    return _origin_ctrl.get();
}

/**
 * Update and redraw control guideLine.
 */
void CanvasItemGuideLine::update(Geom::Affine const &affine)
{
    if (_affine == affine && !_need_update) {
        // Nothing to do.
        return;
    }

    _affine = affine;

    // Queue redraw of new area (and old too).
    request_redraw();

    _need_update = false;
}

/**
 * Render guideLine to screen via Cairo.
 */
void CanvasItemGuideLine::render(Inkscape::CanvasItemBuffer *buf)
{
    if (!buf) {
        std::cerr << "CanvasItemGuideLine::Render: No buffer!" << std::endl;
        return;
    }

    if (!_visible) {
        // Hidden
        return;
    }

    // Document to canvas
    Geom::Point const normal = _normal * _affine.withoutTranslation(); // Direction only
    Geom::Point const origin = _origin * _affine;

    /* Need to use floor()+0.5 such that Cairo will draw us lines with a width of a single pixel,
     * without any aliasing. For this we need to position the lines at exactly half pixels, see
     * https://www.cairographics.org/FAQ/#sharp_lines
     * Must be consistent with the pixel alignment of the grid lines, see CanvasXYGrid::Render(),
     * and the drawing of the rulers.
     * Lastly, the origin control is also pixel-aligned and we want to visually cut through its
     * exact center.
     */
    Geom::Point const aligned_origin = origin.floor() + Geom::Point(0.5, 0.5);

    // Set up the Cairo rendering context
    Cairo::RefPtr<Cairo::Context> ctx = buf->cr;
    ctx->save();
    ctx->translate(-buf->rect.left(), -buf->rect.top()); // Canvas to screen
    ctx->set_source_rgba(SP_RGBA32_R_F(_stroke), SP_RGBA32_G_F(_stroke),
                         SP_RGBA32_B_F(_stroke), SP_RGBA32_A_F(_stroke));
    ctx->set_line_width(1);

    if (_inverted) {
        // operator not available in cairo C++ bindings
        cairo_set_operator(ctx->cobj(), CAIRO_OPERATOR_DIFFERENCE);
    }

    if (!_label.empty()) { // Render text label
        ctx->save();
        ctx->translate(aligned_origin.x(), aligned_origin.y());

        SPDesktop *desktop = nullptr;
        if (_canvas) {
            desktop = _canvas->get_desktop();
        }
        ctx->rotate(atan2(normal.cw()) + M_PI * (desktop && desktop->is_yaxisdown() ? 1 : 0));
        ctx->translate(0, -(_origin_ctrl->radius() + LABEL_SEP)); // Offset by dot radius + 2
        ctx->move_to(0, 0);
        ctx->show_text(_label);
        ctx->restore();
    }

    // Draw guide.
    // Special case: horizontal and vertical lines (easier calculations)

    // Don't use isHorizontal()/isVertical() as they test only exact matches.
    if (Geom::are_near(normal.y(), 0.0)) {
        // Vertical
        double const position = aligned_origin.x();
        ctx->move_to(position, buf->rect.top()    + 0.5);
        ctx->line_to(position, buf->rect.bottom() - 0.5);
    } else if (Geom::are_near(normal.x(), 0.0)) {
        // Horizontal
        double position = aligned_origin.y();
        ctx->move_to(buf->rect.left()  + 0.5, position);
        ctx->line_to(buf->rect.right() - 0.5, position);
    } else {
        // Angled
        Geom::Line line = Geom::Line::from_origin_and_vector(aligned_origin, Geom::rot90(normal));

        // Find intersections of the line with buf rectangle. There should be zero or two.
        std::vector<Geom::Point> intersections;
        for (unsigned i = 0; i < 4; ++i) {
            Geom::LineSegment side(buf->rect.corner(i), buf->rect.corner((i+1)%4));
            try {
                Geom::OptCrossing oc = Geom::intersection(line, side);
                if (oc) {
                    intersections.push_back(line.pointAt((*oc).ta));
                }
            } catch (Geom::InfiniteSolutions) {
                // Shouldn't happen as we have already taken care of horizontal/vertical guides.
                std::cerr << "CanvasItemGuideLine::render: Error: Infinite intersections." << std::endl;
            }
        }

        if (intersections.size() == 2) {
            double const x0 = intersections[0].x();
            double const x1 = intersections[1].x();
            double const y0 = intersections[0].y();
            double const y1 = intersections[1].y();
            ctx->move_to(x0, y0);
            ctx->line_to(x1, y1);
        }
    }
    ctx->stroke();

    ctx->restore();
}

void CanvasItemGuideLine::hide()
{
    CanvasItem::hide();
    _origin_ctrl->hide();
}

void CanvasItemGuideLine::show()
{
    CanvasItem::show();
    _origin_ctrl->show();
}

void CanvasItemGuideLine::set_stroke(guint32 color)
{
    // Make sure the fill of the control is the same as the stroke
    // of the guide-line:
    _origin_ctrl->set_fill(color);
    CanvasItem::set_stroke(color);
}

void CanvasItemGuideLine::set_label(Glib::ustring const & label)
{
    if (_label != label) {
        _label = label;
        request_update();
    }
}

void CanvasItemGuideLine::set_locked(bool locked)
{
    if (_locked != locked) {
        _locked = locked;
        if (_locked) {
            _origin_ctrl->set_shape(CANVAS_ITEM_CTRL_SHAPE_CROSS);
            _origin_ctrl->set_stroke(CONTROL_LOCKED_COLOR);
            _origin_ctrl->set_fill(0x00000000);   // no fill
        } else {
            _origin_ctrl->set_shape(CANVAS_ITEM_CTRL_SHAPE_CIRCLE);
            _origin_ctrl->set_stroke(0x00000000); // no stroke
            _origin_ctrl->set_fill(_stroke);      // fill the control with this guide's color
        }
    }
}

//===============================================================================================

/**
 * @brief Create a handle ("dot") along a guide line
 * @param group - the associated canvas item group
 * @param pos   - position
 * @param line  - pointer to the corresponding guide line
 */
CanvasItemGuideHandle::CanvasItemGuideHandle(CanvasItemGroup *group,
                                             Geom::Point const &pos,
                                             CanvasItemGuideLine* line)
    : CanvasItemCtrl(group, CANVAS_ITEM_CTRL_SHAPE_CIRCLE, pos)
    , _my_line(line) // Save a pointer to our guide line
{
}

/**
 * Return the radius of the handle dot
 */
double CanvasItemGuideHandle::radius() const
{
    return 0.5 * static_cast<double>(_width); // radius is half the width
}

/**
 * Update the size of the handle based on the index from Preferences
 */
void CanvasItemGuideHandle::set_size_via_index(int index)
{
    double const r = static_cast<double>(index) * SCALE;
    unsigned long const rounded_diameter = std::lround(r * 2.0); // diameter is twice the radius
    unsigned long size = rounded_diameter | 0x1; // make sure the size is always odd
    if (size < MINIMUM_SIZE) {
        size = MINIMUM_SIZE;
    }
    if (_width != size) {
        _width = size;
        _height = size;
        _built = false;
        request_update();
        _my_line->request_update();
    }
}

} // namespace Inkscape

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
