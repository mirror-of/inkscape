/**
 * \brief Labelled Widget - Adds a label with optional icon or suffix to
 *        another widget.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Carl Hetherington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_LABELLED_H
#define INKSCAPE_UI_WIDGET_LABELLED_H

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class Labelled : public Gtk::HBox
{
public:
    Labelled(Glib::ustring const &label,
             Gtk::Widget *widget,
             Glib::ustring const &suffix = "",
             Glib::ustring const &icon = "",
             bool mnemonic = false);

    /**
     * Allow the setting of the width of the labelled widget
     */
    void setWidgetSizeRequest(int width, int height);


protected:
    Gtk::Widget  *_widget;
    Gtk::Label   *_label;
    Gtk::Label   *_suffix;
    Gtk::Image   *_icon;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_LABELLED_H

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
