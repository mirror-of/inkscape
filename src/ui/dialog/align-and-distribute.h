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

#ifndef INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H
#define INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/notebook-page.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class AlignAndDistribute : public Dialog {
public:
    AlignAndDistribute();
    virtual ~AlignAndDistribute();

    Glib::ustring getName() const { return _("Align and Distribute"); }
    Glib::ustring getDesc() const { return _("Align and Distribute Dialog"); }

    enum AlignTarget { LAST=0, FIRST, BIGGEST, SMALLEST, PAGE, DRAWING, SELECTION };

    AlignTarget getAlignTarget() const;

protected:

    Gtk::Notebook  _notebook;

    NotebookPage   _page_align;
    NotebookPage   _page_distribute;

private:
    AlignAndDistribute(AlignAndDistribute const &d);
    AlignAndDistribute& operator=(AlignAndDistribute const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H

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
