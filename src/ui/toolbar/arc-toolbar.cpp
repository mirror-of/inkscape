/**
 * @file
 * Arc aux toolbar
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

#include "arc-toolbar.h"

#include <glibmm/i18n.h>

#include <gtkmm/label.h>
#include <gtkmm/radiotoolbutton.h>
#include <gtkmm/separatortoolitem.h>

#include "desktop.h"
#include "document-undo.h"
#include "mod360.h"
#include "selection.h"
#include "verbs.h"

#include "object/sp-ellipse.h"

#include "ui/icon-names.h"
#include "ui/tools/arc-tool.h"
#include "ui/widget/ink-select-one-action.h"
#include "ui/widget/spin-button-tool-item.h"
#include "ui/widget/unit-tracker.h"

#include "widgets/spinbutton-events.h"
#include "widgets/widget-sizes.h"

#include "xml/node-event-vector.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::DocumentUndo;
using Inkscape::Util::Quantity;
using Inkscape::Util::unit_table;

//########################
//##    Circle / Arc    ##
//########################

static Inkscape::XML::NodeEventVector arc_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    Inkscape::UI::Toolbar::ArcToolbar::event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

namespace Inkscape {
namespace UI {
namespace Toolbar {
void
ArcToolbar::check_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec)
{
    static sigc::connection changed;

    if (SP_IS_ARC_CONTEXT(ec)) {
        changed = _desktop->getSelection()->connectChanged(sigc::mem_fun(*this, &ArcToolbar::selection_changed));
        selection_changed(_desktop->getSelection());
    } else {
        if (changed) {
            changed.disconnect();
            purge_repr_listener();
        }
    }
}

void
ArcToolbar::purge_repr_listener()
{
    auto oldrepr = _repr;

    if (oldrepr) { // remove old listener
        sp_repr_remove_listener_by_data(oldrepr, this);
        Inkscape::GC::release(oldrepr);
        oldrepr = 0;
        _repr = nullptr;
    }
}

void
ArcToolbar::sensitivize( double v1, double v2 )
{
    if (v1 == 0 && v2 == 0) {
        if (_single) { // only for a single selected ellipse (for now)
            for (auto btn : _type_buttons) {
                btn->set_sensitive(false);
            }

            _make_whole_btn->set_sensitive(false);
        }
    } else {
        for (auto btn : _type_buttons) {
            btn->set_sensitive(true);
        }
        _make_whole_btn->set_sensitive(true);
    }
}

void
ArcToolbar::startend_value_changed(Glib::RefPtr<Gtk::Adjustment>  adj,
                                   const Glib::ustring           &value_name,
                                   Glib::RefPtr<Gtk::Adjustment>  other_adj)
{
    if (DocumentUndo::getUndoSensitive(_desktop->getDocument())) {
        auto prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/shapes/arc/") + value_name, adj->get_value());
    }

    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }

    // in turn, prevent listener from responding
    _freeze = true;

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name.c_str(), NULL);

    bool modmade = false;
    auto itemlist= _desktop->getSelection()->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_GENERICELLIPSE(item)) {

            SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

            if (!strcmp(value_name.c_str(), "start")) {
                ge->start = (adj->get_value() * M_PI)/ 180;
            } else {
                ge->end = (adj->get_value() * M_PI)/ 180;
            }

            ge->normalize();
            (SP_OBJECT(ge))->updateRepr();
            (SP_OBJECT(ge))->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

            modmade = true;
        }
    }

    g_free(namespaced_name);

    sensitivize( adj->get_value(), other_adj->get_value() );

    if (modmade) {
        DocumentUndo::maybeDone(_desktop->getDocument(), value_name.c_str(), SP_VERB_CONTEXT_ARC,
                                _("Arc: Change start/end"));
    }

    _freeze = false;
}

void
ArcToolbar::type_changed( int type )
{
    if (DocumentUndo::getUndoSensitive(_desktop->getDocument())) {
        auto prefs = Inkscape::Preferences::get();
        prefs->setInt("/tools/shapes/arc/arc_type", type);
    }

    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }

    // in turn, prevent listener from responding
    _freeze = true;

    Glib::ustring arc_type = "slice";
    bool open = false;
    switch (type) {
        case 0:
            arc_type = "slice";
            open = false;
            break;
        case 1:
            arc_type = "arc";
            open = true;
            break;
        case 2:
            arc_type = "chord";
            open = true; // For backward compat, not truly open but chord most like arc.
            break;
        default:
            std::cerr << "sp_arctb_type_changed: bad arc type: " << type << std::endl;
    }
               
    bool modmade = false;
    auto itemlist= _desktop->getSelection()->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_GENERICELLIPSE(item)) {
            Inkscape::XML::Node *repr = item->getRepr();
            repr->setAttribute("sodipodi:open", (open?"true":NULL) );
            repr->setAttribute("sodipodi:arc-type", arc_type.c_str());
            item->updateRepr();
            modmade = true;
        }
    }

    if (modmade) {
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_ARC,
                           _("Arc: Changed arc type"));
    }

    _freeze = false;
}

void
ArcToolbar::defaults()
{
    _start_adj->set_value(0.0);
    _end_adj->set_value(0.0);

#if !GTK_CHECK_VERSION(3,18,0)
    _start_adj->value_changed();
    _end_adj->value_changed();
#endif

    spinbutton_defocus(GTK_WIDGET(gobj()));
}

void
ArcToolbar::selection_changed(Inkscape::Selection *selection)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    SPItem *item = NULL;

    if ( _repr ) {
        _item = nullptr;
    }

    purge_repr_listener();

    auto itemlist= selection->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        if (SP_IS_GENERICELLIPSE(*i)) {
            n_selected++;
            item = *i;
            repr = item->getRepr();
        }
    }

    _single = false;

    if (n_selected == 0) {
        _state_label->set_label(_("<b>New:</b>"));
    } else if (n_selected == 1) {
        _single = true;
        _state_label->set_label(_("<b>Change:</b>"));

        _radius_x_btn->set_sensitive();
        _radius_y_btn->set_sensitive();

        if (repr) {
            _repr = repr;
            _item = item;
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &arc_tb_repr_events, this);
            sp_repr_synthesize_events(repr, &arc_tb_repr_events, this);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        _state_label->set_label(_("<b>Change:</b>"));
        sensitivize( 1, 0 );
    }
}

void
ArcToolbar::event_attr_changed(Inkscape::XML::Node *repr,
                               gchar const         * /*name*/,
                               gchar const         * /*old_value*/,
                               gchar const         * /*new_value*/,
                               bool                  /*is_interactive*/,
                               gpointer              data)
{
    auto toolbar = reinterpret_cast<ArcToolbar *>(data);

    // quit if run by the _changed callbacks
    if (toolbar->_freeze) {
        return;
    }

    // in turn, prevent callbacks from responding
    toolbar->_freeze = true;

    if (toolbar->_item && SP_IS_GENERICELLIPSE(toolbar->_item)) {
        auto ge = SP_GENERICELLIPSE(toolbar->_item);

        Unit const *unit = toolbar->_tracker->getActiveUnit();
        g_return_if_fail(unit != NULL);

        auto rx = ge->getVisibleRx();
        auto ry = ge->getVisibleRy();

        toolbar->_radius_x_adj->set_value(Quantity::convert(rx, "px", unit));
        toolbar->_radius_y_adj->set_value(Quantity::convert(ry, "px", unit));

#if !GTK_CHECK_VERSION(3,18,0)
        toolbar->_radius_x_adj->value_changed();
        toolbar->_radius_y_adj->value_changed();
#endif
    }

    gdouble start = 0.;
    gdouble end = 0.;
    sp_repr_get_double(repr, "sodipodi:start", &start);
    sp_repr_get_double(repr, "sodipodi:end", &end);

    toolbar->_start_adj->set_value(mod360((start * 180)/M_PI));
    toolbar->_end_adj->set_value(mod360((end * 180)/M_PI));

    toolbar->sensitivize( toolbar->_start_adj->get_value(), toolbar->_end_adj->get_value() );

    char const *arctypestr = NULL;
    arctypestr = repr->attribute("sodipodi:arc-type");
    if (!arctypestr) { // For old files.
        char const *openstr = NULL;
        openstr = repr->attribute("sodipodi:open");
        arctypestr = (openstr ? "arc" : "slice");
    }

    if (!strcmp(arctypestr,"slice")) {
        toolbar->_type_buttons[0]->set_active();
    } else if (!strcmp(arctypestr,"arc")) {
        toolbar->_type_buttons[1]->set_active();
    } else {
        toolbar->_type_buttons[2]->set_active();
    }

    toolbar->_freeze = false;
}

