/**
 * @file
 * Node aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <giomm/simpleactiongroup.h>

#include <glibmm/i18n.h>

#include <gtkmm/image.h>
#include <gtkmm/menutoolbutton.h>
#include <gtkmm/separatortoolitem.h>

#include "desktop.h"
#include "document-undo.h"
#include "ink-toggle-action.h"
#include "inkscape.h"
#include "node-toolbar.h"
#include "selection-chemistry.h"
#include "toolbox.h"
#include "verbs.h"

#include "helper/action.h"

#include "object/sp-namedview.h"

#include "ui/icon-names.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tools/node-tool.h"
#include "ui/widget/ink-select-one-action.h"
#include "ui/widget/spin-button-tool-item.h"
#include "ui/widget/unit-tracker.h"

#include "widgets/ege-adjustment-action.h"
#include "widgets/widget-sizes.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::Util::Unit;
using Inkscape::Util::Quantity;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;
using Inkscape::Util::unit_table;
using Inkscape::UI::Tools::NodeTool;

/** Temporary hack: Returns the node tool in the active desktop.
 * Will go away during tool refactoring. */
static NodeTool *get_node_tool()
{
    NodeTool *tool = 0;
    if (SP_ACTIVE_DESKTOP ) {
        Inkscape::UI::Tools::ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
        if (INK_IS_NODE_TOOL(ec)) {
            tool = static_cast<NodeTool*>(ec);
        }
    }
    return tool;
}

