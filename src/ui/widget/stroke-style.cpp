// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <brycehar@bryceharrington.org>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Josh Andler <scislac@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 * Copyright (C) 2008 Maximilian Albert (gtkmm-ification)
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#define noSP_SS_VERBOSE

#include "stroke-style.h"

#include "object/sp-marker.h"
#include "object/sp-namedview.h"
#include "object/sp-rect.h"
#include "object/sp-stop.h"
#include "object/sp-text.h"

#include "svg/svg-color.h"

#include "ui/icon-loader.h"
#include "ui/widget/dash-selector.h"
#include "ui/widget/marker-combo-box.h"
#include "ui/widget/unit-menu.h"
#include "ui/tools/marker-tool.h"
#include "ui/dialog/dialog-base.h"

#include "actions/actions-tools.h"

#include "widgets/style-utils.h"

using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

/**
 * Extract the actual name of the link
 * e.g. get mTriangle from url(#mTriangle).
 * \return Buffer containing the actual name, allocated from GLib;
 * the caller should free the buffer when they no longer need it.
 */
SPObject* getMarkerObj(gchar const *n, SPDocument *doc)
{
    gchar const *p = n;
    while (*p != '\0' && *p != '#') {
        p++;
    }

    if (*p == '\0' || p[1] == '\0') {
        return nullptr;
    }

    p++;
    int c = 0;
    while (p[c] != '\0' && p[c] != ')') {
        c++;
    }

    if (p[c] == '\0') {
        return nullptr;
    }

    gchar* b = g_strdup(p);
    b[c] = '\0';

    // FIXME: get the document from the object and let the caller pass it in
    SPObject *marker = doc->getObjectById(b);

    g_free(b);
    return marker;
}

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a stroke-style radio button with a given icon
 *
 * \param[in] grp          The Gtk::RadioButtonGroup to which to add the new button
 * \param[in] icon         The icon to use for the button
 * \param[in] button_type  The type of stroke-style radio button (join/cap)
 * \param[in] stroke_style The style attribute to associate with the button
 */
StrokeStyle::StrokeStyleButton::StrokeStyleButton(Gtk::RadioButtonGroup &grp,
                                                  char const            *icon,
                                                  StrokeStyleButtonType  button_type,
                                                  gchar const           *stroke_style)
    : 
        Gtk::RadioButton(grp),
        button_type(button_type),
        stroke_style(stroke_style)
{
    show();
    set_mode(false);

    auto px = Gtk::manage(sp_get_icon_image(icon, Gtk::ICON_SIZE_LARGE_TOOLBAR));
    g_assert(px != nullptr);
    px->show();
    add(*px);
}

std::vector<double> parse_pattern(const Glib::ustring& input) {
    std::vector<double> output;
    if (input.empty()) return output;

    std::istringstream stream(input.c_str());
    while (stream) {
        double val;
        stream >> val;
        if (stream) {
            output.push_back(val);
        }
    }

    return output;
}

