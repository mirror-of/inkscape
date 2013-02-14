/**
 * @file
 *
 * Paper-size widget and helper functions
 */
/*
 * Author:
 *   Marcel Lancelle (vaifrax) <vaifrax@marcel-lancelle.de>
 *   Code mostly copied/reused from ui/widget/page-sizer and display/canvas-grid
 *
 * Copyright (C) 2000 - 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ruler-coords.h"

#include <cmath>
#include <string>
#include <string.h>
#include <vector>

#include <glibmm/i18n.h>

#include <2geom/transforms.h>

#include "desktop-handles.h"
#include "document.h"
#include "desktop.h"
#include "helper/action.h"
#include "helper/units.h"
#include "inkscape.h"
#include "sp-namedview.h"
#include "sp-root.h"
#include "ui/widget/button.h"
#include "ui/widget/scalar-unit.h"
#include "verbs.h"
#include "xml/node.h"
#include "xml/repr.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Widget {

    /** \note
     * This only affects the display of the coordinate system on the rulers.
     * The underlying coordinate system is not changed (yet), leading to
     * potential problems:
     * - more transforms, costing more time and precision
     * - users might want to export / use values in svg in the specified
     *   coordinate system
     */
    /** \todo
     * Other text input of dimensions, like the width of a box should be able
     * to be specified in the ruler coordinate system?
     * There should be buttons to set origin to top/center/bottom and
     * left/center/right of the current page dimension for convenience.
     */


//########################################################################
//#  R U L E R   C O O R D   S Y S T E M
//########################################################################

//The default unit for this widget and its calculations
//static const SPUnit _px_unit = sp_unit_get_by_id (SP_UNIT_PX);


/**
 * Constructor
 */
RulerCoordSystem::RulerCoordSystem(Registry & _wr)
    : Gtk::VBox(false,4),
      _rum_deflt(_("Default _units:"), "inkscape:document-units", _wr),
      _rsu_off_x(_("Ruler Origin X:"), _("Origin of ruler coordinate system [default units]"), "inkscape:ruleroffsetx", _rum_deflt, _wr),
      _rsu_off_y(_("Ruler Origin Y:"), _("Origin of ruler coordinate system [default units]"), "inkscape:ruleroffsety", _rum_deflt, _wr),
      _rs_mul_x(_("Ruler Multiplier X:"), _("Ruler values are default units multiplied with this number"), "inkscape:rulermultiplierx", _wr),
      _rs_mul_y(_("Ruler Multiplier Y:"), _("Ruler values are default units multiplied with this number"), "inkscape:rulermultipliery", _wr),
