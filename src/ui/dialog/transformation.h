/**
 * \brief Object Transformation dialog
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_TRANSFORMATION_H
#define INKSCAPE_UI_DIALOG_TRANSFORMATION_H

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "application/application.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/scalar-unit.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Transformation : public Dialog {
public:
    Transformation();
    virtual ~Transformation();

    Glib::ustring getName() const { return _("Transformation"); }
    Glib::ustring getDesc() const { return _("Transformation Dialog"); }

    void setPageMove()   { present(PAGE_MOVE); }
    void setPageScale()  { present(PAGE_SCALE); }
    void setPageRotate() { present(PAGE_ROTATE); }
    void setPageSkew()   { present(PAGE_SKEW); }

protected:
    enum PageType { PAGE_MOVE, PAGE_SCALE, PAGE_ROTATE, PAGE_SKEW, PAGE_QTY };

    Gtk::Notebook     _notebook;

    NotebookPage      _page_move;
    NotebookPage      _page_scale;
    NotebookPage      _page_rotate;
    NotebookPage      _page_skew;

    UnitMenu          _units_move;
    UnitMenu          _units_scale;
    UnitMenu          _units_skew;

    ScalarUnit        _scalar_move_horizontal;
    ScalarUnit        _scalar_move_vertical;
    ScalarUnit        _scalar_scale_horizontal;
    ScalarUnit        _scalar_scale_vertical;
    ScalarUnit        _scalar_rotate;
    ScalarUnit        _scalar_skew_horizontal;
    ScalarUnit        _scalar_skew_vertical;

    Gtk::CheckButton  _check_move_relative;

    void initPageMove();
    void initPageScale();
    void initPageRotate();
    void initPageSkew();

    virtual void _apply();
    void present(PageType page);
    void updateSelection(PageType page, SPSelection *selection);

    void onSelectionChanged(Inkscape::NSApplication::Application *inkscape,
                            SPSelection *selection);
    void onSelectionModified(Inkscape::NSApplication::Application *inkscape,
                             SPSelection *selection,
                             int unsigned flags);
/* TODO
    void onSwitchPage(Gtk::Notebook *notebook,
                      Gtk::Notebook::Page *page,
                      guint pagenum);
*/

    void onMoveValueChanged();
    void onMoveRelativeToggled();
    void onScaleValueChanged();
    void onRotateValueChanged();
    void onSkewValueChanged();

    void updatePageMove(SPSelection *);
    void updatePageScale(SPSelection *);
    void updatePageRotate(SPSelection *);
    void updatePageSkew(SPSelection *);

    void applyPageMove(SPSelection *);
    void applyPageScale(SPSelection *);
    void applyPageRotate(SPSelection *);
    void applyPageSkew(SPSelection *);

    int scaleSetUnit(Unit const *,
                     Unit const *,
                     GObject *dlg);

private:
    Transformation(Transformation const &d);
    Transformation operator=(Transformation const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_TRANSFORMATION_H

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
