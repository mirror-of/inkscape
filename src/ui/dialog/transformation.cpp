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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/stock.h>

#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "transformation.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-scale-ops.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * Constructor for Transformation.  This does the initialization
 * and layout of the dialog used for transforming SVG objects.  It
 * consists of four pages for the four operations it handles:
 * 'Move' allows x,y translation of SVG objects
 * 'Scale' allows linear resizing of SVG objects
 * 'Rotate' allows rotating SVG objects by a degree
 * 'Skew' allows skewing SVG objects
 * 'Transform' allows applying a generic affine transform on SVG objects,
 *     with the user specifying the 6 degrees of freedom manually.
 *
 * The dialog is implemented as a Gtk::Notebook with five pages.
 * The pages are implemented using Inkscape's NotebookPage which
 * is used to help make sure all of Inkscape's notebooks follow
 * the same style.  We then populate the pages with our widgets,
 * we use the ScalarUnit class for this.
 *
 */
Transformation::Transformation()
    : Dialog ("dialogs.transformation", SP_VERB_DIALOG_TRANSFORM),
      _page_move              ("Move",      4, 2),
      _page_scale             ("Scale",     4, 2),
      _page_rotate            ("Rotate",    4, 2),
      _page_skew              ("Skew",      4, 2),
      _page_transform         ("Transform", 2, 3),
      _scalar_move_horizontal ("Horizontal", UNIT_TYPE_LINEAR, "",
                               "arrows_horiz.svg", &_units_move),
      _scalar_move_vertical   ("Vertical",   UNIT_TYPE_LINEAR, "",
                               "arrows_vert.svg", &_units_move),
      _scalar_scale_horizontal("Width",      UNIT_TYPE_LINEAR, "",
                               "scale_hor.xpm", &_units_scale),
      _scalar_scale_vertical  ("Height",     UNIT_TYPE_LINEAR, "",
                               "scale_ver.xpm", &_units_scale),
      _scalar_rotate          ("Rotate",     UNIT_TYPE_RADIAL, "",
                               "rotate.svg"),
      _scalar_skew_horizontal ("Horizontal", UNIT_TYPE_LINEAR, "",
                               "arrows_horiz.svg", &_units_skew),
      _scalar_skew_vertical   ("Vertical",   UNIT_TYPE_LINEAR, "",
                               "arrows_vert.svg", &_units_skew),

      _scalar_transform_a     ("A"),
      _scalar_transform_b     ("B"),
      _scalar_transform_c     ("C"),
      _scalar_transform_d     ("D"),
      _scalar_transform_e     ("E"),
      _scalar_transform_f     ("F"),

      _check_move_relative    ("Relative move")
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);

    _notebook.append_page(_page_move,      _("Move"));
    _notebook.append_page(_page_scale,     _("Scale"));
    _notebook.append_page(_page_rotate,    _("Rotate"));
    _notebook.append_page(_page_skew,      _("Skew"));
    _notebook.append_page(_page_transform, _("Transform"));

    // Unit widgets initialization
    _units_move.setUnitType(UNIT_TYPE_LINEAR);
    _units_scale.setUnitType(UNIT_TYPE_DIMENSIONLESS);
    _units_scale.setUnitType(UNIT_TYPE_LINEAR);
    _units_skew.setUnitType(UNIT_TYPE_DIMENSIONLESS);
    _units_skew.setUnitType(UNIT_TYPE_LINEAR);

    _scalar_move_horizontal.initScalar(0, 100);
    _scalar_move_vertical.initScalar(0, 100);
    _scalar_scale_horizontal.initScalar(0, 100);
    _scalar_scale_vertical.initScalar(0, 100);
    _scalar_rotate.initScalar(0, 100);
    _scalar_skew_horizontal.initScalar(0, 100);
    _scalar_skew_vertical.initScalar(0, 100);

    _scalar_transform_a.setRange(-1e10, 1e10);
    _scalar_transform_a.setDigits(3);
    _scalar_transform_a.setIncrements(0.1, 1.0);
    _scalar_transform_a.setValue(0.0);

    _scalar_transform_b.setRange(-1e10, 1e10);
    _scalar_transform_b.setDigits(3);
    _scalar_transform_b.setIncrements(0.1, 1.0);
    _scalar_transform_b.setValue(0.0);

    _scalar_transform_c.setRange(-1e10, 1e10);
    _scalar_transform_c.setDigits(3);
    _scalar_transform_c.setIncrements(0.1, 1.0);
    _scalar_transform_c.setValue(0.0);

    _scalar_transform_d.setRange(-1e10, 1e10);
    _scalar_transform_d.setDigits(3);
    _scalar_transform_d.setIncrements(0.1, 1.0);
    _scalar_transform_d.setValue(0.0);

    _scalar_transform_e.setRange(-1e10, 1e10);
    _scalar_transform_e.setDigits(3);
    _scalar_transform_e.setIncrements(0.1, 1.0);
    _scalar_transform_e.setValue(0.0);

    _scalar_transform_f.setRange(-1e10, 1e10);
    _scalar_transform_f.setDigits(3);
    _scalar_transform_f.setIncrements(0.1, 1.0);
    _scalar_transform_f.setValue(0.0);



    initPageMove();
    initPageScale();
    initPageRotate();
    initPageSkew();
    initPageTransform();

    updateSelection(PAGE_MOVE, _getSelection());

    applyButton = add_button(Gtk::Stock::APPLY,   Gtk::RESPONSE_APPLY);
    if (applyButton)
        {
        //tips.set_tip((*applyButton), _("Apply transform to object"));
        applyButton->set_sensitive(false);
        }

    show_all_children();
}

