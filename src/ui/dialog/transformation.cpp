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

#include "transformation.h"
#include "libnr/nr-rect.h"
#include "selection.h"
#include "selection-chemistry.h"

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
 *
 * The dialog is implemented as a Gtk::Notebook with four pages.
 * The pages are implemented using Inkscape's NotebookPage which
 * is used to help make sure all of Inkscape's notebooks follow
 * the same style.  We then populate the pages with our widgets,
 * we use the ScalarUnit class for this.
 *
 * Note that skew had been disabled in the previous interface, so
 * maybe the underlying code had a bug in it?  So it may need some
 * additional debugging once it is set up.
 */
Transformation::Transformation()
    : Dialog ("dialogs.transformation"),
      _page_move              ("Move",   4, 2),
      _page_scale             ("Scale",  4, 2),
      _page_rotate            ("Rotate", 4, 2),
      _page_skew              ("Skew",   4, 2),
      _scalar_move_horizontal ("Horizontal", UNIT_TYPE_LINEAR, "",
                               "arrows_hor.xpm", &_units_move),
      _scalar_move_vertical   ("Vertical",   UNIT_TYPE_LINEAR, "",
                               "arrows_ver.xpm", &_units_move),
      _scalar_scale_horizontal("Width",      UNIT_TYPE_LINEAR, "",
                               "scale_hor.xpm", &_units_scale),
      _scalar_scale_vertical  ("Height",     UNIT_TYPE_LINEAR, "",
                               "scale_ver.xpm", &_units_scale),
      _scalar_rotate          ("Rotate",     UNIT_TYPE_RADIAL, "",
                               "rotate.xpm"),
      _scalar_skew_horizontal ("Horizontal", UNIT_TYPE_LINEAR, "",
                               "arrows_hor.xpm", &_units_skew),
      _scalar_skew_vertical   ("Vertical",   UNIT_TYPE_LINEAR, "",
                               "arrows_ver.xpm", &_units_skew),
      _check_move_relative    ("Relative move")
{
    set_title(_("Transformation"));

    transientize();

    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);
    
    _notebook.append_page(_page_move,   _("Move"));
    _notebook.append_page(_page_scale,  _("Scale"));
    _notebook.append_page(_page_rotate, _("Rotate"));
    _notebook.append_page(_page_skew,   _("Skew"));

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

    initPageMove();
    initPageScale();
    initPageRotate();
    initPageSkew();

    updateSelection(PAGE_MOVE, _getSelection());

    show_all_children();
}

Transformation::~Transformation() 
{
}

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
    }

    set_response_sensitive(Gtk::RESPONSE_APPLY, false);
}

void 
Transformation::present(Transformation::PageType page)
{
    _notebook.set_current_page(page);
    Gtk::Dialog::show();
    Gtk::Dialog::present();
}

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
Transformation::applyPageMove(Inkscape::Selection *selection)
{
    //double x = _scalar_move_horizontal.getValue("px");
    //double y = _scalar_move_vertical.getValue("px");

    if (_check_move_relative.get_active()) {
        //sp_selection_move_relative(x, y);
    } else {
        NR::Rect bbox = selection->bounds();
        //sp_selection_move_relative(x - bbox.min()[NR::X], y - bbox.min()[NR::Y]);
    }

    if (selection) {
        //sp_document_done ( SP_DT_DOCUMENT (selection->desktop()) );
    }
}

void 
Transformation::applyPageScale(Inkscape::Selection *selection)
{
    NR::Rect const bbox(selection->bounds());
    NR::Point const center(bbox.midpoint());

    if (_units_scale.isAbsolute()) {
/* TODO
        NR::scale const numerator(_unit_scale.get_value_in_points(ax),
                                  _unit_scale.get_value_in_points(ay));
        NR::scale const denominator(bbox.dimensions());
        selection->scale_relative(center, numerator / denominator);
*/
    } else {
/* TODO
        selection->scale_relative(center,
                                  NR::scale(0.01 * scalar_scale_horizontal.getValue(),
                                  0.01 * scalar_scale_vertical.getValue()));
*/
    }

    if (selection) {
/*
        sp_document_done(SP_DT_DOCUMENT(selection->desktop()));
*/
    }
}

void 
Transformation::applyPageRotate(Inkscape::Selection *selection)
{
    NR::Rect bbox = selection->bounds();
    NR::Point center = bbox.midpoint();
/*
    sp_selection_rotate_relative(center, scalar_rotate.getValue();
*/
    if (selection) {
/*
        sp_document_done (SP_DT_DOCUMENT (selection->desktop()));
*/
    }
}

void 
Transformation::applyPageSkew(Inkscape::Selection *selection)
{
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
