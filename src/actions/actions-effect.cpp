// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 *  Actions for Filters and Extension menu items
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

#include "actions-effect.h"
#include "actions-helper.h"
#include "inkscape-application.h"

#include "extension/effect.h"

void
edit_remove_filter(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    
    // Remove Filter
    selection->removeFilter();    
}

void
last_effect(InkscapeApplication *app)
{
    Inkscape::Extension::Effect *effect = Inkscape::Extension::Effect::get_last_effect();

    if (effect == nullptr) {
        return;
    }
    
    // Last Effect
    effect->effect(InkscapeApplication::instance()->get_active_view());
}

void
last_effect_pref(InkscapeApplication *app)
{
    Inkscape::Extension::Effect *effect = Inkscape::Extension::Effect::get_last_effect();

    if (effect == nullptr) {
        return;
    } 
    
    // Last Effect Pref
    effect->prefs(InkscapeApplication::instance()->get_active_view());
}

std::vector<std::vector<Glib::ustring>> raw_data_effect =
{
    // clang-format off
    {"app.edit-remove-filter",      N_("Remove Filters"),                   "Filter",           N_("Remove any filters from selected objects")},
    {"app.last-effect",             N_("Previous Extension"),               "Extenssion",       N_("Repeat the last extension with the same settings")},
    {"app.last-effect-pref",        N_("Previous Extension Settings"),      "Extenssion",       N_("Repeat the last extension with new settings")}
    // clang-format on
};

void
add_actions_effect(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();
    
    // Debian 9 has 2.50.0
#if GLIB_CHECK_VERSION(2, 52, 0)
    // clang-format off
    gapp->add_action( "edit-remove-filter",     sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&edit_remove_filter), app));
    gapp->add_action( "last-effect",            sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&last_effect), app));
    gapp->add_action( "last-effect-pref",       sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&last_effect_pref), app));
    // clang-format on
#else
    std::cerr << "add_actions: Some actions require Glibmm 2.52, compiled with: " << glib_major_version << "." << glib_minor_version << std::endl;
#endif

    if (!app) {
        std::cerr << "add_actions_edit: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_effect);
}