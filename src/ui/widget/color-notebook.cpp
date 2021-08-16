// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * A notebook with RGB, CMYK, CMS, HSL, and Wheel pages
 *//*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Tomasz Boczkowski <penginsbacon@gmail.com> (c++-sification)
 *
 * Copyright (C) 2001-2014 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"  // only include where actually required!
#endif

#undef SPCS_PREVIEW
#define noDUMP_CHANGE_INFO

#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/radiobutton.h>

#include "cms-system.h"
#include "document.h"
#include "inkscape.h"
#include "preferences.h"
#include "profile-manager.h"

#include "object/color-profile.h"
#include "ui/icon-loader.h"

#include "svg/svg-icc-color.h"

#include "ui/dialog-events.h"
#include "ui/tools/dropper-tool.h"
#include "ui/widget/color-entry.h"
#include "ui/widget/color-icc-selector.h"
#include "ui/widget/color-notebook.h"
#include "ui/widget/color-scales.h"
#include "ui/widget/color-wheel-selector.h"

#include "widgets/spw-utilities.h"

using Inkscape::CMSSystem;

#define XPAD 2
#define YPAD 1

namespace Inkscape {
namespace UI {
namespace Widget {


ColorNotebook::ColorNotebook(SelectedColor &color)
    : Gtk::Grid()
    , _selected_color(color)
{
    set_name("ColorNotebook");

    _available_pages.push_back(new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_HSL), "color-selector-hsx"));
    _available_pages.push_back(new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_HSV), "color-selector-hsx"));
    _available_pages.push_back(new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_RGB), "color-selector-rgb"));
    _available_pages.push_back(new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_CMYK), "color-selector-cmyk"));
    _available_pages.push_back(new Page(new ColorWheelSelectorFactory, "color-selector-wheel"));
    _available_pages.push_back(new Page(new ColorICCSelectorFactory, "color-selector-cms"));

    _initUI();

    _selected_color.signal_changed.connect(sigc::mem_fun(this, &ColorNotebook::_onSelectedColorChanged));
    _selected_color.signal_dragged.connect(sigc::mem_fun(this, &ColorNotebook::_onSelectedColorChanged));
}

ColorNotebook::~ColorNotebook()
{
    if (_onetimepick)
        _onetimepick.disconnect();
}

ColorNotebook::Page::Page(Inkscape::UI::ColorSelectorFactory *selector_factory, const char* icon)
    : selector_factory(selector_factory), icon_name(icon)
{
}

void ColorNotebook::set_label(const Glib::ustring& label) {
    _label->set_markup(label);
}

