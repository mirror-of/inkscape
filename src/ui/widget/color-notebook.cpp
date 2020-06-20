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
#include <gtkmm/eventbox.h>
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
#include "ui/tools-switch.h"
#include "ui/tools/tool-base.h"
#include "ui/widget/color-entry.h"
#include "ui/widget/color-icc-selector.h"
#include "ui/widget/color-notebook.h"
#include "ui/widget/color-scales.h"
#include "ui/widget/color-wheel-selector.h"

#include "widgets/spw-utilities.h"

using Inkscape::CMSSystem;

#define XPAD 4
#define YPAD 1

namespace Inkscape {
namespace UI {
namespace Widget {


ColorNotebook::ColorNotebook(SelectedColor &color)
    : Gtk::Grid()
    , _selected_color(color)
{
    set_name("ColorNotebook");

    Page *page;

    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_RGB), true);
    _available_pages.push_back(page);
    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_HSL), true);
    _available_pages.push_back(page);
    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_HSV), true);
    _available_pages.push_back(page);
    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_CMYK), true);
    _available_pages.push_back(page);
    page = new Page(new ColorWheelSelectorFactory, true);
    _available_pages.push_back(page);
#if defined(HAVE_LIBLCMS2)
    page = new Page(new ColorICCSelectorFactory, true);
    _available_pages.push_back(page);
#endif

    _initUI();

    _selected_color.signal_changed.connect(sigc::mem_fun(this, &ColorNotebook::_onSelectedColorChanged));
    _selected_color.signal_dragged.connect(sigc::mem_fun(this, &ColorNotebook::_onSelectedColorChanged));
}

ColorNotebook::Page::Page(Inkscape::UI::ColorSelectorFactory *selector_factory, bool enabled_full)
    : selector_factory(selector_factory)
    , enabled_full(enabled_full)
{
}


void ColorNotebook::_initUI()
{
    guint row = 0;

    _book = Gtk::make_managed<Gtk::Notebook>();
    _book->show();
    _book->set_show_border(false);
    _book->set_show_tabs(false);

    _buttonbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 2);
    _buttonbox->set_homogeneous(true);

    _buttonbox->show();

    for (int i = 0; static_cast<size_t>(i) < _available_pages.size(); i++) {
        _addPage(_available_pages[i]);
    }

    _buttonbox->set_margin_start( XPAD);
    _buttonbox->set_margin_end(   XPAD);
    _buttonbox->set_margin_top(   YPAD);
    _buttonbox->set_margin_bottom(YPAD);
    _buttonbox->set_hexpand(true);
    _buttonbox->set_valign(Gtk::ALIGN_CENTER);
    attach(*_buttonbox, 0, row, 2, 1);

    row++;

    _book->set_margin_start( XPAD * 2);
    _book->set_margin_end(   XPAD * 2);
    _book->set_margin_top(   YPAD);
    _book->set_margin_bottom(YPAD);
    _book->set_hexpand(true);
    _book->set_vexpand(true);
    attach(*_book, 0, row, 2, 1);

    // restore the last active page
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _setCurrentPage(prefs->getInt("/colorselector/page", 0));
    row++;

    auto rgbabox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 0);

#if defined(HAVE_LIBLCMS2)
    /* Create color management icons */
    _box_colormanaged = Gtk::make_managed<Gtk::EventBox>();
    auto colormanaged = Glib::wrap(sp_get_icon_image("color-management", GTK_ICON_SIZE_SMALL_TOOLBAR));
    _box_colormanaged->add(*colormanaged);
    _box_colormanaged->set_tooltip_text(_("Color Managed"));
    _box_colormanaged->set_sensitive(false);
    rgbabox->pack_start(*_box_colormanaged, false, false, 2);

    _box_outofgamut = Gtk::make_managed<Gtk::EventBox>();
    auto outofgamut = Glib::wrap(sp_get_icon_image("out-of-gamut-icon", GTK_ICON_SIZE_SMALL_TOOLBAR));
    _box_outofgamut->add(*outofgamut);
    _box_outofgamut->set_tooltip_text(_("Out of gamut!"));
    _box_outofgamut->set_sensitive(false);
    rgbabox->pack_start(*_box_outofgamut, false, false, 2);

    _box_toomuchink = Gtk::make_managed<Gtk::EventBox>();
    auto toomuchink = Glib::wrap(sp_get_icon_image("too-much-ink-icon", GTK_ICON_SIZE_SMALL_TOOLBAR));
    _box_toomuchink->add(*toomuchink);
    _box_toomuchink->set_tooltip_text(_("Too much ink!"));
    _box_toomuchink->set_sensitive(false);
    rgbabox->pack_start(*_box_toomuchink, false, false, 2);
