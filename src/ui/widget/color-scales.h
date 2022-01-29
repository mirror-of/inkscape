// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Color selector using sliders for each components, for multiple color modes
 *//*
 * Authors:
 * see git history
 *
 * Copyright (C) 2018, 2021 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_SP_COLOR_SCALES_H
#define SEEN_SP_COLOR_SCALES_H

#include <gtkmm/box.h>
#include <array>

#include "ui/selected-color.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorSlider;
class ColorWheel;

enum class SPColorScalesMode {
    NONE,
    RGB,
    HSL,
    CMYK,
    HSV,
    HSLUV
};

template <SPColorScalesMode MODE = SPColorScalesMode::NONE>
class ColorScales
    : public Gtk::Box
{
public:
    static gchar const *SUBMODE_NAMES[];

    static gfloat getScaled(Glib::RefPtr<Gtk::Adjustment> const &a);
    static void setScaled(Glib::RefPtr<Gtk::Adjustment> &a, gfloat v, bool constrained = false);

    ColorScales(SelectedColor &color);
    ~ColorScales() override;

    void setupMode();
    SPColorScalesMode getMode() const;

    static guchar const *hsluvHueMap(gfloat s, gfloat l,
            std::array<guchar, 4 * 1024> *map);
    static guchar const *hsluvSaturationMap(gfloat h, gfloat l,
            std::array<guchar, 4 * 1024> *map);
    static guchar const *hsluvLightnessMap(gfloat h, gfloat s,
            std::array<guchar, 4 * 1024> *map);

protected:
    void _onColorChanged();
    void on_show() override;

    virtual void _initUI();

    void _sliderAnyGrabbed();
    void _sliderAnyReleased();
    void _sliderAnyChanged();
    void _adjustmentChanged(int channel);
    void _wheelChanged();

    void _getRgbaFloatv(gfloat *rgba);
    void _getCmykaFloatv(gfloat *cmyka);
    guint32 _getRgba32();
    void _updateSliders(guint channels);
    void _recalcColor();
    void _updateDisplay(bool update_wheel = true);

    void _setRangeLimit(gdouble upper);

    SelectedColor &_color;
    gdouble _range_limit;
    gboolean _updating : 1;
    gboolean _dragging : 1;
    std::vector<Glib::RefPtr<Gtk::Adjustment>> _a;        /* Channel adjustments */
    Inkscape::UI::Widget::ColorSlider *_s[5];             /* Channel sliders */
    Gtk::Widget *_b[5];                                   /* Spinbuttons */
    Gtk::Label *_l[5];                                    /* Labels */
    std::array<guchar, 4 * 1024> _sliders_maps[4];
    Inkscape::UI::Widget::ColorWheel *_wheel;

    const Glib::ustring _prefs = "/color_scales";
    static gchar const * const _pref_wheel_visibility;

    sigc::slot_iterator<sigc::slot<void ()>> _color_changed;
    sigc::slot_iterator<sigc::slot<void ()>> _color_dragged;

private:
    // By default, disallow copy constructor and assignment operator
    ColorScales(ColorScales const &obj) = delete;
    ColorScales &operator=(ColorScales const &obj) = delete;
};

template <SPColorScalesMode MODE>
class ColorScalesFactory : public Inkscape::UI::ColorSelectorFactory
{
public:
    ColorScalesFactory();

    Gtk::Widget *createWidget(Inkscape::UI::SelectedColor &color) const override;
    Glib::ustring modeName() const override;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_SP_COLOR_SCALES_H */
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
