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

#include "object/sp-namedview.h"

#include "ui/icon-names.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tools/node-tool.h"
#include "ui/widget/ink-select-one-action.h"
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

//####################################
//# node editing callbacks
//####################################

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


static void sp_node_path_edit_nextLPEparam(GtkAction * /*act*/, gpointer data) {
    sp_selection_next_patheffect_param( reinterpret_cast<SPDesktop*>(data) );
}

/* is called when the node selection is modified */
static void sp_node_toolbox_coord_changed(gpointer /*shape_editor*/, GObject *tbl)
{
    GtkAction* xact = GTK_ACTION( g_object_get_data( tbl, "nodes_x_action" ) );
    GtkAction* yact = GTK_ACTION( g_object_get_data( tbl, "nodes_y_action" ) );
    GtkAdjustment *xadj = ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION(xact));
    GtkAdjustment *yadj = ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION(yact));

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>( g_object_get_data( tbl, "tracker" ) );
    if (!tracker) {
        return;
    }
    Unit const *unit = tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    NodeTool *nt = get_node_tool();
    if (!nt || !(nt->_selected_nodes) ||nt->_selected_nodes->empty()) {
        // no path selected
        gtk_action_set_sensitive(xact, FALSE);
        gtk_action_set_sensitive(yact, FALSE);
    } else {
        gtk_action_set_sensitive(xact, TRUE);
        gtk_action_set_sensitive(yact, TRUE);
        Geom::Coord oldx = Quantity::convert(gtk_adjustment_get_value(xadj), unit, "px");
        Geom::Coord oldy = Quantity::convert(gtk_adjustment_get_value(yadj), unit, "px");
        Geom::Point mid = nt->_selected_nodes->pointwiseBounds()->midpoint();

        if (oldx != mid[Geom::X]) {
            gtk_adjustment_set_value(xadj, Quantity::convert(mid[Geom::X], "px", unit));
        }
        if (oldy != mid[Geom::Y]) {
            gtk_adjustment_set_value(yadj, Quantity::convert(mid[Geom::Y], "px", unit));
        }
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_node_path_value_changed(GtkAdjustment *adj, GObject *tbl, Geom::Dim2 d)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    if (!tracker) {
        return;
    }
    Unit const *unit = tracker->getActiveUnit();

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        prefs->setDouble(Glib::ustring("/tools/nodes/") + (d == Geom::X ? "x" : "y"),
            Quantity::convert(gtk_adjustment_get_value(adj), unit, "px"));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" ) || tracker->isUpdating()) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    NodeTool *nt = get_node_tool();
    if (nt && !nt->_selected_nodes->empty()) {
        double val = Quantity::convert(gtk_adjustment_get_value(adj), unit, "px");
        double oldval = nt->_selected_nodes->pointwiseBounds()->midpoint()[d];
        Geom::Point delta(0,0);
        delta[d] = val - oldval;
        nt->_multipath->move(delta);
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_node_path_x_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_node_path_value_changed(adj, tbl, Geom::X);
}

static void sp_node_path_y_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_node_path_value_changed(adj, tbl, Geom::Y);
}

static void sp_node_toolbox_sel_changed(Inkscape::Selection *selection, GObject *tbl)
{
    {
    GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "nodes_lpeedit" ) );
    SPItem *item = selection->singleItem();
    if (item && SP_IS_LPE_ITEM(item)) {
       if (SP_LPE_ITEM(item)->hasPathEffect()) {
           gtk_action_set_sensitive(w, TRUE);
       } else {
           gtk_action_set_sensitive(w, FALSE);
       }
    } else {
       gtk_action_set_sensitive(w, FALSE);
    }
    }
}

static void sp_node_toolbox_sel_modified(Inkscape::Selection *selection, guint /*flags*/, GObject *tbl)
{
    sp_node_toolbox_sel_changed (selection, tbl);
}

static void node_toolbox_watch_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

//################################
//##    Node Editing Toolbox    ##
//################################

