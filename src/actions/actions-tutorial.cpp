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
#include "help.h"

void
tutorial_basic(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-basic");
}

void
tutorial_shapes(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-shapes");
}

void
tutorial_advanced(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-advanced");
}

void
tutorial_tracing(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-tracing");
}

void
tutorial_tracing_pixelart(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-tracing-pixelart");
}

void
tutorial_calligraphy(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-calligraphy");
}

void
tutorial_interpolate(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-interpolate");
}

void
tutorial_design(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-elements");
}

void
tutorial_tips(SPDocument* document)
{
    sp_help_open_tutorial("tutorial-tips");
}


void
help_about(SPDocument* document)
{
    sp_help_about();
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
    map->add_action( "tutorial-basic",                  sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_basic), document));
    map->add_action( "tutorial-shapes",                 sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_shapes), document));
    map->add_action( "tutorial-advanced",               sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_advanced), document));
    map->add_action( "tutorial-tracing",                sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_tracing), document));
    map->add_action( "tutorial-tracing-pixelart",       sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_tracing_pixelart), document));
    map->add_action( "tutorial-calligraphy",            sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_calligraphy), document));
    map->add_action( "tutorial-interpolate",            sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_interpolate), document));
    map->add_action( "tutorial-design",                 sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_design), document));
    map->add_action( "tutorial-tips",                   sigc::bind<SPDocument*>(sigc::ptr_fun(&tutorial_tips), document));
    map->add_action( "help-about",                      sigc::bind<SPDocument*>(sigc::ptr_fun(&help_about), document));
    // clang-format on
    
    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_tutorial: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_tutorial);
}