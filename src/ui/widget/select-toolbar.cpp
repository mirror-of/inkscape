/*
 * Selector aux toolbar
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2003-2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <giomm/simpleactiongroup.h>
#include <glibmm/i18n.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/toolbar.h>

#include <2geom/rect.h>

#include "select-toolbar.h"

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "inkscape.h"
#include "message-stack.h"
#include "selection-chemistry.h"
#include "verbs.h"

#include "display/sp-canvas.h"

#include "helper/action-context.h"
#include "helper/action.h"

#include "object/sp-item-transform.h"
#include "object/sp-namedview.h"

#include "ui/icon-names.h"
#include "ui/widget/ink-select-one-action.h"
#include "ui/widget/spin-button-tool-item.h"
#include "ui/widget/spinbutton.h"
#include "ui/widget/unit-tracker.h"

#include "widgets/toolbox.h"
#include "widgets/widget-sizes.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::Util::Unit;
using Inkscape::Util::Quantity;
using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;


namespace Inkscape {
namespace UI {
namespace Widget {
SelectToolbar::~SelectToolbar()
{
    delete _tracker;
}

SelectToolbar::SelectToolbar(SPDesktop *desktop)
    : _desktop(desktop),
      _update_flag(false),
      _last_changed(CHANGED_NONE),
      _lock_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _transform_stroke_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _transform_corners_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _transform_gradient_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _transform_pattern_button(Gtk::manage(new Gtk::ToggleToolButton())),
      _tracker(new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR))
{
    auto action_group = Gio::SimpleActionGroup::create();
    insert_action_group("select", action_group);

    auto prefs = Inkscape::Preferences::get();
    auto secondarySize = static_cast<Gtk::IconSize>(Inkscape::UI::ToolboxFactory::prefToSize("/toolbox/secondary", 1));

    // Create tool items
    auto select_all_button               = create_toolbutton_for_verb(SP_VERB_EDIT_SELECT_ALL);
    auto select_all_in_all_layers_button = create_toolbutton_for_verb(SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS);
    auto deselect_button                 = create_toolbutton_for_verb(SP_VERB_EDIT_DESELECT);
    auto object_rotate_90_ccw_button     = create_toolbutton_for_verb(SP_VERB_OBJECT_ROTATE_90_CCW);
    auto object_rotate_90_cw_button      = create_toolbutton_for_verb(SP_VERB_OBJECT_ROTATE_90_CW);
    auto object_flip_horizontal_button   = create_toolbutton_for_verb(SP_VERB_OBJECT_FLIP_HORIZONTAL);
    auto object_flip_vertical_button     = create_toolbutton_for_verb(SP_VERB_OBJECT_FLIP_VERTICAL);
    auto selection_to_back_button        = create_toolbutton_for_verb(SP_VERB_SELECTION_TO_BACK);
    auto selection_lower_button          = create_toolbutton_for_verb(SP_VERB_SELECTION_LOWER);
    auto selection_raise_button          = create_toolbutton_for_verb(SP_VERB_SELECTION_RAISE);
    auto selection_to_front_button       = create_toolbutton_for_verb(SP_VERB_SELECTION_TO_FRONT);

    // Create the units menu.
    _tracker->addUnit(unit_table.getUnit("%"));
    _tracker->setActiveUnit(_desktop->getNamedView()->display_units);

    // Create X, Y, W, H controls.
    auto _x_val = prefs->getDouble("/tools/select/X", 0.0);
    auto _y_val = prefs->getDouble("/tools/select/Y", 0.0);
    auto _w_val = prefs->getDouble("/tools/select/width", 0.0);
    auto _h_val = prefs->getDouble("/tools/select/height", 0.0);

    _x_pos_adj  = Gtk::Adjustment::create(_x_val, -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _y_pos_adj  = Gtk::Adjustment::create(_y_val, -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _width_adj  = Gtk::Adjustment::create(_w_val,  0.0, 1e6, SPIN_STEP, SPIN_PAGE_STEP);
    _height_adj = Gtk::Adjustment::create(_h_val,  0.0, 1e6, SPIN_STEP, SPIN_PAGE_STEP);

    _tracker->addAdjustment(_x_pos_adj->gobj());
    _tracker->addAdjustment(_y_pos_adj->gobj());
    _tracker->addAdjustment(_width_adj->gobj());
    _tracker->addAdjustment(_height_adj->gobj());

    auto x_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(C_("Select toolbar", "X:"), _x_pos_adj,  SPIN_STEP, 3));
    auto y_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(C_("Select toolbar", "Y:"), _y_pos_adj,  SPIN_STEP, 3));
    auto w_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(C_("Select toolbar", "W:"), _width_adj,  SPIN_STEP, 3));
    auto h_btn = Gtk::manage(new Inkscape::UI::Widget::SpinButtonToolItem(C_("Select toolbar", "H:"), _height_adj, SPIN_STEP, 3));

    x_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    y_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    w_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));
    h_btn->set_focus_widget(Glib::wrap(GTK_WIDGET(_desktop->canvas)));

    auto x_val_changed_cb = sigc::mem_fun(*this, &SelectToolbar::on_x_y_adj_value_changed);
    auto y_val_changed_cb = sigc::mem_fun(*this, &SelectToolbar::on_x_y_adj_value_changed);
    auto w_val_changed_cb = sigc::mem_fun(*this, &SelectToolbar::on_w_adj_value_changed);
    auto h_val_changed_cb = sigc::mem_fun(*this, &SelectToolbar::on_h_adj_value_changed);

    _x_pos_adj->signal_value_changed().connect(x_val_changed_cb);
    _y_pos_adj->signal_value_changed().connect(y_val_changed_cb);
    _width_adj->signal_value_changed().connect(w_val_changed_cb);
    _height_adj->signal_value_changed().connect(h_val_changed_cb);

    x_btn->set_all_tooltip_text(C_("Select toolbar", "Horizontal coordinate of selection"));
    y_btn->set_all_tooltip_text(C_("Select toolbar", "Vertical coordinate of selection"));
    w_btn->set_all_tooltip_text(C_("Select toolbar", "Width of selection"));
    h_btn->set_all_tooltip_text(C_("Select toolbar", "Height of selection"));

    // Lock toggle
    _lock_button->set_label(_("Lock width and height"));
    _lock_button->set_icon_name(INKSCAPE_ICON("object-unlocked"));
    _lock_button->set_tooltip_text(_("When locked, change both width and height by the same proportion"));

    auto lock_button_toggled_cb = sigc::mem_fun(*this, &SelectToolbar::on_lock_button_toggled);
    _lock_button->signal_toggled().connect(lock_button_toggled_cb);

    // TODO: Migrate away from GtkAction
    auto unit_menu = _tracker->createAction( "UnitsAction", _("Units"), ("") );
    auto unit_menu_ti = unit_menu->create_tool_item();

    _transform_stroke_button->set_label(_("Scale stroke width"));
    _transform_stroke_button->set_icon_name(INKSCAPE_ICON("transform-affect-stroke"));
    _transform_stroke_button->set_tooltip_text(_("When scaling objects, scale the stroke width by the same proportion"));
    _transform_stroke_button->set_active(prefs->getBool("/options/transform/stroke", true));
    _transform_stroke_button->signal_toggled().connect(sigc::mem_fun(*this, &SelectToolbar::on_transform_stroke_button_toggled));

    _transform_corners_button->set_label(_("Scale rounded corners"));
    _transform_corners_button->set_icon_name(INKSCAPE_ICON("transform-affect-rounded-corners"));
    _transform_corners_button->set_tooltip_text(_("When scaling objects, scale the radii of rounded corners"));
    _transform_corners_button->set_active(prefs->getBool("/options/transform/rectcorners", true));
    _transform_corners_button->signal_toggled().connect(sigc::mem_fun(*this, &SelectToolbar::on_transform_corners_button_toggled));

    _transform_gradient_button->set_label(_("Move gradients"));
    _transform_gradient_button->set_icon_name(INKSCAPE_ICON("transform-affect-gradient"));
    _transform_gradient_button->set_tooltip_text(_("Move gradients (in fill and stroke) along with the objects"));
    _transform_gradient_button->set_active(prefs->getBool("/options/transform/gradient", true));
    _transform_gradient_button->signal_toggled().connect(sigc::mem_fun(*this, &SelectToolbar::on_transform_gradient_button_toggled));

    _transform_pattern_button->set_label(_("Move patterns"));
    _transform_pattern_button->set_icon_name(INKSCAPE_ICON("transform-affect-pattern"));
    _transform_pattern_button->set_tooltip_text(_("Move patterns (in fill or stroke) along with the objects"));
    _transform_pattern_button->set_active(prefs->getBool("/options/transform/pattern", true));
    _transform_pattern_button->signal_toggled().connect(sigc::mem_fun(*this, &SelectToolbar::on_transform_pattern_button_toggled));

    // Add some items to the context_items list
    _context_items.push_back(deselect_button);
    _context_items.push_back(object_rotate_90_ccw_button);
    _context_items.push_back(object_rotate_90_cw_button);
    _context_items.push_back(object_flip_horizontal_button);
    _context_items.push_back(object_flip_vertical_button);
    _context_items.push_back(selection_to_back_button);
    _context_items.push_back(selection_lower_button);
    _context_items.push_back(selection_raise_button);
    _context_items.push_back(selection_to_front_button);
    _context_items.push_back(x_btn);
    _context_items.push_back(y_btn);
    _context_items.push_back(w_btn);
    _context_items.push_back(h_btn);

    // Handle selection changes
    auto selection_modified_cb = sigc::mem_fun(*this, &SelectToolbar::on_inkscape_selection_modified);
    INKSCAPE.signal_selection_modified.connect(selection_modified_cb);
    auto selection_changed_cb = sigc::mem_fun(*this, &SelectToolbar::on_inkscape_selection_changed);
    INKSCAPE.signal_selection_changed.connect(selection_changed_cb);

    // Update now.
    layout_widget_update(SP_ACTIVE_DESKTOP ? SP_ACTIVE_DESKTOP->getSelection() : NULL);

    for(auto item : _context_items) {
        if(item->is_sensitive()) item->set_sensitive(false);
    }

    // Add tool items to the toolbar in the correct order
    add(*select_all_button);
    add(*select_all_in_all_layers_button);
    add(*deselect_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*object_rotate_90_ccw_button);
    add(*object_rotate_90_cw_button);
    add(*object_flip_horizontal_button);
    add(*object_flip_vertical_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*selection_to_back_button);
    add(*selection_lower_button);
    add(*selection_raise_button);
    add(*selection_to_front_button);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*x_btn);
    add(*y_btn);
    add(*w_btn);
    add(*_lock_button);
    add(*h_btn);
    add(*unit_menu_ti);
    add(* Gtk::manage(new Gtk::SeparatorToolItem()));
    add(*_transform_stroke_button);
    add(*_transform_corners_button);
    add(*_transform_gradient_button);
    add(*_transform_pattern_button);
}

GtkWidget *
SelectToolbar::create(SPDesktop *desktop)
{
    auto toolbar = Gtk::manage(new SelectToolbar(desktop));
    return GTK_WIDGET(toolbar->gobj());
}

/**
 * \brief     Create a toolbutton whose "clicked" signal performs an Inkscape verb
 *
 * \param[in] verb_code The code (e.g., SP_VERB_EDIT_SELECT_ALL) for the verb we want
 *
 * \todo This should really attach the toolbutton to a application action instead of
 *       hooking up the "clicked" signal.  This should probably wait until we've
 *       migrated to Gtk::Application
 */
