// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 *  Actions for Undo/Redo tied to document.
 *
 * Authors:
 *   Tavmjong Bah
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <giomm.h>
#include <glibmm/i18n.h>

#include "actions-undo-document.h"

#include "document.h"
#include "document-undo.h"
#include "inkscape-application.h"

// ifdef out for headless operation!
#include "desktop.h"
#include "inkscape-window.h"
#include "ui/widget/canvas.h"

void
undo(SPDocument* document)
{
    auto app = InkscapeApplication::instance();
    auto win = app->get_active_window();
    if (win) {
        // Could be in headless mode.
        auto desktop = win->get_desktop();
        // No undo while dragging, too dangerous.
        if (desktop->getCanvas()->is_dragging()) {
            return;
        }
    }

    Inkscape::DocumentUndo::undo(document);
}

void
redo(SPDocument* document)
{
    auto app = InkscapeApplication::instance();
    auto win = app->get_active_window();
    if (win) {
        // Could be in headless mode.
        auto desktop = win->get_desktop();
        // No redo while dragging, too dangerous.
        if (desktop->getCanvas()->is_dragging()) {
            return;
        }
    }

    Inkscape::DocumentUndo::redo(document);
}

std::vector<std::vector<Glib::ustring>> raw_data_undo_document =
{
    // clang-format off
    {"doc.undo",                                N_("Undo"),                   "Edit Document",     N_("Undo last action")},
    {"doc.redo",                                N_("Redo"),                   "Edit Document",     N_("Do again the last undone action")},
    // clang-format on
};

void
add_actions_undo_document(SPDocument* document)
{
    auto group = document->getActionGroup();
    
    // clang-format off
    group->add_action( "undo",                            sigc::bind<SPDocument*>(sigc::ptr_fun(&undo), document));
    group->add_action( "redo",                            sigc::bind<SPDocument*>(sigc::ptr_fun(&redo), document));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_undo: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_undo_document);
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
