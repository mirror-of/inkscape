/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/stock.h>

#include "prefdialog.h"

namespace Inkscape {
namespace Extension {

PrefDialog::PrefDialog (Glib::ustring name, Gdk::NativeWindow win_id) {
    Gtk::Dialog::Dialog(name + " Preferences", true, true);
    _socket = new Gtk::Socket();

    this->get_vbox()->pack_start(*_socket, true, true, 5);

    _socket->add_id(win_id);
    _socket->show();

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    return;
}

}; }; /* namespace Inkscape, Extension */

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
