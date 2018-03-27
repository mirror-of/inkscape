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
#include "widgets/ink-toggle-action.h"
#include "inkscape.h"
#include "widgets/toolbox.h"
#include "verbs.h"

#include "object/sp-namedview.h"
#include "object/sp-path.h"

#include "ui/icon-names.h"
#include "ui/tools/connector-tool.h"
#include "ui/uxmanager.h"
#include "ui/widget/spin-button-tool-item.h"

#include "widgets/ege-adjustment-action.h"
#include "widgets/spinbutton-events.h"

#include "xml/node-event-vector.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;


static Inkscape::XML::NodeEventVector connector_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    Inkscape::UI::Widget::ConnectorToolbar::event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

namespace Inkscape {
namespace UI {
namespace Widget {

ConnectorToolbar::ConnectorToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _orthogonal_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _freeze_flag(false),
      _direction_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _overlap_button(Gtk::manage(new Gtk::ToggleToolButton()))
{
    // Create a new action group to describe all the actions that relate to tools
    // in this toolbar.  All actions will have the "connector-actions" prefix
    auto action_group = Gio::SimpleActionGroup::create();
    insert_action_group("connector-actions", action_group);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    auto secondarySize = static_cast<Gtk::IconSize>(ToolboxFactory::prefToSize("/toolbox/secondary", 1));

    action_group->add_action("avoid",      sigc::mem_fun(*this, &ConnectorToolbar::on_avoid_activated));
    action_group->add_action("ignore",     sigc::mem_fun(*this, &ConnectorToolbar::on_ignore_activated));
    action_group->add_action("orthogonal", sigc::mem_fun(*this, &ConnectorToolbar::on_orthogonal_activated));
    action_group->add_action("graph",      sigc::mem_fun(*this, &ConnectorToolbar::on_graph_activated));
    action_group->add_action("direction",  sigc::mem_fun(*this, &ConnectorToolbar::on_direction_activated));
    action_group->add_action("overlap",    sigc::mem_fun(*this, &ConnectorToolbar::on_overlap_activated));

    /*******************************************/
    /**** Toolbutton for the "avoid" action ****/
    /*******************************************/
    auto avoid_icon   = Gtk::manage(new Gtk::Image());
    avoid_icon->set_from_icon_name(INKSCAPE_ICON("connector-avoid"), secondarySize);
    auto avoid_button = Gtk::manage(new Gtk::ToolButton(*avoid_icon, _("Avoid")));
    avoid_button->set_tooltip_text(_("Make connectors avoid selected objects"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(avoid_button->gobj()), "connector-actions.avoid");

    /********************************************/
    /**** Toolbutton for the "ignore" action ****/
    /********************************************/
    auto ignore_icon   = Gtk::manage(new Gtk::Image());
    ignore_icon->set_from_icon_name(INKSCAPE_ICON("connector-ignore"), secondarySize);
    auto ignore_button = Gtk::manage(new Gtk::ToolButton(*ignore_icon, _("Ignore")));
    ignore_button->set_tooltip_text(_("Make connectors ignore selected objects"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(ignore_button->gobj()),
                                   "connector-actions.ignore");

    /*******************************************************/
    /**** Toggle toolbutton for the "orthogonal" action ****/
    /*******************************************************/
    auto orthogonal_icon   = Gtk::manage(new Gtk::Image());
    orthogonal_icon->set_from_icon_name(INKSCAPE_ICON("connector-orthogonal"), Gtk::ICON_SIZE_MENU);
    _orthogonal_button->set_label(_("Orthogonal"));
    _orthogonal_button->set_icon_widget(*orthogonal_icon);
    _orthogonal_button->set_tooltip_text(_("Make connector orthogonal or polyline"));
    bool tbuttonstate = prefs->getBool("/tools/connector/orthogonal");
    _orthogonal_button->set_active(tbuttonstate);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(_orthogonal_button->gobj()),
                                   "connector-actions.orthogonal");

    /*******************/
    /**** Separator ****/
    /*******************/
    auto separator = Gtk::manage(new Gtk::SeparatorToolItem());

    /*******************************/
    /**** Curvature spin-button ****/
    /*******************************/
    auto curvature_value = prefs->getDouble("/tools/connector/curvature", defaultConnCurvature);
    _curvature_adj = Gtk::Adjustment::create(curvature_value, 0, 100);
    auto curvature_adj_value_changed_cb = sigc::mem_fun(*this, &ConnectorToolbar::on_curvature_adj_value_changed);
    _curvature_adj->signal_value_changed().connect(curvature_adj_value_changed_cb);

    auto curvature_sb = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(_("Curvature:"),
                                                                                 _curvature_adj));
    curvature_sb->set_all_tooltip_text(_("The amount of connectors curvature"));
    curvature_sb->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    /*****************************/
    /**** Spacing spin-button ****/
    /*****************************/
    auto spacing_value = prefs->getDouble("/tools/connector/spacing", defaultConnSpacing);
    _spacing_adj = Gtk::Adjustment::create(spacing_value, 0, 100);
    auto spacing_adj_value_changed_cb = sigc::mem_fun(*this, &ConnectorToolbar::on_spacing_adj_value_changed);
    _spacing_adj->signal_value_changed().connect(spacing_adj_value_changed_cb);

    auto spacing_sb = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(_("Spacing:"),
                                                                               _spacing_adj));
    spacing_sb->set_all_tooltip_text(_("The amount of space left around objects by auto-routing connectors"));
    spacing_sb->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    /******************************************************/
    /**** Toolbutton for the "graph" action ****/
    /******************************************************/
    auto graph_icon   = Gtk::manage(new Gtk::Image());
    graph_icon->set_from_icon_name(INKSCAPE_ICON("distribute-graph"), secondarySize);
    auto graph_button = Gtk::manage(new Gtk::ToolButton(*graph_icon, _("Graph")));
    graph_button->set_tooltip_text(_("Nicely arrange selected connector network"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(graph_button->gobj()),
                                   "connector-actions.graph");

    /**************************************/
    /**** Connector-length spin button ****/
    /**************************************/
    auto length_value = prefs->getDouble("/tools/connector/length", 100);
    _length_adj = Gtk::Adjustment::create(length_value, 0, 100);
    auto length_adj_value_changed_cb = sigc::mem_fun(*this, &ConnectorToolbar::on_length_adj_value_changed);
    _length_adj->signal_value_changed().connect(length_adj_value_changed_cb);

    auto length_sb = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(_("Length:"),
                                                                               _length_adj));
    length_sb->set_all_tooltip_text(_("Ideal length for connectors when layout is applied"));
    length_sb->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    /*******************************************************/
    /**** Toggle toolbutton for the "direction" action ****/
    /*******************************************************/
    auto direction_icon = Gtk::manage(new Gtk::Image());
    direction_icon->set_from_icon_name(INKSCAPE_ICON("distribute-graph-directed"), Gtk::ICON_SIZE_MENU);
    _direction_button->set_label(_("Downwards"));
    _direction_button->set_icon_widget(*direction_icon);
    _direction_button->set_tooltip_text(_("Make connectors with end-markers (arrows) point downwards"));
    bool direction_button_state = prefs->getBool("/tools/connector/directedLayout");
    _direction_button->set_active(direction_button_state);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(_direction_button->gobj()),
                                   "connector-actions.direction");
    _desktop->getSelection()->connectChanged(sigc::bind(sigc::mem_fun(*this, &ConnectorToolbar::selection_changed), G_OBJECT(gobj())));