void
ArcToolbar::value_changed(Glib::RefPtr<Gtk::Adjustment>  adj,
                          const Glib::ustring           &value_name)
{
    // Per SVG spec "a [radius] value of zero disables rendering of the element".
    // However our implementation does not allow a setting of zero in the UI (not even in the XML editor)
    // and ugly things happen if it's forced here, so better leave the properties untouched.
    if (!adj->get_value()) {
        return;
    }

    auto unit = _tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    auto document = _desktop->getDocument();
    Geom::Scale scale = document->getDocumentScale();

    if (DocumentUndo::getUndoSensitive(document)) {
        auto prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/shapes/arc/") + value_name,
            Quantity::convert(adj->get_value(), unit, "px"));
    }

    // quit if run by the attr_changed listener
    if (_freeze || _tracker->isUpdating()) {
        return;
    }

    // in turn, prevent listener from responding
    _freeze = true;

    bool modmade = false;
    auto selection = _desktop->getSelection();
    auto itemlist= selection->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_GENERICELLIPSE(item)) {

            SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

            if (!strcmp(value_name.c_str(), "rx")) {
                ge->setVisibleRx(Quantity::convert(adj->get_value(), unit, "px"));
            } else {
                ge->setVisibleRy(Quantity::convert(adj->get_value(), unit, "px"));
            }

            ge->normalize();
            (SP_OBJECT(ge))->updateRepr();
            (SP_OBJECT(ge))->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

            modmade = true;
        }
    }

    if (modmade) {
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_CONTEXT_ARC,
                           _("Change arc"));
    }

    _freeze = false;
}

