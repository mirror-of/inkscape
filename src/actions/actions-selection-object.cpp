// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions related to selection of objects which don't require desktop.
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

/*
 * Note: Actions must be app level as different windows can have different selections
 *       and selections must also work from the command line (without GUI).
 */

#include <giomm.h>
#include <glibmm/i18n.h>

#include "actions-selection-object.h"
#include "actions-helper.h"
#include "inkscape-application.h"
#include "inkscape.h"
#include "selection.h"

void
select_object_group(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Group
    selection->group();
}

void
select_object_ungroup(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Ungroup
    selection->ungroup();
}

void
select_object_ungroup_pop(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Pop Selected Objects out of Group
    selection->popFromGroup();
}

void
selection_top(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Raise to Top
    selection->raiseToTop();
}

void
selection_raise(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Raise
    selection->raise();
}

void
selection_lower(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Lower
    selection->lower();
}

void
selection_bottom(InkscapeApplication* app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Lower to Bottom
    selection->lowerToBottom();
}

std::vector<std::vector<Glib::ustring>> raw_data_selection_object =
{
    // clang-format off
    { "app.selection-group",                N_("Group"),                                 "Select",   N_("Group selected objects")},
    { "app.selection-ungroup",              N_("Ungroup"),                               "Select",   N_("Ungroup selected objects")},
    { "app.selection-ungroup-pop",          N_("Pop Selected Objects out of Group"),     "Select",   N_("Pop selected objects out of group")},
    { "app.selection-top",                  N_("Raise to Top"),                          "Select",   N_("Raise selection to top")},
    { "app.selection-raise",                N_("Raise"),                                 "Select",   N_("Raise selection one step")},
    { "app.selection-lower",                N_("Lower"),                                 "Select",   N_("Lower selection one step")},
    { "app.selection-bottom",               N_("Lower to Bottom"),                       "Select",   N_("Lower selection to bottom")}
    // clang-format on
};

void
add_actions_selection_object(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action( "selection-group",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_object_group),           app));
    gapp->add_action( "selection-ungroup",            sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_object_ungroup),         app));
    gapp->add_action( "selection-ungroup-pop",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&select_object_ungroup_pop),     app));
    gapp->add_action( "selection-top",                sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_top),                 app));
    gapp->add_action( "selection-raise",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_raise),               app));
    gapp->add_action( "selection-lower",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_lower),               app));
    gapp->add_action( "selection-bottom",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&selection_bottom),              app));
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_selection_object);
}
