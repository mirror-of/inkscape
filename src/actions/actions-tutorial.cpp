// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Tutorial Actions
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

#include "actions-tutorial.h"
#include "actions/actions-extra-data.h"
#include "inkscape-application.h"
#include "document.h"
#include "inkscape.h"
#include "ui/interface.h"
#include "ui/dialog/about.h"
#include "io/resource.h"

using Inkscape::IO::Resource::UIS;


void help_about()
{
    Inkscape::UI::Dialog::AboutDialog::show_about();
}

void help_open_tutorial(Glib::ustring name)
{
    Glib::ustring filename = name + ".svg";

    filename = Inkscape::IO::Resource::get_filename(Inkscape::IO::Resource::TUTORIALS, filename.c_str(), true);
    if (!filename.empty()) {
        auto *app = InkscapeApplication::instance();
        SPDocument* doc = app->document_new(filename);
        app->window_open(doc);
    } else {
        // TRANSLATORS: Please don't translate link unless the page exists in your language. Add your language code to
        // the link this way: https://inkscape.org/[lang]/learn/tutorials/
        sp_ui_error_dialog(_("The tutorial files are not installed.\nFor Linux, you may need to install "
                             "'inkscape-tutorials'; for Windows, please re-run the setup and select 'Tutorials'.\nThe "
                             "tutorials can also be found online at https://inkscape.org/en/learn/tutorials/"));
    }
}

void
tutorial_basic(SPDocument* document, Glib::ustring tutorial)
{
    help_open_tutorial(tutorial);
}

void
help_about_inkscape(SPDocument* document)
{
    help_about();
}

std::vector<std::vector<Glib::ustring>> raw_data_tutorial =
{
    // clang-format off
    { "doc.tutorial-basic",               N_("Inkscape: Basic"),                  "Tutorial",     N_("Getting started with Inkscape")},
    { "doc.tutorial-shapes",              N_("Inkscape: Shapes"),                 "Tutorial",     N_("Using shape tools to create and edit shapes")},
    { "doc.tutorial-advanced",            N_("Inkscape: Advanced"),               "Tutorial",     N_("Advanced Inkscape topics")},
    { "doc.tutorial-tracing",             N_("Inkscape: Tracing"),                "Tutorial",     N_("Using bitmap tracing"),},
    { "doc.tutorial-tracing-pixelart",    N_("Inkscape: Tracing Pixel Art"),      "Tutorial",     N_("Using Trace Pixel Art dialog")},
    { "doc.tutorial-calligraphy",         N_("Inkscape: Calligraphy"),            "Tutorial",     N_("Using the Calligraphy pen tool")},
    { "doc.tutorial-interpolate",         N_("Inkscape: Interpolate"),            "Tutorial",     N_("Using the interpolate extension")},
    { "doc.tutorial-design",              N_("Elements of Design"),               "Tutorial",     N_("Principles of design in the tutorial form")},
    { "doc.tutorial-tips",                N_("Tips and Tricks"),                  "Tutorial",     N_("Miscellaneous tips and tricks")},
    { "doc.help-about",                   N_("About Inkscape"),                   "Tutorial",     N_("Inkscape version, authors, license")}
    // clang-format on
};

void
add_actions_tutorial(SPDocument* document)
{
    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    // clang-format off
    map->add_action( "tutorial-basic",                  sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-basic"));
    map->add_action( "tutorial-shapes",                 sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-shapes"));
    map->add_action( "tutorial-advanced",               sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-advanced"));
    map->add_action( "tutorial-tracing",                sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-tracing"));
    map->add_action( "tutorial-tracing-pixelart",       sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-tracing-pixelart"));
    map->add_action( "tutorial-calligraphy",            sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-calligraphy"));
    map->add_action( "tutorial-interpolate",            sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-interpolate"));
    map->add_action( "tutorial-design",                 sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-elements"));
    map->add_action( "tutorial-tips",                   sigc::bind<SPDocument*, Glib::ustring>(sigc::ptr_fun(&tutorial_basic), document, "tutorial-tips"));
    map->add_action( "help-about",                      sigc::bind<SPDocument*>(sigc::ptr_fun(&help_about_inkscape), document));
    // clang-format on
    
    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_tutorial: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_tutorial);
}