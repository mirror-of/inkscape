/*
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "document-preferences.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

DocumentPreferences::DocumentPreferences() 
    : Dialog ("dialogs.documentoptions", SP_VERB_DIALOG_NAMEDVIEW),
      _page_page(1, 1),
      _page_grid(1, 1),
      _page_guides(1, 1),
      _page_metadata(1, 1)
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);

    _notebook.append_page(_page_page,      _("Page"));
    _notebook.append_page(_page_grid,      _("Grid"));
    _notebook.append_page(_page_guides,    _("Guides"));
    _notebook.append_page(_page_metadata,  _("Metadata"));

    // TODO:  Insert widgets

    show_all_children();
}

DocumentPreferences::~DocumentPreferences() 
{
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