#if 0
void sp_node_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowTransformHandlesAction",
                                                      _("Show Transform Handles"),
                                                      _("Show transformation handles for selected nodes"),
                                                      "node-transform",
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/nodes/show_transform_handles");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowHandlesAction",
                                                      _("Show Handles"),
                                                      _("Show Bezier handles of selected nodes"),
                                                      INKSCAPE_ICON("show-node-handles"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/nodes/show_handles");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowHelperpath",
                                                      _("Show Outline"),
                                                      _("Show path outline (without path effects)"),
                                                      INKSCAPE_ICON("show-path-outline"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/nodes/show_outline");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        Inkscape::Verb* verb = Inkscape::Verb::get(SP_VERB_EDIT_NEXT_PATHEFFECT_PARAMETER);
        InkAction* inky = ink_action_new( verb->get_id(),
                                          verb->get_name(),
                                          verb->get_tip(),
                                          INKSCAPE_ICON("path-effect-parameter-next"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_nextLPEparam), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        g_object_set_data( holder, "nodes_lpeedit", inky);
    }

    {
        InkToggleAction* inky = ink_toggle_action_new( "ObjectEditClipPathAction",
                                          _("Edit clipping paths"),
                                          _("Show clipping path(s) of selected object(s)"),
                                          INKSCAPE_ICON("path-clip-edit"),
                                          secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(inky), "/tools/nodes/edit_clipping_paths");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        InkToggleAction* inky = ink_toggle_action_new( "ObjectEditMaskPathAction",
                                          _("Edit masks"),
                                          _("Show mask(s) of selected object(s)"),
                                          INKSCAPE_ICON("path-mask-edit"),
                                          secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(inky), "/tools/nodes/edit_masks");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    /* X coord of selected node(s) */
    {
        EgeAdjustmentAction* eact = 0;
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "NodeXAction",
                                         _("X coordinate:"), _("X:"), _("X coordinate of selected node(s)"),
                                         "/tools/nodes/Xcoord", 0,
                                         GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-nodes",
                                         -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_node_path_x_value_changed, tracker );
        g_object_set_data( holder, "nodes_x_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Y coord of selected node(s) */
    {
        EgeAdjustmentAction* eact = 0;
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "NodeYAction",
                                         _("Y coordinate:"), _("Y:"), _("Y coordinate of selected node(s)"),
                                         "/tools/nodes/Ycoord", 0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_node_path_y_value_changed, tracker );
        g_object_set_data( holder, "nodes_y_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // add the units menu
    {
        InkSelectOneAction* act = tracker->createAction( "NodeUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act->gobj() );
    }

    sp_node_toolbox_sel_changed(desktop->getSelection(), holder);
    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(node_toolbox_watch_ec), holder));

} // end of sp_node_toolbox_prep()
#endif

static void node_toolbox_watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection c_selection_changed;
    static sigc::connection c_selection_modified;
    static sigc::connection c_subselection_changed;

    if (INK_IS_NODE_TOOL(ec)) {
        // watch selection
        c_selection_changed = desktop->getSelection()->connectChanged(sigc::bind(sigc::ptr_fun(sp_node_toolbox_sel_changed), holder));
        c_selection_modified = desktop->getSelection()->connectModified(sigc::bind(sigc::ptr_fun(sp_node_toolbox_sel_modified), holder));
        c_subselection_changed = desktop->connectToolSubselectionChanged(sigc::bind(sigc::ptr_fun(sp_node_toolbox_coord_changed), holder));

        sp_node_toolbox_sel_changed(desktop->getSelection(), holder);
    } else {
        if (c_selection_changed)
            c_selection_changed.disconnect();
        if (c_selection_modified)
            c_selection_modified.disconnect();
        if (c_subselection_changed)
            c_subselection_changed.disconnect();
    }
}

namespace Inkscape {
namespace UI {
namespace Toolbar {

NodeToolbar::NodeToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _tracker(new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR))
{
    auto action_group = Gio::SimpleActionGroup::create();
    insert_action_group("node-toolbar", action_group);

    Unit doc_units = *_desktop->getNamedView()->display_units;
    _tracker->setActiveUnit(&doc_units);

    auto secondarySize = static_cast<Gtk::IconSize>(ToolboxFactory::prefToSize("/toolbox/secondary", 1));

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

    delete_button->set_tooltip_text(_("Delete selected nodes"));
    join_button->set_tooltip_text(_("Join selected nodes"));
    break_button->set_tooltip_text(_("Break path at selected nodes"));
    join_seg_button->set_tooltip_text(_("Join selected endnodes with a new segment"));
    delete_seg_button->set_tooltip_text(_("Delete segment between two non-endpoint nodes"));
    cusp_button->set_tooltip_text(_("Make selected nodes corner"));
    smooth_button->set_tooltip_text(_("Make selected nodes smooth"));
    symmetric_button->set_tooltip_text(_("Make selected nodes symmetric"));
    auto_button->set_tooltip_text(_("Make selected nodes auto-smooth"));
    toline_button->set_tooltip_text(_("Make selected segments lines"));
    tocurve_button->set_tooltip_text(_("Make selected segments curves"));

    delete_button->set_icon_name(INKSCAPE_ICON("node-delete"));
    join_button->set_icon_name(INKSCAPE_ICON("node-join"));
    break_button->set_icon_name(INKSCAPE_ICON("node-break"));
    join_seg_button->set_icon_name(INKSCAPE_ICON("node-join-segment"));
    delete_seg_button->set_icon_name(INKSCAPE_ICON("node-delete-segment"));
    cusp_button->set_icon_name(INKSCAPE_ICON("node-type-cusp"));
    smooth_button->set_icon_name(INKSCAPE_ICON("node-type-smooth"));
    symmetric_button->set_icon_name(INKSCAPE_ICON("node-type-symmetric"));
    auto_button->set_icon_name(INKSCAPE_ICON("node-type-auto-smooth"));
    toline_button->set_icon_name(INKSCAPE_ICON("node-segment-line"));
    tocurve_button->set_icon_name(INKSCAPE_ICON("node-segment-curve"));

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
    show_all();
}

NodeToolbar::~NodeToolbar() {
    if(_tracker) delete _tracker;
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
