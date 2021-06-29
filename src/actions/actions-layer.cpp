// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 *  Actions for Layers
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

#include "actions-layer.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "ui/dialog/layer-properties.h"
#include "message-stack.h"
#include "ui/icon-names.h"
#include "document-undo.h"

void 
layer_new(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // New Layer 
    Inkscape::UI::Dialogs::LayerPropertiesDialog::showCreate(dt, dt->currentLayer());
}

void
layer_rename(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Rename Layer
    Inkscape::UI::Dialogs::LayerPropertiesDialog::showRename(dt, dt->currentLayer());
}

void
layer_toggle_hide (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() == dt->currentRoot() ) {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
    } else {
        SP_ITEM(dt->currentLayer())->setHidden(!SP_ITEM(dt->currentLayer())->isHidden());
    }
}

void
layer_toggle_lock (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() == dt->currentRoot() ) {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
    } else {
        SP_ITEM(dt->currentLayer())->setLocked(!SP_ITEM(dt->currentLayer())->isLocked());
    }
}

void
layer_previous (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    
    SPObject *next=Inkscape::next_layer(dt->currentRoot(), dt->currentLayer());
    if (next) {
        dt->setCurrentLayer(next);
        Inkscape::DocumentUndo::done(dt->getDocument(),_("Switch to next layer"),INKSCAPE_ICON("layer-previous"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Switched to next layer."));
    } else {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot go past last layer."));
    }
}

void
layer_next (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    
    SPObject *prev=Inkscape::previous_layer(dt->currentRoot(), dt->currentLayer());
    if (prev) {
        dt->setCurrentLayer(prev);
        Inkscape::DocumentUndo::done(dt->getDocument(), _("Switch to previous layer"),INKSCAPE_ICON("layer-next"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Switched to previous layer."));
    } else {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot go before first layer."));
    }
}

void
selection_move_to_layer_above (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Layer Rise
    dt->selection->toNextLayer();
}

void
selection_move_to_layer_below (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Layer Lower
    dt->selection->toPrevLayer();
}

void
selection_move_to_layer (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    // Selection move to layer 
    Inkscape::UI::Dialogs::LayerPropertiesDialog::showMove(dt, dt->currentLayer());
}

void
layer_top (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

        if ( dt->currentLayer() == dt->currentRoot() ) {
            dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
            return;
        }

        SPItem *layer=SP_ITEM(dt->currentLayer());
        g_return_if_fail(layer != nullptr);
        SPObject *old_pos = layer->getNext();
        layer->raiseToTop();

        if ( layer->getNext() != old_pos ) {
    
            char const * message = g_strdup_printf(_("Raised layer <b>%s</b>."), layer->defaultLabel());
            Inkscape::DocumentUndo::done(dt->getDocument(), _("Layer to top"), INKSCAPE_ICON("layer-top"));
            dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, message);
            g_free((void *) message);
    
        } else {
            dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot move layer any further."));
        }
}

void
layer_raise (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() == dt->currentRoot() ) {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
        return;
    }

    SPItem *layer=SP_ITEM(dt->currentLayer());
    g_return_if_fail(layer != nullptr);

    SPObject *old_pos = layer->getNext();


    layer->raiseOne();


    if ( layer->getNext() != old_pos ) {
    
        char const * message = g_strdup_printf(_("Raised layer <b>%s</b>."), layer->defaultLabel());
        Inkscape::DocumentUndo::done(dt->getDocument(), _("Raise layer"), INKSCAPE_ICON("layer-raise"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, message);
        g_free((void *) message);
    
    } else {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot move layer any further."));
    }
}

void
laye_lower (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() == dt->currentRoot() ) {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
        return;
    }

    SPItem *layer=SP_ITEM(dt->currentLayer());
    g_return_if_fail(layer != nullptr);
    SPObject *old_pos = layer->getNext();
    layer->lowerOne();

    if ( layer->getNext() != old_pos ) {

        char const * message = g_strdup_printf(_("Lowered layer <b>%s</b>."), layer->defaultLabel());
        Inkscape::DocumentUndo::done(dt->getDocument(), _("Lower layer"), INKSCAPE_ICON("layer-lower"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, message);
        g_free((void *) message);
    
    } else {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot move layer any further."));
    }
}

void
layer_bottom (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() == dt->currentRoot() ) {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
        return;
    }

    SPItem *layer=SP_ITEM(dt->currentLayer());
    g_return_if_fail(layer != nullptr);
    SPObject *old_pos = layer->getNext();
    layer->lowerToBottom();

    if ( layer->getNext() != old_pos ) {
        
        char const * message = g_strdup_printf(_("Lowered layer <b>%s</b>."), layer->defaultLabel());
        Inkscape::DocumentUndo::done(dt->getDocument(), _("Layer to bottom"), INKSCAPE_ICON("layer-bottom"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, message);
        g_free((void *) message);
    
    } else {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot move layer any further."));
    }
}

void
layer_duplicate (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() != dt->currentRoot() ) {

        dt->selection->duplicate(true, true);
        Inkscape::DocumentUndo::done(dt->getDocument(), _("Duplicate layer"), INKSCAPE_ICON("layer-duplicate"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Duplicated layer."));
    
    } else {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
    }
}

void
layer_delete (InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    if ( dt->currentLayer() != dt->currentRoot() ) {

        dt->getSelection()->clear();
        SPObject *old_layer = dt->currentLayer();
        SPObject *old_parent = old_layer->parent;
        SPObject *old_parent_parent = (old_parent != nullptr) ? old_parent->parent : nullptr;

        SPObject *survivor = Inkscape::previous_layer(dt->currentRoot(), old_layer);
        if (survivor != nullptr && survivor->parent == old_layer) {
            while (survivor != nullptr &&
                    survivor->parent != old_parent &&
                    survivor->parent != old_parent_parent)
            {
                survivor = Inkscape::previous_layer(dt->currentRoot(), survivor);
            }
        }

        if (survivor == nullptr || (survivor->parent != old_parent && survivor->parent != old_layer)) {
            survivor = Inkscape::next_layer(dt->currentRoot(), old_layer);
            while (survivor != nullptr &&
                    survivor != old_parent &&
                    survivor->parent != old_parent)
            {
                survivor = Inkscape::next_layer(dt->currentRoot(), survivor);
            }
        }

        // Deleting the old layer before switching layers is a hack to trigger the
        old_layer->deleteObject();

        if (survivor) {
            dt->setCurrentLayer(survivor);
        }

        Inkscape::DocumentUndo::done(dt->getDocument(), _("Delete layer"), INKSCAPE_ICON("layer-delete"));
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Deleted layer."));

    } else {
        dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
    }
}

std::vector<std::vector<Glib::ustring>> raw_data_layer =
{
    // clang-format off
    {"win.layer-new",                       N_("Add Layer"),                        "Layers",     N_("Create a new layer")},
    {"win.layer-rename",                    N_("Rename Layer"),                     "Layers",     N_("Rename the current layer")},
    {"win.layer-toggle-hide",               N_("Show/Hide Current Layer"),          "Layers",     N_("Toggle visibility of current layer")},
    {"win.layer-toggle-lock",               N_("Lock/Unlock Current Layer"),        "Layers",     N_("Toggle lock on current layer")},
    {"win.layer-previous",                  N_("Switch to Layer Abov_e"),           "Layers",     N_("Switch to the layer above the current")},
    {"win.layer-next",                      N_("Switch to Layer Belo_w"),           "Layers",     N_("Switch to the layer below the current")},
    {"win.selection-move-to-layer-above",   N_("Move Selection to Layer Above"),    "Layers",     N_("Move selection to the layer above the current")},
    {"win.selection-move-to-layer-below",   N_("Move Selection to Layer Bel_ow"),   "Layers",     N_("Move selection to the layer below the current")},
    {"win.selection-move-to-layer",         N_("Move Selection to Layer..."),       "Layers",     N_("Move selection to layer")},
    {"win.layer-top",                       N_("Layer to Top"),                     "Layers",     N_("Raise the current layer to the top")},
    {"win.layer-raise",                     N_("Raise Layer"),                      "Layers",     N_("Raise the current layer")},
    {"win.layer-lower",                     N_("Lower Layer"),                      "Layers",     N_("Lower the current layer")},
    {"win.layer-bottom",                    N_("Layer to Bottom"),                  "Layers",     N_("Lower the current layer to the bottom")},
    {"win.layer-duplicate",                 N_("Duplicate Current Layer"),          "Layers",     N_("Duplicate an existing layer")},
    {"win.layer-delete",                    N_("Delete Current Layer"),             "Layers",     N_("Delete the current layer")}
    // clang-format on
};

void
add_actions_layer(InkscapeWindow* win)
{
    // clang-format off
    win->add_action("layer-new",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_new), win));
    win->add_action("layer-rename",                         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_rename), win));
    win->add_action("layer-toggle-hide",                    sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_toggle_hide), win));
    win->add_action("layer-toggle-lock",                    sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_toggle_lock), win));
    win->add_action("layer-previous",                       sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_previous), win));
    win->add_action("layer-next",                           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_next), win));
    win->add_action("selection-move-to-layer-above",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&selection_move_to_layer_above), win));
    win->add_action("selection-move-to-layer-below",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&selection_move_to_layer_below), win));
    win->add_action("selection-move-to-layer",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&selection_move_to_layer), win));
    win->add_action("layer-top",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_top), win));
    win->add_action("layer-raise",                          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_raise), win));
    win->add_action("layer-lower",                          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&laye_lower), win));
    win->add_action("layer-bottom",                         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_bottom), win));
    win->add_action("layer-duplicate",                      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_duplicate), win));
    win->add_action("layer-delete",                         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&layer_delete), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_layer: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_layer);
}