StrokeStyle::StrokeStyle() :
    Gtk::Box(),
    miterLimitSpin(),
    widthSpin(),
    unitSelector(),
    joinMiter(),
    joinRound(),
    joinBevel(),
    capButt(),
    capRound(),
    capSquare(),
    dashSelector(),
    update(false),
    desktop(nullptr),
    startMarkerConn(),
    midMarkerConn(),
    endMarkerConn(),
    _old_unit(nullptr)
{
    set_name("StrokeSelector");
    table = Gtk::manage(new Gtk::Grid());
    table->set_border_width(4);
    table->set_row_spacing(4);
    table->set_hexpand(false);
    table->set_halign(Gtk::ALIGN_CENTER);
    table->show();
    add(*table);

    Gtk::Box *hb;
    gint i = 0;

    //spw_label(t, C_("Stroke width", "_Width:"), 0, i);

    hb = spw_hbox(table, 3, 1, i);

// TODO: when this is gtkmmified, use an Inkscape::UI::Widget::ScalarUnit instead of the separate
// spinbutton and unit selector for stroke width. In sp_stroke_style_line_update, use
// setHundredPercent to remember the averaged width corresponding to 100%. Then the
// stroke_width_set_unit will be removed (because ScalarUnit takes care of conversions itself)
    widthAdj = new Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(1.0, 0.0, 1000.0, 0.1, 10.0, 0.0));
    widthSpin = new Inkscape::UI::Widget::SpinButton(*widthAdj, 0.1, 3);
    widthSpin->set_tooltip_text(_("Stroke width"));
    widthSpin->show();
    spw_label(table, C_("Stroke width", "_Width:"), 0, i, widthSpin);

    sp_dialog_defocus_on_enter_cpp(widthSpin);

    hb->pack_start(*widthSpin, false, false, 0);
    unitSelector = Gtk::manage(new Inkscape::UI::Widget::UnitMenu());
    unitSelector->setUnitType(Inkscape::Util::UNIT_TYPE_LINEAR);
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    unitSelector->addUnit(*unit_table.getUnit("%"));
    unitSelector->append("hairline", _("Hairline"));
    _old_unit = unitSelector->getUnit();
    if (desktop) {
        unitSelector->setUnit(desktop->getNamedView()->display_units->abbr);
        _old_unit = desktop->getNamedView()->display_units;
    }
    widthSpin->setUnitMenu(unitSelector);
    unitSelector->signal_changed().connect(sigc::mem_fun(*this, &StrokeStyle::unitChangedCB));
    unitSelector->show();

    hb->pack_start(*unitSelector, FALSE, FALSE, 0);
    (*widthAdj)->signal_value_changed().connect(sigc::mem_fun(*this, &StrokeStyle::setStrokeWidth));

    i++;

    /* Dash */
    spw_label(table, _("Dashes:"), 0, i, nullptr); //no mnemonic for now
                                            //decide what to do:
                                            //   implement a set_mnemonic_source function in the
                                            //   Inkscape::UI::Widget::DashSelector class, so that we do not have to
                                            //   expose any of the underlying widgets?
    dashSelector = Gtk::manage(new Inkscape::UI::Widget::DashSelector);
    _pattern = Gtk::make_managed<Gtk::Entry>();

    dashSelector->show();
    dashSelector->set_hexpand();
    dashSelector->set_halign(Gtk::ALIGN_FILL);
    dashSelector->set_valign(Gtk::ALIGN_CENTER);
    table->attach(*dashSelector, 1, i, 3, 1);
    dashSelector->changed_signal.connect(sigc::mem_fun(*this, &StrokeStyle::setStrokeDash));

    i++;

    table->attach(*_pattern, 1, i, 4, 1);
    _pattern_label = spw_label(table, _("_Pattern:"), 0, i, _pattern);
    _pattern_label->set_tooltip_text(_("Repeating \"dash gap ...\" pattern"));
    _pattern->set_no_show_all();
    _pattern_label->set_no_show_all();
    _pattern->signal_changed().connect([=](){
        if (update || _editing_pattern) return;

        auto pat = parse_pattern(_pattern->get_text());
        _editing_pattern = true;
        update = true;
        dashSelector->set_dash(pat, dashSelector->get_offset());
        update = false;
        setStrokeDash();
        _editing_pattern = false;
    });
    update_pattern(0, nullptr);

    i++;

    /* Drop down marker selectors*/
    // TRANSLATORS: Path markers are an SVG feature that allows you to attach arbitrary shapes
    // (arrowheads, bullets, faces, whatever) to the start, end, or middle nodes of a path.

    spw_label(table, _("Markers:"), 0, i, nullptr);

    hb = spw_hbox(table, 1, 1, i);
    i++;

    startMarkerCombo = Gtk::manage(new MarkerComboBox("marker-start", SP_MARKER_LOC_START));
    startMarkerCombo->set_tooltip_text(_("Start Markers are drawn on the first node of a path or shape"));
    startMarkerConn = startMarkerCombo->signal_changed().connect([=]() { markerSelectCB(startMarkerCombo, SP_MARKER_LOC_START); });
    startMarkerCombo->edit_signal.connect([=] { enterEditMarkerMode(SP_MARKER_LOC_START); });
    startMarkerCombo->show();

    hb->pack_start(*startMarkerCombo, true, true, 0);

    midMarkerCombo = Gtk::manage(new MarkerComboBox("marker-mid", SP_MARKER_LOC_MID));
    midMarkerCombo->set_tooltip_text(_("Mid Markers are drawn on every node of a path or shape except the first and last nodes"));
    midMarkerConn = midMarkerCombo->signal_changed().connect([=]() { markerSelectCB(midMarkerCombo, SP_MARKER_LOC_MID); });
    midMarkerCombo->edit_signal.connect([=] { enterEditMarkerMode(SP_MARKER_LOC_MID); });
    midMarkerCombo->show();

    hb->pack_start(*midMarkerCombo, true, true, 0);

    endMarkerCombo = Gtk::manage(new MarkerComboBox("marker-end", SP_MARKER_LOC_END));
    endMarkerCombo->set_tooltip_text(_("End Markers are drawn on the last node of a path or shape"));
    endMarkerConn = endMarkerCombo->signal_changed().connect([=]() { markerSelectCB(endMarkerCombo, SP_MARKER_LOC_END); });
    endMarkerCombo->edit_signal.connect([=] { enterEditMarkerMode(SP_MARKER_LOC_END); });
    endMarkerCombo->show();

    hb->pack_start(*endMarkerCombo, true, true, 0);
    i++;

    /* Join type */
    // TRANSLATORS: The line join style specifies the shape to be used at the
    //  corners of paths. It can be "miter", "round" or "bevel".
    spw_label(table, _("Join:"), 0, i, nullptr);

    hb = spw_hbox(table, 3, 1, i);

    Gtk::RadioButtonGroup joinGrp;

    joinBevel = makeRadioButton(joinGrp, INKSCAPE_ICON("stroke-join-bevel"),
                                hb, STROKE_STYLE_BUTTON_JOIN, "bevel");

    // TRANSLATORS: Bevel join: joining lines with a blunted (flattened) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    joinBevel->set_tooltip_text(_("Bevel join"));

    joinRound = makeRadioButton(joinGrp, INKSCAPE_ICON("stroke-join-round"),
                                hb, STROKE_STYLE_BUTTON_JOIN, "round");

    // TRANSLATORS: Round join: joining lines with a rounded corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    joinRound->set_tooltip_text(_("Round join"));

    joinMiter = makeRadioButton(joinGrp, INKSCAPE_ICON("stroke-join-miter"),
                                hb, STROKE_STYLE_BUTTON_JOIN, "miter");

    // TRANSLATORS: Miter join: joining lines with a sharp (pointed) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    joinMiter->set_tooltip_text(_("Miter join"));

    /* Miterlimit  */
    // TRANSLATORS: Miter limit: only for "miter join", this limits the length
    //  of the sharp "spike" when the lines connect at too sharp an angle.
    // When two line segments meet at a sharp angle, a miter join results in a
    //  spike that extends well beyond the connection point. The purpose of the
    //  miter limit is to cut off such spikes (i.e. convert them into bevels)
    //  when they become too long.
    //spw_label(t, _("Miter _limit:"), 0, i);
    miterLimitAdj = new Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(4.0, 0.0, 100000.0, 0.1, 10.0, 0.0));
    miterLimitSpin = new Inkscape::UI::Widget::SpinButton(*miterLimitAdj, 0.1, 2);
    miterLimitSpin->set_tooltip_text(_("Maximum length of the miter (in units of stroke width)"));
    miterLimitSpin->set_width_chars(6);
    miterLimitSpin->show();
    sp_dialog_defocus_on_enter_cpp(miterLimitSpin);

    hb->pack_start(*miterLimitSpin, false, false, 0);
    (*miterLimitAdj)->signal_value_changed().connect(sigc::mem_fun(*this, &StrokeStyle::setStrokeMiter));
    i++;

    /* Cap type */
    // TRANSLATORS: cap type specifies the shape for the ends of lines
    //spw_label(t, _("_Cap:"), 0, i);
    spw_label(table, _("Cap:"), 0, i, nullptr);

    hb = spw_hbox(table, 3, 1, i);

    Gtk::RadioButtonGroup capGrp;

    capButt = makeRadioButton(capGrp, INKSCAPE_ICON("stroke-cap-butt"),
                                hb, STROKE_STYLE_BUTTON_CAP, "butt");

    // TRANSLATORS: Butt cap: the line shape does not extend beyond the end point
    //  of the line; the ends of the line are square
    capButt->set_tooltip_text(_("Butt cap"));

    capRound = makeRadioButton(capGrp, INKSCAPE_ICON("stroke-cap-round"),
                                hb, STROKE_STYLE_BUTTON_CAP, "round");

    // TRANSLATORS: Round cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are rounded
    capRound->set_tooltip_text(_("Round cap"));

    capSquare = makeRadioButton(capGrp, INKSCAPE_ICON("stroke-cap-square"),
                                hb, STROKE_STYLE_BUTTON_CAP, "square");

    // TRANSLATORS: Square cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are square
    capSquare->set_tooltip_text(_("Square cap"));

    i++;

    /* Paint order */
    // TRANSLATORS: Paint order determines the order the 'fill', 'stroke', and 'markers are painted.
    spw_label(table, _("Order:"), 0, i, nullptr);

    hb = spw_hbox(table, 4, 1, i);

    Gtk::RadioButtonGroup paintOrderGrp;

    paintOrderFSM = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-fsm"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "normal");
    paintOrderFSM->set_tooltip_text(_("Fill, Stroke, Markers")); 

    paintOrderSFM = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-sfm"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "stroke fill markers");
    paintOrderSFM->set_tooltip_text(_("Stroke, Fill, Markers")); 

    paintOrderFMS = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-fms"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "fill markers stroke");
    paintOrderFMS->set_tooltip_text(_("Fill, Markers, Stroke")); 

    i++;

    hb = spw_hbox(table, 4, 1, i);

    paintOrderMFS = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-mfs"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "markers fill stroke");
    paintOrderMFS->set_tooltip_text(_("Markers, Fill, Stroke")); 

    paintOrderSMF = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-smf"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "stroke markers fill");
    paintOrderSMF->set_tooltip_text(_("Stroke, Markers, Fill")); 

    paintOrderMSF = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-msf"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "markers stroke fill");
    paintOrderMSF->set_tooltip_text(_("Markers, Stroke, Fill")); 

    i++;
}

