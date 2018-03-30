#ifndef SEEN_NODE_TOOLBAR_H
#define SEEN_NODE_TOOLBAR_H

/**
 * @file
 * Node aux toolbar
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

namespace Inkscape {
namespace UI {
namespace Widget {
class UnitTracker;
}

namespace Toolbar {

class NodeToolbar : public Gtk::Toolbar {
private:
    SPDesktop *_desktop;
    Inkscape::UI::Widget::UnitTracker *_tracker;

    // Signal handlers
    void on_insert_node_button_clicked();
    void on_insert_node_min_x_activated();
    void on_insert_node_max_x_activated();
    void on_insert_node_min_y_activated();
    void on_insert_node_max_y_activated();

    void create_insert_node_button();

public:
    NodeToolbar(SPDesktop *desktop);
    ~NodeToolbar();
    static GtkWidget * create(SPDesktop *desktop);
};
}
}
}

#endif /* !SEEN_NODE_TOOLBAR_H */
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
