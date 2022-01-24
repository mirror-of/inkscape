// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Utility functions for UI
 *
 * Authors:
 *   Tavmjong Bah
 *   John Smith
 *
 * Copyright (C) 2013, 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef UI_UTIL_SEEN
#define UI_UTIL_SEEN

#include <cstddef> // size_t

#include <gdkmm/rgba.h>
#include <gtkmm/stylecontext.h>

namespace Glib {
class ustring;
}

namespace Gtk {
class Revealer;
class Widget;
}

Glib::ustring ink_ellipsize_text (Glib::ustring const &src, size_t maxlen);
void reveal_widget(Gtk::Widget *widget, bool show);

// check if widget in a container is actually visible
bool is_widget_effectively_visible(Gtk::Widget* widget);

namespace Inkscape {
namespace UI {
// Utility function to ensure correct sizing after adding child widgets
void resize_widget_children(Gtk::Widget *widget);
}
}

// Get the background-color style property for a given StyleContext
Gdk::RGBA get_background_color(const Glib::RefPtr<Gtk::StyleContext> &context,
                               Gtk::StateFlags                  state = static_cast<Gtk::StateFlags>(0));

#endif

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
