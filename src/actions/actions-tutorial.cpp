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
#include "document.h"
#include "inkscape-application.h"
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
help_about_inkscape()
{
    help_about();
}

std::vector<std::vector<Glib::ustring>> raw_data_tutorial =
{
    // clang-format off
    { "app.tutorial-basic",               N_("Inkscape: Basic"),                  "Tutorial",     N_("Getting started with Inkscape")},
    { "app.tutorial-shapes",              N_("Inkscape: Shapes"),                 "Tutorial",     N_("Using shape tools to create and edit shapes")},
    { "app.tutorial-advanced",            N_("Inkscape: Advanced"),               "Tutorial",     N_("Advanced Inkscape topics")},
    { "app.tutorial-tracing",             N_("Inkscape: Tracing"),                "Tutorial",     N_("Using bitmap tracing"),},
    { "app.tutorial-tracing-pixelart",    N_("Inkscape: Tracing Pixel Art"),      "Tutorial",     N_("Using Trace Pixel Art dialog")},
    { "app.tutorial-calligraphy",         N_("Inkscape: Calligraphy"),            "Tutorial",     N_("Using the Calligraphy pen tool")},
    { "app.tutorial-interpolate",         N_("Inkscape: Interpolate"),            "Tutorial",     N_("Using the interpolate extension")},
    { "app.tutorial-design",              N_("Elements of Design"),               "Tutorial",     N_("Principles of design in the tutorial form")},
    { "app.tutorial-tips",                N_("Tips and Tricks"),                  "Tutorial",     N_("Miscellaneous tips and tricks")},
    { "app.about",                        N_("About Inkscape"),                   "Tutorial",     N_("Inkscape version, authors, license")}
    // clang-format on
};

void
add_actions_tutorial(InkscapeApplication* app)
{
    if (!app->gtk_app()) {
        return;
    }

    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action( "tutorial-basic",                  sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-basic"));
    gapp->add_action( "tutorial-shapes",                 sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-shapes"));
    gapp->add_action( "tutorial-advanced",               sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-advanced"));
    gapp->add_action( "tutorial-tracing",                sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-tracing"));
    gapp->add_action( "tutorial-tracing-pixelart",       sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-tracing-pixelart"));
    gapp->add_action( "tutorial-calligraphy",            sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-calligraphy"));
    gapp->add_action( "tutorial-interpolate",            sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-interpolate"));
    gapp->add_action( "tutorial-design",                 sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-elements"));
    gapp->add_action( "tutorial-tips",                   sigc::bind<Glib::ustring>(sigc::ptr_fun(&help_open_tutorial), "tutorial-tips"));
    gapp->add_action( "about",                           sigc::ptr_fun(&help_about_inkscape));
    // clang-format on

    app->get_action_extra_data().add_data(raw_data_tutorial);
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
