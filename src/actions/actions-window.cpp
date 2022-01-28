// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for window handling tied to the application and with GUI.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-window.h"
#include "actions-helper.h"
#include "inkscape-application.h"
#include "inkscape-window.h"

#include "inkscape.h"             // Inkscape::Application

// Actions for window handling (should be integrated with file dialog).

class InkscapeWindow;

// Open a window for current document
void
window_open(InkscapeApplication *app)
{
    SPDocument *document = app->get_active_document();
    if (document) {
        InkscapeWindow* window = app->get_active_window();
        if (window && window->get_document() && window->get_document()->getVirgin()) {
            // We have a window with an untouched template document, use this window.
            app->document_swap (window, document);
        } else {
            app->window_open(document);
        }
    } else {
        std::cerr << "window_open(): failed to find document!" << std::endl;
    }
}

void
window_close(InkscapeApplication *app)
{
    app->window_close_active();
}

std::vector<std::vector<Glib::ustring>> raw_data_window =
{
    // clang-format off
    {"app.window-open",           N_("Window Open"),     "Window",     N_("Open a window for the active document; GUI only")       },
    {"app.window-close",          N_("Window Close"),    "Window",     N_("Close the active window, does not check for data loss") }
    // clang-format on
};

void
add_actions_window(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action(                "window-open",  sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&window_open),         app));
    gapp->add_action(                "window-close", sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&window_close),        app));
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_window);
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
