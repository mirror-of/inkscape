/**
 * \brief Scalar Unit Widget - A labelled text box, with spin buttons and
 *        optional icon or suffix, for entering the values of various unit
 *        types.
 *
 * A ScalarUnit is a control for entering, viewing, or manipulating
 * numbers with units.  This differs from ordinary numbers like 2 or
 * 3.14 because the number portion of a scalar *only* has meaning
 * when considered with its unit type.  For instance, 12 m and 12 in
 * have very different actual values, but 1 m and 100 cm have the same
 * value.  The ScalarUnit allows us to abstract the presentation of
 * the scalar to the user from the internal representations used by
 * the program.
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/spinbutton.h>
#include "scalar-unit.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a ScalarUnit
 *
 * \param label      Label.
 * \param unit_type  Unit type (defaults to UNIT_TYPE_LINEAR).
 * \param suffix     Suffix, placed after the widget (defaults to "").
 * \param icon       Icon filename, placed before the label (defaults to "").
 * \param unit_menu  UnitMenu drop down; if not specified, one will be created
 *                   and displayed after the widget (defaults to NULL).
 * \param mnemonic   Mnemonic toggle; if true, an underscore (_) in the label
 *                   indicates the next character should be used for the
 *                   mnemonic accelerator key (defaults to false).
 */
ScalarUnit::ScalarUnit(Glib::ustring const &label,
                       UnitType unit_type,
                       Glib::ustring const &suffix,
                       Glib::ustring const &icon,
                       UnitMenu *unit_menu,
                       bool mnemonic)
    : Scalar(label, suffix, icon, mnemonic),
      _unit_menu(unit_menu)
{
    if (_unit_menu == NULL) {
        _unit_menu = new UnitMenu();
        g_assert(_unit_menu);
        _unit_menu->setUnitType(unit_type);
        pack_start(*Gtk::manage(_unit_menu), false, false, 4);

    } else {
        _unit_menu->signal_changed()
            .connect_notify(sigc::mem_fun(*this, &ScalarUnit::on_unit_changed));
    }
}

/**
 * Initializes the scalar based on the settings in _unit_menu.
 * Requires that _unit_menu has already been initialized.
 */
void
ScalarUnit::initScalar(double min_value, double max_value)
{
    g_assert(_unit_menu != NULL);
    Scalar::setDigits(_unit_menu->getDefaultDigits());
    Scalar::setIncrements(_unit_menu->getDefaultStep(),
                          _unit_menu->getDefaultPage());
    Scalar::setRange(min_value, max_value);
}

/** Sets the unit for the ScalarUnit widget */
bool
ScalarUnit::setUnit(Glib::ustring const &unit) {
    g_assert(_unit_menu != NULL);
    // First set the unit
    if (!_unit_menu->setUnit(unit)) {
        return false;
    }
    lastUnits = unit;
    return true;
}

/** Gets the object for the currently selected unit */
Unit
ScalarUnit::getUnit() const {
    g_assert(_unit_menu != NULL);
    return _unit_menu->getUnit();
}

/** Gets the UnitType ID for the unit */
UnitType
ScalarUnit::getUnitType() const {
    g_assert(_unit_menu);
    return _unit_menu->getUnitType();
}

/** Sets the number and unit system */
void
ScalarUnit::setValue(double number, Glib::ustring const &units) {
    g_assert(_unit_menu != NULL);
    _unit_menu->setUnit(units);
    Scalar::setValue(number);
}

/** Returns the value in the given unit system */
double
ScalarUnit::getValue(Glib::ustring const &unit_name) const {
    g_assert(_unit_menu != NULL);
    if (unit_name == "") {
        // Return the value in the default units
        return Scalar::getValue();
    } else {
        double conversion = _unit_menu->getConversion(unit_name);
        return conversion * Scalar::getValue();
    }
}



/** Signal handler for updating the suffix label when unit is changed */
void
ScalarUnit::on_unit_changed()
{
    g_assert(_unit_menu != NULL);
    Glib::ustring abbr = _unit_menu->getUnitAbbr();
    _suffix->set_label(abbr);
    //Show the differences when units are changed
    double conversion = _unit_menu->getConversion(lastUnits);
    double convertedVal = Scalar::getValue() / conversion;
    Scalar::setValue(convertedVal);
    lastUnits = abbr;
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
