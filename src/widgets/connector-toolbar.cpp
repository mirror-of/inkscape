/**
 * @file
 * Connector aux toolbar
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

#include <gtkmm.h>
#include <glibmm.h>
#include <glibmm/i18n.h>

#include "connector-toolbar.h"
#include "conn-avoid-ref.h"

#include "desktop.h"
#include "document-undo.h"
#include "enums.h"
#include "graphlayout.h"
#include "ink-toggle-action.h"
#include "inkscape.h"
#include "toolbox.h"
#include "verbs.h"

#include "object/sp-namedview.h"
#include "object/sp-path.h"

#include "ui/icon-names.h"
#include "ui/tools/connector-tool.h"
#include "ui/uxmanager.h"

#include "widgets/ege-adjustment-action.h"
#include "widgets/spinbutton-events.h"

#include "xml/node-event-vector.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//#########################
//##      Connector      ##
//#########################

/**
 * @brief List of all the tool items
 * @todo  This is mainly provided for quick access to children of the
 *        toolbar.  It probably won't be needed if we make the toolitems
 *        members of a ConnectorToolbar class in C++ as we'd be able
 *        to access them directly rather than by index
 */
enum ConnectorToolItemIndex {
    INDEX_AVOID      = 1,
    INDEX_IGNORE     = 2,
    INDEX_ORTHOGONAL = 3,
    INDEX_SEPARATOR  = 4
};

static void sp_connector_path_set_avoid(GSimpleAction *action,
                                        GVariant      *parameter,
                                        gpointer       user_data)
{
    Inkscape::UI::Tools::cc_selection_set_avoid(true);
}


static void sp_connector_path_set_ignore(GSimpleAction *action,
                                         GVariant      *parameter,
                                         gpointer       user_data)
{
    Inkscape::UI::Tools::cc_selection_set_avoid(false);
}

static void sp_connector_orthogonal_toggled(GSimpleAction *action,
                                            GVariant      *parameter,
                                            gpointer       user_data)
{
    auto tbl = reinterpret_cast<GObject *>(user_data);

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    SPDocument *doc = desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }

    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    // Get the toolbutton
    auto connector_orthogonal_button = gtk_toolbar_get_nth_item(GTK_TOOLBAR(tbl),INDEX_ORTHOGONAL);
    bool is_orthog = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(connector_orthogonal_button));
    gchar orthog_str[] = "orthogonal";
    gchar polyline_str[] = "polyline";
    gchar *value = is_orthog ? orthog_str : polyline_str ;

    bool modmade = false;
    auto itemlist= desktop->getSelection()->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;

        if (Inkscape::UI::Tools::cc_item_is_connector(item)) {
            item->setAttribute( "inkscape:connector-type",
                    value, NULL);
            item->avoidRef->handleSettingChange();
            modmade = true;
        }
    }

    if (!modmade) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/tools/connector/orthogonal", is_orthog);
    } else {

        DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                       is_orthog ? _("Set connector type: orthogonal"): _("Set connector type: polyline"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void connector_curvature_changed(GtkAdjustment *adj, GObject* tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    SPDocument *doc = desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }


    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    gdouble newValue = gtk_adjustment_get_value(adj);
    gchar value[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(value, G_ASCII_DTOSTR_BUF_SIZE, newValue);

    bool modmade = false;
    auto itemlist= desktop->getSelection()->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;

        if (Inkscape::UI::Tools::cc_item_is_connector(item)) {
            item->setAttribute( "inkscape:connector-curvature",
                    value, NULL);
            item->avoidRef->handleSettingChange();
            modmade = true;
        }
    }

    if (!modmade) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/connector/curvature"), newValue);
    }
    else {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                       _("Change connector curvature"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}


static void connector_spacing_changed(GtkAdjustment *adj, GObject* tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    SPDocument *doc = desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }

    Inkscape::XML::Node *repr = desktop->namedview->getRepr();

    if ( !repr->attribute("inkscape:connector-spacing") &&
            ( gtk_adjustment_get_value(adj) == defaultConnSpacing )) {
        // Don't need to update the repr if the attribute doesn't
        // exist and it is being set to the default value -- as will
        // happen at startup.
        return;
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    sp_repr_set_css_double(repr, "inkscape:connector-spacing", gtk_adjustment_get_value(adj));
    desktop->namedview->updateRepr();
    bool modmade = false;

    std::vector<SPItem *> items;
    items = get_avoided_items(items, desktop->currentRoot(), desktop);
    for (std::vector<SPItem *>::const_iterator iter = items.begin(); iter != items.end(); ++iter ) {
        SPItem *item = *iter;
        Geom::Affine m = Geom::identity();
        avoid_item_move(&m, item);
        modmade = true;
    }

    if(modmade) {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                       _("Change connector spacing"));
    }
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_connector_graph_layout(void)
{
    if (!SP_ACTIVE_DESKTOP) {
        return;
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // hack for clones, see comment in align-and-distribute.cpp
    int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

    auto tmp = SP_ACTIVE_DESKTOP->getSelection()->items();
    std::vector<SPItem *> vec(tmp.begin(), tmp.end());
    graphlayout(vec);

    prefs->setInt("/options/clonecompensation/value", saved_compensation);

    DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE, _("Arrange connector network"));
}

static void sp_directed_graph_layout_toggled( GtkToggleAction* act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/connector/directedlayout",
                gtk_toggle_action_get_active( act ));
}

static void sp_nooverlaps_graph_layout_toggled( GtkToggleAction* act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/connector/avoidoverlaplayout",
                gtk_toggle_action_get_active( act ));
}


