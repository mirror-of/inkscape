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
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_UNIT_TRACKER_H
#define INKSCAPE_UI_WIDGET_UNIT_TRACKER_H

#include <map>
#include <vector>

#include <gtkmm/liststore.h>

#include "util/units.h"

using Inkscape::Util::Unit;
using Inkscape::Util::UnitType;

namespace Gtk {
class Adjustment;
}

namespace Inkscape {
namespace UI {
namespace Widget {
class ComboToolItem;

class UnitTracker {
public:
    UnitTracker(UnitType unit_type);
    virtual ~UnitTracker();

    bool isUpdating() const;

    void setActiveUnit(Inkscape::Util::Unit const *unit);
    void setActiveUnitByAbbr(gchar const *abbr);
    Inkscape::Util::Unit const * getActiveUnit() const;

    void addUnit(Inkscape::Util::Unit const *u);
    void addAdjustment(Glib::RefPtr<Gtk::Adjustment> &adj);
    void prependUnit(Inkscape::Util::Unit const *u);
    void setFullVal(Glib::RefPtr<Gtk::Adjustment> &adj, gdouble val);
    void changeLabel(Glib::ustring new_label, gint pos, bool onlylabel = false);

    ComboToolItem *create_tool_item(Glib::ustring const &label,
                                    Glib::ustring const &tooltip);

protected:
    UnitType _type;

private:
    // Callbacks
    void _unitChangedCB(int active);

    void _setActive(gint index);
    void _fixupAdjustments(Inkscape::Util::Unit const *oldUnit, Inkscape::Util::Unit const *newUnit);

    gint _active;
    bool _isUpdating;
    Inkscape::Util::Unit const *_activeUnit;
    bool _activeUnitInitialized;

    Glib::RefPtr<Gtk::ListStore> _store;
    std::vector<ComboToolItem *> _combo_list;
    std::vector<Glib::RefPtr<Gtk::Adjustment>> _adjList;
    std::map <Glib::RefPtr<Gtk::Adjustment>, gdouble> _priorValues;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_UNIT_TRACKER_H