namespace Inkscape {
namespace UI {
namespace Toolbar {

NodeToolbar::NodeToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _tracker(new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR)),
      _edit_clip_path_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _edit_mask_path_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _show_transform_handles_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _show_handles_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _show_helper_path_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _freeze_flag(false)
{
    auto action_group = Gio::SimpleActionGroup::create();
    insert_action_group("node-toolbar", action_group);

    Unit doc_units = *_desktop->getNamedView()->display_units;
    _tracker->setActiveUnit(&doc_units);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    auto secondarySize = static_cast<Gtk::IconSize>(ToolboxFactory::prefToSize("/toolbox/secondary", 1));

    // Define actions that are specific to this toolbar
    action_group->add_action("insert-node-min-x", sigc::mem_fun(*this, &NodeToolbar::on_insert_node_min_x_activated));
    action_group->add_action("insert-node-max-x", sigc::mem_fun(*this, &NodeToolbar::on_insert_node_max_x_activated));
    action_group->add_action("insert-node-min-y", sigc::mem_fun(*this, &NodeToolbar::on_insert_node_min_y_activated));
    action_group->add_action("insert-node-max-y", sigc::mem_fun(*this, &NodeToolbar::on_insert_node_max_y_activated));
    action_group->add_action("delete",            sigc::mem_fun(*this, &NodeToolbar::on_delete_activated));
    action_group->add_action("join",              sigc::mem_fun(*this, &NodeToolbar::on_join_activated));
    action_group->add_action("break",             sigc::mem_fun(*this, &NodeToolbar::on_break_activated));
    action_group->add_action("join-segment",      sigc::mem_fun(*this, &NodeToolbar::on_join_segment_activated));
    action_group->add_action("delete-segment",    sigc::mem_fun(*this, &NodeToolbar::on_delete_segment_activated));
    action_group->add_action("cusp",              sigc::mem_fun(*this, &NodeToolbar::on_cusp_activated));
    action_group->add_action("smooth",            sigc::mem_fun(*this, &NodeToolbar::on_smooth_activated));
    action_group->add_action("symmetrical",       sigc::mem_fun(*this, &NodeToolbar::on_symmetrical_activated));
    action_group->add_action("auto",              sigc::mem_fun(*this, &NodeToolbar::on_auto_activated));
    action_group->add_action("toline",            sigc::mem_fun(*this, &NodeToolbar::on_toline_activated));
    action_group->add_action("tocurve",           sigc::mem_fun(*this, &NodeToolbar::on_tocurve_activated));

    // Create simple button widgets
    auto delete_button     = Gtk::manage(new Gtk::ToolButton(_("Delete node")));
    auto join_button       = Gtk::manage(new Gtk::ToolButton(_("Join node")));
    auto break_button      = Gtk::manage(new Gtk::ToolButton(_("Break nodes")));
    auto join_seg_button   = Gtk::manage(new Gtk::ToolButton(_("Join with segment")));
    auto delete_seg_button = Gtk::manage(new Gtk::ToolButton(_("Delete segment")));
    auto cusp_button       = Gtk::manage(new Gtk::ToolButton(_("Node Cusp")));
    auto smooth_button     = Gtk::manage(new Gtk::ToolButton(_("Node Smooth")));
    auto symmetric_button  = Gtk::manage(new Gtk::ToolButton(_("Node Symmetric")));
    auto auto_button       = Gtk::manage(new Gtk::ToolButton(_("Node Auto")));
    auto toline_button     = Gtk::manage(new Gtk::ToolButton(_("Node Line")));
    auto tocurve_button    = Gtk::manage(new Gtk::ToolButton(_("Node Curve")));

    // Grab more toolbuttons from Inkscape verb definitions (this also sets widget appearance)
    auto context = Inkscape::ActionContext(_desktop);
    auto objecttopath_button = SPAction::create_toolbutton_for_verb(SP_VERB_OBJECT_TO_CURVE, context);
    auto stroketopath_button = SPAction::create_toolbutton_for_verb(SP_VERB_SELECTION_OUTLINE, context);
    _next_pe_param_button  = SPAction::create_toolbutton_for_verb(SP_VERB_EDIT_NEXT_PATHEFFECT_PARAMETER, context);

    // Create adjustments and associated spinbuttons
    auto x_val = prefs->getDouble("/tools/nodes/Xcoord", 0.0);
    auto y_val = prefs->getDouble("/tools/nodes/Ycoord", 0.0);

    _x_coord_adj = Gtk::Adjustment::create(x_val, -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _y_coord_adj = Gtk::Adjustment::create(y_val, -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _tracker->addAdjustment(_x_coord_adj->gobj());
    _tracker->addAdjustment(_y_coord_adj->gobj());

    _x_coord_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(_("X:"), _x_coord_adj, 0.1, 3));
    _y_coord_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(_("Y:"), _y_coord_adj, 0.1, 3));
    _x_coord_btn->set_sensitive(false);
    _y_coord_btn->set_sensitive(false);
    _x_coord_btn->set_all_tooltip_text(_("X coordinate of selected node(s)"));
    _y_coord_btn->set_all_tooltip_text(_("Y coordinate of selected node(s)"));
    _x_coord_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    _y_coord_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    // Handle value changes on adjustment
    auto x_coord_adj_value_changed_cb = sigc::mem_fun(*this, &NodeToolbar::on_x_coord_adj_value_changed);
    auto y_coord_adj_value_changed_cb = sigc::mem_fun(*this, &NodeToolbar::on_y_coord_adj_value_changed);

    _x_coord_adj->signal_value_changed().connect(x_coord_adj_value_changed_cb);
    _y_coord_adj->signal_value_changed().connect(y_coord_adj_value_changed_cb);

    auto unit_menu = _tracker->createAction("NodeUnitsAction", _("Units"), (""));
    auto unit_menu_ti = unit_menu->create_tool_item();

    // Set appearance for buttons that weren't created from verbs
    delete_button->set_tooltip_text(_("Delete selected nodes"));
    delete_button->set_icon_name(INKSCAPE_ICON("node-delete"));

    join_button->set_tooltip_text(_("Join selected nodes"));
    join_button->set_icon_name(INKSCAPE_ICON("node-join"));

    break_button->set_tooltip_text(_("Break path at selected nodes"));
    break_button->set_icon_name(INKSCAPE_ICON("node-break"));

    join_seg_button->set_tooltip_text(_("Join selected endnodes with a new segment"));
    join_seg_button->set_icon_name(INKSCAPE_ICON("node-join-segment"));

    delete_seg_button->set_tooltip_text(_("Delete segment between two non-endpoint nodes"));
    delete_seg_button->set_icon_name(INKSCAPE_ICON("node-delete-segment"));

    cusp_button->set_tooltip_text(_("Make selected nodes corner"));
    cusp_button->set_icon_name(INKSCAPE_ICON("node-type-cusp"));

    smooth_button->set_tooltip_text(_("Make selected nodes smooth"));
    smooth_button->set_icon_name(INKSCAPE_ICON("node-type-smooth"));

    symmetric_button->set_tooltip_text(_("Make selected nodes symmetric"));
    symmetric_button->set_icon_name(INKSCAPE_ICON("node-type-symmetric"));

    auto_button->set_tooltip_text(_("Make selected nodes auto-smooth"));
    auto_button->set_icon_name(INKSCAPE_ICON("node-type-auto-smooth"));

    toline_button->set_tooltip_text(_("Make selected segments lines"));
    toline_button->set_icon_name(INKSCAPE_ICON("node-segment-line"));

    tocurve_button->set_tooltip_text(_("Make selected segments curves"));
    tocurve_button->set_icon_name(INKSCAPE_ICON("node-segment-curve"));

    // Hook up actions to the buttons that weren't created from verbs
    gtk_actionable_set_action_name(GTK_ACTIONABLE(delete_button->gobj()),     "node-toolbar.delete");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(join_button->gobj()),       "node-toolbar.join");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(break_button->gobj()),      "node-toolbar.break");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(join_seg_button->gobj()),   "node-toolbar.join-segment");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(delete_seg_button->gobj()), "node-toolbar.delete-segment");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(cusp_button->gobj()),       "node-toolbar.cusp");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(smooth_button->gobj()),     "node-toolbar.smooth");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(symmetric_button->gobj()),  "node-toolbar.symmetrical");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(auto_button->gobj()),       "node-toolbar.auto");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(toline_button->gobj()),     "node-toolbar.toline");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(tocurve_button->gobj()),    "node-toolbar.tocurve");

    // Set up toggle buttons
    _edit_clip_path_button->set_label(_("Edit clipping paths"));
    _edit_clip_path_button->set_icon_name(INKSCAPE_ICON("path-clip-edit"));
    _edit_clip_path_button->set_tooltip_text(_("Show clipping path(s) of selected object(s)"));
    _edit_clip_pusher = new PrefPusher(_edit_clip_path_button, "/tools/nodes/edit_clipping_paths");

    _edit_mask_path_button->set_label(_("Edit masks"));
    _edit_mask_path_button->set_icon_name(INKSCAPE_ICON("path-mask-edit"));
    _edit_mask_path_button->set_tooltip_text(_("Show mask(s) of selected object(s)"));
    _edit_mask_pusher = new PrefPusher(_edit_mask_path_button, "/tools/nodes/edit_masks");

    _show_transform_handles_button->set_label(_("Show Transform Handles"));
    _show_transform_handles_button->set_icon_name(INKSCAPE_ICON("node-transform"));
    _show_transform_handles_button->set_tooltip_text(_("Show transformation handles for selected nodes"));
    _show_transform_handles_pusher = new PrefPusher(_show_transform_handles_button, "/tools/nodes/show_transform_handles");

    _show_handles_button->set_label(_("Show Handles"));
    _show_handles_button->set_icon_name(INKSCAPE_ICON("show-node-handles"));
    _show_handles_button->set_tooltip_text(_("Show Bezier handles of selected nodes"));
    _show_handles_pusher = new PrefPusher(_show_handles_button, "/tools/nodes/show_handles");

    _show_helper_path_button->set_label(_("Show Outline"));
    _show_helper_path_button->set_icon_name(INKSCAPE_ICON("show-path-outline"));
    _show_helper_path_button->set_tooltip_text(_("Show path outline (without path effects)"));
    _show_helper_path_pusher = new PrefPusher(_show_helper_path_button, "/tools/nodes/show_outline");

    // Pack tool items into toolbar
    create_insert_node_button();
    add(*delete_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*join_button);
    add(*break_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*join_seg_button);
    add(*delete_seg_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*cusp_button);
    add(*smooth_button);
    add(*symmetric_button);
    add(*auto_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*toline_button);
    add(*tocurve_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*objecttopath_button);
    add(*stroketopath_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*_x_coord_btn);
    add(*_y_coord_btn);
    add(*unit_menu_ti);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*_edit_clip_path_button);
    add(*_edit_mask_path_button);
    add(*_next_pe_param_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*_show_transform_handles_button);
    add(*_show_handles_button);
    add(*_show_helper_path_button);
    show_all();

    selection_changed(_desktop->getSelection());
    _desktop->connectEventContextChanged(sigc::mem_fun(*this, &NodeToolbar::watch_ec));
}

