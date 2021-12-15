// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Align and Distribute widget
 */
/* Authors:
 *   Tavmjong Bah
 *
 *   Based on dialog by:
 *     Bryce W. Harrington <bryce@bryceharrington.org>
 *     Aubanel MONNIER <aubi@libertysurf.fr>
 *     Frank Felfe <innerspace@iname.com>
 *     Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2021 Tavmjong Bah
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "align-and-distribute.h" // widget

#include <iostream>

#include <giomm.h>

#include "desktop.h"               // Tool switching.
#include "inkscape-window.h"       // Activate window action.
#include "actions/actions-tools.h" // Tool switching.
#include "io/resource.h"
#include "ui/dialog/dialog-base.h" // Tool switching.

namespace Inkscape {
namespace UI {
namespace Dialog {

using Inkscape::IO::Resource::get_filename;
using Inkscape::IO::Resource::UIS;

AlignAndDistribute::AlignAndDistribute(Inkscape::UI::Dialog::DialogBase* dlg)
    : Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Glib::ustring builder_file = get_filename(UIS, "align-and-distribute.ui");
    auto builder = Gtk::Builder::create();
    try
    {
        builder->add_from_file(builder_file);
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: " << builder_file << " file not read! " << ex.what() << std::endl;
    }

    builder->get_widget("align-and-distribute-box", align_and_distribute_box);
    if (!align_and_distribute_box) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget (box)!" << std::endl;
    } else {
        add(*align_and_distribute_box);
    }

    builder->get_widget("align-and-distribute-object", align_and_distribute_object);
    if (!align_and_distribute_object) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget (object)!" << std::endl;
    } else {
        align_and_distribute_object->show();
    }

    builder->get_widget("align-and-distribute-node", align_and_distribute_node);
    if (!align_and_distribute_node) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget (node)!" << std::endl;
    } else {
        align_and_distribute_node->hide();
    }

    // ------------  Object Align  -------------

    builder->get_widget("align-relative-object", align_relative_object);
    if (!align_relative_object) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget (combobox)!" << std::endl;
    } else {
        int align_to = prefs->getInt("/dialogs/align/align-to", 6);
        align_relative_object->set_active(align_to);

        align_relative_object->signal_changed().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_align_relative_object_changed));
    }

    builder->get_widget("align-move-as-group", align_move_as_group);
    if (!align_move_as_group) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget (group button)!" << std::endl;
    } else {
        bool sel_as_group = prefs->getBool("/dialogs/align/sel-as-groups");
        align_move_as_group->set_active(sel_as_group);

        align_move_as_group->signal_clicked().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_align_as_group_clicked));
    }

    // clang-format off
    std::vector<std::pair<std::string, std::string>> align_buttons = {
        {"align-horizontal-right-to-anchor", "right anchor"  },
        {"align-horizontal-left",            "left"          },
        {"align-horizontal-center",          "hcenter"       },
        {"align-horizontal-right",           "right"         },
        {"align-horizontal-left-to-anchor",  "left anchor"   },
        {"align-vertical-bottom-to-anchor",  "bottom anchor" },
        {"align-vertical-top",               "top"           },
        {"align-vertical-center",            "vcenter"       },
        {"align-vertical-bottom",            "bottom"        },
        {"align-vertical-top-to-anchor",     "top anchor"    }
    };
    // clang-format on

    for (auto align_button: align_buttons) {
        Gtk::Button* button;
        builder->get_widget(align_button.first, button);
        if (!button) {
            std::cerr << "AlignAndDistribute::AlignAndDisribute: failed to get button: "
                      << align_button.first << " " << align_button.second << std::endl;
        } else {
            button->signal_button_press_event().connect(
                sigc::bind<std::string>(sigc::mem_fun(*this, &AlignAndDistribute::on_align_button_press_event), align_button.second), false);
        }
    }


    // ------------ Remove overlap -------------

    builder->get_widget("remove-overlap-button", remove_overlap_button);
    if (!remove_overlap_button) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget!" << std::endl;
    } else {
        remove_overlap_button->signal_button_press_event().connect(
            sigc::mem_fun(*this, &AlignAndDistribute::on_remove_overlap_button_press_event), false); // false => run first.
    }

    builder->get_widget("remove-overlap-hgap", remove_overlap_hgap);
    if (!remove_overlap_hgap) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget!" << std::endl;
    }

    builder->get_widget("remove-overlap-vgap", remove_overlap_vgap);
    if (!remove_overlap_vgap) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget!" << std::endl;
    }

    // ------------  Node Align  -------------

    builder->get_widget("align-relative-node", align_relative_node);
    if (!align_relative_node) {
        std::cerr << "AlignAndDistribute::AlignAndDistribute: failed to load widget (combobox)!" << std::endl;
    } else {
        int align_nodes_to = prefs->getInt("/dialogs/align/align-nodes-to", 2);
        align_relative_node->set_active(align_nodes_to);

        align_relative_node->signal_changed().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_align_relative_node_changed));
    }

    std::vector<std::pair<std::string, std::string>> align_node_buttons = {
        {"align-node-horizontal", "horizontal"},
        {"align-node-vertical",   "vertical"  }
    };

    for (auto align_button: align_node_buttons) {
        Gtk::Button* button;
        builder->get_widget(align_button.first, button);
        if (!button) {
            std::cerr << "AlignAndDistribute::AlignAndDisribute: failed to get button: "
                      << align_button.first << " " << align_button.second << std::endl;
        } else {
            button->signal_button_press_event().connect(
                sigc::bind<std::string>(sigc::mem_fun(*this, &AlignAndDistribute::on_align_node_button_press_event), align_button.second), false);
        }
    }


    // ------------ Set initial values ------------

    // Normal or node alignment?
    auto desktop = dlg->getDesktop();
    if (desktop) {
        desktop_changed(desktop);
    }
}

void
AlignAndDistribute::desktop_changed(SPDesktop* desktop)
{
    tool_connection.disconnect();
    if (desktop) {
        tool_connection =
            desktop->connectEventContextChanged(sigc::mem_fun(*this, &AlignAndDistribute::tool_changed_callback));
        tool_changed(desktop);
    }
}

void
AlignAndDistribute::tool_changed(SPDesktop* desktop)
{
    bool node = get_active_tool(desktop) == "Node";
    if (node) {
        align_and_distribute_object->hide();
        align_and_distribute_node->show();
    } else {
        align_and_distribute_object->show();
        align_and_distribute_node->hide();
    }
}

void
AlignAndDistribute::tool_changed_callback(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec)
{
    tool_changed(desktop);
}


void
AlignAndDistribute::on_align_as_group_clicked()
{
    bool state = align_move_as_group->get_active();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/dialogs/align/sel-as-groups", state);
}

void
AlignAndDistribute::on_align_relative_object_changed()
{
    int index = align_relative_object->get_active_row_number();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/align/align-to", index);
}

void
AlignAndDistribute::on_align_relative_node_changed()
{
    int index = align_relative_node->get_active_row_number();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/align/align-nodes-to", index);
}

bool
AlignAndDistribute::on_align_button_press_event(GdkEventButton* button_event, const std::string& align_to)
{
    Glib::ustring argument = align_to;

    argument += " " + align_relative_object->get_active_id();

    if (align_move_as_group->get_active()) {
        argument += " group";
    }

    auto variant = Glib::Variant<Glib::ustring>::create(argument);
    auto app = Gio::Application::get_default();
    app->activate_action("object-align", variant);

    return true;
}

bool
AlignAndDistribute::on_remove_overlap_button_press_event(GdkEventButton* button_event)
{
    double hgap = remove_overlap_hgap->get_value();
    double vgap = remove_overlap_vgap->get_value();

    auto variant = Glib::Variant<std::tuple<double, double>>::create(std::tuple<double, double>(hgap, vgap));
    auto app = Gio::Application::get_default();
    app->activate_action("object-remove-overlaps", variant);
    return true;
}

bool
AlignAndDistribute::on_align_node_button_press_event(GdkEventButton* button_event, const std::string& direction)
{
    Glib::ustring argument = align_relative_node->get_active_id();

    auto variant = Glib::Variant<Glib::ustring>::create(argument);
    InkscapeWindow* win = InkscapeApplication::instance()->get_active_window();
    if (direction == "horizontal") {
        win->activate_action("node-align-horizontal", variant);
    } else {
        win->activate_action("node-align-vertical", variant);
    }

    return true;
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
