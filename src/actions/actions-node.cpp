// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Actions for Node tool present in Node Toolbar
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

#include "actions-node.h"
#include "inkscape-application.h"
#include "ui/tools/node-tool.h"
#include "inkscape.h"
#include "ui/tool/multi-path-manipulator.h"

using Inkscape::UI::Tools::NodeTool;
using Inkscape::UI::Tools::ToolBase;

static NodeTool *get_node_tool()
{
    NodeTool *tool = nullptr;
    if (SP_ACTIVE_DESKTOP ) {
        ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
        if (INK_IS_NODE_TOOL(ec)) {
            tool = static_cast<NodeTool*>(ec);
        }
    }
    return tool;
}

void
node_edit_add(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->insertNodes();
    }
}


void
node_node_delete(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        nt->_multipath->deleteNodes(prefs->getBool("/tools/nodes/delete_preserves_shape", true));
    }
}

void
node_join(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->joinNodes();
    }
}

void
node_break(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_join_segment(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_delete_segment(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_type_cusp(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_type_smooth(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SMOOTH);
    }
}

void
node_type_symmetric(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SYMMETRIC);
    }
}

void
node_type_auto_smooth(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_AUTO);
    }
}

void
node_segment_line(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_STRAIGHT);
    }
}

void
node_segment_curve(InkscapeApplication* app)
{
    NodeTool *nt = get_node_tool();
 
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_CUBIC_BEZIER);
    }
}

void 
node_path_clip_edit(InkscapeApplication* app){

    auto action = app->gio_app()->lookup_action("node-path-clip-edit");
    if (!action) {
        std::cerr << "node_path_clip_edit: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "node_path_clip_edit: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = true;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/nodes/edit_clipping_paths", state);
}

void
node_path_mask_edit(InkscapeApplication* app){

    auto action = app->gio_app()->lookup_action("node-path-mask-edit");
    if (!action) {
        std::cerr << "node_path_mask_edit: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "node_path_mask_edit: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = true;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/nodes/edit_masks", state);
}


void
node_transform(InkscapeApplication* app){

    auto action = app->gio_app()->lookup_action("node-transform");
    if (!action) {
        std::cerr << "node_transform: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "node_transform: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = true;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/nodes/show_transform_handles", state);
}

void
show_node_handles(InkscapeApplication* app){

    auto action = app->gio_app()->lookup_action("show-node-handles");
    if (!action) {
        std::cerr << "show_node_handles: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "show_node_handles: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = true;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/nodes/show_handles", state);
}

void
show_path_outline(InkscapeApplication* app){

    auto action = app->gio_app()->lookup_action("show-path-outline");
    if (!action) {
        std::cerr << "show_path_outline: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "show_path_outline: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = true;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    auto prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/nodes/show_outline", state);
}

std::vector<std::vector<Glib::ustring>> raw_node_data =
{
    // clang-format off
    {"app.node-edit-add",           N_("Insert node"),              "Node Toolbar",        N_("Insert new nodes into selected segments")},
    {"app.node-node-delete",        N_("Delete node"),              "Node Toolbar",        N_("Delete selected nodes")},
    {"app.node-join",               N_("Join nodes"),               "Node Toolbar",        N_("Join selected nodes")},
    {"app.node-break",              N_("Break nodes"),              "Node Toolbar",        N_("Break path at selected nodes")},
    {"app.node-join-segment",       N_("Join with segment"),        "Node Toolbar",        N_("Join selected endnodes with a new segment")},
    {"app.node-delete-segment",     N_("Delete segment"),           "Node Toolbar",        N_("Delete segment between two non-endpoint nodes")},
    {"app.node-type-cusp",          N_("Node Cusp"),                "Node Toolbar",        N_("Make selected nodes corner")},
    {"app.node-type-smooth",        N_("Node Smooth"),              "Node Toolbar",        N_("Make selected nodes smooth")},
    {"app.node-type-symmetric",     N_("Node Symmetric"),           "Node Toolbar",        N_("Make selected nodes symmetric")},
    {"app.node-type-auto-smooth",   N_("Node Auto"),                "Node Toolbar",        N_("Make selected nodes auto-smooth")},
    {"app.node-segment-line",       N_("Node Line"),                "Node Toolbar",        N_("Make selected segments lines")},
    {"app.node-segment-curve",      N_("Node Curve"),               "Node Toolbar",        N_("Make selected segments curves")},
    {"app.node-path-clip-edit",     N_("Edit clipping paths"),      "Node Toolbar",        N_("Show clipping path(s) of selected object(s)")},
    {"app.node-path-mask-edit",     N_("Edit masks"),               "Node Toolbar",        N_("Show mask(s) of selected object(s)")},
    {"app.node-transform",          N_("Show Transform Handles"),   "Node Toolbar",        N_("Show transformation handles for selected nodes")},
    {"app.show-node-handles",       N_("Show Handles"),             "Node Toolbar",        N_("Show Bezier handles of selected nodes")},
    {"app.show-path-outline",       N_("Show Outline"),             "Node Toolbar",        N_("Show path outline (without path effects)")}
    // clang-format on
};

void
add_actions_node(InkscapeApplication* app)
{
    auto *gapp = app->gio_app();

    // clang-format off
    gapp->add_action(      "node-edit-add",              sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_edit_add),            app));
    gapp->add_action(      "node-node-delete",           sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_node_delete),         app));
    gapp->add_action(      "node-join",                  sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_join),                app));
    gapp->add_action(      "node-break",                 sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_break),               app));
    gapp->add_action(      "node-join-segment",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_join_segment),        app));
    gapp->add_action(      "node-delete-segment",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_delete_segment),      app));
    gapp->add_action(      "node-type-cusp",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_type_cusp),           app));
    gapp->add_action(      "node-type-smooth",           sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_type_smooth),         app));
    gapp->add_action(      "node-type-symmetric",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_type_symmetric),      app));
    gapp->add_action(      "node-type-auto-smooth",      sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_type_auto_smooth),    app));
    gapp->add_action(      "node-segment-line",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_segment_line),        app));
    gapp->add_action(      "node-segment-curve",         sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_segment_curve),       app));
    gapp->add_action_bool( "node-path-clip-edit",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_path_clip_edit),      app));
    gapp->add_action_bool( "node-path-mask-edit",        sigc::bind<InkscapeApplication*>(sigc::ptr_fun(node_path_mask_edit),       app));
    gapp->add_action_bool( "node-transform",             sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&node_transform),           app));
    gapp->add_action_bool( "show-node-handles",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&show_node_handles),        app));
    gapp->add_action_bool( "show-path-outline",          sigc::bind<InkscapeApplication*>(sigc::ptr_fun(&show_path_outline),        app));
    // clang-format on

    // auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_node: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_node_data);
}