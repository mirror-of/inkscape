/**
 *
 * \brief  Dialog for renaming layers
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_LAYER_PROPERTIES_H
#define INKSCAPE_DIALOG_LAYER_PROPERTIES_H

#include <gtkmm/dialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/separator.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>

#include "selection.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

class LayerPropertiesDialog : public Gtk::Dialog {
 public:
    LayerPropertiesDialog();
    virtual ~LayerPropertiesDialog();

    Glib::ustring     getName() const { return "LayerPropertiesDialog"; }

    static LayerPropertiesDialog *getInstance();
    static void showInstance();

    bool       userHidden() { return _userHidden; }
    void       userHidden(bool val) { _userHidden = val; }

 protected:
    LayerPropertiesDialog(const LayerPropertiesDialog&);
    LayerPropertiesDialog& operator=(const LayerPropertiesDialog&);

    bool              _userHidden;
    Gtk::HBox         _layer_name_hbox;
    Gtk::Label        _layer_name_label;
    Gtk::Entry        _layer_name_entry;

    void apply();
    void close();
    void update();

    void setLayerName(gchar const * name);
    gchar const * getLayerName() const;

};

} // namespace
} // namespace
} // namespace


#endif //INKSCAPE_DIALOG_LAYER_PROPERTIES_H

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