StrokeStyle::~StrokeStyle()
{
}

void StrokeStyle::setDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {

        if (this->desktop) {
            _document_replaced_connection.disconnect();
        }
        this->desktop = desktop;

        if (!desktop) {
            return;
        }

        _document_replaced_connection =
            desktop->connectDocumentReplaced(sigc::mem_fun(this, &StrokeStyle::_handleDocumentReplaced));

        _handleDocumentReplaced(nullptr, desktop->getDocument());

        updateLine();
    }
}

void StrokeStyle::_handleDocumentReplaced(SPDesktop *, SPDocument *document)
{
    for (MarkerComboBox *combo : { startMarkerCombo, midMarkerCombo, endMarkerCombo }) {
        combo->setDocument(document);
    }
}


/**
 * Helper function for creating stroke-style radio buttons.
 *
 * \param[in] grp           The Gtk::RadioButtonGroup in which to add the button
 * \param[in] icon          The icon for the button
 * \param[in] hb            The Gtk::Box container in which to add the button
 * \param[in] button_type   The type (join/cap) for the button
 * \param[in] stroke_style  The style attribute to associate with the button
 *
 * \details After instantiating the button, it is added to a container box and
 *          a handler for the toggle event is connected.
 */
StrokeStyle::StrokeStyleButton *
StrokeStyle::makeRadioButton(Gtk::RadioButtonGroup &grp,
                             char const            *icon,
                             Gtk::Box              *hb,
                             StrokeStyleButtonType  button_type,
                             gchar const           *stroke_style)
{
    g_assert(icon != nullptr);
    g_assert(hb  != nullptr);

    StrokeStyleButton *tb = new StrokeStyleButton(grp, icon, button_type, stroke_style);

    hb->pack_start(*tb, false, false, 0);

    tb->signal_toggled().connect(sigc::bind<StrokeStyleButton *, StrokeStyle *>(
                                     sigc::ptr_fun(&StrokeStyle::buttonToggledCB), tb, this));

    return tb;
}

