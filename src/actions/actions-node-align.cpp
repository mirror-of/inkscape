// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for aligning and distributing objects without GUI.
 *
 * Copyright (C) 2020 Tavmjong Bah
 *
 * Some code and ideas from src/ui/dialogs/align-and-distribute.cpp
 *   Authors: Bryce Harrington
 *            Martin Owens
 *            John Smith
 *            Patrick Storz
 *            Jabier Arraiza
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 * To do: Remove GUI dependency!
 */

#include "actions-node-align.h"

#include <iostream>
#include <limits>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include <2geom/coord.h>

#include "inkscape-application.h"
#include "inkscape-window.h"
#include "ui/tool/multi-path-manipulator.h" // Node align/distribute
#include "ui/tools/node-tool.h"             // Node align/distribute

enum class NodeAlignTarget {
    LAST,
    FIRST,
    MIDDLE,
    MIN,
    MAX
};

void
node_align(const Glib::VariantBase& value, InkscapeWindow* win, Geom::Dim2 direction)
{
    auto tool = win->get_desktop()->getEventContext();
    auto node_tool = dynamic_cast<Inkscape::UI::Tools::NodeTool*>(tool);
    if (node_tool) {
    } else {
        std::cerr << "node_align: tool is not Node tool!" << std::endl;
        return;
    }

    Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring> >(value);
    std::vector<Glib::ustring> tokens = Glib::Regex::split_simple(" ", s.get());
    std::cout << "node_align: " << s.get() << std::endl;
    if (tokens.size() > 1) {
        std::cerr << "node_align: too many arguments!" << std::endl;
        return;
    }

    // clang-format off
    auto target = NodeAlignTarget::MIDDLE;
    if (tokens.size() == 1) {
        if      (tokens[0] == "last"   ) target = NodeAlignTarget::LAST;
        else if (tokens[0] == "first"  ) target = NodeAlignTarget::FIRST;
        else if (tokens[0] == "middle" ) target = NodeAlignTarget::MIDDLE;
        else if (tokens[0] == "min"    ) target = NodeAlignTarget::MIN;
        else if (tokens[0] == "max"    ) target = NodeAlignTarget::MAX;
    }
    // clang-format on

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/align/align-nodes-to", (int)target); // NASTY!! But ControlPointSelection::align uses pref!

    node_tool->_multipath->alignNodes(direction);
} 

void
node_distribute(InkscapeWindow* win, Geom::Dim2 direction)
{
    auto tool = win->get_desktop()->getEventContext();
    auto node_tool = dynamic_cast<Inkscape::UI::Tools::NodeTool*>(tool);
    if (node_tool) {
    } else {
        std::cerr << "node_distribute: tool is not Node tool!" << std::endl;
        return;
    }

    node_tool->_multipath->distributeNodes(direction);
} 

std::vector<std::vector<Glib::ustring>> raw_data_node_align =
{
    // clang-format off
    {"win.node-align-horizontal",       N_("Align nodes horizontally"),      "Node", N_("Align selected nodes horizontally; usage [last|first|middle|min|max]" )},
    {"win.node-align-vertical",         N_("Align nodes vertically"),        "Node", N_("Align selected nodes vertically; usage [last|first|middle|min|max]"   )},
    {"win.node-distribute-horizontal",  N_("Distribute nodes horizontally"), "Node", N_("Distribute selected nodes horizontally."                              )},
    {"win.node-distribute-vertical",    N_("Distribute nodes vertically"),   "Node", N_("Distribute selected nodes vertically."                                )}
    // clang-format on
};

std::vector<std::vector<Glib::ustring>> hint_data_node_align =
{
    // clang-format off
    {"app.node-align-horizontal",      N_("Enter string for alignment anchor, one of: first/last/middle/min/max")},
    {"app.node-align-vertical",        N_("Enter string for alignment anchor, one of: first/last/middle/min/max")},
    // clang-format on
};

// These are window actions as the require the node tool to be active and nodes to be selected.
void
add_actions_node_align(InkscapeWindow* win)
{
    Glib::VariantType String(Glib::VARIANT_TYPE_STRING);

    // clang-format off
    win->add_action_with_parameter( "node-align-horizontal",      String, sigc::bind<InkscapeWindow*, Geom::Dim2>(sigc::ptr_fun(&node_align),      win, Geom::X));
    win->add_action_with_parameter( "node-align-vertical",        String, sigc::bind<InkscapeWindow*, Geom::Dim2>(sigc::ptr_fun(&node_align),      win, Geom::Y));
    win->add_action(                "node-distribute-horizontal",         sigc::bind<InkscapeWindow*, Geom::Dim2>(sigc::ptr_fun(&node_distribute), win, Geom::X));
    win->add_action(                "node-distribute-vertical",           sigc::bind<InkscapeWindow*, Geom::Dim2>(sigc::ptr_fun(&node_distribute), win, Geom::Y));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_node_align: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_node_align);
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
