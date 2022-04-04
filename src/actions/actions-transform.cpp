// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for selection tied to the application and without GUI.
 *
 * Copyright (C) 2018 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-transform.h"
#include "actions-helper.h"
#include "document-undo.h"
#include "inkscape-application.h"

#include "inkscape.h"             // Inkscape::Application
#include "selection.h"            // Selection

void
transform_translate(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring> >(value);

    std::vector<Glib::ustring> tokens = Glib::Regex::split_simple(",", s.get());
    if (tokens.size() != 2) {
        std::cerr << "action:transform_translate: requires two comma separated numbers" << std::endl;
        return;
    }
    double dx = 0;
    double dy = 0;

    try {
        dx = std::stod(tokens[0]);
        dy = std::stod(tokens[1]);
    } catch (...) {
        std::cerr << "action:transform-move: invalid arguments" << std::endl;
        return;
    }

    auto selection = app->get_active_selection();
    selection->move(dx, dy);

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), "ActionTransformTranslate", "");
}

void
transform_rotate(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    auto selection = app->get_active_selection();

    selection->rotate(d.get());

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), "ActionTransformRotate", "");
}

void
transform_scale(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    auto selection = app->get_active_selection();
    selection->scale(d.get());

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), "ActionTransformScale", "");
}

void
transform_grow(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    auto selection = app->get_active_selection();
    selection->scaleGrow(d.get());
}

void
transform_grow_step(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    auto selection = app->get_active_selection();
    selection->scaleGrow(d.get() * prefs->getDoubleLimited("/options/defaultscale/value", 2, 0, 1000));
}

void
transform_grow_screen(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    auto selection = app->get_active_selection();
    selection->scaleGrow(d.get());
}

void
transform_remove(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeTransform();

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), "ActionTransformRemoveTransform", "");
}

// SHOULD REALLY BE DOC LEVEL ACTIONS
std::vector<std::vector<Glib::ustring>> raw_data_transform =
{
    // clang-format off
    {"app.transform-translate",   N_("Translate"),          "Transform",  N_("Translate selected objects (dx,dy)")},
    {"app.transform-rotate",      N_("Rotate"),             "Transform",  N_("Rotate selected objects by degrees")},
    {"app.transform-scale",       N_("Scale"),              "Transform",  N_("Scale selected objects by scale factor")},
    {"app.transform-grow",        N_("Grow/Shrink"),        "Transform",  N_("Grow/shrink selected objects")},
    {"app.transform-grow-step",   N_("Grow/Shrink Step"),   "Transform",  N_("Grow/shrink selected objects by multiple of step value")},
    {"app.transform-grow-screen", N_("Grow/Shrink Screen"), "Transform",  N_("Grow/shrink selected objects relative to zoom level")},
    {"app.transform-remove",      N_("Remove Transforms"),  "Transform",  N_("Remove any transforms from selected objects")},
    // clang-format on
};

std::vector<std::vector<Glib::ustring>> hint_data_transform =
{
    // clang-format off
    {"app.transform-translate",     N_("Enter two comma-separated numbers, e.g. 50,-2.5")},
    {"app.transform-rotate",        N_("Enter angle (in degrees) for clockwise rotation")},
    {"app.transform-scale",         N_("Enter scaling factor, e.g. 1.5")},
    {"app.transform-grow",          N_("Enter positive or negative number to grow/shrink selection")},
    {"app.transform-grow-step",     N_("Enter positive or negative number to grow or shrink selection relative to preference step value")},
    {"app.transform-grow-screen",   N_("Enter positive or negative number to grow or shrink selection relative to zoom level")},
    // clang-format on
};

void
add_actions_transform(InkscapeApplication* app)
{
    Glib::VariantType Bool(  Glib::VARIANT_TYPE_BOOL);
    Glib::VariantType Int(   Glib::VARIANT_TYPE_INT32);
    Glib::VariantType Double(Glib::VARIANT_TYPE_DOUBLE);
    Glib::VariantType String(Glib::VARIANT_TYPE_STRING);

    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action_with_parameter( "transform-translate",      String, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_translate),       app));
    gapp->add_action_with_parameter( "transform-rotate",         Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_rotate),          app));
    gapp->add_action_with_parameter( "transform-scale",          Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_scale),           app));
    gapp->add_action_with_parameter( "transform-grow",           Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_grow),            app));
    gapp->add_action_with_parameter( "transform-grow-step",      Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_grow_step),       app));
    gapp->add_action_with_parameter( "transform-grow-screen",    Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_grow_screen),     app));
    gapp->add_action(                "transform-remove",                 sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_remove),          app));
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_transform);
    app->get_action_hint_data().add_data(hint_data_transform);
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
