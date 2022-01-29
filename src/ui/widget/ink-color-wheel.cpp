// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * HSLuv color wheel widget, based on the web implementation at
 * https://www.hsluv.org
 *//*
 * Authors:
 *   Tavmjong Bah
 *   Massinissa Derriche <massinissa.derriche@gmail.com>
 *
 * Copyright (C) 2018, 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "ink-color-wheel.h"

#include <cstring>
#include <algorithm>

#include "hsluv.h"

// Sizes in pixels
static int const SIZE = 400;
static int const OUTER_CIRCLE_RADIUS = 190;

static double const MAX_HUE = 360.0;
static double const MAX_SATURATION = 100.0;
static double const MAX_LIGHTNESS = 100.0;
static double const MIN_HUE = 0.0;
static double const MIN_SATURATION = 0.0;
static double const MIN_LIGHTNESS = 0.0;
static double const OUTER_CIRCLE_DASH_SIZE = 10.0;

using Hsluv::Line;

class ColorPoint {
public:
    ColorPoint();
    ColorPoint(double x, double y, double r, double g, double b);
    ColorPoint(double x, double y, guint color);

    guint32 get_color();
    void set_color(double red, double green, double blue);

    double x;
    double y;
    double r;
    double g;
    double b;
};

class Point {
public:
    Point();
    Point(double x, double y);

    double x;
    double y;
};

class Intersection {
public:
    Intersection();
    Intersection (int line1, int line2, Point intersectionPoint,
            double intersectionPointAngle, double relativeAngle);

    int line1;
    int line2;
    Point intersectionPoint;
    double intersectionPointAngle;
    double relativeAngle;
};

static Point intersect_line_line(Line a, Line b);
static double distance_from_origin(Point point);
static double distance_line_from_origin(Line line);
static double angle_from_origin(Point point);
static double normalize_angle(double angle);
static double lerp(double v0, double v1, double t0, double t1, double t);
static ColorPoint lerp(ColorPoint const &v0, ColorPoint const &v1, double t0, double t1,
        double t);
static guint32 hsv_to_rgb(double h, double s, double v);
static double luminance(guint32 color);
static Point to_pixel_coordinate(Point const &point, double scale, double resize);
static Point from_pixel_coordinate(Point const &point, double scale, double resize);
static std::vector<Point> to_pixel_coordinate( std::vector<Point> const &points, double scale,
        double resize);
static void draw_vertical_padding(ColorPoint p0, ColorPoint p1, int padding,
        bool pad_upwards, guint32 *buffer, int height, int stride);

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Used to represent the in RGB gamut colors polygon of the color wheel.
 *
 * @struct
 */
struct PickerGeometry {
    std::vector<Line> lines;
    /** Ordered such that 1st vertex is intersection between first and second
     *  line, 2nd vertex between second and third line etc. */
    std::vector<Point> vertices;
    /** Angles from origin to corresponding vertex, in radians */
    std::vector<double> angles;
    /** Smallest circle with center at origin such that polygon fits inside */
    double outerCircleRadius;
    /** Largest circle with center at origin such that it fits inside polygon */
    double innerCircleRadius;
};

/**
 * Update the passed in PickerGeometry structure to the given lightness value.
 *
 * @param[out] pickerGeometry The PickerGeometry instance to update.
 * @param lightness The lightness value.
 */
static void get_picker_geometry(PickerGeometry *pickerGeometry, double lightness);

/* Base Color Wheel */
ColorWheel::ColorWheel()
    : _adjusting(false)
{
    set_name("ColorWheel");

    add_events(Gdk::BUTTON_PRESS_MASK	|
	       Gdk::BUTTON_RELEASE_MASK	|
	       Gdk::BUTTON_MOTION_MASK	|
	       Gdk::KEY_PRESS_MASK	);
    set_can_focus();
}

void ColorWheel::setRgb(double /*r*/, double /*g*/, double /*b*/, bool /*overrideHue*/)
{}

void ColorWheel::getRgb(double */*r*/, double */*g*/, double */*b*/) const
{}

void ColorWheel::getRgbV(double *rgb) const {}

guint32 ColorWheel::getRgb() const { return 0; }

void ColorWheel::setHue(double h)
{
    _values[0] = std::clamp(h, MIN_HUE, MAX_HUE);
}

void ColorWheel::setSaturation(double s)
{
    _values[1] = std::clamp(s, MIN_SATURATION, MAX_SATURATION);
}

void ColorWheel::setLightness(double l)
{
    _values[2] = std::clamp(l, MIN_LIGHTNESS, MAX_LIGHTNESS);
}

void ColorWheel::getValues(double *a, double *b, double *c) const
{
    if (a) *a = _values[0];
    if (b) *b = _values[1];
    if (c) *c = _values[2];
}

void ColorWheel::_set_from_xy(double const x, double const y)
{}

bool ColorWheel::on_key_release_event(GdkEventKey* key_event)
{
    unsigned int key = 0;
    gdk_keymap_translate_keyboard_state( Gdk::Display::get_default()->get_keymap(),
                                         key_event->hardware_keycode,
                                         (GdkModifierType)key_event->state,
                                         0, &key, nullptr, nullptr, nullptr );

    switch (key) {
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            _adjusting = false;
            return true;
    }

    return false;
}

sigc::signal<void> ColorWheel::signal_color_changed()
{
    return _signal_color_changed;
}

/* HSL Color Wheel */
void ColorWheelHSL::setRgb(double r, double g, double b, bool overrideHue)
{
    double min = std::min({r, g, b});
    double max = std::max({r, g, b});

    _values[2] = max;

    if (min == max) {
        if (overrideHue) {
            _values[0] = 0.0;
        }
    } else {
        if (max == r) {
            _values[0] = ((g - b) / (max - min)    ) / 6.0;
        } else if (max == g) {
            _values[0] = ((b - r) / (max - min) + 2) / 6.0;
        } else {
            _values[0] = ((r - g) / (max - min) + 4) / 6.0;
        }

        if (_values[0] < 0.0) {
            _values[0] += 1.0;
        }
    }

    if (max == 0) {
        _values[1] = 0;
    } else {
        _values[1] = (max - min) / max;
    }
}