Gtk::ToolButton*
SelectToolbar::create_toolbutton_for_verb(unsigned int  verb_code)
{
    auto verb = Inkscape::Verb::get(verb_code);
    SPAction* targetAction = verb->get_action(Inkscape::ActionContext(_desktop));
    auto icon_name = verb->get_image();

    auto button = Gtk::manage(new Gtk::ToolButton(verb->get_name()));
    button->set_icon_name(icon_name);
    button->set_tooltip_text(verb->get_tip());
    button->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&sp_action_perform), targetAction, nullptr));

    return button;
}

void
SelectToolbar::on_inkscape_selection_modified(Inkscape::Selection *selection, guint flags)
{
    if ((_desktop->getSelection() == selection) // only respond to changes in our desktop
        && (flags & (SP_OBJECT_MODIFIED_FLAG        |
                     SP_OBJECT_PARENT_MODIFIED_FLAG |
                     SP_OBJECT_CHILD_MODIFIED_FLAG   )))
    {
        layout_widget_update(selection);
    }
}

void
SelectToolbar::on_x_y_adj_value_changed()
{
    _last_changed = CHANGED_X_Y;
    on_any_layout_adj_value_changed();
}

void
SelectToolbar::on_w_adj_value_changed()
{
    _last_changed = CHANGED_W;
    on_any_layout_adj_value_changed();
}

