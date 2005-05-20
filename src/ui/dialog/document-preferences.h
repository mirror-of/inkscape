/*
 * \brief  Document Preferences dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/notebook-page.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class DocumentPreferences : public Dialog {
public:
    DocumentPreferences();
    virtual ~DocumentPreferences();

    static DocumentPreferences *create() { return new DocumentPreferences(); }

protected:

    Gtk::Notebook  _notebook;

    NotebookPage   _page_page;
    NotebookPage   _page_grid;
    NotebookPage   _page_guides;
    NotebookPage   _page_metadata;

private:
    DocumentPreferences(DocumentPreferences const &d);
    DocumentPreferences& operator=(DocumentPreferences const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

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