#endif // defined(HAVE_LIBLCMS2)

    /* Color picker */
    auto picker = Glib::wrap(sp_get_icon_image("color-picker", GTK_ICON_SIZE_SMALL_TOOLBAR));
    _btn_picker = Gtk::make_managed<Gtk::Button>();
    _btn_picker->set_relief(Gtk::RELIEF_NONE);
    _btn_picker->add(*picker);
    _btn_picker->set_tooltip_text(_("Pick colors from image"));
    rgbabox->pack_start(*_btn_picker, false, false, 2);
    _btn_picker->signal_clicked().connect(sigc::mem_fun(*this, &ColorNotebook::_onPickerClicked));

    /* Create RGBA entry and color preview */
    _rgbal = Gtk::make_managed<Gtk::Label>(_("RGBA_:"), true);
    _rgbal->set_halign(Gtk::ALIGN_END);
    rgbabox->pack_start(*_rgbal, true, true, 2);

    auto rgba_entry = Gtk::make_managed<ColorEntry>(_selected_color);
    sp_dialog_defocus_on_enter(GTK_WIDGET(rgba_entry->gobj()));
    rgbabox->pack_start(*rgba_entry, false, false, 0);
    _rgbal->set_mnemonic_widget(*rgba_entry);

    rgbabox->show_all();

#if defined(HAVE_LIBLCMS2)
    // the "too much ink" icon is initially hidden
    _box_toomuchink->hide();
#endif // defined(HAVE_LIBLCMS2)

    rgbabox->set_margin_start( XPAD);
    rgbabox->set_margin_end(   XPAD);
    rgbabox->set_margin_top(   YPAD);
    rgbabox->set_margin_bottom(YPAD);
    attach(*rgbabox, 0, row, 2, 1);

#ifdef SPCS_PREVIEW
    _p = sp_color_preview_new(0xffffffff);
    gtk_widget_show(_p);
    attach(*Glib::wrap(_p), 2, 3, row, row + 1, Gtk::FILL, Gtk::FILL, XPAD, YPAD);
#endif

    _book->signal_switch_page().connect(sigc::mem_fun(this, &ColorNotebook::_onPageSwitched));
}

void ColorNotebook::_onPickerClicked()
{
    // Set the dropper into a "one click" mode, so it reverts to the previous tool after a click
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/dropper/onetimepick", true);
    Inkscape::UI::Tools::sp_toggle_dropper(SP_ACTIVE_DESKTOP);
}

void ColorNotebook::_onButtonClicked(int page_num)
{
    if (!_buttons[page_num]->get_active()) {
        return;
    }

    _book->set_current_page(page_num);
}

void ColorNotebook::_onSelectedColorChanged() { _updateICCButtons(); }

void ColorNotebook::_onPageSwitched(Gtk::Widget *page, guint page_num)
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

#if defined(HAVE_LIBLCMS2)
    /* update color management icon*/
    _box_colormanaged->set_sensitive(color.icc != nullptr);

    /* update out-of-gamut icon */
    _box_outofgamut->set_sensitive(false);
    if (color.icc) {
        Inkscape::ColorProfile *target_profile =
            SP_ACTIVE_DOCUMENT->getProfileManager()->find(color.icc->colorProfile.c_str());
        if (target_profile) {
            _box_outofgamut->set_sensitive(target_profile->GamutCheck(color));
        }
    }

    /* update too-much-ink icon */
    _box_toomuchink->set_sensitive(false);
    if (color.icc) {
        Inkscape::ColorProfile *prof = SP_ACTIVE_DOCUMENT->getProfileManager()->find(color.icc->colorProfile.c_str());
        if (prof && CMSSystem::isPrintColorSpace(prof)) {
            _box_toomuchink->show();
            double ink_sum = 0;
            for (double i : color.icc->colors) {
                ink_sum += i;
            }

            /* Some literature states that when the sum of paint values exceed 320%, it is considered to be a satured
               color,
                which means the paper can get too wet due to an excessive amount of ink. This may lead to several
               issues
                such as misalignment and poor quality of printing in general.*/
            if (ink_sum > 3.2) _box_toomuchink->set_sensitive(true);
        }
        else {
            _box_toomuchink->hide();
        }
    }
#endif // defined(HAVE_LIBLCMS2)
}

void ColorNotebook::_setCurrentPage(int i)
{
    _book->set_current_page(i);

    if (!_buttons.empty() && (static_cast<size_t>(i) < _available_pages.size())) {
        _buttons[i]->set_active(true);
    }
}

void ColorNotebook::_addPage(Page &page)
{
    Gtk::Widget *selector_widget;

    selector_widget = page.selector_factory->createWidget(_selected_color);
    if (selector_widget) {
        selector_widget->show();

        Glib::ustring mode_name = page.selector_factory->modeName();
        Gtk::Widget *tab_label = Gtk::manage(new Gtk::Label(mode_name));
        tab_label->set_name("ColorModeLabel");
        auto page_num = _book->append_page(*selector_widget, *tab_label);

        _buttons.push_back(Gtk::make_managed<Gtk::RadioButton>(mode_name));
        _buttons[page_num]->set_name("ColorModeButton");
        _buttons[page_num]->set_mode(false);
        if (page_num > 0) {
            auto g = _buttons[0]->get_group();
            _buttons[page_num]->set_group(g);
        }
        _buttons[page_num]->show();
        _buttonbox->pack_start(*_buttons[page_num], true, true, 0);

        _buttons[page_num]->signal_clicked().connect(sigc::bind(sigc::mem_fun(this, &ColorNotebook::_onButtonClicked), page_num));
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
