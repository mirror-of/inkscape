// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions Related to Files which require document
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

#include "actions-file-document.h"
#include "actions/actions-extra-data.h"
#include "inkscape-application.h"
#include "document.h"
#include "inkscape.h"
#include "object/sp-namedview.h"
#include "file.h"
#include "ui/dialog/new-from-template.h"
#include "ui/interface.h"

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

void
document_new(SPDocument* document)
{
    sp_file_new_default();
}

void
document_dialog_templates(SPDocument* document)
{
    Inkscape::UI::NewFromTemplate::load_new_from_template();
}

void
document_revert(SPDocument* document)
{
    sp_file_revert_dialog();
}


void
document_cleanup(SPDocument* document)
{
    sp_file_vacuum(document);
}

void
window_new(SPDocument* document)
{
    sp_ui_new_view();
}

std::vector<std::vector<Glib::ustring>> raw_data_file_document =
{
    // clang-format off
    {"doc.window-new",                  N_("Duplicate Window"),         "File",             N_("Open a new window with the same document")},
    {"doc.window-previous",             N_("Previous Window"),          "File",             N_("Switch to the previous document window")},
    {"doc.window-next",                 N_("Next Window"),              "File",             N_("Switch to the next document window")},
    {"doc.document-new",                N_("New"),                      "File",             N_("Create new document from the default template")},
    {"doc.document-dialog-templates",   N_("New from  Template"),       "File",             N_("Create new project from template")},
    {"doc.document-revert",             N_("Revert"),                   "File",             N_("Revert to the last saved version of document (changes will be lost)")},
    {"doc.document-cleanup",            N_("Clean Up Document"),        "File",             N_("Remove unused definitions (such as gradients or clipping paths) from the <defs> of the document")}
    // clang-format on
};

void
add_actions_file_document(SPDocument* document)
{
    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    // clang-format off
    map->add_action( "window-new",                  sigc::bind<SPDocument*>(sigc::ptr_fun(&window_new), document));
    map->add_action( "window-previous",             sigc::bind<SPDocument*>(sigc::ptr_fun(&window_previous),  document));
    map->add_action( "window-next",                 sigc::bind<SPDocument*>(sigc::ptr_fun(&window_next),  document));
    map->add_action( "document-new",                sigc::bind<SPDocument*>(sigc::ptr_fun(&document_new),  document));
    map->add_action( "document-dialog-templates",   sigc::bind<SPDocument*>(sigc::ptr_fun(&document_dialog_templates),  document));
    map->add_action( "document-revert",             sigc::bind<SPDocument*>(sigc::ptr_fun(&document_revert),  document));
    map->add_action( "document-cleanup",            sigc::bind<SPDocument*>(sigc::ptr_fun(&document_cleanup), document));
    // clang-format on

    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_file_document: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_file_document);
}