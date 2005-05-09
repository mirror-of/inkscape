/**
 * \brief XML Editor dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xml-editor.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

XmlEditor::XmlEditor() 
{
    set_title(_("XML Editor"));
    set_default_size(200, 200);

    transientize();

    // Top level vbox
    //Gtk::VBox *vbox = get_vbox();

    // TODO:  Insert widgets

    show_all_children();
}

XmlEditor::~XmlEditor() 
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
