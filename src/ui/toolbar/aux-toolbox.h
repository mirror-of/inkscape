// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_AUX_TOOLBOX_H
#define SEEN_AUX_TOOLBOX_H

/**
 * @file
 * Inkscape auxilliary toolbar
 *
 * @authors Inkscape Authors
 * Copyright (C) 1999-2020 authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm/eventbox.h>

class SPDesktop;

namespace Gtk {
    class Box;
    class Grid;
}

namespace Inkscape {
namespace UI {
namespace Tools {
    class ToolBase;
}

namespace Toolbar {

/**
 * A toolbox that can hold tool-specific widgets and swatches.
 * This is the bar that normally appears directly below the main menu.
 *
 * The contents depends on the currently-selected tool.
 */
class AuxToolbox : public Gtk::EventBox {
private:
    SPDesktop *_desktop = nullptr;
    sigc::connection _event_context_connection;

    /**
     * This box stores each possible toolbar that could be displayed within
     * a different cell.  Only one of them should be shown at a time.
     */
    Gtk::Box *_box = nullptr;

    /// A map of all the toolbars that can be shown here
    std::map<Glib::ustring, Gtk::Grid *> _toolbar_map;

    /// The toolbar that is currently being shown
    Gtk::Grid *_shows = nullptr;

    void update(SPDesktop *desktop, Tools::ToolBase *eventcontext);
    void setup(SPDesktop *desktop);

public:
    AuxToolbox();

    void show_aux_toolbox();
    void set_desktop(decltype(_desktop) desktop);
};

} // namespace Toolbar
} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_AUX_TOOLBOX_H */

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