void ColorWheelHSL::getRgb(double *r, double *g, double *b) const
{
    guint32 color = getRgb();
    *r = ((color & 0x00ff0000) >> 16) / 255.0;
    *g = ((color & 0x0000ff00) >>  8) / 255.0;
    *b = ((color & 0x000000ff)      ) / 255.0;
}

void ColorWheelHSL::getRgbV(double *rgb) const
{
    guint32 color = getRgb();
    rgb[0] = ((color & 0x00ff0000) >> 16) / 255.0;
    rgb[1] = ((color & 0x0000ff00) >>  8) / 255.0;
    rgb[2] = ((color & 0x000000ff)      ) / 255.0;
}

guint32 ColorWheelHSL::getRgb() const
{
    return hsv_to_rgb(_values[0], _values[1], _values[2]);
}

void ColorWheelHSL::getHsl(double *h, double *s, double *l) const
{
    getValues(h, s, l);
}

bool ColorWheelHSL::on_draw(::Cairo::RefPtr<::Cairo::Context> const &cr)
{
    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();

    int const cx = width/2;
    int const cy = height/2;

    int const stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_RGB24, width);

    int focus_line_width;
    int focus_padding;
    get_style_property("focus-line-width", focus_line_width);
    get_style_property("focus-padding",    focus_padding);

    // Paint ring
    guint32* buffer_ring = g_new (guint32, height * stride / 4);
    double r_max = std::min(width, height)/2.0 - 2 * (focus_line_width + focus_padding);
    double r_min = r_max * (1.0 - _ring_width);
    double r2_max = (r_max+2) * (r_max+2); // Must expand a bit to avoid edge effects.
    double r2_min = (r_min-2) * (r_min-2); // Must shrink a bit to avoid edge effects.

    for (int i = 0; i < height; ++i) {
        guint32* p = buffer_ring + i * width;
        double dy = (cy - i);
        for (int j = 0; j < width; ++j) {
            double dx = (j - cx);
            double r2 = dx * dx + dy * dy;
            if (r2 < r2_min || r2 > r2_max) {
                *p++ = 0; // Save calculation time.
            } else {
                double angle = atan2 (dy, dx);
                if (angle < 0.0) {
                    angle += 2.0 * M_PI;
                }
                double hue = angle/(2.0 * M_PI);

                *p++ = hsv_to_rgb(hue, 1.0, 1.0);
            }
        }
    }

    Cairo::RefPtr<::Cairo::ImageSurface> source_ring =
        ::Cairo::ImageSurface::create((unsigned char *)buffer_ring,
                                      Cairo::FORMAT_RGB24,
                                      width, height, stride);

    cr->set_antialias(Cairo::ANTIALIAS_SUBPIXEL);

    // Paint line on ring in source (so it gets clipped by stroke).
    double l = 0.0;
    guint32 color_on_ring = hsv_to_rgb(_values[0], 1.0, 1.0);
    if (luminance(color_on_ring) < 0.5) l = 1.0;

    Cairo::RefPtr<::Cairo::Context> cr_source_ring = ::Cairo::Context::create(source_ring);
    cr_source_ring->set_source_rgb(l, l, l);

    cr_source_ring->move_to (cx, cy);
    cr_source_ring->line_to (cx + cos(_values[0] * M_PI * 2.0) * r_max+1,
                             cy - sin(_values[0] * M_PI * 2.0) * r_max+1);
    cr_source_ring->stroke();

    // Paint with ring surface, clipping to ring.
    cr->save();
    cr->set_source(source_ring, 0, 0);
    cr->set_line_width (r_max - r_min);
    cr->begin_new_path();
    cr->arc(cx, cy, (r_max + r_min)/2.0, 0, 2.0 * M_PI);
    cr->stroke();
    cr->restore();

    g_free(buffer_ring);

    // Draw focus
    if (has_focus() && _focus_on_ring) {
        Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
        style_context->render_focus(cr, 0, 0, width, height);
    }
  
    // Paint triangle.
    /* The triangle is painted by first finding color points on the
     * edges of the triangle at the same y value via linearly
     * interpolating between corner values, and then interpolating along
     * x between the those edge points. The interpolation is in sRGB
     * space which leads to a complicated mapping between x/y and
     * saturation/value. This was probably done to remove the need to
     * convert between HSV and RGB for each pixel.
     * Black corner: v = 0, s = 1
     * White corner: v = 1, s = 0
     * Color corner; v = 1, s = 1
     */
    const int padding = 3; // Avoid edge artifacts.
    double x0, y0, x1, y1, x2, y2;
    _triangle_corners(x0, y0, x1, y1, x2, y2);
    guint32 color0 = hsv_to_rgb(_values[0], 1.0, 1.0);
    guint32 color1 = hsv_to_rgb(_values[0], 1.0, 0.0);
    guint32 color2 = hsv_to_rgb(_values[0], 0.0, 1.0);

    ColorPoint p0 (x0, y0, color0);
    ColorPoint p1 (x1, y1, color1);
    ColorPoint p2 (x2, y2, color2);

    // Reorder so we paint from top down.
    if (p1.y > p2.y) {
        std::swap(p1, p2);
    }

    if (p0.y > p2.y) {
        std::swap(p0, p2);
    }

    if (p0.y > p1.y) {
        std::swap(p0, p1);
    }

    guint32* buffer_triangle = g_new(guint32, height * stride / 4);

    for (int y = 0; y < height; ++y) {
        guint32 *p = buffer_triangle + y * (stride / 4);

        if (p0.y <= y+padding && y-padding < p2.y) {

            // Get values on side at position y.
            ColorPoint side0;
            double y_inter = std::clamp(static_cast<double>(y), p0.y, p2.y);
            if (y < p1.y) {
                side0 = lerp(p0, p1, p0.y, p1.y, y_inter);
            } else {
                side0 = lerp(p1, p2, p1.y, p2.y, y_inter);
            }
            ColorPoint side1 = lerp(p0, p2, p0.y, p2.y, y_inter);

            // side0 should be on left
            if (side0.x > side1.x) {
                std::swap (side0, side1);
            }

            int x_start = std::max(0, int(side0.x));
            int x_end   = std::min(int(side1.x), width);

            for (int x = 0; x < width; ++x) {
                if (x <= x_start) {
                    *p++ = side0.get_color();
                } else if (x < x_end) {
                    *p++ = lerp(side0, side1, side0.x, side1.x, x).get_color();
                } else {
                    *p++ = side1.get_color();
                }
            }
        }
    }

    // add vertical padding to each side separately
    ColorPoint temp_point = lerp(p0, p1, p0.x, p1.x, (p0.x + p1.x) / 2.0);
    bool pad_upwards = _is_in_triangle(temp_point.x, temp_point.y + 1);
    draw_vertical_padding(p0, p1, padding, pad_upwards, buffer_triangle, height, stride / 4);

    temp_point = lerp(p0, p2, p0.x, p2.x, (p0.x + p2.x) / 2.0);
    pad_upwards = _is_in_triangle(temp_point.x, temp_point.y + 1);
    draw_vertical_padding(p0, p2, padding, pad_upwards, buffer_triangle, height, stride / 4);

    temp_point = lerp(p1, p2, p1.x, p2.x, (p1.x + p2.x) / 2.0);
    pad_upwards = _is_in_triangle(temp_point.x, temp_point.y + 1);
    draw_vertical_padding(p1, p2, padding, pad_upwards, buffer_triangle, height, stride / 4);

    Cairo::RefPtr<::Cairo::ImageSurface> source_triangle =
        ::Cairo::ImageSurface::create((unsigned char *)buffer_triangle,
                                      Cairo::FORMAT_RGB24,
                                      width, height, stride);

    // Paint with triangle surface, clipping to triangle.
    cr->save();
    cr->set_source(source_triangle, 0, 0);
    cr->move_to(p0.x, p0.y);
    cr->line_to(p1.x, p1.y);
    cr->line_to(p2.x, p2.y);
    cr->close_path();
    cr->fill();
    cr->restore();

    g_free(buffer_triangle);

    // Draw marker
    double mx = x1 + (x2-x1) * _values[2] + (x0-x2) * _values[1] * _values[2];
    double my = y1 + (y2-y1) * _values[2] + (y0-y2) * _values[1] * _values[2];

    double a = 0.0;
    guint32 color_at_marker = getRgb();
    if (luminance(color_at_marker) < 0.5) a = 1.0;

    cr->set_source_rgb(a, a, a);
    cr->begin_new_path();
    cr->arc(mx, my, 4, 0, 2 * M_PI);
    cr->stroke();

    // Draw focus
    if (has_focus() && !_focus_on_ring) {
        Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
        style_context->render_focus(cr, mx-4, my-4, 8, 8); // This doesn't seem to work.
        cr->set_line_width(0.5);
        cr->set_source_rgb(1-a, 1-a, 1-a);
        cr->begin_new_path();
        cr->arc(mx, my, 7, 0, 2 * M_PI);
        cr->stroke();
    }

    return true;
}

