/**
 * \brief Notebook Page Widget - A tabbed notebook page for dialogs.
 *
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_NOTEBOOK_PAGE_H
#define INKSCAPE_UI_WIDGET_NOTEBOOK_PAGE_H

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class NotebookPage : public Gtk::VBox
{
public:
    NotebookPage();
    NotebookPage(Glib::ustring const &label,
                 int n_rows, int n_columns);

    Gtk::Frame& frame() { return _frame; }
    Gtk::Table& table() { return _table; }

protected:
    Gtk::Frame _frame;
    Gtk::Table _table;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_NOTEBOOK_PAGE_H

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