void ColorNotebook::_initUI()
{
    guint row = 0;

    _book = Gtk::make_managed<Gtk::Stack>();
    _book->show();
    _book->set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
    _book->set_transition_duration(130);

    // mode selection switcher widget shows all buttons for color mode selection, side by side
    _switcher = Gtk::make_managed<Gtk::StackSwitcher>();
    _switcher->set_stack(*_book);
    // cannot leave it homogenous - in some themes switcher gets very wide
    _switcher->set_homogeneous(false);
    _switcher->set_halign(Gtk::ALIGN_CENTER);
    _switcher->show();
    attach(*_switcher, 0, row++, 2);

    _buttonbox = Gtk::make_managed<Gtk::Box>();
    _buttonbox->show();

    // combo mode selection is compact and only shows one entry (active)
    _combo = Gtk::manage(new IconComboBox());
    _combo->set_can_focus(false);
    _combo->set_visible();
    _combo->set_tooltip_text(_("Choose style of color selection"));

    for (auto&& page : _available_pages) {
        _addPage(page);
    }

    _label = Gtk::make_managed<Gtk::Label>();
    _label->set_visible();
    _buttonbox->pack_start(*_label, false, true);
    _buttonbox->pack_end(*_combo, false, false);
    _combo->signal_changed().connect([=](){ _setCurrentPage(_combo->get_active_row_id(), false); });

    _buttonbox->set_margin_start(XPAD);
    _buttonbox->set_margin_end(XPAD);
    _buttonbox->set_margin_top(YPAD);
    _buttonbox->set_margin_bottom(YPAD);
    _buttonbox->set_hexpand();
    _buttonbox->set_valign(Gtk::ALIGN_START);
    attach(*_buttonbox, 0, row, 2);

    row++;

    _book->set_margin_start(XPAD);
    _book->set_margin_end(XPAD);
    _book->set_margin_top(YPAD);
    _book->set_margin_bottom(YPAD);
    _book->set_hexpand();
    _book->set_vexpand();
    attach(*_book, 0, row, 2, 1);

    // restore the last active page
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _setCurrentPage(prefs->getInt("/colorselector/page", 0), true);
    row++;

    auto switcher_path = Glib::ustring("/colorselector/switcher");
    auto choose_switch = [=](bool compact) {
        if (compact) {
            _switcher->hide();
            _buttonbox->show();
        }
        else {
            _buttonbox->hide();
            _switcher->show();
        }
    };

    _observer = prefs->createObserver(switcher_path, [=](const Preferences::Entry& new_value) {
        choose_switch(new_value.getBool());
    });

    choose_switch(prefs->getBool(switcher_path));

    GtkWidget *rgbabox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* Create color management icons */
    _box_colormanaged = gtk_event_box_new();
    GtkWidget *colormanaged = sp_get_icon_image("color-management", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(_box_colormanaged), colormanaged);
    gtk_widget_set_tooltip_text(_box_colormanaged, _("Color Managed"));
    gtk_widget_set_sensitive(_box_colormanaged, false);
    gtk_box_pack_start(GTK_BOX(rgbabox), _box_colormanaged, FALSE, FALSE, 2);

    _box_outofgamut = gtk_event_box_new();
    GtkWidget *outofgamut = sp_get_icon_image("out-of-gamut-icon", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(_box_outofgamut), outofgamut);
    gtk_widget_set_tooltip_text(_box_outofgamut, _("Out of gamut!"));
    gtk_widget_set_sensitive(_box_outofgamut, false);
    gtk_box_pack_start(GTK_BOX(rgbabox), _box_outofgamut, FALSE, FALSE, 2);

    _box_toomuchink = gtk_event_box_new();
    GtkWidget *toomuchink = sp_get_icon_image("too-much-ink-icon", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(_box_toomuchink), toomuchink);
    gtk_widget_set_tooltip_text(_box_toomuchink, _("Too much ink!"));
    gtk_widget_set_sensitive(_box_toomuchink, false);
    gtk_box_pack_start(GTK_BOX(rgbabox), _box_toomuchink, FALSE, FALSE, 2);


    /* Color picker */
    GtkWidget *picker = sp_get_icon_image("color-picker", GTK_ICON_SIZE_SMALL_TOOLBAR);
    _btn_picker = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(_btn_picker), GTK_RELIEF_NONE);
    gtk_container_add(GTK_CONTAINER(_btn_picker), picker);
    gtk_widget_set_tooltip_text(_btn_picker, _("Pick colors from image"));
    gtk_box_pack_start(GTK_BOX(rgbabox), _btn_picker, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(_btn_picker), "clicked", G_CALLBACK(ColorNotebook::_onPickerClicked), this);

    /* Create RGBA entry and color preview */
    _rgbal = gtk_label_new_with_mnemonic(_("RGBA_:"));
    gtk_widget_set_halign(_rgbal, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(rgbabox), _rgbal, TRUE, TRUE, 2);

    ColorEntry *rgba_entry = Gtk::manage(new ColorEntry(_selected_color));
    sp_dialog_defocus_on_enter(GTK_WIDGET(rgba_entry->gobj()));
    gtk_box_pack_start(GTK_BOX(rgbabox), GTK_WIDGET(rgba_entry->gobj()), FALSE, FALSE, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(_rgbal), GTK_WIDGET(rgba_entry->gobj()));

    gtk_widget_show_all(rgbabox);

    // the "too much ink" icon is initially hidden
    gtk_widget_hide(GTK_WIDGET(_box_toomuchink));

    gtk_widget_set_margin_start(rgbabox, XPAD);
    gtk_widget_set_margin_end(rgbabox, XPAD);
    gtk_widget_set_margin_top(rgbabox, YPAD);
    gtk_widget_set_margin_bottom(rgbabox, YPAD);
    attach(*Glib::wrap(rgbabox), 0, row, 2, 1);

