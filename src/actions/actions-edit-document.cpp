// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions Related to Edititng which require document
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

#include "actions-edit-document.h"
#include "actions/actions-extra-data.h"
#include "inkscape-application.h"
#include "document.h"
#include "inkscape.h"

#include "object/sp-guide.h"

void 
create_guides_around_page(SPDocument* document)
{
    // Create Guides Around the Page
    sp_guide_create_guides_around_page(document);
}

void
delete_all_guides(SPDocument* document)
{
    // Delete All Guides
    sp_guide_delete_all_guides(document);
}


std::vector<std::vector<Glib::ustring>> raw_data_edit_document =
{

    // clang-format off
    {"doc.create-guides-around-page",            N_("Create Guides Around the Page"),    "Edit Document",     N_("Create four guides aligned with the page borders")},
    {"doc.delete-all-guides",                    N_("Delete All Guides"),                "Edit Document",     N_("Delete all the guides in the document")}
    // clang-format on
};

void
add_actions_edit_document(SPDocument* document)
{

    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    // Debian 9 has 2.50.0
#if GLIB_CHECK_VERSION(2, 52, 0)
    // clang-format off
    map->add_action( "create-guides-around-page",           sigc::bind<SPDocument*>(sigc::ptr_fun(&create_guides_around_page),  document));
    map->add_action( "delete-all-guides",                    sigc::bind<SPDocument*>(sigc::ptr_fun(&delete_all_guides),  document));
    // clang-format on
#else
    std::cerr << "add_actions: Some actions require Glibmm 2.52, compiled with: " << glib_major_version << "." << glib_minor_version << std::endl;
#endif
    
    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit_document: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_edit_document);
}