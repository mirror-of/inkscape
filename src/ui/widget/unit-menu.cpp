// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cmath>

#include "unit-menu.h"

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Widget {

UnitMenu::UnitMenu(Gtk::ComboBoxText* external_combo) : _type(UNIT_TYPE_NONE)
{
    _combo = external_combo ? external_combo : this;
    _combo->set_active(0);
    _combo->add_events(Gdk::SCROLL_MASK | Gdk::SMOOTH_SCROLL_MASK);
    if (external_combo) {
        _combo->signal_scroll_event().connect([](GdkEventScroll*){ return true; }, false);
    }
}

UnitMenu::~UnitMenu() = default;

bool UnitMenu::setUnitType(UnitType unit_type) 
{
    // Expand the unit widget with unit entries from the unit table
    UnitTable::UnitMap m = unit_table.units(unit_type);

    for (auto & i : m) {
        _combo->append(i.first);
    }
    _type = unit_type;
    _combo->set_active_text(unit_table.primary(unit_type));

    return true;
}

bool UnitMenu::resetUnitType(UnitType unit_type) 
{
    _combo->remove_all();

    return setUnitType(unit_type);
}

void UnitMenu::addUnit(Unit const& u)
{
    unit_table.addUnit(u, false);
    _combo->append(u.abbr);
}

Unit const * UnitMenu::getUnit() const
{
    if (_combo->get_active_text() == "") {
        g_assert(_type != UNIT_TYPE_NONE);
        return unit_table.getUnit(unit_table.primary(_type));
    }
    return unit_table.getUnit(_combo->get_active_text());
}

bool UnitMenu::setUnit(Glib::ustring const & unit)
{
    // TODO:  Determine if 'unit' is available in the dropdown.
    //        If not, return false

    _combo->set_active_text(unit);
    return true;
}

Glib::ustring UnitMenu::getUnitAbbr() const
{
    if (get_active_text() == "") {
        return "";
    }
    return getUnit()->abbr;
}

UnitType UnitMenu::getUnitType() const
{
    return getUnit()->type;
}

double  UnitMenu::getUnitFactor() const
{
    return getUnit()->factor;
}

int UnitMenu::getDefaultDigits() const
{
    return getUnit()->defaultDigits();
}

double UnitMenu::getDefaultStep() const
{ 
    int factor_digits = -1*int(log10(getUnit()->factor));
    return pow(10.0, factor_digits);
}

double UnitMenu::getDefaultPage() const
{
    return 10 * getDefaultStep();
}

double UnitMenu::getConversion(Glib::ustring const &new_unit_abbr, Glib::ustring const &old_unit_abbr) const
{
    double old_factor = getUnit()->factor;
    if (old_unit_abbr != "no_unit") {
        old_factor = unit_table.getUnit(old_unit_abbr)->factor;
    }
    Unit const * new_unit = unit_table.getUnit(new_unit_abbr);

    // Catch the case of zero or negative unit factors (error!)
    if (old_factor < 0.0000001 ||
        new_unit->factor < 0.0000001) {
        // TODO:  Should we assert here?
        return 0.00;
    }

    return old_factor / new_unit->factor;
}

bool UnitMenu::isAbsolute() const
{
    return getUnitType() != UNIT_TYPE_DIMENSIONLESS;
}

bool UnitMenu::isRadial() const
{
    return getUnitType() == UNIT_TYPE_RADIAL;
}

bool UnitMenu::on_scroll_event(GdkEventScroll *event) { return false; }

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
