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
      _rsu_off_x(_("Ruler Origin X:"), _("Origin of ruler coordinate system [default units]"), "inkscape:ruleroffsetx", _rum_deflt, _wr, NULL, NULL, true),
      _rsu_off_y(_("Ruler Origin Y:"), _("Origin of ruler coordinate system [default units]"), "inkscape:ruleroffsety", _rum_deflt, _wr, NULL, NULL, true),
      _rs_mul_x(_("Ruler Multiplier X:"), _("Ruler values are default units multiplied with this number"), "inkscape:rulermultiplierx", _wr, NULL, NULL, true),
      _rs_mul_y(_("Ruler Multiplier Y:"), _("Ruler values are default units multiplied with this number"), "inkscape:rulermultipliery", _wr, NULL, NULL, true),
      _widgetRegistry(&_wr)
{
	// set precision and values of scalar entry boxes for ruler scaling and offset
    _wr.setUpdating (true);
    _rs_mul_x.setDigits(5);
    _rs_mul_y.setDigits(5);
    _rsu_off_x.setDigits(5);
    _rsu_off_y.setDigits(5);
    _wr.setUpdating (false);

    pack_start(_rum_deflt, false, false, 0);
    pack_start(_rsu_off_x, false, false, 0);
    pack_start(_rsu_off_y, false, false, 4);
    pack_start(_rs_mul_x, false, false, 0);
    pack_start(_rs_mul_y, false, false, 0);

/*
_rsu_off_x.setProgrammatically = false;
_rsu_off_y.setProgrammatically = false;
_rs_mul_x.setProgrammatically = false;
_rs_mul_y.setProgrammatically = false;
*/
}


/**
 * Destructor
 */
RulerCoordSystem::~RulerCoordSystem()
{
}



/**
 * Initialize this widget
 */
void
RulerCoordSystem::init ()
{
    show_all_children();
}

/**
 * Update dialog widgets from desktop (current document).
 */
void
RulerCoordSystem::updateWidgetsFromDoc() {

    static bool _called = false;
    if (_called) {
        return;
    }

    _called = true;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);

    if (nv->doc_units)
        _rum_deflt.setUnit (nv->doc_units);
    _rs_mul_x.setValue (nv->rulermultiplierx);
    _rs_mul_y.setValue (nv->rulermultipliery);
    _rsu_off_x.setValueKeepUnit (nv->ruleroffsetx, "px"); // todo: check: is "px" correct?
    _rsu_off_y.setValueKeepUnit (nv->ruleroffsety, "px");

    _called = false;

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