Transformation::~Transformation()
{
}


void
Transformation::present(Transformation::PageType page)
{
    _notebook.set_current_page(page);
    Gtk::Dialog::show();
    Gtk::Dialog::present();
}

/*#######################################
# I N I T
#######################################*/

void
Transformation::initPageMove()
{
    _page_move.table()
        .attach(_scalar_move_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);

    _page_move.table()
        .attach(_units_move, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _scalar_move_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveValueChanged));

    /* _scalar_move_vertical.set_label_image( INKSCAPE_STOCK_ARROWS_VER ); */
    _page_move.table()
        .attach(_scalar_move_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);

    _scalar_move_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveValueChanged));

    // Relative moves
    set_data("move_relative", &_check_move_relative);
    _page_move.table()
        .attach(_check_move_relative, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
    _check_move_relative.set_active(true);
    _check_move_relative.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveRelativeToggled));
}

void
Transformation::initPageScale()
{
    _page_scale.table()
        .attach(_scalar_scale_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    _scalar_scale_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleValueChanged));

    // TODO:  Default should be %
    _page_scale.table()
        .attach(_units_scale, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_scale.table()
        .attach(_scalar_scale_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
    _scalar_scale_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleValueChanged));
}

void
Transformation::initPageRotate()
{
    _page_rotate.table()
        .attach(_scalar_rotate, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);

    _scalar_rotate.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onRotateValueChanged));
}

void
Transformation::initPageSkew()
{
    _page_skew.table()
        .attach(_scalar_skew_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    _scalar_skew_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onSkewValueChanged));

    _page_skew.table()
        .attach(_units_skew, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_skew.table()
        .attach(_scalar_skew_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
    _scalar_skew_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onSkewValueChanged));
}

