/**
 * \brief Generic Panel widget - A generic dockable container.
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_UI_WIDGET_PANEL_H
#define SEEN_INKSCAPE_UI_WIDGET_PANEL_H

#include <gtkmm/bin.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class Panel : public Gtk::VBox
{
public:
    Panel();
    Panel(Glib::ustring const &label);

    void setLabel(Glib::ustring const &label);
    Glib::ustring const &getLabel() const;

protected:

private:
    void init();


    Glib::ustring   label;

    Gtk::HBox       topBar;
    Gtk::Label      tabTitle;
    Gtk::Button     tabButton;
    Gtk::Button     closeButton;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // SEEN_INKSCAPE_UI_WIDGET_PANEL_H

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
