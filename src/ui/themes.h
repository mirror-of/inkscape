// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Gtk <themes> helper code.
 */
/*
 * Authors:
 *   Jabiertxof
 *   Martin Owens
 *
 * Copyright (C) 2017-2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef UI_THEMES_H_SEEN
#define UI_THEMES_H_SEEN

#include <cstring>
#include <glibmm.h>
#include <gtkmm.h>
#include <map>
#include <utility>
#include <vector>
#include <sigc++/signal.h>
#include "preferences.h"

namespace Inkscape {
namespace UI {

/**
 * A simple mediator class that sets the state of a Gtk::ToggleToolButton when
 * a preference is changed.  Unlike the PrefPusher class, this does not provide
 * the reverse process, so you still need to write your own handler for the
 * "toggled" signal on the ToggleToolButton.
 */
typedef std::map<Glib::ustring, bool> gtkThemeList;
class ThemeContext
{
public:
ThemeContext();
~ThemeContext() = default;
// Name of theme -> has dark theme
typedef std::map<Glib::ustring, bool> gtkThemeList;
void inkscape_fill_gtk(const gchar *path, gtkThemeList &themes);
std::map<Glib::ustring, bool> get_available_themes();
void add_gtk_css(bool only_providers, bool cached = false);
void add_icon_theme();
Glib::ustring get_symbolic_colors();
Glib::RefPtr<Gtk::CssProvider> getColorizeProvider() { return _colorizeprovider;}
Glib::RefPtr<Gtk::CssProvider> getContrastThemeProvider() { return _contrastthemeprovider;}
Glib::RefPtr<Gtk::CssProvider> getThemeProvider() { return _themeprovider;}
Glib::RefPtr<Gtk::CssProvider> getStyleProvider() { return _styleprovider;}
sigc::signal<void> getChangeThemeSignal() { return _signal_change_theme;}

// True if current theme (applied one) is dark
bool isCurrentThemeDark(Gtk::Container *window);

private:
    // user change theme
    sigc::signal<void> _signal_change_theme;
    Glib::RefPtr<Gtk::CssProvider> _styleprovider;
    Glib::RefPtr<Gtk::CssProvider> _themeprovider;
    Glib::RefPtr<Gtk::CssProvider> _contrastthemeprovider;
    Glib::RefPtr<Gtk::CssProvider> _colorizeprovider;
    Glib::RefPtr<Gtk::CssProvider> _spinbuttonprovider;
    std::unique_ptr<Preferences::Observer> _spinbutton_observer;
};

}
}
#endif /* !UI_THEMES_H_SEEN */

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