void
Transformation::initPageTransform()
{

    _scalar_transform_a.setWidgetSizeRequest(75, -1);
    _page_transform.table()
        .attach(_scalar_transform_a, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_a.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_b.setWidgetSizeRequest(75, -1);
    _page_transform.table()
        .attach(_scalar_transform_b, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_b.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_c.setWidgetSizeRequest(75, -1);
    _page_transform.table()
        .attach(_scalar_transform_c, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_c.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_d.setWidgetSizeRequest(75, -1);
    _page_transform.table()
        .attach(_scalar_transform_d, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_d.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_e.setWidgetSizeRequest(75, -1);
    _page_transform.table()
        .attach(_scalar_transform_e, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_e.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_f.setWidgetSizeRequest(75, -1);
    _page_transform.table()
        .attach(_scalar_transform_f, 2, 3, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_f.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));

}



/*#######################################
#  U P D A T E
#######################################*/


void
Transformation::updateSelection(PageType page, Inkscape::Selection *selection)
{
    switch (page) {
        case PAGE_MOVE: {
            updatePageMove(selection);
            break;
        }
        case PAGE_SCALE: {
            updatePageScale(selection);
            break;
        }
        case PAGE_ROTATE: {
            updatePageRotate(selection);
            break;
        }
        case PAGE_SKEW: {
            updatePageSkew(selection);
            break;
        }
        case PAGE_TRANSFORM: {
            updatePageTransform(selection);
            break;
        }
        case PAGE_QTY: {
            break;
        }
    }

    set_response_sensitive(Gtk::RESPONSE_APPLY,
                           selection && !selection->isEmpty());
}



void
Transformation::onSelectionChanged(Inkscape::NSApplication::Application *inkscape,
                                   Inkscape::Selection *selection)
{
    //TODO: replace with a Tranformation::getCurrentPage() function
    int page = _notebook.get_current_page();
    ++page;
    updateSelection((PageType)page, selection);
}

void
Transformation::onSelectionModified(Inkscape::NSApplication::Application *inkscape,
                                    Inkscape::Selection *selection,
                                    int unsigned flags)
{
    //TODO: replace with a Tranformation::getCurrentPage() function
    int page = _notebook.get_current_page();
    ++page;
    updateSelection((PageType)page, selection);
}

/* TODO:  Is this even needed?
void
Transformation::onSwitchPage(Gtk::Notebook *notebook,
                                   Gtk::Notebook::Page *page,
                                   guint pagenum)
{
    updateSelection(pagenum, _getSelection());
}
*/

void
Transformation::updatePageMove(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        if (_check_move_relative.get_active()) {
            NR::Rect bbox = selection->bounds();
            double x = bbox.min()[NR::X];
            double y = bbox.min()[NR::Y];

            _scalar_move_horizontal.setValue(x, "px");
            _scalar_move_vertical.setValue(y, "px");
        }
        _page_move.set_sensitive(true);
    } else {
        _page_move.set_sensitive(false);
    }
}

void
Transformation::updatePageScale(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
          NR::Rect bbox = selection->bounds();
          double x = bbox.extent(NR::X);
          double y = bbox.extent(NR::Y);
        if (_units_scale.isAbsolute()) {
            _scalar_scale_horizontal.setValue(x, "px");
            _scalar_scale_vertical.setValue(y, "px");
        } else {
            /* TODO - Non-absolute units
            _scalar_scale_horizontal.setValue(x, 100.0);
            _scalar_scale_vertical.setValue(y, 100.0);
            */
        }
        _page_scale.set_sensitive(true);
    } else {
        _page_scale.set_sensitive(false);
    }
}

void
Transformation::updatePageRotate(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        _page_rotate.set_sensitive(true);
    } else {
        _page_rotate.set_sensitive(false);
    }
}

void
Transformation::updatePageSkew(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        _page_skew.set_sensitive(true);
    } else {
        _page_skew.set_sensitive(false);
    }
}

void
Transformation::updatePageTransform(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        if (_check_move_relative.get_active()) {
            NR::Rect bbox = selection->bounds();
            double x = bbox.min()[NR::X];
            double y = bbox.min()[NR::Y];

            _scalar_move_horizontal.setValue(x, "px");
            _scalar_move_vertical.setValue(y, "px");
        }
        _page_move.set_sensitive(true);
    } else {
        _page_move.set_sensitive(false);
    }
}


/*#######################################
# A P P L Y
#######################################*/

