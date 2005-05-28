/**
 * \brief Scalar Unit Widget - A labelled text box, with spin buttons and
 *        optional icon or suffix, for entering the values of various unit
 *        types.
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_SCALAR_UNIT_H
#define INKSCAPE_UI_WIDGET_SCALAR_UNIT_H

#include "scalar.h"
#include "unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ScalarUnit : public Scalar
{
public:
    ScalarUnit(Glib::ustring const &label,
               UnitType unit_type = UNIT_TYPE_LINEAR,
               Glib::ustring const &suffix = "",
               Glib::ustring const &icon = "",
               UnitMenu *unit_menu = NULL,
               bool mnemonic = false);

    void      initScalar(double min_value, double max_value);

    Unit      getUnit() const;
    UnitType  getUnitType() const;
    double    getValue(Glib::ustring const &units) const;

    bool      setUnit(Glib::ustring const &units);
    void      setValue(double number, Glib::ustring const &units);

    void on_unit_changed();

protected:
    UnitMenu  *_unit_menu;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SCALAR_UNIT_H

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
