#ifndef SEEN_SPRAY_TOOLBAR_H
#define SEEN_SPRAY_TOOLBAR_H

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
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2015 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/toolbar.h>

class SPDesktop;

namespace Gtk {
class RadioButtonGroup;
class RadioToolButton;
class ToggleToolButton;
}

namespace Inkscape {
namespace UI {
namespace Widget {
class SpinButtonToolItem;
}

class PrefPusher;

namespace Toolbar {
class SprayToolbar : public Gtk::Toolbar {
private:
    SPDesktop *_desktop;

    // Widgets
    Glib::RefPtr<Gtk::Adjustment> _width_adj;
    Glib::RefPtr<Gtk::Adjustment> _mean_adj;
    Glib::RefPtr<Gtk::Adjustment> _stddev_adj;
    Glib::RefPtr<Gtk::Adjustment> _population_adj;
    Glib::RefPtr<Gtk::Adjustment> _rotation_adj;
    Glib::RefPtr<Gtk::Adjustment> _scale_adj;
    Glib::RefPtr<Gtk::Adjustment> _offset_adj;

    Inkscape::UI::Widget::SpinButtonToolItem *_rotation_btn;
    Inkscape::UI::Widget::SpinButtonToolItem *_scale_btn;
    Inkscape::UI::Widget::SpinButtonToolItem *_offset_btn;

    Gtk::ToggleToolButton *_pressure_width_btn;
    Gtk::ToggleToolButton *_pressure_pop_btn;
    Gtk::ToggleToolButton *_pressure_scale_btn;
    Gtk::ToggleToolButton *_pick_color_btn;
    Gtk::ToggleToolButton *_pick_center_btn;
    Gtk::ToggleToolButton *_pick_inverse_value_btn;
    Gtk::ToggleToolButton *_pick_fill_btn;
    Gtk::ToggleToolButton *_pick_stroke_btn;
    Gtk::ToggleToolButton *_pick_no_overlap_btn;
    Gtk::ToggleToolButton *_over_transparent_btn;
    Gtk::ToggleToolButton *_over_no_transparent_btn;
    Gtk::ToggleToolButton *_no_overlap_btn;

    // Preference pushers
    PrefPusher *_pressure_width_pusher;
    PrefPusher *_pressure_pop_pusher;

    // Signal handlers
    void on_mode_button_clicked(int mode);
    void on_width_adj_value_changed();
    void on_mean_adj_value_changed();
    void on_stddev_adj_value_changed();
    void on_population_adj_value_changed();
    void on_rotation_adj_value_changed();
    void on_scale_adj_value_changed();
    void on_offset_adj_value_changed();
    void on_pressure_scale_btn_toggled();
    void on_no_overlap_btn_toggled();
    void on_pick_color_btn_toggled();
    void on_over_no_transparent_btn_toggled();
    void on_pick_fill_btn_toggled();
    void on_pick_stroke_btn_toggled();
    void on_over_transparent_btn_toggled();
    void on_pick_center_btn_toggled();
    void on_pick_inverse_value_btn_toggled();
    void on_pick_no_overlap_btn_toggled();

    // Widget-creation helpers
    Gtk::RadioToolButton * create_radio_tool_button(Gtk::RadioButtonGroup &group,
                                                    const Glib::ustring   &label,
                                                    const Glib::ustring   &tooltip_text,
                                                    const Glib::ustring   &icon_name);

    void update_widget_visibility();
    void update_widgets();

public:
    SprayToolbar(SPDesktop *desktop);
    ~SprayToolbar();

    static GtkWidget *create(SPDesktop *desktop);
};
}
}
}

#endif /* !SEEN_SELECT_TOOLBAR_H */