void
Transformation::_apply()
{
    Inkscape::Selection * const selection = _getSelection();
    g_return_if_fail (!selection->isEmpty());

    int const page = _notebook.get_current_page();

    switch (page) {
        case PAGE_MOVE: {
            applyPageMove(selection);
            break;
        }
        case PAGE_ROTATE: {
            applyPageRotate(selection);
            break;
        }
        case PAGE_SCALE: {
            applyPageScale(selection);
            break;
        }
        case PAGE_SKEW: {
            applyPageSkew(selection);
            break;
        }
        case PAGE_TRANSFORM: {
            applyPageTransform(selection);
            break;
        }
    }

    set_response_sensitive(Gtk::RESPONSE_APPLY, false);
}

void
Transformation::applyPageMove(Inkscape::Selection *selection)
{
    double x = _scalar_move_horizontal.getValue("px");
    double y = _scalar_move_vertical.getValue("px");

    if (_check_move_relative.get_active()) {

        sp_selection_move_relative(selection, x, y);

    } else {

        NR::Rect bbox = selection->bounds();
        sp_selection_move_relative(selection,
            x - bbox.min()[NR::X], y - bbox.min()[NR::Y]);

    }

    sp_document_done ( SP_DT_DOCUMENT (selection->desktop()) );

}

void
Transformation::applyPageScale(Inkscape::Selection *selection)
{
    double scaleX = _scalar_scale_horizontal.getValue("px");
    double scaleY = _scalar_scale_vertical.getValue("px");

    NR::Rect const bbox(selection->bounds());
    NR::Point const center(bbox.midpoint());

    if (_units_scale.isAbsolute()) {

        NR::scale const numerator(scaleX, scaleY);
        NR::scale const denominator(bbox.dimensions());
        sp_selection_scale_relative(selection, center, numerator / denominator);

    } else {

        NR::scale const numerator(scaleX, scaleY);
        NR::scale const denominator(bbox.dimensions());
        sp_selection_scale_relative(selection, center, numerator / denominator);


    }

    sp_document_done(SP_DT_DOCUMENT(selection->desktop()));
}

void
Transformation::applyPageRotate(Inkscape::Selection *selection)
{

    double angle = _scalar_scale_vertical.getValue("deg");

    NR::Rect bbox = selection->bounds();
    NR::Point center = bbox.midpoint();
    sp_selection_rotate_relative(selection, center, angle);

    sp_document_done(SP_DT_DOCUMENT(selection->desktop()));

}

void
Transformation::applyPageSkew(Inkscape::Selection *selection)
{

    double skewX = _scalar_skew_horizontal.getValue("px");
    double skewY = _scalar_skew_vertical.getValue("px");

    NR::Rect bbox = selection->bounds();
    NR::Point center = bbox.midpoint();

    sp_selection_skew_relative(selection, center, skewX, skewY);

    sp_document_done(SP_DT_DOCUMENT(selection->desktop()));

}


void
Transformation::applyPageTransform(Inkscape::Selection *selection)
{
    double a = _scalar_transform_a.getValue();
    double b = _scalar_transform_b.getValue();
    double c = _scalar_transform_c.getValue();
    double d = _scalar_transform_d.getValue();
    double e = _scalar_transform_e.getValue();
    double f = _scalar_transform_f.getValue();

    NR::Matrix matrix(a, b, c, d, e, f);

    sp_selection_apply_affine(selection, matrix);

    sp_document_done(SP_DT_DOCUMENT(selection->desktop()));
}



////////////////////////////////////////////////////////////////////////
// Internal routines
////////////////////////////////////////////////////////////////////////

