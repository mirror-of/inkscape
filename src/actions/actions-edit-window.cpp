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
undo(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Undo
    sp_undo(dt, dt->getDocument());
}

void 
redo(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Redo
    sp_redo(dt, dt->getDocument());
}

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
select_all(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Select All
    Inkscape::SelectionHelper::selectAll(dt);
}

void 
select_all_layers(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Select All in All Layers
    Inkscape::SelectionHelper::selectAllInAll(dt);
}

void 
select_same_fill_and_stroke(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Fill and Stroke
    Inkscape::SelectionHelper::selectSameFillStroke(dt);
}

void 
select_same_fill(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Fill Color
    Inkscape::SelectionHelper::selectSameFillColor(dt);
}

void 
select_same_stroke_color(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Stroke Color
    Inkscape::SelectionHelper::selectSameStrokeColor(dt);
}

void 
select_same_stroke_style(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Stroke Style
    Inkscape::SelectionHelper::selectSameStrokeStyle(dt);
}

void 
select_same_object_type(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Object Type
    Inkscape::SelectionHelper::selectSameObjectType(dt);
}

void 
select_invert(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Invert Selection
    Inkscape::SelectionHelper::invert(dt);
}

void 
select_none(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Deselect
    Inkscape::SelectionHelper::selectNone(dt);
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

/* Node toolbar : deactivate button when no effect is done ( Currently not added this feature ) */
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
    {"win.undo",                                N_("Undo"),                             "Edit",     N_("Undo last action")},
    {"win.redo",                                N_("Redo"),                             "Edit",     N_("Do again the last undone action")},
    {"win.paste",                               N_("Paste"),                            "Edit",     N_("Paste objects from clipboard to mouse point, or paste text")},
    {"win.paste-in-place",                      N_("Paste In Place"),                   "Edit",     N_("Paste objects from clipboard to mouse point, or paste text")},
    {"win.select-all",                          N_("Select All"),                       "Edit",     N_("Select all objects or all nodes")},
    {"win.select-all-layers",                   N_("Select All in All Layers"),         "Edit",     N_("Select all objects in all visible and unlocked layers")},
    {"win.select-same-fill-and-stroke",         N_("Fill and Stroke"),                  "Edit",     N_("Select all objects with the same fill and stroke as the selected objects")},
    {"win.select-same-fill",                    N_("Fill Color"),                       "Edit",     N_("Select all objects with the same fill as the selected objects")},
    {"win.select-same-stroke-color",            N_("Stroke Color"),                     "Edit",     N_("Select all objects with the same stroke as the selected objects")},
    {"win.select-same-stroke-style",            N_("Stroke Style"),                     "Edit",     N_("Select all objects with the same stroke style (width, dash, markers) as the selected objects")},
    {"win.select-same-object-type",             N_("Object Type"),                      "Edit",     N_("Select all objects with the same object type (rect, arc, text, path, bitmap etc) as the selected objects")},
    {"win.select-invert",                       N_("Invert Selection"),                 "Edit",     N_("Invert selection (unselect what is selected and select everything else)")},
    {"win.select-none",                         N_("Deselect"),                         "Edit",     N_("Deselect any selected objects or nodes")},
    {"win.lock-all-guides",                     N_("Lock All Guides"),                  "Edit",     N_("Toggle lock of all guides in the document")},
    {"win.path-effect-parameter-next",          N_("Next path effect parameter"),       "Edit",     N_("Show next editable path effect parameter")}
    // clang-format on
};

void
add_actions_edit_window(InkscapeWindow* win)
{
    // clang-format off
    win->add_action(        "undo",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&undo), win));
    win->add_action(        "redo",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&redo), win));
    win->add_action(        "paste",                           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste), win));
    win->add_action(        "paste-in-place",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_in_place), win));
    win->add_action(        "select-all",                      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_all), win));
    win->add_action(        "select-all-layers",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_all_layers), win));
    win->add_action(        "select-same-fill-and-stroke",     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_fill_and_stroke), win));
    win->add_action(        "select-same-fill",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_fill), win));
    win->add_action(        "select-same-stroke-color",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_stroke_color), win));
    win->add_action(        "select-same-stroke-style",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_stroke_style), win));
    win->add_action(        "select-same-object-type",         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_object_type), win));
    win->add_action(        "select-invert",                   sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_invert), win));
    win->add_action(        "select-none",                     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_none), win));
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