/*
      _dimensionUnits( _("U_nits:"), "units", _wr ),
      _dimensionWidth( _("_Width:"), _("Width of paper"), "width", _dimensionUnits, _wr ),
      _dimensionHeight( _("_Height:"), _("Height of paper"), "height", _dimensionUnits, _wr ),
      _marginTop( _("T_op margin:"), _("Top margin"), "fit-margin-top", _wr ),
      _marginLeft( _("L_eft:"), _("Left margin"), "fit-margin-left", _wr),
      _marginRight( _("Ri_ght:"), _("Right margin"), "fit-margin-right", _wr),
      _marginBottom( _("Botto_m:"), _("Bottom margin"), "fit-margin-bottom", _wr),
      _lockMarginUpdate(false),
*/
      _widgetRegistry(&_wr)
{
g_message("### rcs() 1");
	// set precision and values of scalar entry boxes for ruler scaling and offset
    _wr.setUpdating (true);
    _rs_mul_x.setDigits(5);
    _rs_mul_y.setDigits(5);
    _rsu_off_x.setDigits(5);
    _rsu_off_y.setDigits(5);
    _wr.setUpdating (false);
g_message("### rcs() 2");

    pack_start(_rum_deflt, false, false, 0);
    pack_start(_rsu_off_x, false, false, 0);
    pack_start(_rsu_off_y, false, false, 0);
    pack_start(_rs_mul_x, false, false, 0);
    pack_start(_rs_mul_y, false, false, 0);

g_message("### rcs() 3");
/*
    //# Set up the Paper Size combo box
    _paperSizeListStore = Gtk::ListStore::create(_paperSizeListColumns);
    _paperSizeList.set_model(_paperSizeListStore);
    _paperSizeList.append_column(_("Name"),
                                 _paperSizeListColumns.nameColumn);
    _paperSizeList.append_column(_("Description"),
                                 _paperSizeListColumns.descColumn);
    _paperSizeList.set_headers_visible(false);
    _paperSizeListSelection = _paperSizeList.get_selection();
    _paper_size_list_connection =
        _paperSizeListSelection->signal_changed().connect (
            sigc::mem_fun (*this, &PageSizer::on_paper_size_list_changed));
    _paperSizeListScroller.add(_paperSizeList);
    _paperSizeListScroller.set_shadow_type(Gtk::SHADOW_IN);
    _paperSizeListScroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _paperSizeListScroller.set_size_request(-1, 90);

    pack_start (_paperSizeListScroller, true, true, 0);

    //## Set up orientation radio buttons
    pack_start (_orientationBox, false, false, 0);
    _orientationLabel.set_label(_("Orientation:"));
    _orientationBox.pack_start(_orientationLabel, false, false, 0);
    _landscapeButton.set_use_underline();
    _landscapeButton.set_label(_("_Landscape"));
    _landscapeButton.set_active(true);
    Gtk::RadioButton::Group group = _landscapeButton.get_group();
    _orientationBox.pack_end (_landscapeButton, false, false, 5);
    _portraitButton.set_use_underline();
    _portraitButton.set_label(_("_Portrait"));
    _portraitButton.set_active(true);
    _orientationBox.pack_end (_portraitButton, false, false, 5);
    _portraitButton.set_group (group);
    _portraitButton.set_active (true);
    // Setting default custom unit to document unit
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);
    _wr.setUpdating (true);
    if (nv->units) {
        _dimensionUnits.setUnit(nv->units);
    } else if (nv->doc_units) {
        _dimensionUnits.setUnit(nv->doc_units);
    }
    _wr.setUpdating (false);
    
    //## Set up custom size frame
    _customFrame.set_label(_("Custom size"));
    pack_start (_customFrame, false, false, 0);
    _customFrame.add(_customDimTable);

    _customDimTable.set_border_width(4);

#if WITH_GTKMM_3_0
    _customDimTable.set_row_spacing(4);
    _customDimTable.set_column_spacing(4);

    _dimensionWidth.set_hexpand();
    _dimensionWidth.set_vexpand();
    _customDimTable.attach(_dimensionWidth,        0, 0, 1, 1);

    _dimensionUnits.set_hexpand();
    _dimensionUnits.set_vexpand();
    _customDimTable.attach(_dimensionUnits,        1, 0, 1, 1);

    _dimensionHeight.set_hexpand();
    _dimensionHeight.set_vexpand();
    _customDimTable.attach(_dimensionHeight,       0, 1, 1, 1);

    _fitPageMarginExpander.set_hexpand();
    _fitPageMarginExpander.set_vexpand();
    _customDimTable.attach(_fitPageMarginExpander, 0, 2, 2, 1);
#else
    _customDimTable.resize(3, 2);
    _customDimTable.set_row_spacings(4);
    _customDimTable.set_col_spacings(4);
    _customDimTable.attach(_dimensionWidth,        0,1, 0,1);
    _customDimTable.attach(_dimensionUnits,        1,2, 0,1);
    _customDimTable.attach(_dimensionHeight,       0,1, 1,2);
    _customDimTable.attach(_fitPageMarginExpander, 0,2, 2,3);
#endif
    
    _dimTabOrderGList = NULL;
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _dimensionWidth.gobj());
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _dimensionHeight.gobj());
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _dimensionUnits.gobj());
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _fitPageMarginExpander.gobj());
    Glib::ListHandle<Widget *> dimFocusChain(_dimTabOrderGList, Glib::OWNERSHIP_NONE);
    _customDimTable.set_focus_chain(dimFocusChain);    

    //## Set up fit page expander
    _fitPageMarginExpander.set_use_underline();
    _fitPageMarginExpander.set_label(_("Resi_ze page to content..."));
    _fitPageMarginExpander.add(_marginTable);
    
    //## Set up margin settings
    _marginTable.set_border_width(4);

#if WITH_GTKMM_3_0
    _marginTable.set_row_spacing(4);
    _marginTable.set_column_spacing(4);

    _marginTopAlign.set_hexpand();
    _marginTopAlign.set_vexpand();
    _marginTable.attach(_marginTopAlign,     0, 0, 2, 1);

    _marginLeftAlign.set_hexpand();
    _marginLeftAlign.set_vexpand();
    _marginTable.attach(_marginLeftAlign,    0, 1, 1, 1);

    _marginRightAlign.set_hexpand();
    _marginRightAlign.set_vexpand();
    _marginTable.attach(_marginRightAlign,   1, 1, 1, 1);

    _marginBottomAlign.set_hexpand();
    _marginBottomAlign.set_vexpand();
    _marginTable.attach(_marginBottomAlign,  0, 2, 2, 1);

    _fitPageButtonAlign.set_hexpand();
    _fitPageButtonAlign.set_vexpand();
    _marginTable.attach(_fitPageButtonAlign, 0, 3, 2, 1);
#else
    _marginTable.set_border_width(4);
    _marginTable.set_row_spacings(4);
    _marginTable.set_col_spacings(4);
    _marginTable.attach(_marginTopAlign,     0,2, 0,1);
    _marginTable.attach(_marginLeftAlign,    0,1, 1,2);
    _marginTable.attach(_marginRightAlign,   1,2, 1,2);
    _marginTable.attach(_marginBottomAlign,  0,2, 2,3);
    _marginTable.attach(_fitPageButtonAlign, 0,2, 3,4);
#endif
    
    _marginTopAlign.set(0.5, 0.5, 0.0, 1.0);
    _marginTopAlign.add(_marginTop);
    _marginLeftAlign.set(0.0, 0.5, 0.0, 1.0);
    _marginLeftAlign.add(_marginLeft);
    _marginRightAlign.set(1.0, 0.5, 0.0, 1.0);
    _marginRightAlign.add(_marginRight);
    _marginBottomAlign.set(0.5, 0.5, 0.0, 1.0);
    _marginBottomAlign.add(_marginBottom);
    
    _fitPageButtonAlign.set(0.5, 0.5, 0.0, 1.0);
    _fitPageButtonAlign.add(_fitPageButton);
    _fitPageButton.set_use_underline();
    _fitPageButton.set_label(_("_Resize page to drawing or selection"));
    _fitPageButton.set_tooltip_text(_("Resize the page to fit the current selection, or the entire drawing if there is no selection"));
*/

/*
    0,                 &_rum_deflt,
    0,                 &_rsu_off_x,
    0,                 &_rsu_off_y,
    0,                 &_rs_mul_x,
    0,                 &_rs_mul_y,
*/
}


