#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>
#include "actions-node.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
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
node_edit_add(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodes();
    }
}


void
node_node_delete(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        nt->_multipath->deleteNodes(prefs->getBool("/tools/nodes/delete_preserves_shape", true));
    }
}

void
node_join(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinNodes();
    }
}

void
node_break(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_join_segment(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_delete_segment(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_type_cusp(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
node_type_smooth(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SMOOTH);
    }
}

void
node_type_symmetric(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SYMMETRIC);
    }
}

void
node_type_auto_smooth(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_AUTO);
    }
}

void
node_segment_line(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_STRAIGHT);
    }
}

void
node_segment_curve(InkscapeWindow* win)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_CUBIC_BEZIER);
    }
}

void 
node_path_clip_edit(InkscapeWindow* win){

    auto action = win->lookup_action("node-path-clip-edit");
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
node_path_mask_edit(InkscapeWindow* win){

    auto action = win->lookup_action("node-path-mask-edit");
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
node_transform(InkscapeWindow* win){

    auto action = win->lookup_action("node-transform");
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
show_node_handles(InkscapeWindow* win){

    auto action = win->lookup_action("show-node-handles");
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
show_path_outline(InkscapeWindow* win){

    auto action = win->lookup_action("show-path-outline");
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
    /* Should be done without desktop ? */

    // clang-format off
    {"win.node-edit-add",           N_("Insert node"),              "Node Toolbar",        N_("Insert new nodes into selected segments")},
    {"win.node-node-delete",        N_("Delete node"),              "Node Toolbar",        N_("Delete selected nodes")},
    {"win.node-join",               N_("Join nodes"),               "Node Toolbar",        N_("Join selected nodes")},
    {"win.node-break",              N_("Break nodes"),              "Node Toolbar",        N_("Break path at selected nodes")},
    {"win.node-join-segment",       N_("Join with segment"),        "Node Toolbar",        N_("Join selected endnodes with a new segment")},
    {"win.node-delete-segment",     N_("Delete segment"),           "Node Toolbar",        N_("Delete segment between two non-endpoint nodes")},
    {"win.node-type-cusp",          N_("Node Cusp"),                "Node Toolbar",        N_("Make selected nodes corner")},
    {"win.node-type-smooth",        N_("Node Smooth"),              "Node Toolbar",        N_("Make selected nodes smooth")},
    {"win.node-type-symmetric",     N_("Node Symmetric"),           "Node Toolbar",        N_("Make selected nodes symmetric")},
    {"win.node-type-auto-smooth",   N_("Node Auto"),                "Node Toolbar",        N_("Make selected nodes auto-smooth")},
    {"win.node-segment-line",       N_("Node Line"),                "Node Toolbar",        N_("Make selected segments lines")},
    {"win.node-segment-curve",      N_("Node Curve"),               "Node Toolbar",        N_("Make selected segments curves")},
    {"win.node-path-clip-edit",     N_("Edit clipping paths"),      "Node Toolbar",        N_("Show clipping path(s) of selected object(s)")},
    {"win.node-path-mask-edit",     N_("Edit masks"),               "Node Toolbar",        N_("Show mask(s) of selected object(s)")},
    {"win.node-transform",          N_("Show Transform Handles"),   "Node Toolbar",        N_("Show transformation handles for selected nodes")},
    {"win.show-node-handles",       N_("Show Handles"),             "Node Toolbar",        N_("Show Bezier handles of selected nodes")},
    {"win.show-path-outline",       N_("Show Outline"),             "Node Toolbar",        N_("Show path outline (without path effects)")}
    // clang-format on
};

void
add_actions_node(InkscapeWindow* win)
{
    // clang-format off
    win->add_action(      "node-edit-add",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_edit_add), win));
    win->add_action(      "node-node-delete",           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_node_delete), win));
    win->add_action(      "node-join",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_join), win));
    win->add_action(      "node-break",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_break), win));
    win->add_action(      "node-join-segment",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_join_segment), win));
    win->add_action(      "node-delete-segment",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_delete_segment), win));
    win->add_action(      "node-type-cusp",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_type_cusp), win));
    win->add_action(      "node-type-smooth",           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_type_smooth), win));
    win->add_action(      "node-type-symmetric",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_type_symmetric), win));
    win->add_action(      "node-type-auto-smooth",      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_type_auto_smooth), win));
    win->add_action(      "node-segment-line",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_segment_line), win));
    win->add_action(      "node-segment-curve",         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_segment_curve), win));
    win->add_action_bool( "node-path-clip-edit",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_path_clip_edit), win));
    win->add_action_bool( "node-path-mask-edit",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_path_mask_edit), win));
    win->add_action_bool( "node-transform",             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&node_transform), win));
    win->add_action_bool( "show-node-handles",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&show_node_handles), win));
    win->add_action_bool( "show-path-outline",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&show_path_outline), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_node: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_node_data);
}