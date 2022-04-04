// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Color selector using sliders for each components, for multiple color modes
 *//*
 * Authors:
 * see git history
 *   bulia byak <buliabyak@users.sf.net>
 *   Massinissa Derriche <massinissa.derriche@gmail.com>
 *
 * Copyright (C) 2018, 2021 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/grid.h>
#include <glibmm/i18n.h>
#include <functional>

#include "ui/dialog-events.h"
#include "ui/widget/color-scales.h"
#include "ui/widget/color-slider.h"
#include "ui/widget/scrollprotected.h"
#include "ui/icon-loader.h"
#include "preferences.h"

#include "ui/widget/ink-color-wheel.h"

static int const CSC_CHANNEL_R     = (1 << 0);
static int const CSC_CHANNEL_G     = (1 << 1);
static int const CSC_CHANNEL_B     = (1 << 2);
static int const CSC_CHANNEL_A     = (1 << 3);
static int const CSC_CHANNEL_H     = (1 << 0);
static int const CSC_CHANNEL_S     = (1 << 1);
static int const CSC_CHANNEL_V     = (1 << 2);
static int const CSC_CHANNEL_C     = (1 << 0);
static int const CSC_CHANNEL_M     = (1 << 1);
static int const CSC_CHANNEL_Y     = (1 << 2);
static int const CSC_CHANNEL_K     = (1 << 3);
static int const CSC_CHANNEL_CMYKA = (1 << 4);

static int const CSC_CHANNELS_ALL  = 0;

static int const XPAD = 2;
static int const YPAD = 2;