/**
 * Destructor
 */
RulerCoordSystem::~RulerCoordSystem()
{
//    g_list_free(_dimTabOrderGList);
}



/**
 * Initialize this widget
 */
void
RulerCoordSystem::init ()
{
/*
	_landscape_connection = _landscapeButton.signal_toggled().connect (sigc::mem_fun (*this, &PageSizer::on_landscape));
    _portrait_connection = _portraitButton.signal_toggled().connect (sigc::mem_fun (*this, &PageSizer::on_portrait));
    _changedw_connection = _dimensionWidth.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_value_changed));
    _changedh_connection = _dimensionHeight.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_value_changed));
    _fitPageButton.signal_clicked().connect(sigc::mem_fun(*this, &PageSizer::fire_fit_canvas_to_selection_or_drawing));
*/
//    unit    = SP_UNIT_PX;

g_message("### rcs:init() 1");

    show_all_children();
g_message("### rcs:init() 2");
}

/**
 * Update dialog widgets from desktop (current document).
 */
void
RulerCoordSystem::updateWidgetsFromDoc() {
	g_message("### rcs:updatewfd() 1");
/*
 *     static bool _called = false;
    if (_called) {
        return;
    }

    _called = true;

    _changedw_connection.block();
    _changedh_connection.block();
*/
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);

    if (nv->doc_units)
        _rum_deflt.setUnit (nv->doc_units);
    _rs_mul_x.setValue (nv->rulermultiplierx);
    _rs_mul_y.setValue (nv->rulermultipliery);
    _rsu_off_x.setValueKeepUnit (nv->ruleroffsetx, "px");
    _rsu_off_y.setValueKeepUnit (nv->ruleroffsety, "px");
/*
    _changedw_connection.unblock();
    _changedh_connection.unblock();

    _called = false;
*/
}


