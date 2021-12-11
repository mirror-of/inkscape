// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions related to fit canvas
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-fit-canvas.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "object/sp-namedview.h"
#include "object/sp-page.h"
#include "page-manager.h"
#include "selection-chemistry.h"

void
canvas_to_selection_or_drawing(InkscapeWindow* win)
{
    auto desktop = win->get_desktop();
    auto document = desktop->getDocument();

    if (auto manager = document->getNamedView()->getPageManager()) {
        if (auto page = manager->getSelected()) {
            // SPPage will decide how to resize itself if there's nothing selected.
            page->fitToSelection(desktop->selection);
            return;
        }
    }
    // No pages in this document: Resize Viewbox to Selection
    fit_canvas_to_selection_or_drawing(desktop);
}

std::vector<std::vector<Glib::ustring>> raw_fit_canvas_data =
{
    // clang-format off
    {"win.fit-canvas-to-selection-or-drawing",          N_("Resize Page to Selection"),     "Selection Desktop",  N_("Fit the page to the current selection or the drawing if there is no selection")}
    // clang-format on
};

void
add_actions_fit_canvas(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "fit-canvas-to-selection-or-drawing",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&canvas_to_selection_or_drawing), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_fit_canvas: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_fit_canvas_data);
}