Gtk::RadioToolButton *
ArcToolbar::create_radio_tool_button(Gtk::RadioButtonGroup &group,
                                     const Glib::ustring   &label,
                                     const Glib::ustring   &tooltip_text,
                                     const Glib::ustring   &icon_name)
{
    auto btn = Gtk::manage(new Gtk::RadioToolButton(group, label));
    btn->set_tooltip_text(tooltip_text);
    btn->set_icon_name(icon_name);

    return btn;
}

ArcToolbar::ArcToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _tracker(new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR))
{
    auto prefs = Inkscape::Preferences::get();
    _tracker->setActiveUnit(unit_table.getUnit("px"));

    _state_label = Gtk::manage(new Gtk::Label(_("<b>New:</b>")));
    _state_label->set_use_markup();
    auto state_ti = Gtk::manage(new Gtk::ToolItem());
    state_ti->add(*_state_label);

    auto radius_x_val = prefs->getDouble("/tools/shapes/arc/rx",    0);
    auto radius_y_val = prefs->getDouble("/tools/shapes/arc/ry",    0);
    auto start_val    = prefs->getDouble("/tools/shapes/arc/start", 0);
    auto end_val      = prefs->getDouble("/tools/shapes/arc/end",   0);

    _radius_x_adj = Gtk::Adjustment::create(radius_x_val, 0, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _radius_y_adj = Gtk::Adjustment::create(radius_y_val, 0, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _start_adj    = Gtk::Adjustment::create(start_val, -360.0, 360.0, 1.0, 10.0);
    _end_adj      = Gtk::Adjustment::create(end_val,   -360.0, 360.0, 1.0, 10.0);

    _tracker->addAdjustment(_radius_x_adj->gobj());
    _tracker->addAdjustment(_radius_y_adj->gobj());

    auto radius_x_adj_value_changed_cb = sigc::bind(sigc::mem_fun(*this, &ArcToolbar::value_changed), _radius_x_adj, "rx");
    auto radius_y_adj_value_changed_cb = sigc::bind(sigc::mem_fun(*this, &ArcToolbar::value_changed), _radius_y_adj, "ry");
    auto start_adj_value_changed_cb    = sigc::bind(sigc::mem_fun(*this, &ArcToolbar::startend_value_changed), _start_adj, "start", _end_adj);
    auto end_adj_value_changed_cb      = sigc::bind(sigc::mem_fun(*this, &ArcToolbar::startend_value_changed), _end_adj,   "end",   _start_adj);

    _radius_x_adj->signal_value_changed().connect(radius_x_adj_value_changed_cb);
    _radius_y_adj->signal_value_changed().connect(radius_y_adj_value_changed_cb);
    _start_adj->signal_value_changed().connect(start_adj_value_changed_cb);
    _end_adj->signal_value_changed().connect(end_adj_value_changed_cb);

    _radius_x_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("arc-radius-x",
                                                                              _("Rx:"),
                                                                              _radius_x_adj,
                                                                              0.1, 3));
    _radius_y_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("arc-radius-y",
                                                                             _("Ry:"),
                                                                             _radius_y_adj,
                                                                             0.1, 3));
    auto start_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("arc-start",
                                                                              _("Start:"),
                                                                              _start_adj));
    auto end_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem("arc-end",
                                                                            _("End:"),
                                                                            _end_adj));

    _radius_x_btn->set_all_tooltip_text(_("Horizontal radius of the circle, ellipse, or arc"));
    _radius_y_btn->set_all_tooltip_text(_("Vertical radius of the circle, ellipse, or arc"));
    start_btn->set_all_tooltip_text(_("The angle (in degrees) from the horizontal to the arc's start point"));
    end_btn->set_all_tooltip_text(_("The angle (in degrees) from the horizontal to the arc's end point"));

    _radius_x_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    _radius_y_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    start_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    end_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    _radius_x_btn->set_sensitive(false);
    _radius_y_btn->set_sensitive(false);