void
SelectToolbar::on_h_adj_value_changed()
{
    _last_changed = CHANGED_H;
    on_any_layout_adj_value_changed();
}

void
SelectToolbar::layout_widget_update(Inkscape::Selection *sel)
{
    if (_update_flag) {
        return;
    }

    _update_flag = true;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    using Geom::X;
    using Geom::Y;
    if ( sel && !sel->isEmpty() ) {
        int prefs_bbox = prefs->getInt("/tools/bounding_box", 0);
        SPItem::BBoxType bbox_type = (prefs_bbox ==0)?
            SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
        Geom::OptRect const bbox(sel->bounds(bbox_type));
        if ( bbox ) {
            Unit const *unit = _tracker->getActiveUnit();
            g_return_if_fail(unit != NULL);

            struct { char const *key; double val; } const keyval[] = {
                { "X", bbox->min()[X] },
                { "Y", bbox->min()[Y] },
                { "width", bbox->dimensions()[X] },
                { "height", bbox->dimensions()[Y] }
            };

            if (unit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) {
                double const val = unit->factor * 100;

                _x_pos_adj->set_value(val);
                _y_pos_adj->set_value(val);
                _width_adj->set_value(val);
                _height_adj->set_value(val);

                _tracker->setFullVal(_x_pos_adj->gobj(),  bbox->min()[X]);
                _tracker->setFullVal(_y_pos_adj->gobj(),  bbox->min()[Y]);
                _tracker->setFullVal(_width_adj->gobj(),  bbox->dimensions()[X]);
                _tracker->setFullVal(_height_adj->gobj(), bbox->dimensions()[Y]);
            } else {
                _x_pos_adj->set_value(Quantity::convert(bbox->min()[X], "px", unit));
                _y_pos_adj->set_value(Quantity::convert(bbox->min()[Y], "px", unit));
                _width_adj->set_value(Quantity::convert(bbox->dimensions()[X], "px", unit));
                _height_adj->set_value(Quantity::convert(bbox->dimensions()[Y], "px", unit));
            }
        }
    }

    _update_flag = false;
}

