#ifndef SEEN_ARC_TOOLBAR_H
#define SEEN_ARC_TOOLBAR_H

/**
 * @file
 * 3d box aux toolbar
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

#include <gtkmm/adjustment.h>
#include <gtkmm/toolbar.h>

class SPDesktop;
class SPItem;

namespace Gtk {
class Label;
class RadioButtonGroup;
class RadioToolButton;
class ToolButton;
}

namespace Inkscape {
class Selection;

namespace XML {
class Node;
}

namespace UI {

namespace Tools {
class ToolBase;
}

namespace Widget {
class SpinButtonToolItem;
class UnitTracker;
}

namespace Toolbar {
class ArcToolbar : public Gtk::Toolbar {
private:
    SPDesktop *_desktop;

    // State flags
    bool _freeze; ///< Don't process action if true
    bool _single; ///< A single, whole ellipse is selected if true

    Inkscape::UI::Widget::UnitTracker *_tracker;

    // Widgets
    Gtk::Label *_state_label;

    Inkscape::UI::Widget::SpinButtonToolItem *_radius_x_btn;
    Inkscape::UI::Widget::SpinButtonToolItem *_radius_y_btn;

    Glib::RefPtr<Gtk::Adjustment> _radius_x_adj;
    Glib::RefPtr<Gtk::Adjustment> _radius_y_adj;
    Glib::RefPtr<Gtk::Adjustment> _start_adj;
    Glib::RefPtr<Gtk::Adjustment> _end_adj;

    Gtk::ToolButton *_make_whole_btn;

    SPItem *_item;
    Inkscape::XML::Node *_repr;
    std::vector<Gtk::RadioToolButton *> _type_buttons;

    // Event handlers
    void value_changed(Glib::RefPtr<Gtk::Adjustment>  adj,
                       const Glib::ustring           &value_name);
    void startend_value_changed(Glib::RefPtr<Gtk::Adjustment>  adj,
                                const Glib::ustring           &value_name,
                                Glib::RefPtr<Gtk::Adjustment>  other_adj);
    void type_changed( int type );
    void defaults();

    Gtk::RadioToolButton * create_radio_tool_button(Gtk::RadioButtonGroup &group,
                                                    const Glib::ustring   &label,
                                                    const Glib::ustring   &tooltip_text,
                                                    const Glib::ustring   &icon_name);
    void sensitivize( double v1, double v2 );
    void selection_changed(Inkscape::Selection *selection);
    void check_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec);

    void purge_repr_listener();

public:
    ArcToolbar(SPDesktop *desktop);
    ~ArcToolbar();

    static GtkWidget * create(SPDesktop *desktop);

    static void event_attr_changed(Inkscape::XML::Node *repr,
                                   gchar const *name,
                                   gchar const *old_value,
                                   gchar const *new_value,
                                   bool is_interactive,
                                   gpointer data);
};
}
}
}
#endif /* !SEEN_ARC_TOOLBAR_H */

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