bool ColorWheelHSL::on_focus(Gtk::DirectionType direction)
{
    // In forward direction, focus passes from no focus to ring focus to triangle
    // focus to no focus.
    if (!has_focus()) {
        _focus_on_ring = (direction == Gtk::DIR_TAB_FORWARD);
        grab_focus();
        return true;
    }

    // Already have focus
    bool keep_focus = false;

    switch (direction) {
        case Gtk::DIR_UP:
        case Gtk::DIR_LEFT:
        case Gtk::DIR_TAB_BACKWARD:
            if (!_focus_on_ring) {
                _focus_on_ring = true;
                keep_focus = true;
            }
            break;

        case Gtk::DIR_DOWN:
        case Gtk::DIR_RIGHT:
        case Gtk::DIR_TAB_FORWARD:
            if (_focus_on_ring) {
                _focus_on_ring = false;
                keep_focus = true;
            }
            break;
    }

    queue_draw();  // Update focus indicators.

    return keep_focus;
}

void ColorWheelHSL::_set_from_xy(double const x, double const y)
{
    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();

    double const cx = width/2.0;
    double const cy = height/2.0;
    double const r = std::min(cx, cy) * (1 - _ring_width);

    // We calculate RGB value under the cursor by rotating the cursor
    // and triangle by the hue value and looking at position in the
    // now right pointing triangle.
    double angle = _values[0]  * 2 * M_PI;
    double sin = std::sin(angle);
    double cos = std::cos(angle);
    double xp =  ((x - cx) * cos - (y - cy) * sin) / r;
    double yp =  ((x - cx) * sin + (y - cy) * cos) / r;

    double xt = lerp(0.0, 1.0, -0.5, 1.0, xp);
    xt = std::clamp(xt, 0.0, 1.0);

    double dy = (1-xt) * std::cos(M_PI / 6.0);
    double yt = lerp(0.0, 1.0, -dy, dy, yp);
    yt = std::clamp(yt, 0.0, 1.0);

    ColorPoint c0(0, 0, yt, yt, yt);                    // Grey point along base.
    ColorPoint c1(0, 0, hsv_to_rgb(_values[0], 1, 1));  // Hue point at apex
    ColorPoint c = lerp(c0, c1, 0, 1, xt);

    setRgb(c.r, c.g, c.b, false); // Don't override previous hue.
}

