// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for working with objects without GUI.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-object.h"
#include "actions-helper.h"
#include "document-undo.h"
#include "inkscape-application.h"

#include "inkscape.h"             // Inkscape::Application
#include "selection.h"            // Selection
#include "path/path-simplify.h"

#include "live_effects/lpe-powerclip.h"
#include "live_effects/lpe-powermask.h"
#include "ui/icon-names.h"

// No sanity checking is done... should probably add.
void
object_set_attribute(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring> >(value);

    std::vector<Glib::ustring> tokens = Glib::Regex::split_simple(",", s.get());
    if (tokens.size() != 2) {
        std::cerr << "action:object_set_attribute: requires 'attribute name, attribute value'" << std::endl;
        return;
    }

    auto selection = app->get_active_selection();
    if (selection->isEmpty()) {
        std::cerr << "action:object_set_attribute: selection empty!" << std::endl;
        return;
    }

    // Should this be a selection member function?
    auto items = selection->items();
    for (auto i = items.begin(); i != items.end(); ++i) {
        Inkscape::XML::Node *repr = (*i)->getRepr();
        repr->setAttribute(tokens[0], tokens[1]);
    }

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), "ActionObjectSetAttribute", "");
}


// No sanity checking is done... should probably add.
void
object_set_property(const Glib::VariantBase& value, InkscapeApplication *app)
{
    Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring> >(value);

    std::vector<Glib::ustring> tokens = Glib::Regex::split_simple(",", s.get());
    if (tokens.size() != 2) {
        std::cerr << "action:object_set_property: requires 'property name, property value'" << std::endl;
        return;
    }

    auto selection = app->get_active_selection();
    if (selection->isEmpty()) {
        std::cerr << "action:object_set_property: selection empty!" << std::endl;
        return;
    }

    // Should this be a selection member function?
    auto items = selection->items();
    for (auto i = items.begin(); i != items.end(); ++i) {
        Inkscape::XML::Node *repr = (*i)->getRepr();
        SPCSSAttr *css = sp_repr_css_attr(repr, "style");
        sp_repr_css_set_property(css, tokens[0].c_str(), tokens[1].c_str());
        sp_repr_css_set(repr, css, "style");
        sp_repr_css_attr_unref(css);
    }

    // Needed to update repr (is this the best way?).
    Inkscape::DocumentUndo::done(app->get_active_document(), "ActionObjectSetProperty", "");
}


void
object_unlink_clones(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();

    // We should not have to do this!
    auto document  = app->get_active_document();
    selection->setDocument(document);

    selection->unlink();
}

void
object_clip_set(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Clip Set
    selection->setMask(true, false);
}

void
object_clip_set_inverse(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Clip Set Inverse
    selection->setMask(true, false);
    Inkscape::LivePathEffect::sp_inverse_powerclip(app->get_active_selection());
    Inkscape::DocumentUndo::done(app->get_active_document(), _("Set Inverse Clip(LPE)"), "");
}

void
object_clip_release(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Clip Release
    Inkscape::LivePathEffect::sp_remove_powerclip(app->get_active_selection());
    selection->unsetMask(true);
    Inkscape::DocumentUndo::done(app->get_active_document(), _("Release clipping path"), "");
}

void
object_clip_set_group(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();
    selection->setClipGroup();
    // Undo added in setClipGroup().
}

void
object_mask_set(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Mask Set
    selection->setMask(false, false);
    // Undo added in setMask().
}

void
object_mask_set_inverse(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Mask Set Inverse
    selection->setMask(false, false);
    Inkscape::LivePathEffect::sp_inverse_powermask(app->get_active_selection());
    Inkscape::DocumentUndo::done(app->get_active_document(), _("Set Inverse Mask (LPE)"), "");
}

void
object_mask_release(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Mask Release
    Inkscape::LivePathEffect::sp_remove_powermask(app->get_active_selection());
    selection->unsetMask(false);
    Inkscape::DocumentUndo::done(app->get_active_document(), _("Release mask"), "");
}

void
object_rotate_90_cw(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Rotate 90
    selection->rotate90(false);
}

void
object_rotate_90_ccw(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    // Object Rotate 90 CCW
    selection->rotate90(true);
}

void
object_flip_horizontal(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    Geom::OptRect bbox = selection->visualBounds();
    if (!bbox) {
        return;
    }

    // Get center
    Geom::Point center;
    if (selection->center()) {
        center = *selection->center();
    } else {
        center = bbox->midpoint();
    }

    // Object Flip Horizontal
    selection->setScaleRelative(center, Geom::Scale(-1.0, 1.0));
    Inkscape::DocumentUndo::done(app->get_active_document(), _("Flip horizontally"), INKSCAPE_ICON("object-flip-horizontal"));
}

void
object_flip_vertical(InkscapeApplication *app)
{
    Inkscape::Selection *selection = app->get_active_selection();

    Geom::OptRect bbox = selection->visualBounds();
    if (!bbox) {
        return;
    }

    // Get center
    Geom::Point center;
    if (selection->center()) {
        center = *selection->center();
    } else {
        center = bbox->midpoint();
    }

    // Object Flip Vertical
    selection->setScaleRelative(center, Geom::Scale(1.0, -1.0));
    Inkscape::DocumentUndo::done(app->get_active_document(), _("Flip vertically"), INKSCAPE_ICON("object-flip-vertical"));
}


