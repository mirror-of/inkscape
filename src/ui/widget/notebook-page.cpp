/**
 * \brief Notebook page widget
 *
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
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
#include "notebook-page.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 *    Construct a NotebookPage
 *
 *    \param label Label.
 */
  
NotebookPage::NotebookPage(Glib::ustring const &label,
                                       int n_rows, int n_columns)
    : _frame(label),
      _table(n_rows,n_columns)
{
    set_border_width(4);
    pack_start(_frame, true, true, 0);
    _frame.add(_table);
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