#if 0
    // Consider adding custom menu-item options for tool items:
    // radius_x
    gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
    // radius_y
    gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
#endif

    auto unit_menu = _tracker->createAction( "ArcUnitsAction", _("Units"), ("") );
    auto unit_menu_ti = unit_menu->create_tool_item();

    Gtk::RadioToolButton::Group type_button_group;

    _type_buttons.push_back(create_radio_tool_button(type_button_group,
                                                     _("Slice"),
                                                     _("Switch to slice (closed shape with two radii)"),
                                                     INKSCAPE_ICON("draw-ellipse-segment")));
    _type_buttons.push_back(create_radio_tool_button(type_button_group,
                                                     _("Arc (Open)"),
                                                     _("Switch to arc (unclosed shape)"),
                                                     INKSCAPE_ICON("draw-ellipse-arc")));

    _type_buttons.push_back(create_radio_tool_button(type_button_group,
                                                     _("Chord"),
                                                     _("Switch to chord (closed shape)"),
                                                     INKSCAPE_ICON("draw-ellipse-chord")));

    // Activate the appropriate type button according to preference
    int current_type = prefs->getInt("/tools/shapes/arc/arc_type", 0);
    _type_buttons[current_type]->set_active();

    _make_whole_btn = Gtk::manage(new Gtk::ToolButton(_("Make whole")));
    _make_whole_btn->set_icon_name(INKSCAPE_ICON("draw-ellipse-whole"));
    _make_whole_btn->signal_clicked().connect(sigc::mem_fun(*this, &ArcToolbar::defaults));
    _make_whole_btn->set_tooltip_text(_("Make the shape a whole ellipse, not arc or segment"));
    _make_whole_btn->set_sensitive();

    add(*state_ti);
    add(*_radius_x_btn);
    add(*_radius_y_btn);
    add(*unit_menu_ti);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*start_btn);
    add(*end_btn);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));

    int btn_index = 0;

    for (auto btn : _type_buttons) {
        add(*btn);
        btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ArcToolbar::type_changed), btn_index));
        ++btn_index;
    }

    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*_make_whole_btn);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    show_all();

    _desktop->connectEventContextChanged(sigc::mem_fun(*this, &ArcToolbar::check_ec));
    _single = true;
    sensitivize(_start_adj->get_value(), _end_adj->get_value());
}

ArcToolbar::~ArcToolbar()
{
    purge_repr_listener();
}

GtkWidget *
ArcToolbar::create(SPDesktop *desktop)
{
    auto toolbar = Gtk::manage(new ArcToolbar(desktop));
    return GTK_WIDGET(toolbar->gobj());
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
