// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions related to selection wich require desktop
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

#include "actions-selection-desktop.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "ui/dialog/dialog-container.h"
#include "path/path-offset.h"
#include "actions/actions-tools.h"
#include "selection-chemistry.h"

void 
selection_make_bitmap_copy(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Make a Bitmap Copy
    dt->selection->createBitmapCopy();
}

void
select_path_inset(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Inset selected paths
    dt->selection->removeLPESRecursive(true);
    dt->selection->unlinkRecursive(true);
    sp_selected_path_inset(dt);
}

void
select_path_outset(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Outset selected paths
    dt->selection->removeLPESRecursive(true);
    dt->selection->unlinkRecursive(true);
    sp_selected_path_offset(dt);
}

void
select_path_offset_dynamic(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Dynamic Offset
    dt->selection->removeLPESRecursive(true);
    dt->selection->unlinkRecursive(true);
    sp_selected_path_create_offset_object_zero(dt);
    set_active_tool(dt,"Node");
}

void
select_path_offset_linked(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Linked Offset
    dt->selection->removeLPESRecursive(true);
    dt->selection->unlinkRecursive(true);
    sp_selected_path_create_updating_offset_object_zero(dt);
    set_active_tool(dt, "Node");
}

void
select_path_reverse(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Reverse
    Inkscape::SelectionHelper::reverse(dt);
}

std::vector<std::vector<Glib::ustring>> raw_selection_dekstop_data =
{
    // clang-format off
    {"win.selection-make-bitmap-copy",  N_("Make a Bitmap Copy"),  "Selection Desktop",        N_("Export selection to a bitmap and insert it into document")},
    {"win.select-path-inset",           N_("Inset"),               "Selection Desktop",        N_("Inset selected paths")},
    {"win.select-path-outset",          N_("Outset"),              "Selection Desktop",        N_("Outset selected paths")},
    {"win.select-path-offset-dynamic",  N_("Dynamic Offset"),      "Selection Desktop",        N_("Create a dynamic offset object")},
    {"win.select-path-offset-linked",   N_("Linked Offset"),       "Selection Desktop",        N_("Create a dynamic offset object linked to the original path")},
    {"win.select-path-reverse",         N_("Reverse"),             "Selection Desktop",        N_("Reverse the direction of selected paths (useful for flipping markers)")}
    // clang-format on
};

void
add_actions_select_desktop(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "selection-make-bitmap-copy",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&selection_make_bitmap_copy), win));
    win->add_action( "select-path-inset",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_path_inset),win));
    win->add_action( "select-path-outset",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_path_outset),win));
    win->add_action( "select-path-offset-dynamic",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_path_offset_dynamic),win));
    win->add_action( "select-path-offset-linked",       sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_path_offset_linked),win));
    win->add_action( "select-path-reverse",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_path_reverse),win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_selection_dekstop_data);
}