bool ColorWheelHSL::_is_in_ring(double x, double y)
{
    Gtk::Allocation allocation = get_allocation();
    int const width  = allocation.get_width();
    int const height = allocation.get_height();

    int const cx = width/2;
    int const cy = height/2;

    int focus_line_width;
    int focus_padding;
    get_style_property("focus-line-width", focus_line_width);
    get_style_property("focus-padding",    focus_padding);

    double r_max = std::min( width, height)/2.0 - 2 * (focus_line_width + focus_padding);
    double r_min = r_max * (1.0 - _ring_width);
    double r2_max = r_max * r_max;
    double r2_min = r_min * r_min;

    double dx = x - cx;
    double dy = y - cy;
    double r2 = dx * dx + dy * dy;
  
    return (r2_min < r2 && r2 < r2_max);
}

bool ColorWheelHSL::_is_in_triangle(double x, double y)
{
    double x0, y0, x1, y1, x2, y2;
    _triangle_corners(x0, y0, x1, y1, x2, y2);

    double det = (x2-x1) * (y0-y1) - (y2-y1) * (x0-x1);
    double s = ((x -x1) * (y0-y1) - (y -y1) * (x0-x1)) / det; 
    double t = ((x2-x1) * (y -y1) - (y2-y1) * (x -x1)) / det; 

    return (s >= 0.0 && t >= 0.0 && s+t <= 1.0);
}

void ColorWheelHSL::_update_triangle_color(double x, double y)
{
    _set_from_xy(x, y);
    _signal_color_changed.emit();
    queue_draw();
}

void ColorWheelHSL::_triangle_corners(double &x0, double &y0, double &x1, double &y1,
        double &x2, double &y2)
{
    Gtk::Allocation allocation = get_allocation();
    int const width  = allocation.get_width();
    int const height = allocation.get_height();

    int const cx = width / 2;
    int const cy = height / 2;

    int focus_line_width;
    int focus_padding;
    get_style_property("focus-line-width", focus_line_width);
    get_style_property("focus-padding",    focus_padding);

    double r_max = std::min(width, height) / 2.0 - 2 * (focus_line_width + focus_padding);
    double r_min = r_max * (1.0 - _ring_width);

    double angle = _values[0] * 2.0 * M_PI;

    x0 = cx + std::cos(angle) * r_min;
    y0 = cy - std::sin(angle) * r_min;
    x1 = cx + std::cos(angle + 2.0 * M_PI / 3.0) * r_min;
    y1 = cy - std::sin(angle + 2.0 * M_PI / 3.0) * r_min;
    x2 = cx + std::cos(angle + 4.0 * M_PI / 3.0) * r_min;
    y2 = cy - std::sin(angle + 4.0 * M_PI / 3.0) * r_min;
}

void ColorWheelHSL::_update_ring_color(double x, double y)
{
    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();

    double cx = width / 2.0;
    double cy = height / 2.0;
    double angle = -atan2(y - cy, x - cx);

    if (angle < 0) {
        angle += 2.0 * M_PI;
    }
    _values[0] = angle / (2.0 * M_PI);

    queue_draw();
    _signal_color_changed.emit();
}

bool ColorWheelHSL::on_button_press_event(GdkEventButton* event)
{
    // Seat is automatically grabbed.
    double x = event->x;
    double y = event->y;

    if (_is_in_ring(x, y) ) {
        _adjusting = true;
        _mode = DragMode::HUE;
        grab_focus();
        _focus_on_ring = true;
        _update_ring_color(x, y);
        return true;
    } else if (_is_in_triangle(x, y)) {
        _adjusting = true;
        _mode = DragMode::SATURATION_VALUE;
        grab_focus();
        _focus_on_ring = false;
        _update_triangle_color(x, y);
        return true;
    }

    return false;
}

bool ColorWheelHSL::on_button_release_event(GdkEventButton */*event*/)
{
    _mode = DragMode::NONE;

    _adjusting = false;
    return true;
}

bool ColorWheelHSL::on_motion_notify_event(GdkEventMotion* event)
{
    if (!_adjusting) { return false; }

    double x = event->x;
    double y = event->y;

    if (_mode == DragMode::HUE) {
        _update_ring_color(x, y);
        return true;
    } else if (_mode == DragMode::SATURATION_VALUE) {
        _update_triangle_color(x, y);
        return true;
    } else {
        return false;
    }
}

bool ColorWheelHSL::on_key_press_event(GdkEventKey* key_event)
{
    bool consumed = false;

    unsigned int key = 0;
    gdk_keymap_translate_keyboard_state(Gdk::Display::get_default()->get_keymap(),
                                        key_event->hardware_keycode,
                                        (GdkModifierType)key_event->state,
                                        0, &key, nullptr, nullptr, nullptr);

    double x0, y0, x1, y1, x2, y2;
    _triangle_corners(x0, y0, x1, y1, x2, y2);

    // Marker position
    double mx = x1 + (x2-x1) * _values[2] + (x0-x2) * _values[1] * _values[2];
    double my = y1 + (y2-y1) * _values[2] + (y0-y2) * _values[1] * _values[2];

    double const delta_hue = 2.0 / 360.0;

    switch (key) {
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            if (_focus_on_ring) {
                _values[0] += delta_hue;
            } else {
                my -= 1.0;
                _set_from_xy(mx, my);
            }
            consumed = true;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            if (_focus_on_ring) {
                _values[0] -= delta_hue;
            } else {
                my += 1.0;
                _set_from_xy(mx, my);
            }
            consumed = true;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            if (_focus_on_ring) {
                _values[0] += delta_hue;
            } else {
                mx -= 1.0;
                _set_from_xy(mx, my);
            }
            consumed = true;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            if (_focus_on_ring) {
                _values[0] -= delta_hue;
            } else {
                mx += 1.0;
                _set_from_xy(mx, my);
            }
            consumed = true;
            break;
    }

    if (consumed) {
        if (_values[0] >= 1.0) {
            _values[0] -= 1.0;
        } else if (_values[0] <  0.0) {
            _values[0] += 1.0;
        }

        _signal_color_changed.emit();
        queue_draw();
    }

    return consumed;
}

