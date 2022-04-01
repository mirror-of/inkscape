// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 *  Actions for opening, saving, etc. files which (mostly) open a dialog or an Inkscape window.
 *  Used by menu items under the "File" submenu.
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

#include "actions-file-window.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "file.h"
#include "ui/dialog/save-template-dialog.h"
#include "ui/dialog/new-from-template.h"

void
document_new(InkscapeWindow* win)
{
    sp_file_new_default();
}

void
document_dialog_templates(InkscapeWindow* win)
{
    Inkscape::UI::NewFromTemplate::load_new_from_template();
}

void
document_open(InkscapeWindow* win)
{
    // Open File Dialog
    sp_file_open_dialog(*win, nullptr, nullptr);
}

void
document_revert(InkscapeWindow* win)
{
    sp_file_revert_dialog();
}

void
document_save(InkscapeWindow* win)
{
    // Save File
    sp_file_save(*win, nullptr, nullptr);
}

void
document_save_as(InkscapeWindow* win)
{
    // Save File As
    sp_file_save_as(*win, nullptr, nullptr);
}

void
document_save_copy(InkscapeWindow* win)
{
    // Save A copy
    sp_file_save_a_copy(*win, nullptr, nullptr);
}

void
document_save_template(InkscapeWindow* win)
{
    // Save As Template
    Inkscape::UI::Dialog::SaveTemplate::save_document_as_template(*win);
}

void
document_import(InkscapeWindow* win)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    prefs->setBool("/options/onimport", true);
    sp_file_import(*win);
    prefs->setBool("/options/onimport", false);
}

void
document_print(InkscapeWindow* win)
{
    // Print File
    sp_file_print(*win);
}

void
document_cleanup(InkscapeWindow* win)
{
    // Cleanup Up Document
    sp_file_vacuum(win->get_document());
}

// Close window, checking for data loss. If it's the last window, keep open with new document.
void
document_close(InkscapeWindow* win)
{
    // Close
    auto app = InkscapeApplication::instance();
    app->destroy_window(win, true); // true == keep alive last window
}

std::vector<std::vector<Glib::ustring>> raw_data_dialog_window =
{
    // clang-format off
    {"win.document-new",                N_("New"),                  "Window-File",     N_("Create new document from the default template")},
    {"win.document-dialog-templates",   N_("New from Template"),   "Window-File",     N_("Create new project from template")},
    {"win.document-open",               N_("Open File Dialog"),     "Window-File",     N_("Open an existing document")},
    {"win.document-revert",             N_("Revert"),               "Window-File",     N_("Revert to the last saved version of document (changes will be lost)")},
    {"win.document-save",               N_("Save"),                 "Window-File",     N_("Save document")},
    {"win.document-save-as",            N_("Save As"),              "Window-File",     N_("Save document under a new name")},
    {"win.document-save-copy",          N_("Save a Copy"),          "Window-File",     N_("Save a copy of the document under a new name")},
    {"win.document-save-template",      N_("Save Template"),        "Window-File",     N_("Save a copy of the document as template")},
    {"win.document-import",             N_("Import"),               "Window-File",     N_("Import a bitmap or SVG image into this document")},
    {"win.document-print",              N_("Print"),                "Window-File",     N_("Print document")},
    {"win.document-cleanup",            N_("Clean Up Document"),    "Window-File",     N_("Remove unused definitions (such as gradients or clipping paths) from the <defs> of the document")},
    {"win.document-close",              N_("Close"),                "Window-File",     N_("Close window (unless last window)")},
    // clang-format on
};

void
add_actions_file_window(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "document-new",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_new),               win));
    win->add_action( "document-dialog-templates",   sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_dialog_templates),  win));
    win->add_action( "document-open",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_open),              win));
    win->add_action( "document-revert",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_revert),            win));
    win->add_action( "document-save",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save),              win));
    win->add_action( "document-save-as",            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save_as),           win));
    win->add_action( "document-save-copy",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save_copy),         win));
    win->add_action( "document-save-template",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save_template),     win));
    win->add_action( "document-import",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_import),            win));
    win->add_action( "document-print",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_print),             win));
    win->add_action( "document-cleanup",            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_cleanup),           win));
    win->add_action( "document-close",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_close),             win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_file_window: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_dialog_window);
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
