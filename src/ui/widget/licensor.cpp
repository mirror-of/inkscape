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
#include <gtkmm/optionmenu.h>
#include <gtkmm/menuitem.h>
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
: Gtk::VBox(false,4), _frame(_("License"))
{
}

Licensor::~Licensor()
{
    if (_eentry) delete _eentry;
}

void
Licensor::init (Gtk::Tooltips& tt, Registry& wr)
{
    _wr = &wr;
    show();
    _frame.show();
    _frame.add (*this);
    pack_start (_omenu, true, true, 0);
    _wr->add ("licenses", &_omenu);
    Gtk::Menu *m = manage (new Gtk::Menu);

    LicenseItem *i;
    for (struct rdf_license_t * license = rdf_licenses;
             license && license->name;
             license++) {
        i = manage (new LicenseItem (license->name));
        m->append (*i);
    }

    i = manage (new LicenseItem (_("Proprietary")));
    m->prepend (*i);
    _omenu.set_menu (*m);

    Gtk::HBox *box = manage (new Gtk::HBox);
    pack_start (*box, true, true, 0);

    /* add license-specific metadata entry areas */
    rdf_work_entity_t* entity = rdf_find_entity ( "license_uri" );
    _eentry = EntityEntry::create (entity, tt, *_wr);
    box->pack_start (_eentry->_label, false, false, 5);
    box->pack_start (*_eentry->_packable, true, true, 0);

    show_all_children();
}
    
void 
Licensor::update (SPDocument *doc)
{
    /* identify the license info */
    struct rdf_license_t * license = rdf_get_license (doc);

    if (license) {
        for (int i=0; rdf_licenses[i].name; i++) {
            if (license == &rdf_licenses[i]) {
                _omenu.set_history (i+1);
                break;
            }
        }
    }
    else {
        _omenu.set_history (0);
    }
    
    /* update the URI */
    _eentry->update (doc);
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
