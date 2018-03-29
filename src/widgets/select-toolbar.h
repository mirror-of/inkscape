#ifndef SEEN_SELECT_TOOLBAR_H
#define SEEN_SELECT_TOOLBAR_H

/** \file
 * Selector aux toolbar
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

class SPDesktop;

namespace Gtk {
class ToggleToolButton;
}

namespace Inkscape {
class Selection;
class Verb;

namespace UI {
namespace Widget {
class UnitTracker;

class SelectToolbar : public Gtk::Toolbar {
private:
    SPDesktop *_desktop;
    Gtk::ToolButton * create_toolbutton_for_verb(unsigned int  verb_code);

    bool _update_flag;

    Glib::RefPtr<Gtk::Adjustment>  _x_pos_adj;
    Glib::RefPtr<Gtk::Adjustment>  _y_pos_adj;
    Glib::RefPtr<Gtk::Adjustment>  _width_adj;
    Glib::RefPtr<Gtk::Adjustment>  _height_adj;
    Gtk::ToggleToolButton         *_lock_button;
    Gtk::ToggleToolButton         *_transform_stroke_button;
    Gtk::ToggleToolButton         *_transform_corners_button;
    Gtk::ToggleToolButton         *_transform_gradient_button;
    Gtk::ToggleToolButton         *_transform_pattern_button;

    std::vector<Gtk::ToolItem *> _context_items;

    UnitTracker *_tracker;

    enum LastChanged { CHANGED_NONE, CHANGED_X_Y, CHANGED_W, CHANGED_H };

    LastChanged _last_changed;

    void on_inkscape_selection_changed(Inkscape::Selection *selection);
    void on_inkscape_selection_modified(Inkscape::Selection *selection, guint flags);
    void layout_widget_update(Inkscape::Selection *selection);
    void on_x_y_adj_value_changed();
    void on_w_adj_value_changed();
    void on_h_adj_value_changed();
    void on_any_layout_adj_value_changed();
    void on_lock_button_toggled();
    void on_transform_stroke_button_toggled();
    void on_transform_corners_button_toggled();
    void on_transform_gradient_button_toggled();
    void on_transform_pattern_button_toggled();

public:
    SelectToolbar(SPDesktop *desktop);
    ~SelectToolbar();

    static GtkWidget *create(SPDesktop *desktop);
};
}
}
}

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