NodeToolbar::~NodeToolbar() {
    if(_tracker)          delete _tracker;
    if(_edit_clip_pusher) delete _edit_clip_pusher;
    if(_edit_mask_pusher) delete _edit_mask_pusher;
    if(_show_transform_handles_pusher) delete _show_transform_handles_pusher;
    if(_show_handles_pusher) delete _show_handles_pusher;
    if(_show_helper_path_pusher) delete _show_helper_path_pusher;
}

GtkWidget *
NodeToolbar::create(SPDesktop *desktop)
{
    auto toolbar = Gtk::manage(new NodeToolbar(desktop));
    return GTK_WIDGET(toolbar->gobj());
}

void
NodeToolbar::on_insert_node_button_clicked()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodes();
    }
}

void
NodeToolbar::on_insert_node_min_x_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MIN_X);
    }
}

void
NodeToolbar::on_insert_node_max_x_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MAX_X);
    }
}

void
NodeToolbar::on_insert_node_min_y_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MIN_Y);
    }
}

void
NodeToolbar::on_insert_node_max_y_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MAX_Y);
    }
}

/**
 * Create the "Insert node" button and associated drop-down menu
 */
void
NodeToolbar::create_insert_node_button()
{
    auto insert_node_button_clicked_cb = sigc::mem_fun(*this, &NodeToolbar::on_insert_node_button_clicked);

    auto insert_node_button = Gtk::manage(new Gtk::MenuToolButton());
    insert_node_button->set_icon_name(INKSCAPE_ICON("node-add"));
    insert_node_button->set_label(_("Insert node"));
    insert_node_button->set_tooltip_text(_("Insert new nodes into selected segments"));
    insert_node_button->signal_clicked().connect(insert_node_button_clicked_cb);

    auto insert_node_min_x_icon = Gtk::manage(new Gtk::Image());
    auto insert_node_max_x_icon = Gtk::manage(new Gtk::Image());
    auto insert_node_min_y_icon = Gtk::manage(new Gtk::Image());
    auto insert_node_max_y_icon = Gtk::manage(new Gtk::Image());

    insert_node_min_x_icon->set_from_icon_name(INKSCAPE_ICON("node_insert_min_x"), Gtk::ICON_SIZE_LARGE_TOOLBAR);
    insert_node_max_x_icon->set_from_icon_name(INKSCAPE_ICON("node_insert_max_x"), Gtk::ICON_SIZE_LARGE_TOOLBAR);
    insert_node_min_y_icon->set_from_icon_name(INKSCAPE_ICON("node_insert_min_y"), Gtk::ICON_SIZE_LARGE_TOOLBAR);
    insert_node_max_y_icon->set_from_icon_name(INKSCAPE_ICON("node_insert_max_y"), Gtk::ICON_SIZE_LARGE_TOOLBAR);

    auto insert_node_min_x_item = Gtk::manage(new Gtk::MenuItem(*insert_node_min_x_icon));
    auto insert_node_max_x_item = Gtk::manage(new Gtk::MenuItem(*insert_node_max_x_icon));
    auto insert_node_min_y_item = Gtk::manage(new Gtk::MenuItem(*insert_node_min_y_icon));
    auto insert_node_max_y_item = Gtk::manage(new Gtk::MenuItem(*insert_node_max_y_icon));

    insert_node_min_x_item->set_tooltip_text(_("Insert new nodes at min X into selected segments"));
    insert_node_max_x_item->set_tooltip_text(_("Insert new nodes at max X into selected segments"));
    insert_node_min_y_item->set_tooltip_text(_("Insert new nodes at min Y into selected segments"));
    insert_node_max_y_item->set_tooltip_text(_("Insert new nodes at max Y into selected segments"));

    gtk_actionable_set_action_name(GTK_ACTIONABLE(insert_node_min_x_item->gobj()),"node-toolbar.insert-node-min-x");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(insert_node_max_x_item->gobj()),"node-toolbar.insert-node-max-x");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(insert_node_min_y_item->gobj()),"node-toolbar.insert-node-min-y");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(insert_node_max_y_item->gobj()),"node-toolbar.insert-node-max-y");

    // Pack items into node-insert menu
    auto insert_node_menu = Gtk::manage(new Gtk::Menu());
    insert_node_menu->append(*insert_node_min_x_item);
    insert_node_menu->append(*insert_node_max_x_item);
    insert_node_menu->append(*insert_node_min_y_item);
    insert_node_menu->append(*insert_node_max_y_item);
    insert_node_menu->show_all();

    insert_node_button->set_menu(*insert_node_menu);
    add(*insert_node_button);
}

