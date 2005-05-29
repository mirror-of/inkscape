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

    static Transformation *create() { return new Transformation(); }

    void setPageMove()      { present(PAGE_MOVE);      }
    void setPageScale()     { present(PAGE_SCALE);     }
    void setPageRotate()    { present(PAGE_ROTATE);    }
    void setPageSkew()      { present(PAGE_SKEW);      }
    void setPageTransform() { present(PAGE_TRANSFORM); }

protected:
    typedef enum {
        PAGE_MOVE, PAGE_SCALE, PAGE_ROTATE, PAGE_SKEW, PAGE_TRANSFORM, PAGE_QTY
    } PageType;

    Gtk::Notebook     _notebook;

    NotebookPage      _page_move;
    NotebookPage      _page_scale;
    NotebookPage      _page_rotate;
    NotebookPage      _page_skew;
    NotebookPage      _page_transform;

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

    Scalar            _scalar_transform_a;
    Scalar            _scalar_transform_b;
    Scalar            _scalar_transform_c;
    Scalar            _scalar_transform_d;
    Scalar            _scalar_transform_e;
    Scalar            _scalar_transform_f;

    Gtk::CheckButton  _check_move_relative;

    void layoutPageMove();
    void layoutPageScale();
    void layoutPageRotate();
    void layoutPageSkew();
    void layoutPageTransform();

    virtual void _apply();
    void present(PageType page);
    void updateSelection(PageType page, Inkscape::Selection *selection);

    void onSelectionChanged(Inkscape::NSApplication::Application *inkscape,
                            Inkscape::Selection *selection);
    void onSelectionModified(Inkscape::NSApplication::Application *inkscape,
                             Inkscape::Selection *selection,
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
    void onTransformValueChanged();

    void updatePageMove(Inkscape::Selection *);
    void updatePageScale(Inkscape::Selection *);
    void updatePageRotate(Inkscape::Selection *);
    void updatePageSkew(Inkscape::Selection *);
    void updatePageTransform(Inkscape::Selection *);

    void applyPageMove(Inkscape::Selection *);
    void applyPageScale(Inkscape::Selection *);
    void applyPageRotate(Inkscape::Selection *);
    void applyPageSkew(Inkscape::Selection *);
    void applyPageTransform(Inkscape::Selection *);

private:
    Transformation(Transformation const &d);
    Transformation operator=(Transformation const &d);

    Gtk::Button *applyButton;
    Gtk::Button *cancelButton;


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
