/**
 * \brief Panel widget
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include "panel.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 *    Construct a Panel
 *
 *    \param label Label.
 */

Panel::Panel()
{
    init();
}

Panel::Panel(Glib::ustring const &label)
{
    this->label = label;
    init();
}

void Panel::init()
{
    tabTitle.set_label(this->label);
    tabButton.set_label("<");
    closeButton.set_label("X");

/*
    topBar.pack_start(tabTitle);


    topBar.pack_end(closeButton, false, false);
    topBar.pack_end(tabButton, false, false);

    pack_start( topBar, false, false );
*/

    show_all_children();
}

void Panel::setLabel(Glib::ustring const &label)
{
    this->label = label;
    tabTitle.set_label(this->label);
}

Glib::ustring const &Panel::getLabel() const
{
    return label;
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
