/**
 * \brief Align and Distribute dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Aubanel MONNIER <aubi@libertysurf.fr>
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "align-and-distribute.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

AlignAndDistribute::AlignAndDistribute() 
    : _page_align("Align", 1, 1),
      _page_distribute("Distribute", 1, 1)
{
    set_title(_("Align"));
    set_default_size(200, 200);

    transientize();

    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);

    _notebook.append_page(_page_align,      _("Align"));
    _notebook.append_page(_page_distribute, _("Distribute"));

    // TODO:  Insert widgets

    show_all_children();
}

AlignAndDistribute::~AlignAndDistribute() 
{
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
