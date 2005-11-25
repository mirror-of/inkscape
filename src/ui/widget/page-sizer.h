/** \file
 * \brief Widget for specifying page size.
 *
 * Part of Document Preferences dialog.
 *
 * \todo
 * This should be even finer granulated with the menus their own widget class.
 *
 * Author:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_PAGE_SIZER__H
#define INKSCAPE_UI_WIDGET_PAGE_SIZER__H

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"

namespace Inkscape {    
    namespace UI {
        namespace Widget {

/// Widget containing all widgets for specifying page size.
class PageSizer : public Gtk::VBox {
public:
    PageSizer();
    virtual ~PageSizer();
    void init (Registry& reg);

protected:
    Gtk::HBox       _hbox_size;
    Gtk::Label      _label_size;    
    Gtk::OptionMenu _omenu_size;
    Gtk::Menu       _menu_size;
    //-----------------------------
    Gtk::HBox       _hbox_orient;
    Gtk::Label      _label_orient;    
    Gtk::OptionMenu _omenu_orient;
    Gtk::Menu       _menu_orient;
    //-----------------------------
    Gtk::Frame           _frame;
    Gtk::Table           _table;
    RegisteredUnitMenu   _rum;
    RegisteredScalarUnit _rusw, _rush;

    Registry _wr;
};

}}}

#endif /* INKSCAPE_UI_WIDGET_PAGE_SIZER__H */

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
