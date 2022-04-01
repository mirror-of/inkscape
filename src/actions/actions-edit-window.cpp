// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 *  Actions for Editing an object which require desktop
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <giomm.h>
#include <glibmm/i18n.h>

#include "actions-edit-window.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "selection-chemistry.h"
#include "object/sp-guide.h"

void
paste(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Paste
    sp_selection_paste(dt, false);
}

void
paste_in_place(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Paste In Place
    sp_selection_paste(dt, true);
}

void
lock_all_guides(InkscapeWindow *win)
{
    // Get Action
    auto action = win->lookup_action("lock-all-guides");
    if (!action) {
        std::cerr << "lock_all_guides: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "lock_all_guides: action not SimpleAction!" << std::endl;
        return;
    }

    // Toggle State
    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    SPDesktop* dt = win->get_desktop();

    // Lock All Guides
    dt->toggleGuidesLock();
}

void
path_effect_parameter_next(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Next path effect parameter
    sp_selection_next_patheffect_param(dt);
}

std::vector<std::vector<Glib::ustring>> raw_data_edit_window =
{
    // clang-format off
    {"win.paste",                               N_("Paste"),                            "Edit",     N_("Paste objects from clipboard to mouse point, or paste text")},
    {"win.paste-in-place",                      N_("Paste In Place"),                   "Edit",     N_("Paste objects from clipboard to the original position of the copied objects")},
    {"win.lock-all-guides",                     N_("Lock All Guides"),                  "Edit",     N_("Toggle lock of all guides in the document")},
    {"win.path-effect-parameter-next",          N_("Next path effect parameter"),       "Edit",     N_("Show next editable path effect parameter")}
    // clang-format on
};

void
add_actions_edit_window(InkscapeWindow* win)
{
    // clang-format off
    win->add_action(        "paste",                           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste), win));
    win->add_action(        "paste-in-place",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_in_place), win));
    win->add_action_bool(   "lock-all-guides",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&lock_all_guides),   win));
    win->add_action(        "path-effect-parameter-next",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&path_effect_parameter_next), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit_window: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_edit_window);
}