void
NodeToolbar::on_delete_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        nt->_multipath->deleteNodes(prefs->getBool("/tools/nodes/delete_preserves_shape", true));
    }
}

void
NodeToolbar::on_join_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinNodes();
    }
}

void
NodeToolbar::on_break_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

void
NodeToolbar::on_join_segment_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinSegments();
    }
}

void
NodeToolbar::on_delete_segment_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->deleteSegments();
    }
}

void
NodeToolbar::on_toline_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_STRAIGHT);
    }
}

void
NodeToolbar::on_tocurve_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_CUBIC_BEZIER);
    }
}

void
NodeToolbar::on_cusp_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_CUSP);
    }
}

void
NodeToolbar::on_smooth_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SMOOTH);
    }
}

void
NodeToolbar::on_symmetrical_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SYMMETRIC);
    }
}

void
NodeToolbar::on_auto_activated()
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_AUTO);
    }
}

void
NodeToolbar::on_x_coord_adj_value_changed()
{
    path_value_changed(Geom::X);
}

void
NodeToolbar::on_y_coord_adj_value_changed()
{
    path_value_changed(Geom::Y);
}

void
NodeToolbar::path_value_changed(Geom::Dim2 d)
{
    // Get either the x or y adjustment depending on the dimension argument
    auto adj = (d == Geom::X ? _x_coord_adj : _y_coord_adj);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (!_tracker) {
        return;
    }

    Unit const *unit = _tracker->getActiveUnit();

    if (DocumentUndo::getUndoSensitive(_desktop->getDocument())) {
        prefs->setDouble(Glib::ustring("/tools/nodes/") + (d == Geom::X ? "x" : "y"),
            Quantity::convert(adj->get_value(), unit, "px"));
    }

    // quit if run by the attr_changed listener
    if (_freeze_flag || _tracker->isUpdating()) {
        return;
    }

    // in turn, prevent listener from responding
    _freeze_flag = true;

    NodeTool *nt = get_node_tool();
    if (nt && !nt->_selected_nodes->empty()) {
        double val = Quantity::convert(adj->get_value(), unit, "px");
        double oldval = nt->_selected_nodes->pointwiseBounds()->midpoint()[d];
        Geom::Point delta(0,0);
        delta[d] = val - oldval;
        nt->_multipath->move(delta);
    }

    _freeze_flag = false;
}

