// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions Related to Text
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

#include "actions-text.h"
#include "actions/actions-extra-data.h"
#include "inkscape-application.h"
#include "text-chemistry.h"

void
selection_text_put_on_path()
{
    text_put_on_path();
}

void
selection_text_remove_from_path()
{
    text_remove_from_path();
}

void
text_flow_into_frame()
{
    text_flow_into_shape();
}

void
text_flow_subtract_frame()
{
    text_flow_shape_subtract();
}

void
select_text_unflow()
{
    text_unflow();
}

void
text_convert_to_regular()
{
    flowtext_to_text();
}

void
text_unkern()
{
    text_remove_all_kerns();
}

std::vector<std::vector<Glib::ustring>> raw_data_text =
{
    // clang-format off
    {"doc.text-put-on-path",            N_("Put on Path"),                   "Text",             N_("Put text on path")},
    {"doc.text-remove-from-path",       N_("Remove from Path"),              "Text",             N_("Remove text from path")},
    {"doc.text-flow-into-frame",        N_("Flow into Frame"),               "Text",             N_("Put text into a frame (path or shape), creating a flowed text linked to the frame object")},
    {"doc.text-flow-subtract-frame",    N_("Set _Subtraction Frames"),       "Text",             N_("Flow text around a frame (path or shape), only available for SVG 2.0 Flow text.")},
    {"doc.text-unflow",                 N_("_Unflow"),                       "Text",             N_("Remove text from frame (creates a single-line text object)")},
    {"doc.text-convert-to-regular",     N_("_Convert to Text"),              "Text",             N_("Convert flowed text to regular text object (preserves appearance)")},
    {"doc.text-unkern",                 N_("Remove Manual _Kerns"),          "Text",             N_("Remove all manual kerns and glyph rotations from a text object")}
    // clang-format on
};

void
add_actions_text(SPDocument* document)
{

    Glib::RefPtr<Gio::SimpleActionGroup> map = document->getActionGroup();

    // clang-format off
    map->add_action( "text-put-on-path",            sigc::ptr_fun(selection_text_put_on_path));
    map->add_action( "text-remove-from-path",       sigc::ptr_fun(selection_text_remove_from_path));
    map->add_action( "text-flow-into-frame",        sigc::ptr_fun(text_flow_into_frame));
    map->add_action( "text-flow-subtract-frame",    sigc::ptr_fun(text_flow_subtract_frame));
    map->add_action( "text-unflow",                 sigc::ptr_fun(select_text_unflow));
    map->add_action( "text-convert-to-regular",     sigc::ptr_fun(text_convert_to_regular));
    map->add_action( "text-unkern",                 sigc::ptr_fun(text_unkern));
    // clang-format on

    // Check if there is already an application instance (GUI or non-GUI).
    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_file_document: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_text);
}