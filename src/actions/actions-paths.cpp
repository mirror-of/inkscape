// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for Path Actions
 *
 * Copyright (C) 2021 Sushant A.A.
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-paths.h"
#include "actions-helper.h"
#include "inkscape-application.h"
#include "inkscape.h"             // Inkscape::Application
#include "selection.h"            // Selection
#include "object/sp-root.h"       // select_all: document->getRoot();
#include "object/sp-item-group.h" // select_all

void
object_path_union(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeLPESRecursive(true);
    selection->unlinkRecursive(true);
    selection->pathUnion();
}

void
select_path_difference(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeLPESRecursive(true);
    selection->unlinkRecursive(true);
    selection->pathDiff();
}

void
select_path_intersection(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeLPESRecursive(true);
    selection->unlinkRecursive(true);
    selection->pathIntersect();
}

void
select_path_exclusion(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeLPESRecursive(true);
    selection->unlinkRecursive(true);
    selection->pathSymDiff();
}

void
select_path_division(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeLPESRecursive(true);
    selection->unlinkRecursive(true);
    selection->pathCut();
}

void
select_path_cut(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeLPESRecursive(true);
    selection->unlinkRecursive(true);
    selection->pathSlice();
}

void
select_path_combine(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->unlinkRecursive(true);
    selection->combine();
}

void
select_path_break_apart(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->breakApart();
}

void
fill_between_paths(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->fillBetweenMany();
}

void
select_path_simplify(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->simplifyPaths();
}

std::vector<std::vector<Glib::ustring>> raw_data_path =
{
    // clang-format offs
    {"app.select-path-union",               N_("Union"),                "Select",   N_("Create union of selected paths")},
    {"app.select-path-difference",          N_("Difference"),           "Select",   N_("Create difference of selected paths (bottom minus top)")},
    {"app.select-path-intersection",        N_("Intersection"),         "Select",   N_("Create intersection of selected paths")},
    {"app.select-path-exclusion",           N_("Exclusion"),            "Select",   N_("Create exclusive OR of selected paths (those parts that belong to only one path)")},
    {"app.select-path-division",            N_("Division"),             "Select",   N_("Cut the bottom path into pieces")},
    {"app.select-path-cut",                 N_("Cut Path"),             "Select",   N_("Cut the bottom path's stroke into pieces, removing fill")},
    {"app.select-path-combine",             N_("Combine"),              "Select",   N_("Combine several paths into one")},
    {"app.select-path-break-apart",         N_("Break Apart"),          "Select",   N_("Break selected paths into subpaths")},
    {"app.select-fill-between-paths",       N_("Fill between paths"),   "Select",   N_("Create a fill object using the selected paths")},
    {"app.select-path-simplify",            N_("Simplify"),             "Select",   N_("Simplify selected paths (remove extra nodes)")},
    // clang-format on
};

void
add_actions_path(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action(               "select-path-union",            sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_path_union),         app));
    gapp->add_action(               "select-path-difference",       sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_difference),    app));
    gapp->add_action(               "select-path-intersection",     sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_intersection),  app));
    gapp->add_action(               "select-path-exclusion",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_exclusion),     app));
    gapp->add_action(               "select-path-division",         sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_division),      app));
    gapp->add_action(               "select-path-cut",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_cut),           app));
    gapp->add_action(               "select-path-combine",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_combine),       app));
    gapp->add_action(               "select-path-break-apart",      sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_break_apart),   app));
    gapp->add_action(               "select-fill-between-paths",    sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&fill_between_paths),        app));
    gapp->add_action(               "select-path-simplify",         sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_path_simplify),      app));
    // clangt on

    app->get_action_extra_data().add_data(raw_data_path);
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