void StrokeStyle::enterEditMarkerMode(SPMarkerLoc _editMarkerMode)
{
    SPDesktop *desktop = this->desktop;

    if (desktop) {
        set_active_tool(desktop, "Marker");
        Inkscape::UI::Tools::MarkerTool *mt = dynamic_cast<Inkscape::UI::Tools::MarkerTool*>(desktop->event_context);

        if(mt) {
            mt->editMarkerMode = _editMarkerMode;
            mt->selection_changed(desktop->getSelection());
        }
    }
}


bool StrokeStyle::areMarkersBeingUpdated()
{
    return startMarkerCombo->in_update() || midMarkerCombo->in_update() || endMarkerCombo->in_update();
}

/**
 * Handles when user selects one of the markers from the marker combobox.
 * Gets the marker uri string and applies it to all selected
 * items in the current desktop.
 */
void StrokeStyle::markerSelectCB(MarkerComboBox *marker_combo, SPMarkerLoc const which)
{
    if (update || areMarkersBeingUpdated()) {
        return;
    }

    SPDocument *document = desktop->getDocument();
    if (!document) {
        return;
    }

    // Get marker ID; could be empty (to remove marker)
    std::string marker = marker_combo->get_active_marker_uri();

    update = true;

    SPCSSAttr *css = sp_repr_css_attr_new();
    gchar const *combo_id = marker_combo->get_id();
    sp_repr_css_set_property(css, combo_id, marker.c_str());

    Inkscape::Selection *selection = desktop->getSelection();
    auto itemlist= selection->items();
    for(auto i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (!SP_IS_SHAPE(item)) {
            continue;
        }
        Inkscape::XML::Node *selrepr = item->getRepr();
        if (selrepr) {
            sp_repr_css_change_recursive(selrepr, css, "style");
        }

        item->requestModified(SP_OBJECT_MODIFIED_FLAG);
        item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

        DocumentUndo::done(document, _("Set markers"), INKSCAPE_ICON("dialog-fill-and-stroke"));
    }

    /* edit marker mode - update */
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (desktop) {
        Inkscape::UI::Tools::MarkerTool *mt = dynamic_cast<Inkscape::UI::Tools::MarkerTool*>(desktop->event_context);

        if(mt) {
            mt->editMarkerMode = which;
            mt->selection_changed(desktop->getSelection());
        }
    }

    sp_repr_css_attr_unref(css);
    css = nullptr;

    update = false;
};

/**
 * Callback for when UnitMenu widget is modified.
 * Triggers update action.
 */
void StrokeStyle::unitChangedCB()
{
    Inkscape::Util::Unit const *new_unit = unitSelector->getUnit();

    if (_old_unit == new_unit)
        return;

    // If the unit selector is set to hairline, don't do the normal conversion.
    if (isHairlineSelected()) {
        // Force update in setStrokeWidth
        _old_unit = new_unit;
        _last_width = -1;
        setStrokeWidth();
        return;
    }

    if (new_unit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) {
        // Prevent update in setStrokeWidth
        _last_width = 100.0;
        widthSpin->set_value(100);
    } else {
        // Remove the non-scaling-stroke effect and the hairline extensions
        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_unset_property(css, "vector-effect");
        sp_repr_css_unset_property(css, "-inkscape-stroke");
        sp_desktop_set_style(desktop, css);
        sp_repr_css_attr_unref(css);
        css = nullptr;
        if (_old_unit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) {
            // Prevent update of unit (inf-loop) in updateLine
            _old_unit = new_unit;
            // Going from % to any other unit means our widthSpin is completely invalid.
            updateLine();
        } else {
            // Scale the value and record the old_unit
            widthSpin->set_value(Inkscape::Util::Quantity::convert(widthSpin->get_value(), _old_unit, new_unit));
        }
    }
    _old_unit = new_unit;
}