namespace Inkscape {
namespace UI {
namespace Widget {


static guchar const *sp_color_scales_hue_map();
static guchar const *sp_color_scales_hsluv_map(guchar *map,
        std::function<void(float*, float)> callback);


template <SPColorScalesMode MODE>
gchar const *ColorScales<MODE>::SUBMODE_NAMES[] = { N_("None"), N_("RGB"), N_("HSL"),
    N_("CMYK"), N_("HSV"), N_("HSLuv") };


// Preference name for the saved state of toggle-able color wheel
template <>
gchar const * const ColorScales<SPColorScalesMode::HSL>::_pref_wheel_visibility =
    "/wheel_vis_hsl";

template <>
gchar const * const ColorScales<SPColorScalesMode::HSV>::_pref_wheel_visibility =
    "/wheel_vis_hsv";

template <>
gchar const * const ColorScales<SPColorScalesMode::HSLUV>::_pref_wheel_visibility =
    "/wheel_vis_hsluv";


template <SPColorScalesMode MODE>
ColorScales<MODE>::ColorScales(SelectedColor &color)
    : Gtk::Box()
    , _color(color)
    , _range_limit(255.0)
    , _updating(false)
    , _dragging(false)
    , _wheel(nullptr)
{
    for (gint i = 0; i < 5; i++) {
        _l[i] = nullptr;
        _s[i] = nullptr;
        _b[i] = nullptr;
    }

    _initUI();

    _color_changed = _color.signal_changed.connect([this](){ _onColorChanged(); });
    _color_dragged = _color.signal_dragged.connect([this](){ _onColorChanged(); });
}

template <SPColorScalesMode MODE>
ColorScales<MODE>::~ColorScales()
{
    _color_changed.disconnect();
    _color_dragged.disconnect();

    for (gint i = 0; i < 5; i++) {
        _l[i] = nullptr;
        _s[i] = nullptr;
        _b[i] = nullptr;
    }
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_initUI()
{
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    Gtk::Expander *wheel_frame = nullptr;

    if constexpr (
            MODE == SPColorScalesMode::HSL ||
            MODE == SPColorScalesMode::HSV ||
            MODE == SPColorScalesMode::HSLUV)
    {
        /* Create wheel */
        if constexpr (MODE == SPColorScalesMode::HSLUV) {
            _wheel = Gtk::manage(new Inkscape::UI::Widget::ColorWheelHSLuv());
        } else {
            _wheel = Gtk::manage(new Inkscape::UI::Widget::ColorWheelHSL());
        }

        _wheel->show();
        _wheel->set_halign(Gtk::ALIGN_FILL);
        _wheel->set_valign(Gtk::ALIGN_FILL);
        _wheel->set_hexpand(true);
        _wheel->set_vexpand(true);
        _wheel->set_name("ColorWheel");
        _wheel->set_size_request(-1, 130); // minimal size

        /* Signal */
        _wheel->signal_color_changed().connect([this](){ _wheelChanged(); });

        /* Expander */
        // Label icon
        Gtk::Image *expander_icon = Gtk::manage(
                sp_get_icon_image("color-wheel", Gtk::ICON_SIZE_BUTTON)
        );
        expander_icon->show();
        expander_icon->set_margin_start(2 * XPAD);
        expander_icon->set_margin_end(3 * XPAD);
        // Label
        Gtk::Label *expander_label = Gtk::manage(new Gtk::Label(_("Color Wheel")));
        expander_label->show();
        // Content
        Gtk::Box *expander_box = Gtk::manage(new Gtk::Box());
        expander_box->show();
        expander_box->pack_start(*expander_icon);
        expander_box->pack_start(*expander_label);
        expander_box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        // Expander
        wheel_frame = Gtk::manage(new Gtk::Expander());
        wheel_frame->show();
        wheel_frame->set_margin_start(2 * XPAD);
        wheel_frame->set_margin_end(XPAD);
        wheel_frame->set_margin_top(2 * YPAD);
        wheel_frame->set_margin_bottom(2 * YPAD);
        wheel_frame->set_halign(Gtk::ALIGN_FILL);
        wheel_frame->set_valign(Gtk::ALIGN_FILL);
        wheel_frame->set_hexpand(true);
        wheel_frame->set_vexpand(false);
        wheel_frame->set_label_widget(*expander_box);

        // Signal
        wheel_frame->property_expanded().signal_changed().connect([=](){
            bool visible = wheel_frame->get_expanded();
            wheel_frame->set_vexpand(visible);

            // Save wheel visibility
            Inkscape::Preferences::get()->setBool(_prefs + _pref_wheel_visibility, visible);
        });

        wheel_frame->add(*_wheel);
        add(*wheel_frame);
    }

    /* Create sliders */
    Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());
    grid->show();
    add(*grid);

    for (gint i = 0; i < 5; i++) {
        /* Label */
        _l[i] = Gtk::manage(new Gtk::Label("", true));

        _l[i]->set_halign(Gtk::ALIGN_START);
        _l[i]->show();

        _l[i]->set_margin_start(2 * XPAD);
        _l[i]->set_margin_end(XPAD);
        _l[i]->set_margin_top(YPAD);
        _l[i]->set_margin_bottom(YPAD);
        grid->attach(*_l[i], 0, i, 1, 1);

        /* Adjustment */
        _a.push_back(Gtk::Adjustment::create(0.0, 0.0, _range_limit, 1.0, 10.0, 10.0));
        /* Slider */
        _s[i] = Gtk::manage(new Inkscape::UI::Widget::ColorSlider(_a[i]));
        _s[i]->show();

        _s[i]->set_margin_start(XPAD);
        _s[i]->set_margin_end(XPAD);
        _s[i]->set_margin_top(YPAD);
        _s[i]->set_margin_bottom(YPAD);
        _s[i]->set_hexpand(true);
        grid->attach(*_s[i], 1, i, 1, 1);

        /* Spinbutton */
        _b[i] = Gtk::manage(new ScrollProtected<Gtk::SpinButton>(_a[i], 1.0));
        sp_dialog_defocus_on_enter(_b[i]->gobj());
        _l[i]->set_mnemonic_widget(*_b[i]);
        _b[i]->show();

        _b[i]->set_margin_start(XPAD);
        _b[i]->set_margin_end(XPAD);
        _b[i]->set_margin_top(YPAD);
        _b[i]->set_margin_bottom(YPAD);
        _b[i]->set_halign(Gtk::ALIGN_END);
        _b[i]->set_valign(Gtk::ALIGN_CENTER);
        grid->attach(*_b[i], 2, i, 1, 1);

        /* Signals */
        _a[i]->signal_value_changed().connect([this, i](){ _adjustmentChanged(i); });
        _s[i]->signal_grabbed.connect([this](){ _sliderAnyGrabbed(); });
        _s[i]->signal_released.connect([this](){ _sliderAnyReleased(); });
        _s[i]->signal_value_changed.connect([this](){ _sliderAnyChanged(); });
    }

    // Prevent 5th bar from being shown by PanelDialog::show_all_children
    _l[4]->set_no_show_all(true);
    _s[4]->set_no_show_all(true);
    _b[4]->set_no_show_all(true);

    setupMode();

    if constexpr (
            MODE == SPColorScalesMode::HSL ||
            MODE == SPColorScalesMode::HSV ||
            MODE == SPColorScalesMode::HSLUV)
    {
        // Restore the visibility of the wheel
        bool visible = Inkscape::Preferences::get()->getBool(_prefs + _pref_wheel_visibility,
            false);
        wheel_frame->set_expanded(visible);
        wheel_frame->set_vexpand(visible);
    }
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_recalcColor()
{
    SPColor color;
    gfloat alpha = 1.0;
    gfloat c[5];

    if constexpr (
            MODE == SPColorScalesMode::RGB ||
            MODE == SPColorScalesMode::HSL ||
            MODE == SPColorScalesMode::HSV ||
            MODE == SPColorScalesMode::HSLUV)
    {
        _getRgbaFloatv(c);
        color.set(c[0], c[1], c[2]);
        alpha = c[3];
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        _getCmykaFloatv(c);

        float rgb[3];
        SPColor::cmyk_to_rgb_floatv(rgb, c[0], c[1], c[2], c[3]);
        color.set(rgb[0], rgb[1], rgb[2]);
        alpha = c[4];
    } else {
        g_warning("file %s: line %d: Illegal color selector mode NONE", __FILE__, __LINE__);
    }

    _color.preserveICC();
    _color.setColorAlpha(color, alpha);
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_updateDisplay(bool update_wheel)
{
#ifdef DUMP_CHANGE_INFO
    g_message("ColorScales::_onColorChanged( this=%p, %f, %f, %f, %f) %d", this,
            _color.color().v.c[0],
            _color.color().v.c[1], _color.color().v.c[2], _color.alpha(), int(update_wheel);
#endif

    gfloat tmp[3];
    gfloat c[5] = { 0.0, 0.0, 0.0, 0.0 };

    SPColor color = _color.color();

    if constexpr (MODE == SPColorScalesMode::RGB) {
        color.get_rgb_floatv(c);
        c[3] = _color.alpha();
        c[4] = 0.0;
    } else if constexpr (MODE == SPColorScalesMode::HSL) {
        color.get_rgb_floatv(tmp);
        SPColor::rgb_to_hsl_floatv(c, tmp[0], tmp[1], tmp[2]);
        c[3] = _color.alpha();
        c[4] = 0.0;
        if (update_wheel) { _wheel->setRgb(tmp[0], tmp[1], tmp[2]); }
    } else if constexpr (MODE == SPColorScalesMode::HSV) {
        color.get_rgb_floatv(tmp);
        SPColor::rgb_to_hsv_floatv(c, tmp[0], tmp[1], tmp[2]);
        c[3] = _color.alpha();
        c[4] = 0.0;
        if (update_wheel) { _wheel->setRgb(tmp[0], tmp[1], tmp[2]); }
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        color.get_cmyk_floatv(c);
        c[4] = _color.alpha();
    } else if constexpr (MODE == SPColorScalesMode::HSLUV) {
        color.get_rgb_floatv(tmp);
        SPColor::rgb_to_hsluv_floatv(c, tmp[0], tmp[1], tmp[2]);
        c[3] = _color.alpha();
        c[4] = 0.0;
        if (update_wheel) { _wheel->setRgb(tmp[0], tmp[1], tmp[2]); }
    } else {
        g_warning("file %s: line %d: Illegal color selector mode NONE", __FILE__, __LINE__);
    }

    _updating = true;
    setScaled(_a[0], c[0]);
    setScaled(_a[1], c[1]);
    setScaled(_a[2], c[2]);
    setScaled(_a[3], c[3]);
    setScaled(_a[4], c[4]);
    _updateSliders(CSC_CHANNELS_ALL);
    _updating = false;
}

/* Helpers for setting color value */
template <SPColorScalesMode MODE>
gfloat ColorScales<MODE>::getScaled(Glib::RefPtr<Gtk::Adjustment> const &a)
{
    gfloat val = a->get_value() / a->get_upper();
    return val;
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::setScaled(Glib::RefPtr<Gtk::Adjustment> &a, gfloat v, bool constrained)
{
    auto upper = a->get_upper();
    gfloat val = v * upper;
    if (constrained) {
        // TODO: do we want preferences for these?
        if (upper == 255) {
            val = round(val/16) * 16;
        } else {
            val = round(val/10) * 10;
        }
    }
    a->set_value(val);
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_setRangeLimit(gdouble upper)
{
    _range_limit = upper;
    for (auto & i : _a) {
        i->set_upper(upper);
    }
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_onColorChanged()
{
    if (!get_visible()) { return; }

    _updateDisplay();
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::on_show()
{
    Gtk::Box::on_show();

    _updateDisplay();
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_getRgbaFloatv(gfloat *rgba)
{
    g_return_if_fail(rgba != nullptr);

    if constexpr (MODE == SPColorScalesMode::RGB) {
        rgba[0] = getScaled(_a[0]);
        rgba[1] = getScaled(_a[1]);
        rgba[2] = getScaled(_a[2]);
        rgba[3] = getScaled(_a[3]);
    } else if constexpr (MODE == SPColorScalesMode::HSL) {
        SPColor::hsl_to_rgb_floatv(rgba, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]));
        rgba[3] = getScaled(_a[3]);
    } else if constexpr (MODE == SPColorScalesMode::HSV) {
        SPColor::hsv_to_rgb_floatv(rgba, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]));
        rgba[3] = getScaled(_a[3]);
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        SPColor::cmyk_to_rgb_floatv(rgba, getScaled(_a[0]), getScaled(_a[1]),
                getScaled(_a[2]), getScaled(_a[3]));
        rgba[3] = getScaled(_a[4]);
    } else if constexpr (MODE == SPColorScalesMode::HSLUV) {
        SPColor::hsluv_to_rgb_floatv(rgba, getScaled(_a[0]), getScaled(_a[1]),
                getScaled(_a[2]));
        rgba[3] = getScaled(_a[3]);
    } else {
        g_warning("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
    }
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_getCmykaFloatv(gfloat *cmyka)
{
    gfloat rgb[3];

    g_return_if_fail(cmyka != nullptr);

    if constexpr (MODE == SPColorScalesMode::RGB) {
        SPColor::rgb_to_cmyk_floatv(cmyka, getScaled(_a[0]), getScaled(_a[1]),
                getScaled(_a[2]));
        cmyka[4] = getScaled(_a[3]);
    } else if constexpr (MODE == SPColorScalesMode::HSL) {
        SPColor::hsl_to_rgb_floatv(rgb, getScaled(_a[0]), getScaled(_a[1]),
                getScaled(_a[2]));
        SPColor::rgb_to_cmyk_floatv(cmyka, rgb[0], rgb[1], rgb[2]);
        cmyka[4] = getScaled(_a[3]);
    } else if constexpr (MODE == SPColorScalesMode::HSLUV) {
        SPColor::hsluv_to_rgb_floatv(rgb, getScaled(_a[0]), getScaled(_a[1]),
                getScaled(_a[2]));
        SPColor::rgb_to_cmyk_floatv(cmyka, rgb[0], rgb[1], rgb[2]);
        cmyka[4] = getScaled(_a[3]);
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        cmyka[0] = getScaled(_a[0]);
        cmyka[1] = getScaled(_a[1]);
        cmyka[2] = getScaled(_a[2]);
        cmyka[3] = getScaled(_a[3]);
        cmyka[4] = getScaled(_a[4]);
    } else {
        g_warning("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
    }
}

template <SPColorScalesMode MODE>
guint32 ColorScales<MODE>::_getRgba32()
{
    gfloat c[4];
    guint32 rgba;

    _getRgbaFloatv(c);

    rgba = SP_RGBA32_F_COMPOSE(c[0], c[1], c[2], c[3]);

    return rgba;
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::setupMode()
{
    gfloat rgba[4];
    gfloat c[4];

    if constexpr (MODE == SPColorScalesMode::NONE) {
        rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1.0;
    } else {
        _getRgbaFloatv(rgba);
    }

    if constexpr (MODE == SPColorScalesMode::RGB) {
        _setRangeLimit(255.0);
        _a[3]->set_upper(100.0);
        _l[0]->set_markup_with_mnemonic(_("_R:"));
        _s[0]->set_tooltip_text(_("Red"));
        _b[0]->set_tooltip_text(_("Red"));
        _l[1]->set_markup_with_mnemonic(_("_G:"));
        _s[1]->set_tooltip_text(_("Green"));
        _b[1]->set_tooltip_text(_("Green"));
        _l[2]->set_markup_with_mnemonic(_("_B:"));
        _s[2]->set_tooltip_text(_("Blue"));
        _b[2]->set_tooltip_text(_("Blue"));
        _l[3]->set_markup_with_mnemonic(_("_A:"));
        _s[3]->set_tooltip_text(_("Alpha (opacity)"));
        _b[3]->set_tooltip_text(_("Alpha (opacity)"));
        _s[0]->setMap(nullptr);
        _l[4]->hide();
        _s[4]->hide();
        _b[4]->hide();
        _updating = true;
        setScaled(_a[0], rgba[0]);
        setScaled(_a[1], rgba[1]);
        setScaled(_a[2], rgba[2]);
        setScaled(_a[3], rgba[3]);
        _updateSliders(CSC_CHANNELS_ALL);
        _updating = false;
    } else if constexpr (MODE == SPColorScalesMode::HSL) {
        _setRangeLimit(100.0);

        _l[0]->set_markup_with_mnemonic(_("_H:"));
        _s[0]->set_tooltip_text(_("Hue"));
        _b[0]->set_tooltip_text(_("Hue"));
        _a[0]->set_upper(360.0);

        _l[1]->set_markup_with_mnemonic(_("_S:"));
        _s[1]->set_tooltip_text(_("Saturation"));
        _b[1]->set_tooltip_text(_("Saturation"));

        _l[2]->set_markup_with_mnemonic(_("_L:"));
        _s[2]->set_tooltip_text(_("Lightness"));
        _b[2]->set_tooltip_text(_("Lightness"));

        _l[3]->set_markup_with_mnemonic(_("_A:"));
        _s[3]->set_tooltip_text(_("Alpha (opacity)"));
        _b[3]->set_tooltip_text(_("Alpha (opacity)"));
        _s[0]->setMap(sp_color_scales_hue_map());
        _l[4]->hide();
        _s[4]->hide();
        _b[4]->hide();
        _updating = true;
        c[0] = 0.0;

        SPColor::rgb_to_hsl_floatv(c, rgba[0], rgba[1], rgba[2]);

        setScaled(_a[0], c[0]);
        setScaled(_a[1], c[1]);
        setScaled(_a[2], c[2]);
        setScaled(_a[3], rgba[3]);

        _updateSliders(CSC_CHANNELS_ALL);
        _updating = false;
    } else if constexpr (MODE == SPColorScalesMode::HSV) {
        _setRangeLimit(100.0);

        _l[0]->set_markup_with_mnemonic(_("_H:"));
        _s[0]->set_tooltip_text(_("Hue"));
        _b[0]->set_tooltip_text(_("Hue"));
        _a[0]->set_upper(360.0);

        _l[1]->set_markup_with_mnemonic(_("_S:"));
        _s[1]->set_tooltip_text(_("Saturation"));
        _b[1]->set_tooltip_text(_("Saturation"));

        _l[2]->set_markup_with_mnemonic(_("_V:"));
        _s[2]->set_tooltip_text(_("Value"));
        _b[2]->set_tooltip_text(_("Value"));

        _l[3]->set_markup_with_mnemonic(_("_A:"));
        _s[3]->set_tooltip_text(_("Alpha (opacity)"));
        _b[3]->set_tooltip_text(_("Alpha (opacity)"));
        _s[0]->setMap(sp_color_scales_hue_map());
        _l[4]->hide();
        _s[4]->hide();
        _b[4]->hide();
        _updating = true;
        c[0] = 0.0;

        SPColor::rgb_to_hsv_floatv(c, rgba[0], rgba[1], rgba[2]);

        setScaled(_a[0], c[0]);
        setScaled(_a[1], c[1]);
        setScaled(_a[2], c[2]);
        setScaled(_a[3], rgba[3]);

        _updateSliders(CSC_CHANNELS_ALL);
        _updating = false;
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        _setRangeLimit(100.0);
        _l[0]->set_markup_with_mnemonic(_("_C:"));
        _s[0]->set_tooltip_text(_("Cyan"));
        _b[0]->set_tooltip_text(_("Cyan"));

        _l[1]->set_markup_with_mnemonic(_("_M:"));
        _s[1]->set_tooltip_text(_("Magenta"));
        _b[1]->set_tooltip_text(_("Magenta"));

        _l[2]->set_markup_with_mnemonic(_("_Y:"));
        _s[2]->set_tooltip_text(_("Yellow"));
        _b[2]->set_tooltip_text(_("Yellow"));

        _l[3]->set_markup_with_mnemonic(_("_K:"));
        _s[3]->set_tooltip_text(_("Black"));
        _b[3]->set_tooltip_text(_("Black"));

        _l[4]->set_markup_with_mnemonic(_("_A:"));
        _s[4]->set_tooltip_text(_("Alpha (opacity)"));
        _b[4]->set_tooltip_text(_("Alpha (opacity)"));

        _s[0]->setMap(nullptr);
        _l[4]->show();
        _s[4]->show();
        _b[4]->show();
        _updating = true;

        SPColor::rgb_to_cmyk_floatv(c, rgba[0], rgba[1], rgba[2]);
        setScaled(_a[0], c[0]);
        setScaled(_a[1], c[1]);
        setScaled(_a[2], c[2]);
        setScaled(_a[3], c[3]);

        setScaled(_a[4], rgba[3]);
        _updateSliders(CSC_CHANNELS_ALL);
        _updating = false;
    } else if constexpr (MODE == SPColorScalesMode::HSLUV) {
        _setRangeLimit(100.0);

        _l[0]->set_markup_with_mnemonic(_("_H*:"));
        _s[0]->set_tooltip_text(_("Hue"));
        _b[0]->set_tooltip_text(_("Hue"));
        _a[0]->set_upper(360.0);

        _l[1]->set_markup_with_mnemonic(_("_S*:"));
        _s[1]->set_tooltip_text(_("Saturation"));
        _b[1]->set_tooltip_text(_("Saturation"));

        _l[2]->set_markup_with_mnemonic(_("_L*:"));
        _s[2]->set_tooltip_text(_("Lightness"));
        _b[2]->set_tooltip_text(_("Lightness"));

        _l[3]->set_markup_with_mnemonic(_("_A:"));
        _s[3]->set_tooltip_text(_("Alpha (opacity)"));
        _b[3]->set_tooltip_text(_("Alpha (opacity)"));

        _s[0]->setMap(hsluvHueMap(0.0f, 0.0f, &_sliders_maps[0]));
        _s[1]->setMap(hsluvSaturationMap(0.0f, 0.0f, &_sliders_maps[1]));
        _s[2]->setMap(hsluvLightnessMap(0.0f, 0.0f, &_sliders_maps[2]));

        _l[4]->hide();
        _s[4]->hide();
        _b[4]->hide();
        _updating = true;
        c[0] = 0.0;

        SPColor::rgb_to_hsluv_floatv(c, rgba[0], rgba[1], rgba[2]);

        setScaled(_a[0], c[0]);
        setScaled(_a[1], c[1]);
        setScaled(_a[2], c[2]);
        setScaled(_a[3], rgba[3]);

        _updateSliders(CSC_CHANNELS_ALL);
        _updating = false;
    } else {
        g_warning("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
    }
}

template <SPColorScalesMode MODE>
SPColorScalesMode ColorScales<MODE>::getMode() const { return MODE; }

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_sliderAnyGrabbed()
{
    if (_updating) { return; }

    if (!_dragging) {
        _dragging = true;
        _color.setHeld(true);
    }
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_sliderAnyReleased()
{
    if (_updating) { return; }

    if (_dragging) {
        _dragging = false;
        _color.setHeld(false);
    }
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_sliderAnyChanged()
{
    if (_updating) { return; }

    _recalcColor();
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_adjustmentChanged(int channel)
{
    if (_updating) { return; }

    _updateSliders((1 << channel));
    _recalcColor();
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_wheelChanged()
{
    if constexpr (
            MODE == SPColorScalesMode::NONE ||
            MODE == SPColorScalesMode::RGB ||
            MODE == SPColorScalesMode::CMYK)
    {
        return;
    }

    if (_updating) { return; }

    _updating = true;

    double rgb[3];
    _wheel->getRgbV(rgb);
    SPColor color(rgb[0], rgb[1], rgb[2]);

    _color_changed.block();
    _color_dragged.block();

    // Color
    _color.preserveICC();
    _color.setHeld(_wheel->isAdjusting());
    _color.setColor(color);

    // Sliders
    _updateDisplay(false);

    _color_changed.unblock();
    _color_dragged.unblock();

    _updating = false;
}

template <SPColorScalesMode MODE>
void ColorScales<MODE>::_updateSliders(guint channels)
{
    gfloat rgb0[3], rgbm[3], rgb1[3];

#ifdef SPCS_PREVIEW
    guint32 rgba;
#endif

    std::array<gfloat, 4> const adj = [this]() -> std::array<gfloat, 4> {
        if constexpr (MODE == SPColorScalesMode::CMYK) {
            return { getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), getScaled(_a[3]) };
        } else {
            return { getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 0.0 };
        }
    }();

    if constexpr (MODE == SPColorScalesMode::RGB) {
        if ((channels != CSC_CHANNEL_R) && (channels != CSC_CHANNEL_A)) {
            /* Update red */
            _s[0]->setColors(SP_RGBA32_F_COMPOSE(0.0, adj[1], adj[2], 1.0),
                             SP_RGBA32_F_COMPOSE(0.5, adj[1], adj[2], 1.0),
                             SP_RGBA32_F_COMPOSE(1.0, adj[1], adj[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_G) && (channels != CSC_CHANNEL_A)) {
            /* Update green */
            _s[1]->setColors(SP_RGBA32_F_COMPOSE(adj[0], 0.0, adj[2], 1.0),
                             SP_RGBA32_F_COMPOSE(adj[0], 0.5, adj[2], 1.0),
                             SP_RGBA32_F_COMPOSE(adj[0], 1.0, adj[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_B) && (channels != CSC_CHANNEL_A)) {
            /* Update blue */
            _s[2]->setColors(SP_RGBA32_F_COMPOSE(adj[0], adj[1], 0.0, 1.0),
                             SP_RGBA32_F_COMPOSE(adj[0], adj[1], 0.5, 1.0),
                             SP_RGBA32_F_COMPOSE(adj[0], adj[1], 1.0, 1.0));
        }
        if (channels != CSC_CHANNEL_A) {
            /* Update alpha */
            _s[3]->setColors(SP_RGBA32_F_COMPOSE(adj[0], adj[1], adj[2], 0.0),
                             SP_RGBA32_F_COMPOSE(adj[0], adj[1], adj[2], 0.5),
                             SP_RGBA32_F_COMPOSE(adj[0], adj[1], adj[2], 1.0));
        }
    } else if constexpr (MODE == SPColorScalesMode::HSL) {
        /* Hue is never updated */
        if ((channels != CSC_CHANNEL_S) && (channels != CSC_CHANNEL_A)) {
            /* Update saturation */
            SPColor::hsl_to_rgb_floatv(rgb0, adj[0], 0.0, adj[2]);
            SPColor::hsl_to_rgb_floatv(rgbm, adj[0], 0.5, adj[2]);
            SPColor::hsl_to_rgb_floatv(rgb1, adj[0], 1.0, adj[2]);
            _s[1]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_V) && (channels != CSC_CHANNEL_A)) {
            /* Update value */
            SPColor::hsl_to_rgb_floatv(rgb0, adj[0], adj[1], 0.0);
            SPColor::hsl_to_rgb_floatv(rgbm, adj[0], adj[1], 0.5);
            SPColor::hsl_to_rgb_floatv(rgb1, adj[0], adj[1], 1.0);
            _s[2]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if (channels != CSC_CHANNEL_A) {
            /* Update alpha */
            SPColor::hsl_to_rgb_floatv(rgb0, adj[0], adj[1], adj[2]);
            _s[3]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.0),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.5),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0));
        }
    } else if constexpr (MODE == SPColorScalesMode::HSV) {
        /* Hue is never updated */
        if ((channels != CSC_CHANNEL_S) && (channels != CSC_CHANNEL_A)) {
            /* Update saturation */
            SPColor::hsv_to_rgb_floatv(rgb0, adj[0], 0.0, adj[2]);
            SPColor::hsv_to_rgb_floatv(rgbm, adj[0], 0.5, adj[2]);
            SPColor::hsv_to_rgb_floatv(rgb1, adj[0], 1.0, adj[2]);
            _s[1]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_V) && (channels != CSC_CHANNEL_A)) {
            /* Update value */
            SPColor::hsv_to_rgb_floatv(rgb0, adj[0], adj[1], 0.0);
            SPColor::hsv_to_rgb_floatv(rgbm, adj[0], adj[1], 0.5);
            SPColor::hsv_to_rgb_floatv(rgb1, adj[0], adj[1], 1.0);
            _s[2]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if (channels != CSC_CHANNEL_A) {
            /* Update alpha */
            SPColor::hsv_to_rgb_floatv(rgb0, adj[0], adj[1], adj[2]);
            _s[3]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.0),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.5),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0));
        }
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        if ((channels != CSC_CHANNEL_C) && (channels != CSC_CHANNEL_CMYKA)) {
            /* Update C */
            SPColor::cmyk_to_rgb_floatv(rgb0, 0.0, adj[1], adj[2], adj[3]);
            SPColor::cmyk_to_rgb_floatv(rgbm, 0.5, adj[1], adj[2], adj[3]);
            SPColor::cmyk_to_rgb_floatv(rgb1, 1.0, adj[1], adj[2], adj[3]);
            _s[0]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_M) && (channels != CSC_CHANNEL_CMYKA)) {
            /* Update M */
            SPColor::cmyk_to_rgb_floatv(rgb0, adj[0], 0.0, adj[2], adj[3]);
            SPColor::cmyk_to_rgb_floatv(rgbm, adj[0], 0.5, adj[2], adj[3]);
            SPColor::cmyk_to_rgb_floatv(rgb1, adj[0], 1.0, adj[2], adj[3]);
            _s[1]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_Y) && (channels != CSC_CHANNEL_CMYKA)) {
            /* Update Y */
            SPColor::cmyk_to_rgb_floatv(rgb0, adj[0], adj[1], 0.0, adj[3]);
            SPColor::cmyk_to_rgb_floatv(rgbm, adj[0], adj[1], 0.5, adj[3]);
            SPColor::cmyk_to_rgb_floatv(rgb1, adj[0], adj[1], 1.0, adj[3]);
            _s[2]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if ((channels != CSC_CHANNEL_K) && (channels != CSC_CHANNEL_CMYKA)) {
            /* Update K */
            SPColor::cmyk_to_rgb_floatv(rgb0, adj[0], adj[1], adj[2], 0.0);
            SPColor::cmyk_to_rgb_floatv(rgbm, adj[0], adj[1], adj[2], 0.5);
            SPColor::cmyk_to_rgb_floatv(rgb1, adj[0], adj[1], adj[2], 1.0);
            _s[3]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgbm[0], rgbm[1], rgbm[2], 1.0),
                             SP_RGBA32_F_COMPOSE(rgb1[0], rgb1[1], rgb1[2], 1.0));
        }
        if (channels != CSC_CHANNEL_CMYKA) {
            /* Update alpha */
            SPColor::cmyk_to_rgb_floatv(rgb0, adj[0], adj[1], adj[2], adj[3]);
            _s[4]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.0),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.5),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0));
        }
    } else if constexpr (MODE == SPColorScalesMode::HSLUV) {
        if ((channels != CSC_CHANNEL_H) && (channels != CSC_CHANNEL_A)) {
            /* Update hue */
            _s[0]->setMap(hsluvHueMap(adj[1], adj[2], &_sliders_maps[0]));
        }
        if ((channels != CSC_CHANNEL_S) && (channels != CSC_CHANNEL_A)) {
            /* Update saturation (scaled chroma) */
            _s[1]->setMap(hsluvSaturationMap(adj[0], adj[2], &_sliders_maps[1]));
        }
        if ((channels != CSC_CHANNEL_V) && (channels != CSC_CHANNEL_A)) {
            /* Update lightness */
            _s[2]->setMap(hsluvLightnessMap(adj[0], adj[1], &_sliders_maps[2]));
        }
        if (channels != CSC_CHANNEL_A) {
            /* Update alpha */
            SPColor::hsluv_to_rgb_floatv(rgb0, adj[0], adj[1], adj[2]);
            _s[3]->setColors(SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.0),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 0.5),
                             SP_RGBA32_F_COMPOSE(rgb0[0], rgb0[1], rgb0[2], 1.0));
        }
    } else {
        g_warning("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
    }

#ifdef SPCS_PREVIEW
    rgba = sp_color_scales_get_rgba32(cs);
    sp_color_preview_set_rgba32(SP_COLOR_PREVIEW(_p), rgba);
#endif
}

static guchar const *sp_color_scales_hue_map()
{
    static std::array<guchar, 4 * 1024> const map = []() {
        std::array<guchar, 4 * 1024> m;

        guchar *p;
        p = m.data();
        for (gint h = 0; h < 1024; h++) {
            gfloat rgb[3];
            SPColor::hsl_to_rgb_floatv(rgb, h / 1024.0, 1.0, 0.5);
            *p++ = SP_COLOR_F_TO_U(rgb[0]);
            *p++ = SP_COLOR_F_TO_U(rgb[1]);
            *p++ = SP_COLOR_F_TO_U(rgb[2]);
            *p++ = 0xFF;
        }

        return m;
    }();

    return map.data();
}

static void sp_color_interp(guchar *out, gint steps, gfloat *start, gfloat *end)
{
    gfloat s[3] = {
        (end[0] - start[0]) / steps,
        (end[1] - start[1]) / steps,
        (end[2] - start[2]) / steps
    };

    guchar *p = out;
    for (int i = 0; i < steps; i++) {
        *p++ = SP_COLOR_F_TO_U(start[0] + s[0] * i);
        *p++ = SP_COLOR_F_TO_U(start[1] + s[1] * i);
        *p++ = SP_COLOR_F_TO_U(start[2] + s[2] * i);
        *p++ = 0xFF;
    }
}

template <typename T>
static std::vector<T> range (int const steps, T start, T end)
{
    T step = (end - start) / (steps - 1);

    std::vector<T> out;
    out.reserve(steps);

    for (int i = 0; i < steps-1; i++) {
        out.emplace_back(start + step * i);
    }
    out.emplace_back(end);

    return out;
}

static guchar const *sp_color_scales_hsluv_map(guchar *map,
        std::function<void(float*, float)> callback)
{
    // Only generate 21 colors and interpolate between them to get 1024
    static int const STEPS = 21;
    static int const COLORS = (STEPS+1) * 3;

    std::vector<float> steps = range<float>(STEPS+1, 0.f, 1.f);

    // Generate color steps
    gfloat colors[COLORS];
    for (int i = 0; i < STEPS+1; i++) {
        callback(colors+(i*3), steps[i]);
    }

    for (int i = 0; i < STEPS; i++) {
        int a = steps[i] * 1023,
            b = steps[i+1] * 1023;
        sp_color_interp(map+(a * 4), b-a, colors+(i*3), colors+((i+1)*3));
    }

    return map;
}

template <SPColorScalesMode MODE>
guchar const *ColorScales<MODE>::hsluvHueMap(gfloat s, gfloat l,
        std::array<guchar, 4 * 1024> *map)
{
    return sp_color_scales_hsluv_map(map->data(), [s, l] (float *colors, float h) {
        SPColor::hsluv_to_rgb_floatv(colors, h, s, l);
    });
}

template <SPColorScalesMode MODE>
guchar const *ColorScales<MODE>::hsluvSaturationMap(gfloat h, gfloat l,
        std::array<guchar, 4 * 1024> *map)
{
    return sp_color_scales_hsluv_map(map->data(), [h, l] (float *colors, float s) {
        SPColor::hsluv_to_rgb_floatv(colors, h, s, l);
    });
}

template <SPColorScalesMode MODE>
guchar const *ColorScales<MODE>::hsluvLightnessMap(gfloat h, gfloat s,
        std::array<guchar, 4 * 1024> *map)
{
    return sp_color_scales_hsluv_map(map->data(), [h, s] (float *colors, float l) {
        SPColor::hsluv_to_rgb_floatv(colors, h, s, l);
    });
}

template <SPColorScalesMode MODE>
ColorScalesFactory<MODE>::ColorScalesFactory()
{}

template <SPColorScalesMode MODE>
Gtk::Widget *ColorScalesFactory<MODE>::createWidget(Inkscape::UI::SelectedColor &color) const
{
    Gtk::Widget *w = Gtk::manage(new ColorScales<MODE>(color));
    return w;
}

template <SPColorScalesMode MODE>
Glib::ustring ColorScalesFactory<MODE>::modeName() const
{
    if constexpr (MODE == SPColorScalesMode::RGB) {
        return gettext(ColorScales<>::SUBMODE_NAMES[1]);
    } else if constexpr (MODE == SPColorScalesMode::HSL) {
        return gettext(ColorScales<>::SUBMODE_NAMES[2]);
    } else if constexpr (MODE == SPColorScalesMode::CMYK) {
        return gettext(ColorScales<>::SUBMODE_NAMES[3]);
    } else if constexpr (MODE == SPColorScalesMode::HSV) {
        return gettext(ColorScales<>::SUBMODE_NAMES[4]);
    } else if constexpr (MODE == SPColorScalesMode::HSLUV) {
        return gettext(ColorScales<>::SUBMODE_NAMES[5]);
    } else {
        return gettext(ColorScales<>::SUBMODE_NAMES[0]);
    }
}

// Explicit instantiations
template class ColorScales<SPColorScalesMode::NONE>;
template class ColorScales<SPColorScalesMode::RGB>;
template class ColorScales<SPColorScalesMode::HSL>;
template class ColorScales<SPColorScalesMode::CMYK>;
template class ColorScales<SPColorScalesMode::HSV>;
template class ColorScales<SPColorScalesMode::HSLUV>;

template class ColorScalesFactory<SPColorScalesMode::NONE>;
template class ColorScalesFactory<SPColorScalesMode::RGB>;
template class ColorScalesFactory<SPColorScalesMode::HSL>;
template class ColorScalesFactory<SPColorScalesMode::CMYK>;
template class ColorScalesFactory<SPColorScalesMode::HSV>;
template class ColorScalesFactory<SPColorScalesMode::HSLUV>;

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
