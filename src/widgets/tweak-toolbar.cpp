/**
 * @file
 * Tweak aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tweak-toolbar.h"

#include <glibmm/i18n.h>
#include <gtkmm/radiotoolbutton.h>
#include <gtkmm/separatortoolitem.h>

#include "desktop.h"
#include "document-undo.h"

#include "ui/icon-names.h"
#include "ui/tools/tweak-tool.h"
#include "ui/widget/spinbutton.h"
#include "ui/widget/spin-button-tool-item.h"

#include "widgets/toolbox.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

namespace Inkscape {
namespace UI {
namespace Toolbar {

void
TweakToolbar::on_do_H_btn_toggled() {
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/doh", _do_H_btn->get_active());
}

void
TweakToolbar::on_do_S_btn_toggled() {
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/dos", _do_S_btn->get_active());
}

void
TweakToolbar::on_do_L_btn_toggled() {
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/dol", _do_L_btn->get_active());
}

void
TweakToolbar::on_do_O_btn_toggled() {
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/doo", _do_O_btn->get_active());
}

void
TweakToolbar::on_width_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/tweak/width",
            _width_adj->get_value() * 0.01 );
}

void
TweakToolbar::on_force_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/tweak/force",
            _force_adj->get_value() * 0.01 );
}

void
TweakToolbar::on_fidelity_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/tweak/fidelity",
                      _fidelity_adj->get_value() * 0.01 );
}


void
TweakToolbar::on_pressure_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/usepressure", _pressure_btn->get_active());
}

void
TweakToolbar::on_mode_button_clicked(int mode)
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/tweak/mode", mode);

    bool flag = ((mode == Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT) ||
                 (mode == Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER));

    _channels_label->set_visible(flag);
    _do_H_btn->set_visible(flag);
    _do_S_btn->set_visible(flag);
    _do_L_btn->set_visible(flag);
    _do_O_btn->set_visible(flag);

    _fidelity_btn->set_visible(!flag);
}

Gtk::RadioToolButton *
TweakToolbar::create_radio_tool_button(Gtk::RadioButtonGroup &group,
                                       const Glib::ustring   &label,
                                       const Glib::ustring   &tooltip_text,
                                       const Glib::ustring   &icon_name)
{
    auto btn = Gtk::manage(new Gtk::RadioToolButton(group, label));
    btn->set_tooltip_text(tooltip_text);
    btn->set_icon_name(icon_name);

    return btn;
}

TweakToolbar::TweakToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _pressure_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _do_H_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _do_S_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _do_L_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _do_O_btn(Gtk::manage(new Gtk::ToggleToolButton()))
{
    auto prefs = Inkscape::Preferences::get();

    auto width_val    = prefs->getDouble("/tools/tweak/width", 15) * 100.0;
    auto force_val    = prefs->getDouble("/tools/tweak/force", 20) * 100.0;
    auto fidelity_val = prefs->getDouble("/tools/tweak/fidelity", 50) * 100.0;

    _width_adj    = Gtk::Adjustment::create(width_val, 1, 100, 1.0, 10.0);
    _force_adj    = Gtk::Adjustment::create(force_val, 1, 100, 1.0, 10.0);
    _fidelity_adj = Gtk::Adjustment::create(force_val, 1, 100, 1.0, 10.0);

    auto width_btn    = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("tweak-width", _("Width:"), _width_adj, 0.01, 0));
    auto force_btn    = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("tweak-force", _("Force:"), _force_adj, 0.01, 0));
    _fidelity_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("tweak-fidelity", _("Fidelity:"), _fidelity_adj, 0.01, 0));

    width_btn->set_all_tooltip_text(_("The width of the tweak area (relative to the visible canvas area)"));
    force_btn->set_all_tooltip_text(_("The force of the tweak action"));
    _fidelity_btn->set_all_tooltip_text(_("Low fidelity simplifies paths; high fidelity preserves path features but may generate a lot of new nodes"));

    width_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    force_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    _fidelity_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    auto width_adj_value_changed_cb    = sigc::mem_fun(*this, &TweakToolbar::on_width_adj_value_changed);
    auto force_adj_value_changed_cb    = sigc::mem_fun(*this, &TweakToolbar::on_force_adj_value_changed);
    auto fidelity_adj_value_changed_cb = sigc::mem_fun(*this, &TweakToolbar::on_fidelity_adj_value_changed);

    _width_adj->signal_value_changed().connect(width_adj_value_changed_cb);
    _force_adj->signal_value_changed().connect(force_adj_value_changed_cb);
    _fidelity_adj->signal_value_changed().connect(fidelity_adj_value_changed_cb);

    // TODO: Consider adding explicit items for width button's proxy menu item.
    //       However, there may be no point, as this tool probably won't fall off the toolbar
    // gchar const* labels[] = {_("(pinch tweak)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad tweak)")};
    // gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};

    // TODO: Explicit labels for force button's proxy menu item.
    // gchar const* labels[] = {_("(minimum force)"), 0, 0, _("(default)"), 0, 0, 0, _("(maximum force)")};
    // gdouble values[] = {1, 5, 10, 20, 30, 50, 70, 100};

    // TODO: Explicity labels for fidelity button
    // gchar const* labels[] = {_("(rough, simplified)"), 0, 0, _("(default)"), 0, 0, _("(fine, but many nodes)")};
    //  gdouble values[] = {10, 25, 35, 50, 60, 80, 100};

    _pressure_btn->set_label(_("Pressure"));

    //TRANSLATORS: Here, "H" stands for "hue"
    _do_H_btn->set_label(C_("Hue",        "H"));

    //TRANSLATORS: "S" here stands for Saturation
    _do_S_btn->set_label(C_("Saturation", "S"));

    //TRANSLATORS: "L" here stands for Lightness
    _do_L_btn->set_label(C_("Lightness",  "L"));

    //TRANSLATORS: "O" here stands for Opacity
    _do_O_btn->set_label(C_("Opacity",    "O"));

    _pressure_btn->set_icon_name(INKSCAPE_ICON("draw-use-pressure"));

    _pressure_btn->set_tooltip_text(_("Use the pressure of the input device to alter the force of tweak action"));
    _do_H_btn->set_tooltip_text(_("In color mode, act on object's hue"));
    _do_S_btn->set_tooltip_text(_("In color mode, act on object's saturation"));
    _do_L_btn->set_tooltip_text(_("In color mode, act on object's lightness"));
    _do_O_btn->set_tooltip_text(_("In color mode, act on object's opacity"));

    _pressure_btn->set_active(prefs->getBool("/tools/tweak/usepressure", true));
    _do_H_btn->set_active(prefs->getBool("/tools/tweak/doh", true));
    _do_S_btn->set_active(prefs->getBool("/tools/tweak/dos", true));
    _do_L_btn->set_active(prefs->getBool("/tools/tweak/dol", true));
    _do_O_btn->set_active(prefs->getBool("/tools/tweak/doo", true));

    auto pressure_btn_toggled_cb = sigc::mem_fun(*this, &TweakToolbar::on_pressure_btn_toggled);
    auto do_H_btn_toggled_cb = sigc::mem_fun(*this, &TweakToolbar::on_do_H_btn_toggled);
    auto do_S_btn_toggled_cb = sigc::mem_fun(*this, &TweakToolbar::on_do_S_btn_toggled);
    auto do_L_btn_toggled_cb = sigc::mem_fun(*this, &TweakToolbar::on_do_L_btn_toggled);
    auto do_O_btn_toggled_cb = sigc::mem_fun(*this, &TweakToolbar::on_do_O_btn_toggled);

    _pressure_btn->signal_toggled().connect(pressure_btn_toggled_cb);
    _do_H_btn->signal_toggled().connect(do_H_btn_toggled_cb);
    _do_S_btn->signal_toggled().connect(do_S_btn_toggled_cb);
    _do_L_btn->signal_toggled().connect(do_L_btn_toggled_cb);
    _do_O_btn->signal_toggled().connect(do_O_btn_toggled_cb);

    // Add items to toolbar in correct order
    add(*width_btn);
    add(*force_btn);
    add(*_pressure_btn);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));

    // Add Move-mode radio buttons
    auto mode_label = Gtk::manage(new Gtk::Label(_("Move:")));
    auto mode_label_ti = Gtk::manage(new Gtk::ToolItem());
    mode_label_ti->add(*mode_label);
    add(*mode_label_ti);

    Gtk::RadioToolButton::Group mode_button_group;

    std::vector<Gtk::RadioToolButton *> mode_buttons;

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Move mode"),
                                                    _("Move objects in any direction"),
                                                    INKSCAPE_ICON("object-tweak-push")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Move in/out mode"),
                                                    _("Move objects towards cursor; with Shift from cursor"),
                                                    INKSCAPE_ICON("object-tweak-attract")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Move jitter mode"),
                                                    _("Move objects in random directions"),
                                                    INKSCAPE_ICON("object-tweak-randomize")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Scale mode"),
                                                    _("Shrink objects, with Shift enlarge"),
                                                    INKSCAPE_ICON("object-tweak-shrink")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Rotate mode"),
                                                    _("Rotate objects, with Shift counterclockwise"),
                                                    INKSCAPE_ICON("object-tweak-rotate")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Duplicate/delete mode"),
                                                    _("Duplicate objects, with Shift delete"),
                                                    INKSCAPE_ICON("object-tweak-duplicate")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Push mode"),
                                                    _("Push parts of paths in any direction"),
                                                    INKSCAPE_ICON("path-tweak-push")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Shrink/grow mode"),
                                                    _("Shrink (inset) parts of paths; with Shift grow (outset)"),
                                                    INKSCAPE_ICON("path-tweak-shrink")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Attract/repel mode"),
                                                    _("Attract parts of paths towards cursor; with Shift from cursor"),
                                                    INKSCAPE_ICON("path-tweak-attract")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Roughen mode"),
                                                    _("Roughen parts of paths"),
                                                    INKSCAPE_ICON("path-tweak-roughen")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Color paint mode"),
                                                    _("Paint the tool's color upon selected objects"),
                                                    INKSCAPE_ICON("object-tweak-paint")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Color jitter mode"),
                                                    _("Jitter the colors of selected objects"),
                                                    INKSCAPE_ICON("object-tweak-jitter-color")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Blur mode"),
                                                    _("Blur selected objects more; with Shift, blur less"),
                                                    INKSCAPE_ICON("object-tweak-blur")));

    // Activate the appropriate mode button according to preference
    int current_mode = prefs->getInt("/tools/tweak/mode", 0);
    mode_buttons[current_mode]->set_active();

    auto btn_index = 0;

    for (auto btn : mode_buttons) {
        add(*btn);
        btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &TweakToolbar::on_mode_button_clicked), btn_index));
        ++btn_index;
    }

    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*_fidelity_btn);

    auto channels_label_widget = Gtk::manage(new Gtk::Label(_("Channels:")));
    _channels_label = Gtk::manage(new Gtk::ToolItem());
    _channels_label->add(*channels_label_widget);
    add(*_channels_label);
    add(*_do_H_btn);
    add(*_do_S_btn);
    add(*_do_L_btn);
    add(*_do_O_btn);

    show_all();

    // Hide selected widgets depending on whether we're in colorpaint/colorjitter mode
    if (current_mode == Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT || current_mode == Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
        _fidelity_btn->set_visible (false);
    } else {
        _channels_label->set_visible(false);
        _do_H_btn->set_visible(false);
        _do_S_btn->set_visible(false);
        _do_L_btn->set_visible(false);
        _do_O_btn->set_visible(false);
    }
}

GtkWidget *
TweakToolbar::create(SPDesktop *desktop)
{
    auto toolbar = Gtk::manage(new TweakToolbar(desktop));
    return GTK_WIDGET(toolbar->gobj());
}
} // namespace Toolbar
} // namespace UI
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
