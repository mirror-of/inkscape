// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * Authors:
 *   Jabier Arraiza <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2019 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include "paned-canvas-.h"


namespace Inkscape {

SPPanedCanvas::SPPanedCanvas()
    : Gtk::Paned(Gtk::ORIENTATION_VERTICAL),
      _canvas1(nullptr),
      _canvas2(nullptr),
      _splited(false),
      _orientation(Gtk::ORIENTATION_VERTICAL),
{
    _canvas1 = Gtk::manage(SP_CANVAS(SPCanvas::createAA()));
    #if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        _canvas1->_enable_cms_display_adj = prefs->getBool("/options/displayprofile/enable");
    #endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    gtk_widget_set_can_focus (GTK_WIDGET (_canvas1), TRUE);
    gtk_widget_set_hexpand(GTK_WIDGET(_canvas1), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(_canvas1), TRUE);
    add1(_canvas1);

    _canvas2 = Gtk::manage(SP_CANVAS(SPCanvas::createAA()));
    #if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        _canvas2->_enable_cms_display_adj = prefs->getBool("/options/displayprofile/enable");
    #endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    gtk_widget_set_can_focus (GTK_WIDGET (_canvas2), TRUE);
    gtk_widget_set_hexpand(GTK_WIDGET(_canvas2), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(_canvas2), TRUE);
    add2(_canvas2);
    
    show_all_children();
}

SPPanedCanvas::~SPPanedCanvas()
{
    if (_panned) {
        delete _panned;
        _panned = nullptr;
    }
} 

void SPPanedCanvas::toggleOrientation(){
    if (_orientation == Gtk::ORIENTATION_HORIZONTAL) {
        _orientation  = Gtk::ORIENTATION_VERTICAL;
    } else {
        _orientation  = Gtk::ORIENTATION_HORIZONTAL;
    }
    reload();
}

void SPPanedCanvas::toggleSplit(){
    _split = !_split;
    reload();
}

void SPPanedCanvas::reload(){

}

}// namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