/* HSLuv Color Wheel */
ColorWheelHSLuv::ColorWheelHSLuv()
    : _scale(1.0)
    , _cache_width(0)
    , _cache_height(0)
    , _square_size(1)
{
    _picker_geometry = new PickerGeometry;
    setHsluv(0.0, 100.0, 50.0);
}

ColorWheelHSLuv::~ColorWheelHSLuv()
{
    delete _picker_geometry;
}

void ColorWheelHSLuv::setRgb(double r, double g, double b, bool /*overrideHue*/)
{
    double h, s ,l;
    Hsluv::rgb_to_hsluv(r, g, b, &h, &s, &l);

    setHue(h);
    setSaturation(s);
    setLightness(l);
}

void ColorWheelHSLuv::getRgb(double *r, double *g, double *b) const
{
    Hsluv::hsluv_to_rgb(_values[0], _values[1], _values[2], r, g, b);
}

void ColorWheelHSLuv::getRgbV(double *rgb) const
{
    Hsluv::hsluv_to_rgb(_values[0], _values[1], _values[2], &rgb[0], &rgb[1], &rgb[2]);
}

guint32 ColorWheelHSLuv::getRgb() const
{
    double r, g, b;
    Hsluv::hsluv_to_rgb(_values[0], _values[1], _values[2], &r, &g, &b);

    return (
        (static_cast<guint32>(r * 255.0) << 16) |
        (static_cast<guint32>(g * 255.0) <<  8) |
        (static_cast<guint32>(b * 255.0)      )
    );
}

void ColorWheelHSLuv::setHsluv(double h, double s, double l)
{
    setHue(h);
    setSaturation(s);
    setLightness(l);
}

void ColorWheelHSLuv::setLightness(double l)
{
    _values[2] = std::clamp(l, MIN_LIGHTNESS, MAX_LIGHTNESS);

    // Update polygon
    get_picker_geometry(_picker_geometry, _values[2]);
    _scale = OUTER_CIRCLE_RADIUS / _picker_geometry->outerCircleRadius;
    _update_polygon();

    queue_draw();
}

void ColorWheelHSLuv::getHsluv(double *h, double *s, double *l) const
{
    getValues(h, s, l);
}

bool ColorWheelHSLuv::on_draw(::Cairo::RefPtr<::Cairo::Context> const &cr)
{
    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();

    int const cx = width/2;
    int const cy = height/2;

    if (_values[2] < 1e-4 || _values[2] > 99.9999) {
        return true;
    }

    double const resize = std::min(width, height) / static_cast<double>(SIZE);

    int const marginX = std::max(0.0, (width - height) / 2.0);
    int const marginY = std::max(0.0, (height - width) / 2.0);

    std::vector<Point> shapePointsPixel =
        to_pixel_coordinate(_picker_geometry->vertices, _scale, resize);
    for (Point &point : shapePointsPixel) {
        point.x += marginX;
        point.y += marginY;
    }

    cr->set_antialias(Cairo::ANTIALIAS_SUBPIXEL);

    if (width > _square_size && height > _square_size) {
        if (_cache_width != width || _cache_height != height) {
            _update_polygon();
        }

        // Paint with surface, clipping to polygon
        cr->save();
        cr->set_source(_surface_polygon, 0, 0);
        cr->move_to(shapePointsPixel[0].x, shapePointsPixel[0].y);
        for (int i = 1; i < shapePointsPixel.size(); i++) {
            Point const &point = shapePointsPixel[i];
            cr->line_to(point.x, point.y);
        }
        cr->close_path();
        cr->fill();
        cr->restore();
    }

    // Draw foreground

    // Outer circle
    std::vector<double> dashes{OUTER_CIRCLE_DASH_SIZE};
    cr->set_line_width(1);
    // White dashes
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->set_dash(dashes, 0.0);
    cr->begin_new_path();
    cr->arc(cx, cy, _scale * resize * _picker_geometry->outerCircleRadius, 0, 2 * M_PI);
    cr->stroke();
    // Black dashes
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->set_dash(dashes, OUTER_CIRCLE_DASH_SIZE);
    cr->begin_new_path();
    cr->arc(cx, cy, _scale * resize * _picker_geometry->outerCircleRadius, 0, 2 * M_PI);
    cr->stroke();
    cr->unset_dash();

    // Contrast
    double a = (_values[2] > 70.0) ? 0.0 : 1.0;
    cr->set_source_rgb(a, a, a);

    // Pastel circle
    cr->set_line_width(2);
    cr->begin_new_path();
    cr->arc(cx, cy, _scale * resize * _picker_geometry->innerCircleRadius, 0, 2 * M_PI);
    cr->stroke();

    // Center
    cr->begin_new_path();
    cr->arc(cx, cy, 2, 0, 2 * M_PI);
    cr->fill();

    // Draw marker
    double l, u, v;
    Hsluv::hsluv_to_luv(_values[0], _values[1], _values[2], &l, &u, &v);
    Point mp = to_pixel_coordinate(Point(u, v), _scale, resize);
    mp.x += marginX;
    mp.y += marginY;

    cr->set_line_width(2);
    cr->begin_new_path();
    cr->arc(mp.x, mp.y, 4, 0, 2 * M_PI);
    cr->stroke();

    // Focus
    if (has_focus()) {
        Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
        style_context->render_focus(cr, mp.x-4, mp.y-4, 8, 8);

        cr->set_line_width(0.5);
        cr->set_source_rgb(1-a, 1-a, 1-a);
        cr->begin_new_path();
        cr->arc(mp.x, mp.y, 7, 0, 2 * M_PI);
        cr->stroke();
    }

    return true;
}

