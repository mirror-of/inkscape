// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Gio::Actions for window handling that are not useful from the command line (thus tied to window map).
 * Found under the "View" menu.
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "actions-view-window.h"

#include <giomm.h>
#include <glibmm/i18n.h>

#include "inkscape-application.h"
#include "inkscape-window.h"
#include "inkscape.h"      // previous/next window
#include "actions/actions-extra-data.h"
#include "ui/interface.h"  // sp_ui_new_view()

void
window_previous(InkscapeWindow* win)
{
    INKSCAPE.switch_desktops_prev();
}

void
window_next(InkscapeWindow* win)
{
    INKSCAPE.switch_desktops_next();
}

void
window_new(InkscapeWindow* win)
{
    sp_ui_new_view();
}

std::vector<std::vector<Glib::ustring>> raw_data_view_window =
{
    // clang-format off
    {"win.window-duplicate",            N_("Duplicate Window"),         "View",             N_("Open a new window with the same document")},
    {"win.window-previous",             N_("Previous Window"),          "View",             N_("Switch to the previous document window")},
    {"win.window-next",                 N_("Next Window"),              "View",             N_("Switch to the next document window")},
    // clang-format on
};

void
add_actions_view_window(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "window-new",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&window_new),       win));
    win->add_action( "window-previous",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&window_previous),  win));
    win->add_action( "window-next",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&window_next),      win));
    // clang-format on

    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_view_window: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_view_window);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