void
SelectToolbar::on_inkscape_selection_changed(Inkscape::Selection *selection)
{
    if (_desktop->getSelection() == selection) { // only respond to changes in our desktop
        bool setActive = (selection && !selection->isEmpty());

        for (auto item : _context_items) {
            if ( setActive != item->get_sensitive() ) {
                item->set_sensitive(setActive);
            }
        }

        layout_widget_update(selection);
    }
}

void
SelectToolbar::on_any_layout_adj_value_changed()
{
    if (_update_flag) {
        return;
    }

    if ( !_tracker || _tracker->isUpdating() ) {
        /*
         * When only units are being changed, don't treat changes
         * to adjuster values as object changes.
         */
        return;
    }
    _update_flag = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = desktop->getSelection();
    SPDocument *document = desktop->getDocument();

    document->ensureUpToDate ();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Geom::OptRect bbox_vis = selection->visualBounds();
    Geom::OptRect bbox_geom = selection->geometricBounds();

    int prefs_bbox = prefs->getInt("/tools/bounding_box");
    SPItem::BBoxType bbox_type = (prefs_bbox == 0)?
        SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
    Geom::OptRect bbox_user = selection->bounds(bbox_type);

    if ( !bbox_user ) {
        _update_flag = false;
        return;
    }

    gdouble x0 = 0;
    gdouble y0 = 0;
    gdouble x1 = 0;
    gdouble y1 = 0;
    gdouble xrel = 0;
    gdouble yrel = 0;
    Unit const *unit = _tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
        x0 = Quantity::convert(_x_pos_adj->get_value(), unit, "px");
        y0 = Quantity::convert(_y_pos_adj->get_value(), unit, "px");
        x1 = x0 + Quantity::convert(_width_adj->get_value(), unit, "px");
        xrel = Quantity::convert(_width_adj->get_value(), unit, "px") / bbox_user->dimensions()[Geom::X];
        y1 = y0 + Quantity::convert(_height_adj->get_value(), unit, "px");;
        yrel = Quantity::convert(_height_adj->get_value(), unit, "px") / bbox_user->dimensions()[Geom::Y];
    } else {
        double const x0_propn = _x_pos_adj->get_value() / 100 / unit->factor;
        x0 = bbox_user->min()[Geom::X] * x0_propn;
        double const y0_propn = _y_pos_adj->get_value() / 100 / unit->factor;
        y0 = y0_propn * bbox_user->min()[Geom::Y];
        xrel = _width_adj->get_value() / (100 / unit->factor);
        x1 = x0 + xrel * bbox_user->dimensions()[Geom::X];
        yrel = _height_adj->get_value() / (100 / unit->factor);
        y1 = y0 + yrel * bbox_user->dimensions()[Geom::Y];
    }

    // Keep proportions if lock is on
    if ( _lock_button->get_active() ) {
        if (_last_changed == CHANGED_H) {
            x1 = x0 + yrel * bbox_user->dimensions()[Geom::X];
        } else if (_last_changed == CHANGED_W) {
            y1 = y0 + xrel * bbox_user->dimensions()[Geom::Y];
        }
    }

    // scales and moves, in px
    double mh = fabs(x0 - bbox_user->min()[Geom::X]);
    double sh = fabs(x1 - bbox_user->max()[Geom::X]);
    double mv = fabs(y0 - bbox_user->min()[Geom::Y]);
    double sv = fabs(y1 - bbox_user->max()[Geom::Y]);

    // unless the unit is %, convert the scales and moves to the unit
    if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
        mh = Quantity::convert(mh, "px", unit);
        sh = Quantity::convert(sh, "px", unit);
        mv = Quantity::convert(mv, "px", unit);
        sv = Quantity::convert(sv, "px", unit);
    }

    // do the action only if one of the scales/moves is greater than half the last significant
    // digit in the spinbox (currently spinboxes have 3 fractional digits, so that makes 0.0005). If
    // the value was changed by the user, the difference will be at least that much; otherwise it's
    // just rounding difference between the spinbox value and actual value, so no action is
    // performed
    char const * const actionkey = ( mh > 5e-4 ? "selector:toolbar:move:horizontal" :
                                     sh > 5e-4 ? "selector:toolbar:scale:horizontal" :
                                     mv > 5e-4 ? "selector:toolbar:move:vertical" :
                                     sv > 5e-4 ? "selector:toolbar:scale:vertical" : NULL );

    if (actionkey != NULL) {

        // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed
        desktop->getCanvas()->forceFullRedrawAfterInterruptions(0);

        bool transform_stroke = prefs->getBool("/options/transform/stroke", true);
        bool preserve = prefs->getBool("/options/preservetransform/value", false);

        Geom::Affine scaler;
        if (bbox_type == SPItem::VISUAL_BBOX) {
            scaler = get_scale_transform_for_variable_stroke (*bbox_vis, *bbox_geom, transform_stroke, preserve, x0, y0, x1, y1);
        } else {
            // 1) We could have use the newer get_scale_transform_for_variable_stroke() here, but to avoid regressions
            // we'll just use the old get_scale_transform_for_uniform_stroke() for now.
            // 2) get_scale_transform_for_uniform_stroke() is intended for visual bounding boxes, not geometrical ones!
            // we'll trick it into using a geometric bounding box though, by setting the stroke width to zero
            scaler = get_scale_transform_for_uniform_stroke (*bbox_geom, 0, 0, false, false, x0, y0, x1, y1);
        }

        selection->applyAffine(scaler);
        DocumentUndo::maybeDone(document, actionkey, SP_VERB_CONTEXT_SELECT,
                                _("Transform by toolbar"));

        // resume interruptibility
        desktop->getCanvas()->endForcedFullRedraws();
    }

    _update_flag = false;
}

