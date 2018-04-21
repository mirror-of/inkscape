#ifndef SEEN_TWEAK_TOOLBAR_H
#define SEEN_TWEAK_TOOLBAR_H

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

namespace Toolbar {
class TweakToolbar : public Gtk::Toolbar {
private:
    SPDesktop *_desktop;

    // Widgets
    Glib::RefPtr<Gtk::Adjustment>  _width_adj;
    Glib::RefPtr<Gtk::Adjustment>  _force_adj;
    Glib::RefPtr<Gtk::Adjustment>  _fidelity_adj;

    Gtk::ToggleToolButton         *_pressure_btn;

    Inkscape::UI::Widget::SpinButtonToolItem *_fidelity_btn;

    // Signal handlers
    void on_width_adj_value_changed();
    void on_force_adj_value_changed();
    void on_fidelity_adj_value_changed();
    void on_pressure_btn_toggled();
    void on_mode_button_clicked(int mode);

    // Widget-creation helpers
    Gtk::RadioToolButton * create_radio_tool_button(Gtk::RadioButtonGroup &group,
                                                    const Glib::ustring   &label,
                                                    const Glib::ustring   &tooltip_text,
                                                    const Glib::ustring   &icon_name);

public:
    TweakToolbar(SPDesktop *desktop);

    static GtkWidget * create(SPDesktop *desktop);
};
} // namespace Toolbar
} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_SELECT_TOOLBAR_H */

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