void
object_to_path(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();

    // We should not have to do this!
    auto document  = app->get_active_document();
    selection->setDocument(document);

    selection->toCurves();  // TODO: Rename toPaths()
}


void
object_stroke_to_path(InkscapeApplication *app)
{
    auto selection = app->get_active_selection();

    // We should not have to do this!
    auto document  = app->get_active_document();
    selection->setDocument(document);

    selection->strokesToPaths();
}


std::vector<std::vector<Glib::ustring>> raw_data_object =
{
    // clang-format off
    {"app.object-set-attribute",        N_("Set Attribute"),                    "Object",     N_("Set or update an attribute of selected objects; usage: object-set-attribute:attribute name, attribute value;")},
    {"app.object-set-property",         N_("Set Property"),                     "Object",     N_("Set or update a property on selected objects; usage: object-set-property:property name, property value;")},

    {"app.object-unlink-clones",        N_("Unlink Clones"),                    "Object",     N_("Unlink clones and symbols")},
    {"app.object-to-path",              N_("Object To Path"),                   "Object",     N_("Convert shapes to paths")},
    {"app.object-stroke-to-path",       N_("Stroke to Path"),                   "Object",     N_("Convert strokes to paths")},

    {"app.object-set-clip",             N_("Object Clip Set"),                  "Object",     N_("Apply clipping path to selection (using the topmost object as clipping path)")},
    {"app.object-set-inverse-clip",     N_("Object Clip Set Inverse"),          "Object",     N_("Apply inverse clipping path to selection (Power Clip LPE)")},
    {"app.object-release-clip",         N_("Object Clip Release"),              "Object",     N_("Remove clipping path from selection")},
    {"app.object-set-clip-group",       N_("Object Clip Set Group"),            "Object",     N_("Create a self-clipping group to which objects (not contributing to the clip-path) can be added")},
    {"app.object-set-mask",             N_("Object Mask Set"),                  "Object",     N_("Apply mask to selection (using the topmost object as mask)")},
    {"app.object-set-inverse-mask",     N_("Object Mask Set Inverse"),          "Object",     N_("Apply inverse mask to selection (Power Mask LPE)")},
    {"app.object-release-mask",         N_("Object Mask Release"),              "Object",     N_("Remove mask from selection")},

    {"app.object-rotate-90-cw",         N_("Object Rotate 90"),                 "Object",     N_("Rotate selection 90° clockwise")},
    {"app.object-rotate-90-ccw",        N_("Object Rotate 90 CCW"),             "Object",     N_("Rotate selection 90° counter-clockwise")},
    {"app.object-flip-horizontal",      N_("Object Flip Horizontal"),           "Object",     N_("Flip selected objects horizontally")},
    {"app.object-flip-vertical",        N_("Object Flip Vertical"),             "Object",     N_("Flip selected objects vertically")}
    // clang-format on
};

std::vector<std::vector<Glib::ustring>> hint_data_object =
{
    // clang-format off
    {"app.object-set-attribute",        N_("Enter comma-separated string for attribute name, attribute value") },
    {"app.object-set-property",         N_("Enter comma-separated string for property name, property value")  }
    // clang-format on
};

void
add_actions_object(InkscapeApplication* app)
{
    Glib::VariantType Bool(  Glib::VARIANT_TYPE_BOOL);
    Glib::VariantType Int(   Glib::VARIANT_TYPE_INT32);
    Glib::VariantType Double(Glib::VARIANT_TYPE_DOUBLE);
    Glib::VariantType String(Glib::VARIANT_TYPE_STRING);

    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action_with_parameter( "object-set-attribute",            String, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_set_attribute),  app));
    gapp->add_action_with_parameter( "object-set-property",             String, sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_set_property),   app));

    gapp->add_action(                "object-unlink-clones",            sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_unlink_clones),          app));
    gapp->add_action(                "object-to-path",                  sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_to_path),                app));
    gapp->add_action(                "object-stroke-to-path",           sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_stroke_to_path),         app));

    gapp->add_action(                "object-set-clip",                 sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_clip_set),               app));
    gapp->add_action(                "object-set-inverse-clip",         sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_clip_set_inverse),       app));
    gapp->add_action(                "object-release-clip",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_clip_release),           app));
    gapp->add_action(                "object-set-clip-group",           sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_clip_set_group),         app));
    gapp->add_action(                "object-set-mask",                 sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_mask_set),               app));
    gapp->add_action(                "object-set-inverse-mask",         sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_mask_set_inverse),       app));
    gapp->add_action(                "object-release-mask",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_mask_release),           app));

    gapp->add_action(                "object-rotate-90-cw",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_rotate_90_cw),           app));
    gapp->add_action(                "object-rotate-90-ccw",            sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_rotate_90_ccw),          app));
    gapp->add_action(                "object-flip-horizontal",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_flip_horizontal),        app));
    gapp->add_action(                "object-flip-vertical",            sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&object_flip_vertical),          app));
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_object);
    app->get_action_hint_data().add_data(hint_data_object);
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
