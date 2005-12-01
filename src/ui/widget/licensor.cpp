/** \file
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/frame.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/tooltips.h>

#include "ui/widget/entity-entry.h"
#include "ui/widget/registry.h"
#include "dialogs/rdf.h"

#include "licensor.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

class LicenseItem : public Gtk::MenuItem {
public:
    LicenseItem (const Glib::ustring &s);
    virtual ~LicenseItem();
protected:
    void on_activate_item();
};

LicenseItem::LicenseItem (const Glib::ustring &s)
: Gtk::MenuItem(s)
{
}

LicenseItem::~LicenseItem()
{
}

void
LicenseItem::on_activate_item()
{
}

//---------------------------------------------------

Licensor::Licensor()
: Gtk::VBox(false,4), _frame(_("License")), _table(5,2,false)
{
}

Licensor::~Licensor()
{
    if (_eentry) delete _eentry;
}

void
Licensor::init (Gtk::Tooltips& tt, Registry& wr)
{
    _tt = &tt;
    _wr = &wr;
    show();
    _frame.show();
    _frame.add (*this);
    _om.show();
    pack_start (_om, true, true, 0);
    _wr->add ("licenses", &_om);
    _menu.show();

    LicenseItem *i;
    for (struct rdf_license_t * license = rdf_licenses;
             license && license->name;
             license++) {
        i = manage (new LicenseItem (license->name));
        i->show();
        _menu.append (*i);
    }

    i = manage (new LicenseItem (_("Proprietary")));
    i->show();
    _menu.prepend (*i);
    _om.set_menu (_menu);

    _table.show();
    _table.set_border_width (4);
    _table.set_row_spacings (4);
    _table.set_col_spacings (4);
    pack_start (_table, true, true, 0);

    int row = 0;
    /* add license-specific metadata entry areas */
    rdf_work_entity_t* entity = rdf_find_entity ( "license_uri" );
    _eentry = EntityEntry::create (entity, *_tt, *_wr);
    _table.attach (_eentry->_label, 0,1, row, row+1, Gtk::SHRINK, (Gtk::AttachOptions)0,0,0);
    _table.attach (*_eentry->_packable, 1,2, row, row+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
}

} // namespace Dialog
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
