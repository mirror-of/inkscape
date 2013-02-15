/*
 * Author:
 *   Marcel Lancelle (vaifrax) <vaifrax@marcel-lancelle.de>
 *   Code mostly reused/copied from ui/widget/page-sizer and display/canvas-grid
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_RULER_COORDS_H
#define INKSCAPE_UI_WIDGET_RULER_COORDS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stddef.h>
#include "ui/widget/registered-widget.h"
#include <sigc++/sigc++.h>

#include "helper/units.h"

#include <gtkmm/alignment.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <gtkmm/radiobutton.h>

namespace Inkscape {    
namespace XML {
class Node;
}

namespace UI {
namespace Widget {

class Registry;

/**
 * Data class and Widget for ruler coordinate system.
 * This also affects status message of mouse coordinates.
 */ 
class RulerCoordSystem : public Gtk::VBox
{
public:
	RulerCoordSystem(Registry & _wr);
    virtual ~RulerCoordSystem();
    void init();
    void updateWidgetsFromDoc();

protected:

    RegisteredUnitMenu    _rum_deflt;
    RegisteredScalarUnit  _rsu_off_x; // ruler offset
    RegisteredScalarUnit  _rsu_off_y;
    RegisteredScalar      _rs_mul_x; // ruler multiplier
    RegisteredScalar      _rs_mul_y;


/*
 some copied code snippets for later use

//    void fire_fit_canvas_to_selection_or_drawing();
    //Gtk::Button                       _rlr_setoffs_top;
    void on_portrait();
    void on_landscape();
    sigc::connection    _portrait_connection;
    sigc::connection    _landscape_connection;

    Gtk::Frame           _customFrame;

#if WITH_GTKMM_3_0
    Gtk::Grid            _marginTable;
#else
    Gtk::Table           _marginTable;
#endif

    Gtk::Alignment       _fitPageButtonAlign;
    Gtk::Button          _fitPageButton;
*/

    //callbacks
//    void on_value_changed();
//    void on_unit_changed();
//    sigc::connection    _changedw_connection;
//    sigc::connection    _changedh_connection;

    Registry            *_widgetRegistry;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif // INKSCAPE_UI_WIDGET_RULER_COORDS_H

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
