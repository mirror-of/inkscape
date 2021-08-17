// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 *  Actions for Editing an object
 *  Contains many actions of Edit Verb
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

void
file_open(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    Gtk::Window *parent = dt->getToplevel();
    g_assert(parent != nullptr);

    sp_file_open_dialog(*parent, nullptr, nullptr);
}

void
document_save(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    Gtk::Window *parent = dt->getToplevel();
    g_assert(parent != nullptr);

    sp_file_save(*parent, nullptr, nullptr);
}

void
document_save_as(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    Gtk::Window *parent = dt->getToplevel();
    g_assert(parent != nullptr);

    sp_file_save_as(*parent, nullptr, nullptr);
}

void
document_save_copy(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    Gtk::Window *parent = dt->getToplevel();
    g_assert(parent != nullptr);

    sp_file_save_a_copy(*parent, nullptr, nullptr);
}

void
document_save_template(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    Gtk::Window *parent = dt->getToplevel();
    g_assert(parent != nullptr);

    Inkscape::UI::Dialog::SaveTemplate::save_document_as_template(*parent);
}

void
document_print(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();

    Gtk::Window *parent = dt->getToplevel();
    g_assert(parent != nullptr);

    sp_file_print(*parent);
}

void
document_import(InkscapeWindow* win)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    prefs->setBool("/options/onimport",true);
    sp_file_import(*win);
    prefs->setBool("/options/onimport",false);
}

std::vector<std::vector<Glib::ustring>> raw_data_file_window =
{
    // clang-format off
    {"win.file-open",               N_("Open File Dialog"),     "File",     N_("Open an existing document")},
    {"win.document-save",           N_("Save"),                 "File",     N_("Save document")},
    {"win.document-save-as",        N_("Save As"),              "File",     N_("Save document under a new name")},
    {"win.document-save-copy",      N_("Save a Copy"),          "File",     N_("Save a copy of the document under a new name")},
    {"win.document-save-template",  N_("Save Template"),        "File",     N_("Save a copy of the document as template")},
    {"win.document-print",          N_("Print"),                "File",     N_("Print document")},
    {"win.document-import",         N_("Import"),               "File",     N_("Import a bitmap or SVG image into this document")}
    // clang-format on
};

void
add_actions_file_window(InkscapeWindow* win)
{
    // clang-format off
    win->add_action( "file-open",                   sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&file_open), win));
    win->add_action( "document-save",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save), win));
    win->add_action( "document-save-as",            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save_as), win));
    win->add_action( "document-save-copy",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save_copy), win));
    win->add_action( "document-save-template",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_save_template), win));
    win->add_action( "document-print",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_print), win));
    win->add_action( "document-import",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&document_import), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_file_window: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_file_window);
}