/**
 * Callback for when stroke style widget is modified.
 * Triggers update action.
 */
void
StrokeStyle::selectionModifiedCB(guint flags)
{
    // We care deeply about only updating when the style is updated
    // if we update on other flags, we slow inkscape down when dragging
    if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG)) {
        updateLine();
    }
}

/**
 * Callback for when stroke style widget is changed.
 * Triggers update action.
 */
void
StrokeStyle::selectionChangedCB()
{
    updateLine();
}

/**
 * Get a dash array and offset from the style.
 *
 * Both values are de-scaled by the style's width if needed.
 */
std::vector<double>
StrokeStyle::getDashFromStyle(SPStyle *style, double &offset)
{
    auto prefs = Inkscape::Preferences::get();

    std::vector<double> ret;
    size_t len = style->stroke_dasharray.values.size();

    double scaledash = 1.0;
    if (prefs->getBool("/options/dash/scale", true) && style->stroke_width.computed) {
        scaledash = style->stroke_width.computed;
    }

    offset = style->stroke_dashoffset.value / scaledash;
    for (unsigned i = 0; i < len; i++) {
        ret.push_back(style->stroke_dasharray.values[i].value / scaledash);
    }
    return ret;
}

/**
 * Sets selector widgets' dash style from an SPStyle object.
 */
void
StrokeStyle::setDashSelectorFromStyle(Inkscape::UI::Widget::DashSelector *dsel, SPStyle *style)
{
    double offset = 0;
    auto d = getDashFromStyle(style, offset);
    if (!d.empty()) {
        dsel->set_dash(d, offset);
        update_pattern(d.size(), d.data());
    } else {
        dsel->set_dash(std::vector<double>(), 0.0);
        update_pattern(0, nullptr);
    }
}

void StrokeStyle::update_pattern(int ndash, const double* pattern) {
    if (_editing_pattern || _pattern->has_focus()) return;

    std::ostringstream ost;
    for (int i = 0; i < ndash; ++i) {
        ost << pattern[i] << ' ';
    }
    _pattern->set_text(ost.str().c_str());
    if (ndash > 0) {
        _pattern_label->show();
        _pattern->show();
    }
    else {
        _pattern_label->hide();
        _pattern->hide();
    }
}

/**
 * Sets the join type for a line, and updates the stroke style widget's buttons
 */
