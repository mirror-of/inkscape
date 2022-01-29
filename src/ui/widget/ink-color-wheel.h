// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * @file
 * HSLuv color wheel widget, based on the web implementation at
 * https://www.hsluv.org
 *
 * Authors:
 *   Tavmjong Bah
 *   Massinissa Derriche <massinissa.derriche@gmail.com>
 *
 * Copyright (C) 2018, 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INK_COLORWHEEL_H
#define INK_COLORWHEEL_H

#include <gtkmm.h>

namespace Inkscape {
namespace UI {
namespace Widget {
  
class PickerGeometry;

/**
 * @class ColorWheel
 */
class ColorWheel : public Gtk::DrawingArea
{
public:
    ColorWheel();

    virtual void setRgb(double r, double g, double b, bool overrideHue = true);
    virtual void getRgb(double *r, double *g, double *b) const;
    virtual void getRgbV(double *rgb) const;
    virtual guint32 getRgb() const;

    void setHue(double h);
    void setSaturation(double s);
    virtual void setLightness(double l);
    void getValues(double *a, double *b, double *c) const;

    bool isAdjusting() const { return _adjusting; }

protected:
    virtual void _set_from_xy(double const x, double const y);

    double _values[3];
    bool _adjusting;

private:
    // Callbacks
    bool on_key_release_event(GdkEventKey* key_event) override;

    // Signals
public:
    sigc::signal<void> signal_color_changed();

protected:
    sigc::signal<void> _signal_color_changed;
};

/**
 * @class ColorWheelHSL
 */
class ColorWheelHSL : public ColorWheel
{
public:
    void setRgb(double r, double g, double b, bool overrideHue = true) override;
    void getRgb(double *r, double *g, double *b) const override;
    void getRgbV(double *rgb) const override;
    guint32 getRgb() const override;

    void getHsl(double *h, double *s, double *l) const;

protected:
    bool on_draw(const::Cairo::RefPtr<::Cairo::Context>& cr) override;
    bool on_focus(Gtk::DirectionType direction) override;

private:
    void _set_from_xy(double const x, double const y) override;
    bool _is_in_ring(double x, double y);
    bool _is_in_triangle(double x, double y);
    void _update_triangle_color(double x, double y);
    void _update_ring_color(double x, double y);
    void _triangle_corners(double& x0, double& y0, double& x1, double& y1, double& x2,
            double& y2);

    enum class DragMode {
        NONE,
        HUE,
        SATURATION_VALUE
    };

    double _ring_width = 0.2;
    DragMode _mode = DragMode::NONE;
    bool _focus_on_ring = true;

    // Callbacks
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_key_press_event(GdkEventKey* key_event) override;
};

/**
 * @class ColorWheelHSLuv
 */
class ColorWheelHSLuv : public ColorWheel
{
public:
    ColorWheelHSLuv();
    ~ColorWheelHSLuv() override;

    void setRgb(double r, double g, double b, bool overrideHue = true) override;
    void getRgb(double *r, double *g, double *b) const override;
    void getRgbV(double *rgb) const override;
    guint32 getRgb() const override;

    void setHsluv(double h, double s, double l);
    void setLightness(double l) override;

    void getHsluv(double *h, double *s, double *l) const;

protected:
    bool on_draw(const::Cairo::RefPtr<::Cairo::Context>& cr) override;

private:
    void _set_from_xy(double const x, double const y) override;
    void _update_polygon();

    // Callbacks
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    bool on_key_press_event(GdkEventKey* key_event) override;

    double _scale;
    PickerGeometry *_picker_geometry;
    std::vector<guint32> _buffer_polygon;
    Cairo::RefPtr<::Cairo::ImageSurface> _surface_polygon;
    int _cache_width, _cache_height;
    int _square_size;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INK_COLORWHEEL_HSLUV_H
