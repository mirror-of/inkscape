/**
 * \brief Messages dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_MESSAGES_H
#define INKSCAPE_UI_DIALOG_MESSAGES_H

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/notebook-page.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Messages : public Dialog {
public:
    Messages();
    virtual ~Messages();

    Glib::ustring getName() const { return _("Messages"); }
    Glib::ustring getDesc() const { return _("Messages Dialog"); }

protected:
    Gtk::Notebook  _notebook;

    NotebookPage   _page_messages;

private:
    Messages(Messages const &d);
    Messages& operator=(Messages const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_MESSAGES_H

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