int
Transformation::scaleSetUnit(Unit const *old_unit,
                             Unit const *new_unit,
                             GObject *dlg)
{
    Inkscape::Selection *selection = _getSelection();

    if (!selection || selection->isEmpty()) {
        return false;
    }

    if (old_unit->isAbsolute() && !new_unit->isAbsolute()) {
/* TODO:  Gtkmm
       // Absolute to percentage
       set_data("update", GUINT_TO_POINTER (TRUE));
       double x = _scalar_scale_horizontal.getValue("px");
       double y = _scalar_scale_vertical.getValue("px");
*/
       NR::Rect bbox = selection->bounds();
/*
       _scalar_scale_horizontal.setValue(100.0 * x / bbox.extent(NR::X), "%");
       _scalar_scale_vertical.setValue(100.0 * y / bbox.extent(NR::Y), "%");
       set_data("update", GUINT_TO_POINTER (FALSE));
       return true;
*/

   } else if (old_unit->isAbsolute() && new_unit->isAbsolute()) {
/* TODO:  Gtkmm
    // Percentage to absolute
    set_data("update", GUINT_TO_POINTER (TRUE));
    double x = _scalar_scale_horizontal.getValue("px");
    double y = _scalar_scale_vertical.getValue("px");
*/
    NR::Rect bbox = selection->bounds();
/*
    _scalar_scale_horizontal.setValue( 0.01 * x * bbox.extent(NR::X), new_units);
    _scalar_scale_vertical.setValue(   0.01 * y * bbox.extent(NR::Y), new_units);
    set_data("update", GUINT_TO_POINTER (FALSE));
    return true;
*/
    }

    return false;
}

void
Transformation::onMoveValueChanged()
{
    if (get_data("update")) {
        return;
    }
    g_message("Move value changed to %f, %f px\n",
              _scalar_move_horizontal.getValue("px"),
              _scalar_move_vertical.getValue("px"));
    set_response_sensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onMoveRelativeToggled()
{
    if (get_data("update")) {
        return;
    }
    Inkscape::Selection *selection = _getSelection();

    if (!selection || selection->isEmpty()) {
        return;
    }



/* TODO:
    NR::Rect bbox = selection->bounds();

    // Read values from widget
    double x = _scalar_move_horizontal.getValue("px");
    double y = _scalar_move_vertical.getValue("px");

    set_data("update", GUINT_TO_POINTER (TRUE));

    if (_check_move_relative.get_active(tb)) {
        // From absolute to relative
        _scalar_move_horizontal.setValue(x - bbox.min()[NR::X], "px");
        _scalar_move_vertical.setValue(  y - bbox.min()[NR::Y], "px");
    } else {
        // From relative to absolute
        _scalar_move_horizontal.setValue(bbox.min()[NR::X] + x, "px");
        _scalar_move_vertical.setValue(  bbox.min()[NR::Y] + y, "px");
    }
*/

    g_message("Move relative changed\n");
    set_response_sensitive(Gtk::RESPONSE_APPLY, true);

    set_data("update", GUINT_TO_POINTER(false));
}

void
Transformation::onScaleValueChanged()
{
    if (get_data("update")) {
        return;
    }
    g_message("Scale value changed to %f, %f px\n",
              _scalar_scale_horizontal.getValue("px"),
              _scalar_scale_vertical.getValue("px"));
    set_response_sensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onRotateValueChanged()
{
    if (get_data("update")) {
        return;
    }
    g_message("Rotate value changed to %f deg\n",
              _scalar_scale_vertical.getValue("deg"));
    set_response_sensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onSkewValueChanged()
{
    if (get_data("update")) {
        return;
    }
    g_message("Skew value changed to %f, %f px\n",
              _scalar_skew_horizontal.getValue("px"),
              _scalar_skew_vertical.getValue("px"));
    set_response_sensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onTransformValueChanged()
{
    if (get_data("update")) {
        return;
    }
    g_message("Transform value changed to (%f, %f, %f, %f, %f, %f)\n",
              _scalar_transform_a.getValue(),
              _scalar_transform_b.getValue(),
              _scalar_transform_c.getValue(),
              _scalar_transform_d.getValue(),
              _scalar_transform_e.getValue(),
              _scalar_transform_f.getValue());

    set_response_sensitive(Gtk::RESPONSE_APPLY, true);
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
