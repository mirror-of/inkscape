/**
 * @file
 * Spray aux toolbar
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
 *   Jabiertxo Arraiza <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2015 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "spray-toolbar.h"

#include <gtkmm/adjustment.h>
#include <gtkmm/label.h>
#include <gtkmm/radiotoolbutton.h>
#include <gtkmm/separatortoolitem.h>

#include "desktop.h"
#include "inkscape.h"
#include "widgets/toolbox.h"
#include "ui/dialog/clonetiler.h"
#include "ui/dialog/dialog-manager.h"
#include "ui/dialog/panel-dialog.h"
#include "ui/tools/spray-tool.h"
#include "ui/widget/spin-button-tool-item.h"
#include "ui/icon-names.h"

#include <glibmm/i18n.h>

using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

// Disabled in 0.91 because of Bug #1274831 (crash, spraying an object 
// with the mode: spray object in single path)
// Please enable again when working on 1.0
#define ENABLE_SPRAY_MODE_SINGLE_PATH

Inkscape::UI::Dialog::CloneTiler *get_clone_tiler_panel(SPDesktop *desktop)
{
    if (Inkscape::UI::Dialog::PanelDialogBase *panel_dialog =
        dynamic_cast<Inkscape::UI::Dialog::PanelDialogBase *>(desktop->_dlg_mgr->getDialog("CloneTiler"))) {
        try {
            Inkscape::UI::Dialog::CloneTiler &clone_tiler =
                dynamic_cast<Inkscape::UI::Dialog::CloneTiler &>(panel_dialog->getPanel());
            return &clone_tiler;
        } catch (std::exception &e) { }
    }

    return 0;
}

namespace Inkscape {
namespace UI {
namespace Toolbar {

void
SprayToolbar::on_pick_no_overlap_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _pick_no_overlap_btn->get_active();
    prefs->setBool("/tools/spray/pick_no_overlap", active);
}

void
SprayToolbar::on_pick_inverse_value_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _pick_inverse_value_btn->get_active();
    prefs->setBool("/tools/spray/pick_inverse_value", active);
}

void
SprayToolbar::on_over_no_transparent_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _over_no_transparent_btn->get_active();
    prefs->setBool("/tools/spray/over_no_transparent", active);
}

void
SprayToolbar::on_pick_center_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _pick_center_btn->get_active();
    prefs->setBool("/tools/spray/pick_center", active);
}


void
SprayToolbar::on_pick_fill_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _pick_fill_btn->get_active();
    prefs->setBool("/tools/spray/pick_fill", active);
}

void
SprayToolbar::on_over_transparent_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _over_transparent_btn->get_active();
    prefs->setBool("/tools/spray/over_transparent", active);
}

void
SprayToolbar::on_pick_stroke_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _pick_stroke_btn->get_active();
    prefs->setBool("/tools/spray/pick_stroke", active);
}

void
SprayToolbar::update_widget_visibility(){
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int mode = prefs->getInt("/tools/spray/mode", 0);

    bool show = true;
    if(mode == Inkscape::UI::Tools::SPRAY_MODE_ERASER ||
       mode == Inkscape::UI::Tools::SPRAY_MODE_SINGLE_PATH){
        show = false;
    }

    _no_overlap_btn->set_visible(show);
    _over_no_transparent_btn->set_visible(show);
    _over_transparent_btn->set_visible(show);
    _pick_no_overlap_btn->set_visible(show);
    _pick_stroke_btn->set_visible(show);
    _pick_fill_btn->set_visible(show);
    _pick_inverse_value_btn->set_visible(show);
    _pick_center_btn->set_visible(show);
    _pick_color_btn->set_visible(show);
    _offset_btn->set_visible(show);
    _pick_stroke_btn->set_visible(show);

    if(mode == Inkscape::UI::Tools::SPRAY_MODE_SINGLE_PATH){
        show = true;
    }

    _rotation_btn->set_visible(show);
    update_widgets();
}

void
SprayToolbar::update_widgets()
{
    _offset_adj->set_value(100.0);
    if (_no_overlap_btn->get_active() && _no_overlap_btn->get_visible()) {
        _offset_btn->set_visible(true );
    } else {
        _offset_btn->set_visible(false);
    }
    if (_pressure_scale_btn->get_active()) {
        _scale_adj->set_value(0.0);
        _scale_btn->set_sensitive(false);
    } else {
        _scale_btn->set_sensitive(true);
    }
    if(_pick_color_btn->get_active() && _pick_color_btn->get_visible()){
        _pick_fill_btn->set_visible(true);
        _pick_stroke_btn->set_visible(true);
        _pick_inverse_value_btn->set_visible(true);
        _pick_center_btn->set_visible(true);
    } else {
        _pick_fill_btn->set_visible(false);
        _pick_stroke_btn->set_visible(false);
        _pick_inverse_value_btn->set_visible(false);
        _pick_center_btn->set_visible(false);
    }
}

void
SprayToolbar::on_width_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/width",
            _width_adj->get_value());
}

void
SprayToolbar::on_no_overlap_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _no_overlap_btn->get_active();
    prefs->setBool("/tools/spray/no_overlap", active);
    update_widgets();
}

void
SprayToolbar::on_pick_color_btn_toggled()
{
    auto prefs = Inkscape::Preferences::get();
    bool active = _pick_color_btn->get_active();
    prefs->setBool("/tools/spray/picker", active);
    if(active){
        prefs->setBool("/dialogs/clonetiler/dotrace", false);
        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        if (Inkscape::UI::Dialog::CloneTiler *ct = get_clone_tiler_panel(dt)){
            dt->_dlg_mgr->showDialog("CloneTiler");
            ct->show_page_trace();
        }
    }
    update_widgets();
}

void
SprayToolbar::on_mean_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/mean",
            _mean_adj->get_value());
}

void
SprayToolbar::on_stddev_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/standard_deviation",
            _stddev_adj->get_value());
}

void
SprayToolbar::on_population_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/population",
            _population_adj->get_value());
}

void
SprayToolbar::on_rotation_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/rotation_variation",
            _rotation_adj->get_value());
}

void
SprayToolbar::on_scale_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/scale_variation",
            _scale_adj->get_value());
}

void
SprayToolbar::on_offset_adj_value_changed()
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/offset",
            _offset_adj->get_value());
}

void
SprayToolbar::on_mode_button_clicked(int mode)
{
    auto prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/spray/mode", mode);

    update_widget_visibility();
}

Gtk::RadioToolButton *
SprayToolbar::create_radio_tool_button(Gtk::RadioButtonGroup &group,
                                       const Glib::ustring   &label,
                                       const Glib::ustring   &tooltip_text,
                                       const Glib::ustring   &icon_name)
{
    auto btn = Gtk::manage(new Gtk::RadioToolButton(group, label));
    btn->set_tooltip_text(tooltip_text);
    btn->set_icon_name(icon_name);

    return btn;
}

void
SprayToolbar::on_pressure_scale_btn_toggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool active = _pressure_scale_btn->get_active();
    prefs->setBool("/tools/spray/usepressurescale", active);
    if(active){
        prefs->setDouble("/tools/spray/scale_variation", 0);
    }
    update_widgets();
}

SprayToolbar::SprayToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _pressure_width_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pressure_pop_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pressure_scale_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pick_color_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pick_center_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pick_inverse_value_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pick_fill_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pick_stroke_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _pick_no_overlap_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _over_transparent_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _over_no_transparent_btn(Gtk::manage(new Gtk::ToggleToolButton())),
      _no_overlap_btn(Gtk::manage(new Gtk::ToggleToolButton()))
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    auto mode_label = Gtk::manage(new Gtk::Label(_("Mode:")));
    auto mode_label_ti = Gtk::manage(new Gtk::ToolItem());
    mode_label_ti->add(*mode_label);

    Gtk::RadioToolButton::Group mode_button_group;
    std::vector<Gtk::RadioToolButton *> mode_buttons;

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Spray with copies"),
                                                    _("Spray copies of the initial selection"),
                                                    INKSCAPE_ICON("spray-mode-copy")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Spray with clones"),
                                                    _("Spray clones of the initial selection"),
                                                    INKSCAPE_ICON("spray-mode-clone")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Spray single path"),
                                                    _("Spray objects in a single path"),
                                                    INKSCAPE_ICON("spray-mode-union")));

    mode_buttons.push_back(create_radio_tool_button(mode_button_group,
                                                    _("Delete sprayed items"),
                                                    _("Delete sprayed items from selection"),
                                                    INKSCAPE_ICON("draw-eraser")));

    int mode = prefs->getInt("/tools/spray/mode", 1);
    mode_buttons[mode]->set_active();

    _pressure_width_btn->set_tooltip_text(_("Use the pressure of the input device to alter the width of spray area"));
    _pressure_width_btn->set_icon_name(INKSCAPE_ICON("draw-use-pressure"));
    _pressure_width_pusher = new PrefPusher(_pressure_width_btn, "/tools/spray/usepressurewidth");

    _pressure_pop_btn->set_tooltip_text(_("Use the pressure of the input device to alter the amount of sprayed objects"));
    _pressure_pop_btn->set_icon_name(INKSCAPE_ICON("draw-use-pressure"));
    _pressure_pop_pusher = new PrefPusher(_pressure_pop_btn, "/tools/spray/usepressurepopulation");

    auto pressure_scale_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pressure_scale_btn_toggled);
    _pressure_scale_btn->set_tooltip_text(_("Use the pressure of the input device to alter the scale of new items"));
    _pressure_scale_btn->set_icon_name(INKSCAPE_ICON("draw-use-pressure"));
    _pressure_scale_btn->signal_toggled().connect(pressure_scale_btn_toggled_cb);
    _pressure_scale_btn->set_active(prefs->getBool("/tools/spray/usepressurescale", false));

    auto pick_color_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pick_color_btn_toggled);
    _pick_color_btn->set_tooltip_text(_("Pick color from the drawing. You can use clonetiler trace dialog "
                                        "for advanced effects. In clone mode original fill or stroke colors "
                                        "must be unset."));
    _pick_color_btn->set_icon_name(INKSCAPE_ICON("color-picker"));
    _pick_color_btn->signal_toggled().connect(pick_color_btn_toggled_cb);
    _pick_color_btn->set_active(prefs->getBool("/tools/spray/picker", false));

    auto pick_center_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pick_center_btn_toggled);
    _pick_center_btn->set_tooltip_text(_("Pick from center instead average area."));
    _pick_center_btn->set_icon_name(INKSCAPE_ICON("snap-bounding-box-center"));
    _pick_center_btn->signal_toggled().connect(pick_center_btn_toggled_cb);
    _pick_center_btn->set_active(prefs->getBool("/tools/spray/pick_center", true));

    auto pick_inverse_value_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pick_inverse_value_btn_toggled);
    _pick_inverse_value_btn->set_tooltip_text(_("Inverted pick value, retaining color in advanced trace mode"));
    _pick_inverse_value_btn->set_icon_name(INKSCAPE_ICON("object-tweak-shrink"));
    _pick_inverse_value_btn->signal_toggled().connect(pick_inverse_value_btn_toggled_cb);
    _pick_inverse_value_btn->set_active(prefs->getBool("/tools/spray/pick_inverse_value", false));

    auto pick_fill_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pick_fill_btn_toggled);
    _pick_fill_btn->set_tooltip_text(_("Apply picked color to fill"));
    _pick_fill_btn->set_icon_name(INKSCAPE_ICON("paint-solid"));
    _pick_fill_btn->signal_toggled().connect(pick_fill_btn_toggled_cb);
    _pick_fill_btn->set_active(prefs->getBool("/tools/spray/pick_fill", false));

    auto pick_stroke_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pick_stroke_btn_toggled);
    _pick_stroke_btn->set_tooltip_text(_("Apply picked color to stroke"));
    _pick_stroke_btn->signal_toggled().connect(pick_stroke_btn_toggled_cb);
    _pick_stroke_btn->set_icon_name(INKSCAPE_ICON("no-marker"));
    _pick_stroke_btn->set_active(prefs->getBool("/tools/spray/pick_stroke", false));

    auto pick_no_overlap_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_pick_no_overlap_btn_toggled);
    _pick_no_overlap_btn->set_tooltip_text(_("No overlap between colors"));
    _pick_no_overlap_btn->set_icon_name(INKSCAPE_ICON("symbol-bigger"));
    _pick_no_overlap_btn->signal_toggled().connect(pick_no_overlap_btn_toggled_cb);
    _pick_no_overlap_btn->set_active(prefs->getBool("/tools/spray/pick_no_overlap", false));

    auto over_transparent_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_over_transparent_btn_toggled);
    _over_transparent_btn->set_tooltip_text(_("Apply over transparent areas"));
    _over_transparent_btn->set_icon_name(INKSCAPE_ICON("object-hidden"));
    _over_transparent_btn->signal_toggled().connect(over_transparent_btn_toggled_cb);
    _over_transparent_btn->set_active(prefs->getBool("/tools/spray/over_transparent", true));

    auto over_no_transparent_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_over_no_transparent_btn_toggled);
    _over_no_transparent_btn->set_tooltip_text(_("Apply over no transparent areas"));
    _over_no_transparent_btn->set_icon_name(INKSCAPE_ICON("object-visible"));
    _over_no_transparent_btn->set_active(prefs->getBool("/tools/spray/over_no_transparent", true));
    _over_no_transparent_btn->signal_toggled().connect(over_no_transparent_btn_toggled_cb);

    auto no_overlap_btn_toggled_cb = sigc::mem_fun(*this, &SprayToolbar::on_no_overlap_btn_toggled);
    _no_overlap_btn->set_tooltip_text(_("Prevent overlapping objects"));
    _no_overlap_btn->set_icon_name(INKSCAPE_ICON("distribute-randomize"));
    _no_overlap_btn->signal_toggled().connect(no_overlap_btn_toggled_cb);
    _no_overlap_btn->set_active(prefs->getBool("/tools/spray/no_overlap", false));

    auto width_val      = prefs->getDouble("/tools/spray/width",               15);
    auto mean_val       = prefs->getDouble("/tools/spray/mean",                 0);
    auto stddev_val     = prefs->getDouble("/tools/spray/standard_deviation",  70);
    auto population_val = prefs->getDouble("/tools/spray/population",          70);
    auto rotation_val   = prefs->getDouble("/tools/spray/rotation_variation",   0);
    auto scale_val      = prefs->getDouble("/tools/spray/scale_variation",      0);
    auto offset_val     = prefs->getDouble("/tools/spray/offset",             100);

    _width_adj      = Gtk::Adjustment::create(width_val,      1, 100,  1.0, 10.0);
    _mean_adj       = Gtk::Adjustment::create(mean_val,       0, 100,  1.0, 10.0);
    _stddev_adj     = Gtk::Adjustment::create(stddev_val,     1, 100,  1.0, 10.0);
    _population_adj = Gtk::Adjustment::create(population_val, 1, 100,  1.0, 10.0);
    _rotation_adj   = Gtk::Adjustment::create(rotation_val,   0, 100,  1.0, 10.0);
    _scale_adj      = Gtk::Adjustment::create(scale_val,      0, 100,  1.0, 10.0);
    _offset_adj     = Gtk::Adjustment::create(offset_val,     0, 1000, 1.0,  4.0);

    auto width_btn      = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-width",
                                                                                   _("Width:"),
                                                                                   _width_adj,
                                                                                   1, 0));
    width_btn->set_all_tooltip_text(_("The width of the spray area (relative to the visible canvas area)"));
    width_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    auto mean_btn       = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-mean",
                                                                                   _("Focus:"),
                                                                                   _mean_adj,
                                                                                   1, 0));
    mean_btn->set_all_tooltip_text(_("0 to spray a spot; increase to enlarge the ring radius"));
    mean_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    auto stddev_btn     = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-stddev",
                                                                                   C_("Spray tool", "Scatter:"),
                                                                                   _stddev_adj,
                                                                                   1, 0));
    stddev_btn->set_all_tooltip_text(_("Increase to scatter sprayed objects"));
    stddev_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    auto population_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-population",
                                                                                   _("Amount:"),
                                                                                   _population_adj,
                                                                                   1, 0));
    population_btn->set_all_tooltip_text(_("Adjusts the number of items sprayed per click"));
    population_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    _rotation_btn   = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-rotation",
                                                                               _("Rotation:"),
                                                                               _rotation_adj,
                                                                               1, 0));
    // xgettext:no-c-format
    _rotation_btn->set_all_tooltip_text(_("Variation of the rotation of the sprayed objects; 0% for the same rotation than the original object"));
    _rotation_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    _scale_btn      = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-scale",
                                                                                   C_("Spray tool", "Scale:"),
                                                                                   _scale_adj,
                                                                                   1, 0));
    // xgettext:no-c-format
    _scale_btn->set_all_tooltip_text(_("Variation in the scale of the sprayed objects; 0% for the same scale than the original object"));
    _scale_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    _offset_btn     = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("spray-offset",
                                                                               _("Offset %:"),
                                                                               _offset_adj,
                                                                               0, 0));
    _offset_btn->set_all_tooltip_text(_("Increase to segregate objects more (value in percent)"));
    _offset_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    auto width_adj_value_changed_cb      = sigc::mem_fun(*this, &SprayToolbar::on_width_adj_value_changed);
    auto mean_adj_value_changed_cb       = sigc::mem_fun(*this, &SprayToolbar::on_mean_adj_value_changed);
    auto stddev_adj_value_changed_cb     = sigc::mem_fun(*this, &SprayToolbar::on_stddev_adj_value_changed);
    auto population_adj_value_changed_cb = sigc::mem_fun(*this, &SprayToolbar::on_population_adj_value_changed);
    auto rotation_adj_value_changed_cb   = sigc::mem_fun(*this, &SprayToolbar::on_rotation_adj_value_changed);
    auto scale_adj_value_changed_cb      = sigc::mem_fun(*this, &SprayToolbar::on_scale_adj_value_changed);
    auto offset_adj_value_changed_cb     = sigc::mem_fun(*this, &SprayToolbar::on_offset_adj_value_changed);

    _width_adj->signal_value_changed().connect(width_adj_value_changed_cb);
    _mean_adj->signal_value_changed().connect(mean_adj_value_changed_cb);
    _stddev_adj->signal_value_changed().connect(stddev_adj_value_changed_cb);
    _population_adj->signal_value_changed().connect(population_adj_value_changed_cb);
    _rotation_adj->signal_value_changed().connect(rotation_adj_value_changed_cb);
    _scale_adj->signal_value_changed().connect(scale_adj_value_changed_cb);
    _offset_adj->signal_value_changed().connect(offset_adj_value_changed_cb);

    // TODO: Consider adding custom numeric menu items for adjustment widgets
    /*
    // Width
    gchar const* labels[] = {_("(narrow spray)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad spray)")};
    gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
    // Mean
    gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(maximum mean)")};
    gdouble values[] = {0, 5, 10, 20, 30, 50, 70, 100};
    // Standard_deviation
    gchar const* labels[] = {_("(minimum scatter)"), 0, 0, 0, 0, 0, _("(default)"), _("(maximum scatter)")};
    gdouble values[] = {1, 5, 10, 20, 30, 50, 70, 100};
    // Population
    gchar const* labels[] = {_("(low population)"), 0, 0, 0, _("(default)"), 0, _("(high population)")};
    gdouble values[] = {5, 20, 35, 50, 70, 85, 100};
    // Rotation
    gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(high rotation variation)")};
    gdouble values[] = {0, 10, 25, 35, 50, 60, 80, 100};
    // Scale
    gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(high scale variation)")};
    gdouble values[] = {0, 10, 25, 35, 50, 60, 80, 100};
    // Offset
    gchar const* labels[] = {_("(minimum offset)"), 0, 0, 0, _("(default)"), 0, 0, _("(maximum offset)")};
    gdouble values[] = {0, 25, 50, 75, 100, 150, 200, 1000};
    */

    add(*mode_label_ti);

    int btn_index = 0;
    for(auto btn : mode_buttons) {
        add(*btn);
        btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &SprayToolbar::on_mode_button_clicked), btn_index));
        ++btn_index;
    }

    add(* Gtk::manage(new Gtk::SeparatorToolItem()));

    add(*width_btn);
    add(*_pressure_width_btn);
    add(*population_btn);
    add(*_pressure_pop_btn);
    add(*_rotation_btn);
    add(*_scale_btn);
    add(*_pressure_scale_btn);

    add(* Gtk::manage(new Gtk::SeparatorToolItem()));

    add(*stddev_btn);
    add(*mean_btn);

    add(* Gtk::manage(new Gtk::SeparatorToolItem()));

    add(*_over_no_transparent_btn);
    add(*_over_transparent_btn);
    add(*_pick_no_overlap_btn);
    add(*_no_overlap_btn);
    add(*_offset_btn);

    add(* Gtk::manage(new Gtk::SeparatorToolItem()));

    add(*_pick_color_btn);
    add(*_pick_fill_btn);
    add(*_pick_stroke_btn);
    add(*_pick_inverse_value_btn);
    add(*_pick_center_btn);

    show_all();

#ifndef ENABLE_SPRAY_MODE_SINGLE_PATH
    // Disable the spray-single-path button if we don't want it
    mode_buttons[2].set_visible(false);
#endif

    update_widget_visibility();
}

SprayToolbar::~SprayToolbar()
{
    if(_pressure_width_pusher) delete _pressure_width_pusher;
    if(_pressure_pop_pusher)   delete _pressure_pop_pusher;
}

GtkWidget *
SprayToolbar::create(SPDesktop *desktop)
{
    auto toolbar = Gtk::manage(new SprayToolbar(desktop));
    return GTK_WIDGET(toolbar->gobj());
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
