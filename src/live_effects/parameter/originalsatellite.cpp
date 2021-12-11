// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Johan Engelen 2012 <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/parameter/originalsatellite.h"

#include <glibmm/i18n.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include "desktop.h"
#include "display/curve.h"
#include "inkscape.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/satellite-reference.h"
#include "object/uri.h"
#include "selection.h"
#include "ui/icon-loader.h"
#include "ui/icon-names.h"

namespace Inkscape {

namespace LivePathEffect {

OriginalSatelliteParam::OriginalSatelliteParam(const Glib::ustring &label, const Glib::ustring &tip,
                                               const Glib::ustring &key, Inkscape::UI::Widget::Registry *wr,
                                               Effect *effect)
    : SatelliteParam(label, tip, key, wr, effect)
{
}

OriginalSatelliteParam::~OriginalSatelliteParam() = default;

Gtk::Widget *OriginalSatelliteParam::param_newWidget()
{
    Gtk::Box *_widget = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));

    { // Label
        Gtk::Label *pLabel = Gtk::manage(new Gtk::Label(param_label));
        _widget->pack_start(*pLabel, true, true);
        pLabel->set_tooltip_text(param_tooltip);
    }

    { // Paste item to link button
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("edit-paste", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalSatelliteParam::on_link_button_click));
        _widget->pack_start(*pButton, true, true);
        pButton->set_tooltip_text(_("Link to item"));
    }

    { // Select original button
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("edit-select-original", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(
            sigc::mem_fun(*this, &OriginalSatelliteParam::on_select_original_button_click));
        _widget->pack_start(*pButton, true, true);
        pButton->set_tooltip_text(_("Select original"));
    }

    _widget->show_all_children();

    return dynamic_cast<Gtk::Widget *> (_widget);
}

void OriginalSatelliteParam::on_select_original_button_click()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPItem *original = dynamic_cast<SPItem *>(lperef->getObject());
    if (desktop == nullptr || original == nullptr) {
        return;
    }
    Inkscape::Selection *selection = desktop->getSelection();
    selection->clear();
    selection->set(original);
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

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
