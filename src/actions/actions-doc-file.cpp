// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions Related to Files
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

#include "actions-doc-file.h"
#include "actions/actions-extra-data.h"
#include "inkscape-application.h"

#include "document.h"
#include "inkscape.h"
#include "object/sp-namedview.h"

void
window_previous(SPDocument* document)
{
    INKSCAPE.switch_desktops_prev();
}

void
window_next(SPDocument* document)
{
    INKSCAPE.switch_desktops_next();
}

std::vector<std::vector<Glib::ustring>> raw_data_doc_file =
{
    {"doc.window-previous",     N_("P_revious Window"),     "File",     N_("Switch to the previous document window")},
    {"doc.window-next",         N_("N_ext Window"),         "File",     N_("Switch to the next document window")}
};

void
add_actions_doc_file(SPDocument* document)
{
    std::cout<<"add_actions_doc_file\n";

    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    map->add_action( "window-previous",     sigc::bind<SPDocument*>(sigc::ptr_fun(&window_previous),  document));
    map->add_action( "window-next",         sigc::bind<SPDocument*>(sigc::ptr_fun(&window_next),  document));

    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_doc_file: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_doc_file);
}