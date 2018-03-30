#ifndef SEEN_CONNECTOR_TOOLBAR_H
#define SEEN_CONNECTOR_TOOLBAR_H

/**
 * @file
 * Connector aux toolbar
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/toolbar.h>

class SPDesktop;

typedef struct _GObject GObject;

void sp_connector_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);

namespace Gtk {
class ToggleToolButton;
}

namespace Inkscape {

class Selection;

namespace XML {
class Node;
}

namespace UI {
namespace Toolbar {
class ConnectorToolbar : public Gtk::Toolbar {
private:
    // Event handlers
    void on_avoid_activated();
    void on_ignore_activated();
    void on_orthogonal_activated();
    void on_curvature_adj_value_changed();
    void on_spacing_adj_value_changed();
    void on_graph_activated();
    void on_length_adj_value_changed();
    void on_direction_activated();
    void on_overlap_activated();
    void selection_changed(Inkscape::Selection *selection,
                           GObject             *data);

    SPDesktop *_desktop;
    Inkscape::XML::Node *_repr;

    // Widgets contained in toolbar
    Gtk::ToggleToolButton         *_orthogonal_button;
    Glib::RefPtr<Gtk::Adjustment>  _curvature_adj;
    Glib::RefPtr<Gtk::Adjustment>  _spacing_adj;
    Glib::RefPtr<Gtk::Adjustment>  _length_adj;
    Gtk::ToggleToolButton         *_direction_button;
    Gtk::ToggleToolButton         *_overlap_button;

    bool _freeze_flag;

public:
    ConnectorToolbar(SPDesktop *desktop);

    static GtkWidget * create(SPDesktop *desktop);

    static void event_attr_changed(Inkscape::XML::Node *repr,
                                   gchar const         *name,
                                   gchar const         *old_value,
                                   gchar const         *new_value,
                                   bool                 is_interactive,
                                   gpointer             data);
};
}
}
}

#endif /* !SEEN_CONNECTOR_TOOLBAR_H */

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