void
NodeToolbar::selection_changed(Inkscape::Selection *selection)
{
    auto item = selection->singleItem();

    if (item && SP_IS_LPE_ITEM(item)) {
       if (SP_LPE_ITEM(item)->hasPathEffect()) {
           _next_pe_param_button->set_sensitive(true);
       } else {
           _next_pe_param_button->set_sensitive(false);
       }
    } else {
        _next_pe_param_button->set_sensitive(false);
    }
}

void
NodeToolbar::selection_modified(Inkscape::Selection *selection, guint /*flags*/)
{
    selection_changed(selection);
}

void
NodeToolbar::watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec)
{
    static sigc::connection c_selection_changed;
    static sigc::connection c_selection_modified;
    static sigc::connection c_subselection_changed;

    if (INK_IS_NODE_TOOL(ec)) {
        // watch selection
        c_selection_changed = desktop->getSelection()->connectChanged(sigc::mem_fun(*this, &NodeToolbar::selection_changed));
        c_selection_modified = desktop->getSelection()->connectModified(sigc::mem_fun(*this, &NodeToolbar::selection_modified));
        c_subselection_changed = desktop->connectToolSubselectionChanged(sigc::mem_fun(*this, &NodeToolbar::coord_changed));

        selection_changed(desktop->getSelection());
    } else {
        if (c_selection_changed)
            c_selection_changed.disconnect();
        if (c_selection_modified)
            c_selection_modified.disconnect();
        if (c_subselection_changed)
            c_subselection_changed.disconnect();
    }
}

/* is called when the node selection is modified */
void
NodeToolbar::coord_changed(gpointer /*shape_editor*/)
{
    // quit if run by the attr_changed listener
    if (_freeze_flag) {
        return;
    }

    // in turn, prevent listener from responding
    _freeze_flag = true;

    if (!_tracker) {
        return;
    }

    Unit const *unit = _tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    NodeTool *nt = get_node_tool();
    if (!nt || !(nt->_selected_nodes) ||nt->_selected_nodes->empty()) {
        // no path selected
        _x_coord_btn->set_sensitive(false);
        _y_coord_btn->set_sensitive(false);
    } else {
        _x_coord_btn->set_sensitive(true);
        _y_coord_btn->set_sensitive(true);
        Geom::Coord oldx = Quantity::convert(_x_coord_adj->get_value(), unit, "px");
        Geom::Coord oldy = Quantity::convert(_y_coord_adj->get_value(), unit, "px");
        Geom::Point mid = nt->_selected_nodes->pointwiseBounds()->midpoint();

        if (oldx != mid[Geom::X]) {
            _x_coord_adj->set_value(Quantity::convert(mid[Geom::X], "px", unit));
        }
        if (oldy != mid[Geom::Y]) {
            _y_coord_adj->set_value(Quantity::convert(mid[Geom::Y], "px", unit));
        }
    }

    _freeze_flag = false;
}

}
}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
