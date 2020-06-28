// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_TOOLBOX_H
#define SEEN_TOOLBOX_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glibmm/ustring.h>

#include <gtkmm/eventbox.h>
#include <gtkmm/enums.h>

#include "preferences.h"

#define TOOLBAR_SLIDER_HINT "compact"

class SPDesktop;

namespace Gtk {
class Bin;
class Box;
class EventBox;
class Grid;
}

namespace Inkscape {
namespace UI {
namespace Tools {

class ToolBase;

}
}
}

namespace Inkscape {
namespace UI {

namespace Widget {
    class UnitTracker;
}

/**
 * Main toolbox source.
 */
class ToolboxFactory
{
public:
    static void setToolboxDesktop(Gtk::EventBox *toolbox, SPDesktop *desktop);
    static void setOrientation(Gtk::EventBox *toolbox, Gtk::Orientation orientation);

    static Gtk::EventBox * createToolToolbox();
    static Gtk::EventBox * createCommandsToolbox();
    static Gtk::EventBox * createSnapToolbox();

    static void updateSnapToolbox(SPDesktop *desktop, Inkscape::UI::Tools::ToolBase *eventcontext, Gtk::Bin *toolbox);

    static GtkIconSize prefToSize(Glib::ustring const &path, int base = 0 );
    static Gtk::IconSize prefToSize_mm(Glib::ustring const &path, int base = 0);

    ToolboxFactory() = delete;
};

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


} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_TOOLBOX_H */

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
