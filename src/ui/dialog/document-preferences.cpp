/*
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "document-preferences.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

DocumentPreferences::DocumentPreferences() 
    : Dialog ("dialogs.documentoptions"),
      _page_page("Page", 1, 1),
      _page_grid("Grid", 1, 1),
      _page_guides("Guides", 1, 1),
      _page_metadata("Metadata", 1, 1)
{
    set_title(_("Document Preferences"));
    set_default_size(200, 200);

    transientize();

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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
