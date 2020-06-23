// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape::UI::Widget::UnitTracker
 * Simple mediator to synchronize changes to unit menus
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *   Matthew Petroff <matthew@mpetroff.net>
 *
 * Copyright (C) 2007 Jon A. Cruz
 * Copyright (C) 2013 Matthew Petroff
 * Copyright (C) 2018 Tavmjong Bah
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "unit-tracker.h"

#include <algorithm>
#include <iostream>

#include <gtkmm/adjustment.h>

#include "combo-tool-item.h"

#define COLUMN_STRING 0

using Inkscape::Util::UnitTable;
using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Widget {

UnitTracker::UnitTracker(UnitType unit_type) :
    _active(0),
    _isUpdating(false),
    _activeUnit(nullptr),
    _activeUnitInitialized(false),
    _store(nullptr),
    _priorValues()
{
    UnitTable::UnitMap m = unit_table.units(unit_type);
    
    ComboToolItemColumns columns;
    _store = Gtk::ListStore::create(columns);
    Gtk::TreeModel::Row row;

    for (auto & m_iter : m) {

        Glib::ustring unit = m_iter.first;

        row = *(_store->append());
        row[columns.col_label    ] = unit;
        row[columns.col_value    ] = unit;
        row[columns.col_tooltip  ] = ("");
        row[columns.col_icon     ] = "NotUsed";
        row[columns.col_sensitive] = true;
    }

    // Why?
    gint count = _store->children().size();
    if ((count > 0) && (_active > count)) {
        _setActive(--count);
    } else {
        _setActive(_active);
    }
}

UnitTracker::~UnitTracker()
{
    _combo_list.clear();
    _adjList.clear();
}

bool UnitTracker::isUpdating() const
{
    return _isUpdating;
}

Inkscape::Util::Unit const * UnitTracker::getActiveUnit() const
{
    return _activeUnit;
}

void UnitTracker::changeLabel(Glib::ustring new_label, gint pos, bool onlylabel)
{
    ComboToolItemColumns columns;
    _store->children()[pos][columns.col_label] = new_label;
    if (!onlylabel) {
        _store->children()[pos][columns.col_value] = new_label;
    }
}

void UnitTracker::setActiveUnit(Inkscape::Util::Unit const *unit)
{
    if (unit) {

        ComboToolItemColumns columns;
        int index = 0;
        for (auto& row: _store->children() ) {
            Glib::ustring storedUnit = row[columns.col_value];
            if (!unit->abbr.compare (storedUnit)) {
                _setActive (index);
                break;
            }
            index++;
        }
    }
}

void UnitTracker::setActiveUnitByAbbr(gchar const *abbr)
{
    Inkscape::Util::Unit const *u = unit_table.getUnit(abbr);
    setActiveUnit(u);
}

void UnitTracker::addAdjustment(Glib::RefPtr<Gtk::Adjustment> &adj)
{
    if (std::find(_adjList.begin(),_adjList.end(),adj) == _adjList.end()) {
        _adjList.push_back(adj);
    } else {
        std::cerr << "UnitTracker::addAjustment: Adjustment already added!" << std::endl;
    }
}

void UnitTracker::addUnit(Inkscape::Util::Unit const *u)
{
    ComboToolItemColumns columns;

    Gtk::TreeModel::Row row;
    row = *(_store->append());
    row[columns.col_label    ] = u ? u->abbr.c_str() : "";
    row[columns.col_value    ] = u ? u->abbr.c_str() : "";
    row[columns.col_tooltip  ] = ("");
    row[columns.col_icon     ] = "NotUsed";
    row[columns.col_sensitive] = true;
}

void UnitTracker::prependUnit(Inkscape::Util::Unit const *u)
{
    ComboToolItemColumns columns;

    Gtk::TreeModel::Row row;
    row = *(_store->prepend());
    row[columns.col_label    ] = u ? u->abbr.c_str() : "";
    row[columns.col_value    ] = u ? u->abbr.c_str() : "";
    row[columns.col_tooltip  ] = ("");
    row[columns.col_icon     ] = "NotUsed";
    row[columns.col_sensitive] = true;

    /* Re-shuffle our default selection here (_active gets out of sync) */
    setActiveUnit(_activeUnit);

}

void UnitTracker::setFullVal(Glib::RefPtr<Gtk::Adjustment> &adj, gdouble val)
{
    _priorValues[adj] = val;
}

ComboToolItem *
UnitTracker::create_tool_item(Glib::ustring const &label,
                              Glib::ustring const &tooltip)
{
    auto combo = ComboToolItem::create(label, tooltip, "NotUsed", _store);
    combo->set_active(_active);
    combo->signal_changed().connect(sigc::mem_fun(*this, &UnitTracker::_unitChangedCB));
    combo->set_name("unit-tracker");
    _combo_list.push_back(combo);
    return combo;    
}

void UnitTracker::_unitChangedCB(int active)
{
    _setActive(active);
}

void UnitTracker::_setActive(gint active)
{
    if ( active != _active || !_activeUnitInitialized ) {
        gint oldActive = _active;

        if (_store) {

            // Find old and new units
            ComboToolItemColumns columns;
            int index = 0;
            Glib::ustring oldAbbr( "NotFound" );
            Glib::ustring newAbbr( "NotFound" );
            for (auto& row: _store->children() ) {
                if (index == _active) {
                    oldAbbr = row[columns.col_value];
                }
                if (index == active) {
                    newAbbr = row[columns.col_value];
                }
                if (newAbbr != "NotFound" && oldAbbr != "NotFound") break;
                ++index;
            }

            if (oldAbbr != "NotFound") {

                if (newAbbr != "NotFound") {
                    Inkscape::Util::Unit const *oldUnit = unit_table.getUnit(oldAbbr);
                    Inkscape::Util::Unit const *newUnit = unit_table.getUnit(newAbbr);
                    _activeUnit = newUnit;

                    if (!_adjList.empty()) {
                        _fixupAdjustments(oldUnit, newUnit);
                    }
                } else {
                    std::cerr << "UnitTracker::_setActive: Did not find new unit: " << active << std::endl;
                }

            } else {
                std::cerr << "UnitTracker::_setActive: Did not find old unit: " << oldActive
                          << "  new: " << active << std::endl;
            }
        }
        _active = active;

        for (auto combo : _combo_list) {
            if(combo) combo->set_active(active);
        }

        _activeUnitInitialized = true;
    }
}

void UnitTracker::_fixupAdjustments(Inkscape::Util::Unit const *oldUnit, Inkscape::Util::Unit const *newUnit)
{
    _isUpdating = true;
    for ( auto adj : _adjList ) {
        auto oldVal = adj->get_value();
        gdouble val = oldVal;

        if ( (oldUnit->type != Inkscape::Util::UNIT_TYPE_DIMENSIONLESS)
            && (newUnit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) )
        {
            val = newUnit->factor * 100;
            _priorValues[adj] = Inkscape::Util::Quantity::convert(oldVal, oldUnit, "px");
        } else if ( (oldUnit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS)
            && (newUnit->type != Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) )
        {
            if (_priorValues.find(adj) != _priorValues.end()) {
                val = Inkscape::Util::Quantity::convert(_priorValues[adj], "px", newUnit);
            }
        } else {
            val = Inkscape::Util::Quantity::convert(oldVal, oldUnit, newUnit);
        }

        adj->set_value(val);
    }
    _isUpdating = false;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
