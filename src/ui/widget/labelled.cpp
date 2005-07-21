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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* For getting the Gtkmmified Icon manager */
#include "widgets/icon.h"

#include "labelled.h"
#include "path-prefix.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a Labelled Widget.
 *
 * \param label     Label.
 * \param widget    Widget to label; should be allocated with new, as it will
 *                  be passed to Gtk::manage().
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the text
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
Labelled::Labelled(Glib::ustring const &label,
                   Gtk::Widget *widget,
                   Glib::ustring const &suffix,
                   Glib::ustring const &icon,
                   bool mnemonic)
    : _widget(widget),
      _label(new Gtk::Label(label, 0.0, 0.5, mnemonic)),
      _suffix(new Gtk::Label(suffix, 0.0, 0.5))
{
    g_assert(g_utf8_validate(icon.c_str(), -1, NULL));
    if (icon != "") {
        _icon = sp_icon_get_icon(icon.c_str(), GTK_ICON_SIZE_DIALOG);
        pack_start(*Gtk::manage(_icon), Gtk::PACK_SHRINK);
    }
    pack_start(*Gtk::manage(_label), Gtk::PACK_EXPAND_WIDGET, 6);
    pack_start(*Gtk::manage(_widget), Gtk::PACK_SHRINK, 6);
    if (mnemonic) {
        _label->set_mnemonic_widget(*_widget);
    }
}


/**
 * Allow the setting of the width of the labelled widget
 */
void Labelled::setWidgetSizeRequest(int width, int height)
{
    if (_widget)
        _widget->set_size_request(width, height);


}




} // namespace Widget
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
