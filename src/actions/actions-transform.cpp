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
    if (value.get_type_string() != "(dd)") {
        std::cerr << "transform_translate:  wrong variant type: " << value.get_type_string() << " (should be '(dd)')" << std::endl;
    }

    auto tuple = Glib::VariantBase::cast_dynamic<Glib::Variant<std::tuple<double, double>>>(value);

    auto [dx, dy] = tuple.get();

    auto selection = app->get_active_selection();
    selection->move(dx, dy);

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), 0, "ActionTransformTranslate");
}

void
transform_rotate(const Glib::VariantBase& value, InkscapeApplication *app)
{

    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    auto selection = app->get_active_selection();

    selection->rotate(d.get());

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), 0, "ActionTransformRotate");
}

void
transform_scale(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<double> d = Glib::VariantBase::cast_dynamic<Glib::Variant<double> >(value);
    auto selection = app->get_active_selection();
    selection->scale(d.get());

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), 0, "ActionTransformScale");
}

void
transform_remove(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();
    selection->removeTransform();

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), 0, "ActionTransformRemoveTransform");
}

// SHOULD REALLY BE DOC LEVEL ACTIONS
std::vector<std::vector<Glib::ustring>> raw_data_transform =
{
    // clang-format off
    {"app.transform-translate",   N_("Translate"),          "Transform",  N_("Translate selected objects (dx,dy).")                 },
    {"app.transform-rotate",      N_("Rotate"),             "Transform",  N_("Rotate selected objects by degrees.")                 },
    {"app.transform-scale",       N_("Scale"),              "Transform",  N_("Scale selected objects by scale factor.")             },
    {"app.transform-remove",      N_("Remove Transforms"),  "Transform",  N_("Remove any transforms from selected objects.")        }
    // clang-format on
};

template<class T>
void
add_actions_transform(ConcreteInkscapeApplication<T>* app)
{
    Glib::VariantType Bool(  Glib::VARIANT_TYPE_BOOL);
    Glib::VariantType Int(   Glib::VARIANT_TYPE_INT32);
    Glib::VariantType Double(Glib::VARIANT_TYPE_DOUBLE);
    Glib::VariantType String(Glib::VARIANT_TYPE_STRING);
    std::vector<Glib::VariantType> dd = {Glib::VARIANT_TYPE_DOUBLE, Glib::VARIANT_TYPE_DOUBLE};
    Glib::VariantType Tuple_DD = Glib::VariantType::create_tuple(dd);

    // Debian 9 has 2.50.0
#if GLIB_CHECK_VERSION(2, 52, 0)

    // clang-format off
    app->add_action_with_parameter( "transform-translate",    Tuple_DD, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_translate),       app));
    app->add_action_with_parameter( "transform-rotate",         Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_rotate),          app));
    app->add_action_with_parameter( "transform-scale",          Double, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_scale),           app));
    app->add_action(                "transform-remove",                 sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&transform_remove),          app));
    // clang-format on

#endif

    app->get_action_extra_data().add_data(raw_data_transform);
}


template void add_actions_transform(ConcreteInkscapeApplication<Gio::Application>* app);
template void add_actions_transform(ConcreteInkscapeApplication<Gtk::Application>* app);



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