static void connector_length_changed(GtkAdjustment *adj, GObject* /*tbl*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/tools/connector/length", gtk_adjustment_get_value(adj));
}

static void connector_tb_event_attr_changed(Inkscape::XML::Node *repr,
                                            gchar const *name, gchar const * /*old_value*/, gchar const * /*new_value*/,
                                            bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    if ( !g_object_get_data(G_OBJECT(tbl), "freeze")
         && (strcmp(name, "inkscape:connector-spacing") == 0) ) {
        GtkAdjustment *adj = static_cast<GtkAdjustment*>(g_object_get_data(G_OBJECT(tbl), "spacing"));
        gdouble spacing = defaultConnSpacing;
        sp_repr_get_double(repr, "inkscape:connector-spacing", &spacing);

        gtk_adjustment_set_value(adj, spacing);

#if !GTK_CHECK_VERSION(3,18,0)
        gtk_adjustment_value_changed(adj);
#endif

        spinbutton_defocus(tbl);
    }
}

static Inkscape::XML::NodeEventVector connector_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    connector_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

static void sp_connector_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "curvature" ) );
    GtkToggleAction *act = GTK_TOGGLE_ACTION( g_object_get_data( tbl, "orthogonal" ) );
    SPItem *item = selection->singleItem();
    if (SP_IS_PATH(item))
    {
        gdouble curvature = SP_PATH(item)->connEndPair.getCurvature();
        bool is_orthog = SP_PATH(item)->connEndPair.isOrthogonal();
        gtk_toggle_action_set_active(act, is_orthog);
        gtk_adjustment_set_value(adj, curvature);
    }

}

/**
 * @brief Add the tools to the toolbox
 *
 * @details Note that this is different from the _prep function
 *          as it:
 *          1. Uses GAction (not GtkAction) to define the action that each tool
 *             performs
 *          2. Actually creates tools (not just their actions)
 *
 *          This is being created as part of the GtkAction -> GAction migration
 *          and hopefully will ultimately completely replace the _prep function
 */
void sp_connector_toolbox_add_tools(GtkWidget* toolbar)
{
    // Create a new action group to describe all the actions that relate to tools
    // in this toolbar.  All actions will have the "connector-actions" prefix
    auto action_group = g_simple_action_group_new();
    gtk_widget_insert_action_group(toolbar,
                                   "connector-actions",
                                   G_ACTION_GROUP(action_group));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkIconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    // Define all the actions.  Each entry contains the:
    // - name of the action
    // - "activate" signal handler
    GActionEntry entries[] = {
        {"avoid",      sp_connector_path_set_avoid},
        {"ignore",     sp_connector_path_set_ignore},
        {"orthogonal", sp_connector_orthogonal_toggled}
    };

    // Add the actions to the action group
    g_action_map_add_action_entries(G_ACTION_MAP(action_group),
                                    entries,
                                    G_N_ELEMENTS(entries),
                                    toolbar);

    // Create toolbutton for the "avoid" action
    auto connector_avoid_icon   = gtk_image_new_from_icon_name(INKSCAPE_ICON("connector-avoid"), secondarySize);
    auto connector_avoid_button = gtk_tool_button_new(connector_avoid_icon, _("Avoid"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(connector_avoid_button),
                                _("Make connectors avoid selected objects"));

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),
                       connector_avoid_button,
                       INDEX_AVOID);

    gtk_actionable_set_action_name(GTK_ACTIONABLE(connector_avoid_button),
                                   "connector-actions.avoid");

    // Create toolbutton for the "ignore" action
    auto connector_ignore_icon   = gtk_image_new_from_icon_name(INKSCAPE_ICON("connector-ignore"), secondarySize);
    auto connector_ignore_button = gtk_tool_button_new(connector_ignore_icon, _("Ignore"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(connector_ignore_button),
                                _("Make connectors ignore selected objects"));

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),
                       connector_ignore_button,
                       INDEX_IGNORE);

    gtk_actionable_set_action_name(GTK_ACTIONABLE(connector_ignore_button),
                                   "connector-actions.ignore");

    // Create toggle toolbutton for the "orthogonal" action
    auto connector_orthogonal_icon   = gtk_image_new_from_icon_name(INKSCAPE_ICON("connector-orthogonal"), GTK_ICON_SIZE_MENU);
    auto connector_orthogonal_button = gtk_toggle_tool_button_new();
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(connector_orthogonal_button),
                              _("Orthogonal"));
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(connector_orthogonal_button),
                                    connector_orthogonal_icon);
    gtk_widget_set_tooltip_text(GTK_WIDGET(connector_orthogonal_button), _("Make connector orthogonal or polyline"));
    bool tbuttonstate = prefs->getBool("/tools/connector/orthogonal");
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(connector_orthogonal_button), tbuttonstate);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(connector_orthogonal_button),
                                   "connector-actions.orthogonal");

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),
                       connector_orthogonal_button,
                       INDEX_ORTHOGONAL);

    // Separator
    auto separator = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),
                       separator,
                       INDEX_SEPARATOR);

    gtk_widget_show_all(toolbar);
}

void sp_connector_toolbox_prep( SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkIconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);
    auto toolbar = GTK_TOOLBAR(holder);

    EgeAdjustmentAction* eact = 0;
    // Curvature spinbox
    eact = create_adjustment_action( "ConnectorCurvatureAction",
                                    _("Connector Curvature"), _("Curvature:"),
                                    _("The amount of connectors curvature"),
                                    "/tools/connector/curvature", defaultConnCurvature,
                                    GTK_WIDGET(desktop->canvas), holder, TRUE, "inkscape:connector-curvature",
                                    0, 100, 1.0, 10.0,
                                    0, 0, 0,
                                    connector_curvature_changed, NULL /*unit tracker*/, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );

    // Spacing spinbox
    eact = create_adjustment_action( "ConnectorSpacingAction",
                                    _("Connector Spacing"), _("Spacing:"),
                                    _("The amount of space left around objects by auto-routing connectors"),
                                    "/tools/connector/spacing", defaultConnSpacing,
                                    GTK_WIDGET(desktop->canvas), holder, TRUE, "inkscape:connector-spacing",
                                    0, 100, 1.0, 10.0,
                                    0, 0, 0,
                                    connector_spacing_changed, NULL /*unit tracker*/, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );

    // Graph (connector network) layout
    {
        GtkAction* inky = gtk_action_new( "ConnectorGraphAction",
                                          _("Graph"),
                                          _("Nicely arrange selected connector network"),
                                          NULL);
        gtk_action_set_icon_name(inky, INKSCAPE_ICON("distribute-graph"));
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_graph_layout), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    // Default connector length spinbox
    eact = create_adjustment_action( "ConnectorLengthAction",
                                     _("Connector Length"), _("Length:"),
                                     _("Ideal length for connectors when layout is applied"),
                                     "/tools/connector/length", 100,
                                     GTK_WIDGET(desktop->canvas), holder, TRUE, "inkscape:connector-length",
                                     10, 1000, 10.0, 100.0,
                                     0, 0, 0,
                                     connector_length_changed, NULL /*unit tracker*/, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );


    // Directed edges toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorDirectedAction",
                                                      _("Downwards"),
                                                      _("Make connectors with end-markers (arrows) point downwards"),
                                                      INKSCAPE_ICON("distribute-graph-directed"),
                                                      GTK_ICON_SIZE_MENU );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        bool tbuttonstate = prefs->getBool("/tools/connector/directedlayout");
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act), ( tbuttonstate ? TRUE : FALSE ));

        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_directed_graph_layout_toggled), holder );
        desktop->getSelection()->connectChanged(sigc::bind(sigc::ptr_fun(sp_connector_toolbox_selection_changed), holder));
    }

    // Avoid overlaps toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorOverlapAction",
                                                      _("Remove overlaps"),
                                                      _("Do not allow overlapping shapes"),
                                                      INKSCAPE_ICON("distribute-remove-overlaps"),
                                                      GTK_ICON_SIZE_MENU );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        bool tbuttonstate = prefs->getBool("/tools/connector/avoidoverlaplayout");
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act), (tbuttonstate ? TRUE : FALSE ));

        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_nooverlaps_graph_layout_toggled), holder );
    }


    // Code to watch for changes to the connector-spacing attribute in
    // the XML.
    Inkscape::XML::Node *repr = desktop->namedview->getRepr();
    g_assert(repr != NULL);

    purge_repr_listener( holder, holder );

    if (repr) {
        g_object_set_data( holder, "repr", repr );
        Inkscape::GC::anchor(repr);
        sp_repr_add_listener( repr, &connector_tb_repr_events, holder );
        sp_repr_synthesize_events( repr, &connector_tb_repr_events, holder );
    }
} // end of sp_connector_toolbox_prep()


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