void
SelectToolbar::on_lock_button_toggled() {
    bool active = _lock_button->get_active();
    if ( active ) {
        _lock_button->set_icon_name(INKSCAPE_ICON("object-locked"));
    } else {
        _lock_button->set_icon_name(INKSCAPE_ICON("object-unlocked"));
    }
}

void
SelectToolbar::on_transform_stroke_button_toggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool active = _transform_stroke_button->get_active();
    prefs->setBool("/options/transform/stroke", active);
    if ( active ) {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>scaled</b> when objects are scaled."));
    } else {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>not scaled</b> when objects are scaled."));
    }
}

void
SelectToolbar::on_transform_corners_button_toggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool active = _transform_corners_button->get_active();
    prefs->setBool("/options/transform/rectcorners", active);
    if ( active ) {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>scaled</b> when rectangles are scaled."));
    } else {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>not scaled</b> when rectangles are scaled."));
    }
}

void
SelectToolbar::on_transform_gradient_button_toggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool active = _transform_gradient_button->get_active();
    prefs->setBool("/options/transform/gradient", active);
    if ( active ) {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

void
SelectToolbar::on_transform_pattern_button_toggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool active = _transform_pattern_button->get_active();
    prefs->setInt("/options/transform/pattern", active);
    if ( active ) {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        _desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