/**
 * Set document dimensions (if not called by Doc prop's update()) and
 * set the PageSizer's widgets and text entries accordingly. If
 * 'changeList' is true, then adjust the paperSizeList to show the closest
 * standard page size.
 *
 * \param w, h given in px
 * \param changeList whether to modify the paper size list
 */
/*
void
RulerCoordSystem::setDim (double w, double h, bool changeList)
{
    static bool _called = false;
    if (_called) {
        return;
    }

    _called = true;

    _changedw_connection.block();
    _changedh_connection.block();



    if (SP_ACTIVE_DESKTOP && !_widgetRegistry->isUpdating()) {
        SPDocument *doc = sp_desktop_document(SP_ACTIVE_DESKTOP);
        double const old_height = doc->getHeight();
        doc->setWidth (w, &_px_unit);
        doc->setHeight (h, &_px_unit);
        // The origin for the user is in the lower left corner; this point should remain stationary when
        // changing the page size. The SVG's origin however is in the upper left corner, so we must compensate for this
        Geom::Translate const vert_offset(Geom::Point(0, (old_height - h)));
        doc->getRoot()->translateChildItems(vert_offset);
        DocumentUndo::done(doc, SP_VERB_NONE, _("Set page size"));
    }

    if ( w != h ) {
        _landscapeButton.set_sensitive(true);
        _portraitButton.set_sensitive (true);
        _landscape = ( w > h );
        _landscapeButton.set_active(_landscape ? true : false);
        _portraitButton.set_active (_landscape ? false : true);
    } else {
        _landscapeButton.set_sensitive(false);
        _portraitButton.set_sensitive (false);
    }

    if (changeList)
        {
        Gtk::TreeModel::Row row = (*find_paper_size(w, h));
        if (row)
            _paperSizeListSelection->select(row);
        }

    Unit const& unit = _dimensionUnits.getUnit();
    _dimensionWidth.setValue (w / unit.factor);
    _dimensionHeight.setValue (h / unit.factor);

    _changedw_connection.unblock();
    _changedh_connection.unblock();

    _called = false;
}
*/
/**
 * Updates the scalar widgets for the fit margins.  (Just changes the value
 * of the ui widgets to match the xml).
 */
/*
void 
RulerCoordSystem::updateFitMarginsUI(Inkscape::XML::Node *nv_repr)
{
    if (!_lockMarginUpdate) {
        double value = 0.0;
        if (sp_repr_get_double(nv_repr, "fit-margin-top", &value)) {
            _marginTop.setValue(value);
        }
        if (sp_repr_get_double(nv_repr, "fit-margin-left", &value)) {
            _marginLeft.setValue(value);
        }
        if (sp_repr_get_double(nv_repr, "fit-margin-right", &value)) {
            _marginRight.setValue(value);
        }
        if (sp_repr_get_double(nv_repr, "fit-margin-bottom", &value)) {
            _marginBottom.setValue(value);
        }
    }
}
*/


/**
 * Tell the desktop to fit the page size to the selection or drawing.
 */
/*
void
RulerCoordSystem::fire_fit_canvas_to_selection_or_drawing()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }
    SPDocument *doc;
    SPNamedView *nv;
    Inkscape::XML::Node *nv_repr;
	
    if ((doc = sp_desktop_document(SP_ACTIVE_DESKTOP))
        && (nv = sp_document_namedview(doc, 0))
        && (nv_repr = nv->getRepr())) {
        _lockMarginUpdate = true;
        sp_repr_set_svg_double(nv_repr, "fit-margin-top", _marginTop.getValue());
        sp_repr_set_svg_double(nv_repr, "fit-margin-left", _marginLeft.getValue());
        sp_repr_set_svg_double(nv_repr, "fit-margin-right", _marginRight.getValue());
        sp_repr_set_svg_double(nv_repr, "fit-margin-bottom", _marginBottom.getValue());
        _lockMarginUpdate = false;
    }

    Verb *verb = Verb::get( SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING );
    if (verb) {
        SPAction *action = verb->get_action(dt);
        if (action) {
            sp_action_perform(action, NULL);
        }
    }
}
*/


/**
 * Callback for the dimension widgets
 */
void
RulerCoordSystem::on_value_changed()
{
g_message("### rcs:on_val_changed() 1");
    if (_widgetRegistry->isUpdating()) return;

g_message("### rcs:on_val_changed() 1");

/*
    setDim (_dimensionWidth.getValue("px"),
            _dimensionHeight.getValue("px"));
*/
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