void
StrokeStyle::setJoinType (unsigned const jointype)
{
    Gtk::RadioButton *tb = nullptr;
    switch (jointype) {
        case SP_STROKE_LINEJOIN_MITER:
            tb = joinMiter;
            break;
        case SP_STROKE_LINEJOIN_ROUND:
            tb = joinRound;
            break;
        case SP_STROKE_LINEJOIN_BEVEL:
            tb = joinBevel;
            break;
        default:
            // Should not happen
            std::cerr << "StrokeStyle::setJoinType(): Invalid value: " << jointype << std::endl;
            tb = joinMiter;
            break;
    }
    setJoinButtons(tb);
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
void
StrokeStyle::setCapType (unsigned const captype)
{
    Gtk::RadioButton *tb = nullptr;
    switch (captype) {
        case SP_STROKE_LINECAP_BUTT:
            tb = capButt;
            break;
        case SP_STROKE_LINECAP_ROUND:
            tb = capRound;
            break;
        case SP_STROKE_LINECAP_SQUARE:
            tb = capSquare;
            break;
        default:
            // Should not happen
            std::cerr << "StrokeStyle::setCapType(): Invalid value: " << captype << std::endl;
            tb = capButt;
            break;
    }
    setCapButtons(tb);
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
void
StrokeStyle::setPaintOrder (gchar const *paint_order)
{
    Gtk::RadioButton *tb = paintOrderFSM;

    SPIPaintOrder temp;
    temp.read( paint_order );

    if (temp.layer[0] != SP_CSS_PAINT_ORDER_NORMAL) {

        if (temp.layer[0] == SP_CSS_PAINT_ORDER_FILL) {
            if (temp.layer[1] == SP_CSS_PAINT_ORDER_STROKE) {
                tb = paintOrderFSM;
            } else {
                tb = paintOrderFMS;
            }
        } else if (temp.layer[0] == SP_CSS_PAINT_ORDER_STROKE) {
            if (temp.layer[1] == SP_CSS_PAINT_ORDER_FILL) {
                tb = paintOrderSFM;
            } else {
                tb = paintOrderSMF;
            }
        } else {
            if (temp.layer[1] == SP_CSS_PAINT_ORDER_STROKE) {
                tb = paintOrderMSF;
            } else {
                tb = paintOrderMFS;
            }
        }

    }
    setPaintOrderButtons(tb);
}

/**
 * Callback for when stroke style widget is updated, including markers, cap type,
 * join type, etc.
 */
void
StrokeStyle::updateLine()
{
    if (update) {
        return;
    }

    auto *widg = get_parent()->get_parent()->get_parent()->get_parent(); 
    auto dialogbase = dynamic_cast<Inkscape::UI::Dialog::DialogBase*>(widg);
    if (dialogbase && !dialogbase->getShowing()) {
        return;
    }

    update = true;

    Inkscape::Selection *sel = desktop ? desktop->getSelection() : nullptr;

    if (!sel || sel->isEmpty()) {
        // Nothing selected, grey-out all controls in the stroke-style dialog
        table->set_sensitive(false);

        update = false;

        return;
    }

    FillOrStroke kind = STROKE;

    // create temporary style
    SPStyle query(SP_ACTIVE_DOCUMENT);
    // query into it
    int result_sw = sp_desktop_query_style(desktop, &query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
    int result_ml = sp_desktop_query_style(desktop, &query, QUERY_STYLE_PROPERTY_STROKEMITERLIMIT);
    int result_cap = sp_desktop_query_style(desktop, &query, QUERY_STYLE_PROPERTY_STROKECAP);
    int result_join = sp_desktop_query_style(desktop, &query, QUERY_STYLE_PROPERTY_STROKEJOIN);
    int result_order = sp_desktop_query_style(desktop, &query, QUERY_STYLE_PROPERTY_PAINTORDER);

    SPIPaint &targPaint = *query.getFillOrStroke(kind == FILL);

    {
        table->set_sensitive(true);
        widthSpin->set_sensitive(true);

        if (result_sw == QUERY_STYLE_MULTIPLE_AVERAGED) {
            unitSelector->setUnit("%");
        } else if (query.stroke_extensions.hairline) {
            unitSelector->set_active_id("hairline");
        } else {
            // same width, or only one object; no sense to keep percent, switch to absolute
            Inkscape::Util::Unit const *tempunit = unitSelector->getUnit();
            if (tempunit->type != Inkscape::Util::UNIT_TYPE_LINEAR) {
                unitSelector->setUnit(desktop->getNamedView()->display_units->abbr);
            }
        }

        Inkscape::Util::Unit const *unit = unitSelector->getUnit();

        if (query.stroke_extensions.hairline) {
            widthSpin->set_sensitive(false);
            (*widthAdj)->set_value(1);
        } else if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
            double avgwidth = Inkscape::Util::Quantity::convert(query.stroke_width.computed, "px", unit);
            (*widthAdj)->set_value(avgwidth);
        } else {
            (*widthAdj)->set_value(100);
        }

        // if none of the selected objects has a stroke, than quite some controls should be disabled
        // These options should also be disabled for hairlines, since they don't make sense for
        // 0-width lines.
        // The markers might still be shown though, so marker and stroke-width widgets stay enabled
        bool is_enabled = (result_sw != QUERY_STYLE_NOTHING) && !targPaint.isNoneSet()
                           && !query.stroke_extensions.hairline;
        joinMiter->set_sensitive(is_enabled);
        joinRound->set_sensitive(is_enabled);
        joinBevel->set_sensitive(is_enabled);

        miterLimitSpin->set_sensitive(is_enabled);

        capButt->set_sensitive(is_enabled);
        capRound->set_sensitive(is_enabled);
        capSquare->set_sensitive(is_enabled);

        dashSelector->set_sensitive(is_enabled);
        _pattern->set_sensitive(is_enabled);
    }

    if (result_ml != QUERY_STYLE_NOTHING)
        (*miterLimitAdj)->set_value(query.stroke_miterlimit.value); // TODO: reflect averagedness?

    using Inkscape::is_query_style_updateable;
    if (! is_query_style_updateable(result_join)) {
        setJoinType(query.stroke_linejoin.value);
    } else {
        setJoinButtons(nullptr);
    }

    if (! is_query_style_updateable(result_cap)) {
        setCapType (query.stroke_linecap.value);
    } else {
        setCapButtons(nullptr);
    }

    if (! is_query_style_updateable(result_order)) {
        setPaintOrder (query.paint_order.value);
    } else {
        setPaintOrder (nullptr);
    }

    std::vector<SPItem*> const objects(sel->items().begin(), sel->items().end());
    if (objects.size()) {
        SPObject *const object = objects[0];
        SPStyle *const style = object->style;
        /* Markers */
        updateAllMarkers(objects, true); // FIXME: make this desktop query too

        /* Dash */
        setDashSelectorFromStyle(dashSelector, style); // FIXME: make this desktop query too
    }
    table->set_sensitive(true);

    update = false;
}

/**
 * Sets a line's dash properties in a CSS style object.
 */
void
StrokeStyle::setScaledDash(SPCSSAttr *css,
                                int ndash, const double *dash, double offset,
                                double scale)
{
    if (ndash > 0) {
        Inkscape::CSSOStringStream osarray;
        for (int i = 0; i < ndash; i++) {
            osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
                osarray << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", osarray.str().c_str());

        Inkscape::CSSOStringStream osoffset;
        osoffset << offset * scale;
        sp_repr_css_set_property(css, "stroke-dashoffset", osoffset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", nullptr);
    }
}

static inline double calcScaleLineWidth(const double width_typed, SPItem *const item, Inkscape::Util::Unit const *const unit)
{
    if (unit->abbr == "%") {
        auto scale = item->i2doc_affine().descrim();;
        const gdouble old_w = item->style->stroke_width.computed;
        return (old_w * width_typed / 100) * scale;
    } else if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
        return Inkscape::Util::Quantity::convert(width_typed, unit, "px");
    }
    return width_typed;
}

/**
 * Set the stroke width and adjust the dash pattern if needed.
 */
void StrokeStyle::setStrokeWidth()
{
    double width_typed = (*widthAdj)->get_value();

    // Don't change the selection if an update is happening,
    // but also store the value for later comparison.
    if (update || fabs(_last_width - width_typed) < 1E-6) {
        _last_width = width_typed;
        return;
    }
    update = true;

    auto prefs = Inkscape::Preferences::get();
    auto unit = unitSelector->getUnit();

    SPCSSAttr *css = sp_repr_css_attr_new();
    if (isHairlineSelected()) {
        /* For renderers that don't understand -inkscape-stroke:hairline, fall back to 1px non-scaling */
        width_typed = 1;
        sp_repr_css_set_property(css, "vector-effect", "non-scaling-stroke");
        sp_repr_css_set_property(css, "-inkscape-stroke", "hairline");
    } else {
        sp_repr_css_unset_property(css, "vector-effect");
        sp_repr_css_unset_property(css, "-inkscape-stroke");
    }

    for (auto item : desktop->getSelection()->items()) {
        const double width = calcScaleLineWidth(width_typed, item, unit);
        sp_repr_css_set_property_double(css, "stroke-width", width);

        if (prefs->getBool("/options/dash/scale", true)) {
            // This will read the old stroke-width to un-scale the pattern.
            double offset = 0;
            auto dash = getDashFromStyle(item->style, offset);
            setScaledDash(css, dash.size(), dash.data(), offset, width);
        }
        sp_desktop_apply_css_recursive (item, css, true);
    }
    sp_desktop_set_style (desktop, css, false);

    sp_repr_css_attr_unref(css);
    DocumentUndo::done(desktop->getDocument(), _("Set stroke width"),
                       INKSCAPE_ICON("dialog-fill-and-stroke"));

    if (unit->abbr == "%") {
        // reset to 100 percent
        _last_width = 100.0;
        (*widthAdj)->set_value(100.0);
    } else {
        _last_width = width_typed;
    }
    update = false;
}

/**
 * Set the stroke dash pattern, scale to the existing width if needed
 */
void StrokeStyle::setStrokeDash()
{
    if (update) return;
    update = true;

    auto document = desktop->getDocument();
    auto prefs = Inkscape::Preferences::get();

    double offset = 0;
    const auto& dash = dashSelector->get_dash(&offset);
    update_pattern(dash.size(), dash.data());

    SPCSSAttr *css = sp_repr_css_attr_new();
    for (auto item : desktop->getSelection()->items()) {
        double scale = item->i2doc_affine().descrim();
        if(prefs->getBool("/options/dash/scale", true)) {
            scale = item->style->stroke_width.computed * scale;
        }

        setScaledDash(css, dash.size(), dash.data(), offset, scale);
        sp_desktop_apply_css_recursive (item, css, true);
    }
    sp_desktop_set_style (desktop, css, false);

    sp_repr_css_attr_unref(css);
    DocumentUndo::done(document, _("Set stroke dash"),
                       INKSCAPE_ICON("dialog-fill-and-stroke"));
    update = false;
}

/**
 * Set the Miter Limit value only.
 */
void StrokeStyle::setStrokeMiter()
{
    if (update) return;
    update = true;

    SPCSSAttr *css = sp_repr_css_attr_new();
    auto value = (*miterLimitAdj)->get_value();
    sp_repr_css_set_property_double(css, "stroke-miterlimit", value);

    for (auto item : desktop->getSelection()->items()) {
        sp_desktop_apply_css_recursive(item, css, true);
    }
    sp_desktop_set_style (desktop, css, false);
    sp_repr_css_attr_unref(css);
    DocumentUndo::done(desktop->getDocument(), _("Set stroke miter"),
                       INKSCAPE_ICON("dialog-fill-and-stroke"));
    update = false;
}

/**
 * Returns whether the currently selected stroke width is "hairline"
 *
 */
bool
StrokeStyle::isHairlineSelected() const
{
    return unitSelector->get_active_id() == "hairline";
}


/**
 * This routine handles toggle events for buttons in the stroke style dialog.
 *
 * When activated, this routine gets the data for the various widgets, and then
 * calls the respective routines to update css properties, etc.
 *
 */
void StrokeStyle::buttonToggledCB(StrokeStyleButton *tb, StrokeStyle *spw)
{
    if (spw->update) {
        return;
    }

    if (tb->get_active()) {
        if (tb->get_button_type() == STROKE_STYLE_BUTTON_JOIN) {
            spw->miterLimitSpin->set_sensitive(!strcmp(tb->get_stroke_style(), "miter"));
        }

        /* TODO: Create some standardized method */
        SPCSSAttr *css = sp_repr_css_attr_new();

        switch (tb->get_button_type()) {
            case STROKE_STYLE_BUTTON_JOIN: 
                sp_repr_css_set_property(css, "stroke-linejoin", tb->get_stroke_style());
                sp_desktop_set_style (spw->desktop, css);
                spw->setJoinButtons(tb);
                break;
            case STROKE_STYLE_BUTTON_CAP:
                sp_repr_css_set_property(css, "stroke-linecap", tb->get_stroke_style());
                sp_desktop_set_style (spw->desktop, css);
                spw->setCapButtons(tb);
                break;
            case STROKE_STYLE_BUTTON_ORDER:
                sp_repr_css_set_property(css, "paint-order", tb->get_stroke_style());
                sp_desktop_set_style (spw->desktop, css);
                //spw->setPaintButtons(tb);
        }

        sp_repr_css_attr_unref(css);
        css = nullptr;

        DocumentUndo::done(spw->desktop->getDocument(), _("Set stroke style"), INKSCAPE_ICON("dialog-fill-and-stroke"));
    }
}

/**
 * Updates the join style toggle buttons
 */
void
StrokeStyle::setJoinButtons(Gtk::ToggleButton *active)
{
    joinMiter->set_active(active == joinMiter);
    miterLimitSpin->set_sensitive(active == joinMiter && !isHairlineSelected());
    joinRound->set_active(active == joinRound);
    joinBevel->set_active(active == joinBevel);
}

/**
 * Updates the cap style toggle buttons
 */
void
StrokeStyle::setCapButtons(Gtk::ToggleButton *active)
{
    capButt->set_active(active == capButt);
    capRound->set_active(active == capRound);
    capSquare->set_active(active == capSquare);
}


/**
 * Updates the paint order style toggle buttons
 */
void
StrokeStyle::setPaintOrderButtons(Gtk::ToggleButton *active)
{
    paintOrderFSM->set_active(active == paintOrderFSM);
    paintOrderSFM->set_active(active == paintOrderSFM);
    paintOrderFMS->set_active(active == paintOrderFMS);
    paintOrderMFS->set_active(active == paintOrderMFS);
    paintOrderSMF->set_active(active == paintOrderSMF);
    paintOrderMSF->set_active(active == paintOrderMSF);
}


/**
 * Recursively builds a simple list from an arbitrarily complex selection
 * of items and grouped items
 */
static void buildGroupedItemList(SPObject *element, std::vector<SPObject*> &simple_list)
{
    if (SP_IS_GROUP(element)) {
        for (SPObject *i = element->firstChild(); i; i = i->getNext()) {
            buildGroupedItemList(i, simple_list);
        }
    } else {
        simple_list.push_back(element);
    }
}


/**
 * Updates the marker combobox to highlight the appropriate marker and scroll to
 * that marker.
 */
void
StrokeStyle::updateAllMarkers(std::vector<SPItem*> const &objects, bool skip_undo)
{
    struct { MarkerComboBox *key; int loc; } const keyloc[] = {
            { startMarkerCombo, SP_MARKER_LOC_START },
            { midMarkerCombo, SP_MARKER_LOC_MID },
            { endMarkerCombo, SP_MARKER_LOC_END }
    };

    bool all_texts = true;

    auto simplified_list = std::vector<SPObject *>();
    for (SPItem *item : objects) {
        buildGroupedItemList(item, simplified_list);
    }

    for (SPObject *object : simplified_list) {
        if (!SP_IS_TEXT(object)) {
            all_texts = false;
            break;
        }
    }

    // We show markers of the last object in the list only
    // FIXME: use the first in the list that has the marker of each type, if any

    for (auto const &markertype : keyloc) {
        // For all three marker types,

        // find the corresponding combobox item
        MarkerComboBox *combo = markertype.key;

        // Quit if we're in update state
        if (combo->in_update()) {
            return;
        }

        // Per SVG spec, text objects cannot have markers; disable combobox if only texts are selected
        // They should also be disabled for hairlines, since scaling against a 0-width line doesn't
        // make sense.
        combo->set_sensitive(!all_texts && !isHairlineSelected());

        SPObject *marker = nullptr;

        if (!all_texts && !isHairlineSelected()) {
            for (SPObject *object : simplified_list) {
                char const *value = object->style->marker_ptrs[markertype.loc]->value();

                // If the object has this type of markers,
                if (value == nullptr)
                    continue;

                // Extract the name of the marker that the object uses
                marker = getMarkerObj(value, object->document);
            }
        }

        // Scroll the combobox to that marker
        combo->set_current(marker);
    }

}

} // namespace Widget
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