void ColorWheelHSLuv::_set_from_xy(double const x, double const y)
{
    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();

    double const resize = std::min(width, height) / static_cast<double>(SIZE);

    int const margin_x = std::max(0.0, (width - height) / 2.0);
    int const margin_y = std::max(0.0, (height - width) / 2.0);

    Point const p = from_pixel_coordinate(Point(
        x - margin_x,
        y - margin_y
    ), _scale, resize);

    double h, s, l;
    Hsluv::luv_to_hsluv(_values[2], p.x, p.y, &h, &s, &l);

    setHue(h);
    setSaturation(s);

    _signal_color_changed.emit();
    queue_draw();
}

void ColorWheelHSLuv::_update_polygon()
{
    if (_values[2] < 1e-4 || _values[2] > 99.9999) {
        return;
    }

    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();
    int const size = std::min(width, height);

    // Update square size
    _square_size = std::max(1, static_cast<int>(size/50));

    if (width < _square_size || height < _square_size) {
        return;
    }

    _cache_width = width;
    _cache_height = height;

    double const resize = size / static_cast<double>(SIZE);

    int const marginX = std::max(0.0, (width - height) / 2.0);
    int const marginY = std::max(0.0, (height - width) / 2.0);

    std::vector<Point> shapePointsPixel =
        to_pixel_coordinate(_picker_geometry->vertices, _scale, resize);

    for (Point &point : shapePointsPixel) {
        point.x += marginX;
        point.y += marginY;
    }

    std::vector<double> xs;
    std::vector<double> ys;

    for (Point const &point : shapePointsPixel) {
        xs.emplace_back(point.x);
        ys.emplace_back(point.y);
    }

    int const xmin = std::floor(*std::min_element(xs.begin(), xs.end()) / _square_size);
    int const ymin = std::floor(*std::min_element(ys.begin(), ys.end()) / _square_size);
    int const xmax = std::ceil(*std::max_element(xs.begin(), xs.end()) / _square_size);
    int const ymax = std::ceil(*std::max_element(ys.begin(), ys.end()) / _square_size);

    int const stride =
        Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_RGB24, width);

    _buffer_polygon.resize(height * stride / 4);
    std::vector<guint32> buffer_line;
    buffer_line.resize(stride / 4);

    ColorPoint clr;

    // Set the color of each pixel/square
    for (int y = ymin; y < ymax; y++) {
        for (int x = xmin; x < xmax; x++) {
            double px = x * _square_size;
            double py = y * _square_size;
            Point point = from_pixel_coordinate(Point(
                px + (_square_size / 2) - marginX,
                py + (_square_size / 2) - marginY
            ), _scale, resize);

            double r, g ,b;
            Hsluv::luv_to_rgb(_values[2], point.x, point.y, &r, &g, &b);

            r = std::clamp(r, 0.0, 1.0);
            g = std::clamp(g, 0.0, 1.0);
            b = std::clamp(b, 0.0, 1.0);

            clr.set_color(r, g, b);

            guint32 *p = buffer_line.data() + (x * _square_size);
            for (int i = 0; i < _square_size; i++) {
                p[i] = clr.get_color();
            }
        }

        // Copy the line buffer to the surface buffer
        int Y = y * _square_size;
        for (int i = 0; i < _square_size; i++) {
            guint32 *t = _buffer_polygon.data() + (Y + i) * (stride / 4);
            std::memcpy(t, buffer_line.data(), stride);
        }
    }

    _surface_polygon = ::Cairo::ImageSurface::create(
        reinterpret_cast<unsigned char *>(_buffer_polygon.data()),
        Cairo::FORMAT_RGB24, width, height, stride
    );
}

bool ColorWheelHSLuv::on_button_press_event(GdkEventButton* event)
{
    double const x = event->x;
    double const y = event->y;

    Gtk::Allocation allocation = get_allocation();
    int const width = allocation.get_width();
    int const height = allocation.get_height();

    int const margin_x = std::max(0.0, (width - height) / 2.0);
    int const margin_y = std::max(0.0, (height - width) / 2.0);
    int const size = std::min(width, height);

    if (x > margin_x && x < (margin_x+size) && y > margin_y && y < (margin_y+size)) {
        _adjusting = true;
        grab_focus();
        _set_from_xy(x, y);
        return true;
    }

    return false;
}

bool ColorWheelHSLuv::on_button_release_event(GdkEventButton */*event*/)
{
    _adjusting = false;
    return true;
}

bool ColorWheelHSLuv::on_motion_notify_event(GdkEventMotion* event)
{
    if (!_adjusting) { return false; }

    double x = event->x;
    double y = event->y;

    _set_from_xy(x, y);

    return true;
}