#ifdef SPCS_PREVIEW
    _p = sp_color_preview_new(0xffffffff);
    gtk_widget_show(_p);
    attach(*Glib::wrap(_p), 2, 3, row, row + 1, Gtk::FILL, Gtk::FILL, XPAD, YPAD);
#endif
}

void ColorNotebook::_onPickerClicked(GtkWidget * /*widget*/, ColorNotebook *colorbook)
{
    // Set the dropper into a "one click" mode, so it reverts to the previous tool after a click
    if (colorbook->_onetimepick) {
        colorbook->_onetimepick.disconnect();
    }
    else {
        Inkscape::UI::Tools::sp_toggle_dropper(SP_ACTIVE_DESKTOP);
        auto tool = dynamic_cast<Inkscape::UI::Tools::DropperTool *>(SP_ACTIVE_DESKTOP->event_context);
        if (tool) {
            colorbook->_onetimepick = tool->onetimepick_signal.connect(sigc::mem_fun(*colorbook, &ColorNotebook::_pickColor));
        }
    }
}

void ColorNotebook::_pickColor(ColorRGBA *color) {
    // Set color to color notebook here.
    _selected_color.setValue(color->getIntValue());
    _onSelectedColorChanged();
}

void ColorNotebook::_onSelectedColorChanged() { _updateICCButtons(); }

void ColorNotebook::_onPageSwitched(int page_num)
{
    if (get_visible()) {
        // remember the page we switched to
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt("/colorselector/page", page_num);
    }
}


// TODO pass in param so as to avoid the need for SP_ACTIVE_DOCUMENT
void ColorNotebook::_updateICCButtons()
{
    SPColor color = _selected_color.color();
    gfloat alpha = _selected_color.alpha();

    g_return_if_fail((0.0 <= alpha) && (alpha <= 1.0));

    /* update color management icon*/
    gtk_widget_set_sensitive(_box_colormanaged, color.icc != nullptr);

    /* update out-of-gamut icon */
    gtk_widget_set_sensitive(_box_outofgamut, false);
    if (color.icc) {
        Inkscape::ColorProfile *target_profile =
            SP_ACTIVE_DOCUMENT->getProfileManager()->find(color.icc->colorProfile.c_str());
        if (target_profile)
            gtk_widget_set_sensitive(_box_outofgamut, target_profile->GamutCheck(color));
    }

    /* update too-much-ink icon */
    gtk_widget_set_sensitive(_box_toomuchink, false);
    if (color.icc) {
        Inkscape::ColorProfile *prof = SP_ACTIVE_DOCUMENT->getProfileManager()->find(color.icc->colorProfile.c_str());
        if (prof && CMSSystem::isPrintColorSpace(prof)) {
            gtk_widget_show(GTK_WIDGET(_box_toomuchink));
            double ink_sum = 0;
            for (double i : color.icc->colors) {
                ink_sum += i;
            }

            /* Some literature states that when the sum of paint values exceed 320%, it is considered to be a satured color,
                which means the paper can get too wet due to an excessive amount of ink. This may lead to several issues
                such as misalignment and poor quality of printing in general.*/
            if (ink_sum > 3.2)
                gtk_widget_set_sensitive(_box_toomuchink, true);
        }
        else {
            gtk_widget_hide(GTK_WIDGET(_box_toomuchink));
        }
    }
}

void ColorNotebook::_setCurrentPage(int i, bool sync_combo)
{
    const auto pages = _book->get_children();
    if (i >= 0 && i < pages.size()) {
        _book->set_visible_child(*pages[i]);
        if (sync_combo) {
            _combo->set_active_by_id(i);
        }
        _onPageSwitched(i);
    }
}

void ColorNotebook::_addPage(Page &page)
{
    if (auto selector_widget = page.selector_factory->createWidget(_selected_color)) {
        selector_widget->show();

        Glib::ustring mode_name = page.selector_factory->modeName();
        _book->add(*selector_widget, mode_name, mode_name);
        int page_num = _book->get_children().size() - 1;

        _combo->add_row(page.icon_name, mode_name, page_num);
    }
}

}
}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