    /****************************************************/
    /**** Toggle toolbutton for the "overlap" action ****/
    /****************************************************/
    auto overlap_icon = Gtk::manage(new Gtk::Image());
    overlap_icon->set_from_icon_name(INKSCAPE_ICON("distribute-remove-overlaps"), Gtk::ICON_SIZE_MENU);
    _overlap_button->set_label(_("Remove overlaps"));
    _overlap_button->set_icon_widget(*overlap_icon);
    _overlap_button->set_tooltip_text(_("Do not allow overlapping states"));
    bool overlap_button_state = prefs->getBool("/tools/connector/avoidoverlapayout");
    _overlap_button->set_active(overlap_button_state);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(_overlap_button->gobj()),
                                   "connector-actions.overlap");

    // Append all widgets to toolbar
    append(*avoid_button);
    append(*ignore_button);
    append(*_orthogonal_button);
    append(*separator);
    append(*curvature_sb);
    append(*spacing_sb);
    append(*graph_button);
    append(*length_sb);
    append(*_direction_button);
    append(*_overlap_button);
    show_all();

    // Code to watch for changes to the connector-spacing attribute in
    // the XML.
    auto repr = desktop->namedview->getRepr();
    g_assert(repr != NULL);

    purge_repr_listener( G_OBJECT(gobj()), G_OBJECT(gobj()));

    if (repr) {
        _repr = repr;
        Inkscape::GC::anchor(repr);
        sp_repr_add_listener( repr, &connector_tb_repr_events, this );
        sp_repr_synthesize_events( repr, &connector_tb_repr_events, this );
    }
}

GtkWidget *
ConnectorToolbar::create(SPDesktop *desktop)
{
    auto connector_toolbar = Gtk::manage(new ConnectorToolbar(desktop));
    return GTK_WIDGET(connector_toolbar->gobj());
}

void
ConnectorToolbar::on_avoid_activated()
{
    Inkscape::UI::Tools::cc_selection_set_avoid(true);
}

void
ConnectorToolbar::on_ignore_activated()
{
    Inkscape::UI::Tools::cc_selection_set_avoid(false);
}

void
ConnectorToolbar::on_orthogonal_activated()
{
    SPDocument *doc = _desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }

    // quit if run by the _changed callbacks
    if (_freeze_flag) {
        return;
    }

    // in turn, prevent callbacks from responding
    _freeze_flag = true;

    // Get the toolbutton state
    bool is_orthog = _orthogonal_button->get_active();
    gchar orthog_str[] = "orthogonal";
    gchar polyline_str[] = "polyline";
    gchar *value = is_orthog ? orthog_str : polyline_str ;

    bool modmade = false;
    auto itemlist= _desktop->getSelection()->items();
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

    _freeze_flag = false;
}

void
ConnectorToolbar::on_curvature_adj_value_changed()
{
    SPDocument *doc = _desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }

    // quit if run by the _changed callbacks
    if (_freeze_flag) {
        return;
    }

    // in turn, prevent callbacks from responding
    _freeze_flag = true;

    gdouble newValue = _curvature_adj->get_value();
    gchar value[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(value, G_ASCII_DTOSTR_BUF_SIZE, newValue);

    bool modmade = false;
    auto itemlist= _desktop->getSelection()->items();
    for(auto item : itemlist){
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

    _freeze_flag = false;
}

void
ConnectorToolbar::on_spacing_adj_value_changed()
{
    auto doc = _desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }

    Inkscape::XML::Node *repr = _desktop->namedview->getRepr();

    if ( !repr->attribute("inkscape:connector-spacing") &&
            ( _spacing_adj->get_value() == defaultConnSpacing )) {
        // Don't need to update the repr if the attribute doesn't
        // exist and it is being set to the default value -- as will
        // happen at startup.
        return;
    }

    // quit if run by the attr_changed listener
    if (_freeze_flag) {
        return;
    }

    // in turn, prevent listener from responding
    _freeze_flag = true;

    sp_repr_set_css_double(repr, "inkscape:connector-spacing", _spacing_adj->get_value());
    _desktop->namedview->updateRepr();
    bool modmade = false;

    std::vector<SPItem *> items;
    items = get_avoided_items(items, _desktop->currentRoot(), _desktop);
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
    _freeze_flag = false;
}

void
ConnectorToolbar::on_graph_activated()
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

void
ConnectorToolbar::on_length_adj_value_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/tools/connector/length", _length_adj->get_value());
}

void
ConnectorToolbar::on_direction_activated()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/connector/directedlayout",
                _direction_button->get_active());
}

void
ConnectorToolbar::selection_changed(Inkscape::Selection *selection, GObject * /* data */)
{
    SPItem *item = selection->singleItem();
    if (SP_IS_PATH(item))
    {
        gdouble curvature = SP_PATH(item)->connEndPair.getCurvature();
        bool is_orthog = SP_PATH(item)->connEndPair.isOrthogonal();
        _orthogonal_button->set_active(is_orthog);
        _curvature_adj->set_value(curvature);
    }

}

void
ConnectorToolbar::on_overlap_activated()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/connector/avoidoverlaplayout",
                _overlap_button->get_active());
}

void
ConnectorToolbar::event_attr_changed(Inkscape::XML::Node *repr,
                                     gchar const *name, gchar const * /*old_value*/, gchar const * /*new_value*/,
                                     bool /*is_interactive*/, gpointer data)
{
    auto toolbar = reinterpret_cast<ConnectorToolbar *>(data);

    if ( !toolbar->_freeze_flag
         && (strcmp(name, "inkscape:connector-spacing") == 0) ) {
        gdouble spacing = defaultConnSpacing;
        sp_repr_get_double(repr, "inkscape:connector-spacing", &spacing);

        toolbar->_spacing_adj->set_value(spacing);

#if !GTK_CHECK_VERSION(3,18,0)
        _spacing_adj->value_changed();
#endif

        gtk_widget_grab_focus(GTK_WIDGET(toolbar->_desktop->canvas));
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