bool ColorWheelHSLuv::on_key_press_event(GdkEventKey* key_event)
{
    bool consumed = false;

    unsigned int key = 0;
    gdk_keymap_translate_keyboard_state(Gdk::Display::get_default()->get_keymap(),
                                        key_event->hardware_keycode,
                                        (GdkModifierType)key_event->state,
                                        0, &key, nullptr, nullptr, nullptr);

    // Get current point
    double l, u, v;
    Hsluv::hsluv_to_luv(_values[0], _values[1], _values[2], &l, &u, &v);

    double const marker_move = 1.0 / _scale;

    switch (key) {
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            v += marker_move;
            consumed = true;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            v -= marker_move;
            consumed = true;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            u -= marker_move;
            consumed = true;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            u += marker_move;
            consumed = true;
            break;
    }

    if (consumed) {
        double h, s, l;
        Hsluv::luv_to_hsluv(_values[2], u, v, &h, &s, &l);

        setHue(h);
        setSaturation(s);

        _adjusting = true;
        _signal_color_changed.emit();
        queue_draw();
    }

    return consumed;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* ColorPoint */
ColorPoint::ColorPoint()
    : x(0), y(0), r(0), g(0), b(0)
{}

ColorPoint::ColorPoint(double x, double y, double r, double g, double b)
    : x(x), y(y), r(r), g(g), b(b)
{}

ColorPoint::ColorPoint(double x, double y, guint color)
    : x(x)
    , y(y)
    , r(((color & 0xff0000) >> 16) / 255.0)
    , g(((color & 0x00ff00) >>  8) / 255.0)
    , b(((color & 0x0000ff)      ) / 255.0)
{}

guint32 ColorPoint::get_color()
{
    return (static_cast<int>(r * 255) << 16 |
            static_cast<int>(g * 255) <<  8 |
            static_cast<int>(b * 255)
    );
};

void ColorPoint::set_color(double red, double green, double blue)
{
    r = red;
    g = green;
    b = blue;
};

/* Point */
Point::Point() : x(0), y(0)
{}

Point::Point(double x, double y) : x(x), y(y)
{}

/* Intersection */
Intersection::Intersection() : line1(0), line2(0)
{}

Intersection::Intersection(int line1, int line2, Point intersectionPoint,
        double intersectionPointAngle, double relativeAngle)
    : line1(line1)
    , line2(line2)
    , intersectionPoint(intersectionPoint)
    , intersectionPointAngle(intersectionPointAngle)
    , relativeAngle(relativeAngle)
{}

/* Utility functions */
static Point intersect_line_line(Line a, Line b)
{
    double x = (a.intercept - b.intercept) / (b.slope - a.slope);
    double y = a.slope * x + a.intercept;
    return {x, y};
}

static double distance_from_origin(Point point)
{
    return std::sqrt(std::pow(point.x, 2) + std::pow(point.y, 2));
}

static double distance_line_from_origin(Line line)
{
    // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
    return std::abs(line.intercept) / std::sqrt(std::pow(line.slope, 2) + 1);
}

static double angle_from_origin(Point point)
{
    return std::atan2(point.y, point.x);
}

static double normalize_angle(double angle)
{
    double m = 2 * M_PI;
    return std::fmod(std::fmod(angle, m) + m, m);
}

static double lerp(double v0, double v1, double t0, double t1, double t)
{
    double s = 0;

    if (t0 != t1) {
        s = (t - t0) / (t1 - t0);
    }

    return (1.0 - s) * v0 + s * v1;
}

static ColorPoint lerp(ColorPoint const &v0, ColorPoint const &v1, double t0, double t1,
        double t)
{
    double x = lerp(v0.x, v1.x, t0, t1, t);
    double y = lerp(v0.y, v1.y, t0, t1, t);
    double r = lerp(v0.r, v1.r, t0, t1, t);
    double g = lerp(v0.g, v1.g, t0, t1, t);
    double b = lerp(v0.b, v1.b, t0, t1, t);

    return ColorPoint(x, y, r, g, b);
}

/**
 * @param h Hue. Between 0 and 1.
 * @param s Saturation. Between 0 and 1.
 * @param v Value. Between 0 and 1.
 */
static guint32 hsv_to_rgb(double h, double s, double v)
{
    h = std::clamp(h, 0.0, 1.0);
    s = std::clamp(s, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);

    double r = v;
    double g = v;
    double b = v;

    if (s != 0.0) {
        if (h == 1.0) h = 0.0;
        h *= 6.0;

        double f = h - (int)h;
        double p = v * (1.0 - s);
        double q = v * (1.0 - s * f);
        double t = v * (1.0 - s * (1.0 - f));

        switch (static_cast<int>(h)) {
            case 0:     r = v;  g = t;  b = p;  break;
            case 1:     r = q;  g = v;  b = p;  break;
            case 2:     r = p;  g = v;  b = t;  break;
            case 3:     r = p;  g = q;  b = v;  break;
            case 4:     r = t;  g = p;  b = v;  break;
            case 5:     r = v;  g = p;  b = q;  break;
            default:    g_assert_not_reached();
        }
    }

    guint32 rgb = (static_cast<int>(floor(r * 255 + 0.5)) << 16) |
                  (static_cast<int>(floor(g * 255 + 0.5)) <<  8) |
                  (static_cast<int>(floor(b * 255 + 0.5))      );
    return rgb;
}

double luminance(guint32 color)
{
    double r = ((color & 0xff0000) >> 16) / 255.0;
    double g = ((color &   0xff00) >>  8) / 255.0;
    double b = ((color &     0xff)      ) / 255.0;
    return (r * 0.2125 + g * 0.7154 + b * 0.0721);
}

/**
 * Convert the vertice of the in gamut color polygon (Luv) to pixel coordinates.
 *
 * @param point The point in Luv coordinates.
 * @param scale Zoom amount to fit polygon to outer circle.
 * @param resize Zoom amount to fit wheel in widget.
 */
static Point to_pixel_coordinate(Point const &point, double scale, double resize)
{
    return Point(
        point.x * scale * resize + (SIZE * resize / 2.0),
        (SIZE * resize / 2.0) - point.y * scale * resize
    );
}

/**
 * Convert a point in pixels on the widget to Luv coordinates.
 *
 * @param point The point in pixel coordinates.
 * @param scale Zoom amount to fit polygon to outer circle.
 * @param resize Zoom amount to fit wheel in widget.
 */
static Point from_pixel_coordinate(Point const &point, double scale, double resize)
{
    return Point(
        (point.x - (SIZE * resize / 2.0)) / (scale * resize),
        ((SIZE * resize / 2.0) - point.y) / (scale * resize)
    );
}

/**
 * @overload
 * @param point A vector of points in Luv coordinates.
 * @param scale Zoom amount to fit polygon to outer circle.
 * @param resize Zoom amount to fit wheel in widget.
 */
static std::vector<Point> to_pixel_coordinate(std::vector<Point> const &points,
        double scale, double resize)
{
    std::vector<Point> result;

    for (Point const &p : points) {
        result.emplace_back(to_pixel_coordinate(p, scale, resize));
    }

    return result;
}

/**
  * Paints padding for an edge of the triangle,
  * using the (vertically) closest point.
  *
  * @param p0 A corner of the triangle. Not the same corner as p1
  * @param p1 A corner of the triangle. Not the same corner as p0
  * @param padding The height of the padding
  * @param pad_upwards True if padding is above the line
  * @param buffer Array that the triangle is painted to
  * @param height Height of buffer
  * @param stride Stride of buffer
*/
void draw_vertical_padding(ColorPoint p0, ColorPoint p1, int padding, bool pad_upwards,
        guint32 *buffer, int height, int stride)
{
    // skip if horizontal padding is more accurate, e.g. if the edge is vertical
    double gradient = (p1.y - p0.y) / (p1.x - p0.x);
    if (std::abs(gradient) > 1.0) {
        return;
    }

    double min_y = std::min(p0.y, p1.y);
    double max_y = std::max(p0.y, p1.y);

    double min_x = std::min(p0.x, p1.x);
    double max_x = std::max(p0.x, p1.x);

    // go through every point on the line
    for (int y = min_y; y <= max_y; ++y) {
        double start_x = lerp(p0, p1, p0.y, p1.y, std::clamp(static_cast<double>(y), min_y,
                    max_y)).x;
        double end_x = lerp(p0, p1, p0.y, p1.y, std::clamp(static_cast<double>(y) + 1, min_y,
                    max_y)).x;
        if (start_x > end_x) {
            std::swap(start_x, end_x);
        }

        guint32 *p = buffer + y * stride;
        p += static_cast<int>(start_x);
        for (int x = start_x; x <= end_x; ++x) {
            // get the color at this point on the line
            ColorPoint point = lerp(p0, p1, p0.x, p1.x, std::clamp(static_cast<double>(x),
                        min_x, max_x));
            // paint the padding vertically above or below this point
            for (int offset = 0; offset <= padding; ++offset) {
                if (pad_upwards && (point.y - offset) >= 0) {
                    *(p - (offset * stride)) = point.get_color();
                } else if (!pad_upwards && (point.y + offset) < height) {
                    *(p + (offset * stride)) = point.get_color();
                }
            }
            ++p;
        }
    }
}

static void Inkscape::UI::Widget::get_picker_geometry(PickerGeometry *pickerGeometry,
        double lightness)
{
    // Add a lambda to avoid overlapping intersections
    lightness = std::clamp(lightness + 0.01, 0.1, 99.9);

    // Array of lines
    std::array<Line, 6> const lines = Hsluv::getBounds(lightness);
    int numLines = lines.size();
    double outerCircleRadius = 0.0;

    // Find the line closest to origin
    int closestIndex2 = -1;
    double closestLineDistance = -1;

    for (int i = 0; i < numLines; i++) {
        double d = distance_line_from_origin(lines[i]);
        if (closestLineDistance < 0 || d < closestLineDistance) {
            closestLineDistance = d;
            closestIndex2 = i;
        }
    }

    Line closestLine = lines[closestIndex2];
    Line perpendicularLine (0 - (1 / closestLine.slope), 0.0);

    Point intersectionPoint = intersect_line_line(closestLine,
            perpendicularLine);
    double startingAngle = angle_from_origin(intersectionPoint);

    std::vector<Intersection> intersections;
    double intersectionPointAngle;
    double relativeAngle;

    for (int i = 0; i < numLines - 1; i++) {
        for (int j = i + 1; j < numLines; j++) {
            intersectionPoint = intersect_line_line(lines[i], lines[j]);
            intersectionPointAngle = angle_from_origin(intersectionPoint);
            relativeAngle = normalize_angle(
                    intersectionPointAngle - startingAngle);
            intersections.emplace_back(i, j, intersectionPoint,
                    intersectionPointAngle, relativeAngle);
        }
    }

    std::sort(intersections.begin(), intersections.end(),
            [] (Intersection const &lhs, Intersection const &rhs)
    {
        return lhs.relativeAngle >= rhs.relativeAngle;
    });

    std::vector<Line> orderedLines;
    std::vector<Point> orderedVertices;
    std::vector<double> orderedAngles;

    int nextIndex;
    double intersectionPointDistance;
    int currentIndex = closestIndex2;

    for (Intersection intersection : intersections) {
        nextIndex = -1;

        if (intersection.line1 == currentIndex) {
            nextIndex = intersection.line2;
        }
        else if (intersection.line2 == currentIndex) {
            nextIndex = intersection.line1;
        }

        if (nextIndex > -1) {
            currentIndex = nextIndex;

            orderedLines.emplace_back(lines[nextIndex]);
            orderedVertices.emplace_back(intersection.intersectionPoint);
            orderedAngles.emplace_back(intersection.intersectionPointAngle);

            intersectionPointDistance = distance_from_origin(intersection.intersectionPoint);
            if (intersectionPointDistance > outerCircleRadius) {
                outerCircleRadius = intersectionPointDistance;
            }
        }
    }

    pickerGeometry->lines = orderedLines;
    pickerGeometry->vertices = orderedVertices;
    pickerGeometry->angles = orderedAngles;
    pickerGeometry->outerCircleRadius = outerCircleRadius;
    pickerGeometry->innerCircleRadius = closestLineDistance;
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace .0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8: textwidth=99:
