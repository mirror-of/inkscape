// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Authors:
 *   Jon A. Cruz
 *   Johan B. C. Engelen
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm/iconinfo.h>

#include "ui/widget/imagetoggler.h"

#include "ui/icon-loader.h"
#include "ui/icon-names.h"
#include "widgets/toolbox.h"

namespace Inkscape {
namespace UI {
namespace Widget {

ImageToggler::ImageToggler( char const* on, char const* off) :
    Glib::ObjectBase(typeid(ImageToggler)),
    Gtk::CellRenderer(),
    _pixOnName(on),
    _pixOffName(off),
    _property_active(*this, "active", false),
    _property_activatable(*this, "activatable", true),
    _property_gossamer(*this, "gossamer", false),
    _property_pixbuf_on(*this, "pixbuf_on", Glib::RefPtr<Gdk::Pixbuf>(nullptr)),
    _property_pixbuf_off(*this, "pixbuf_off", Glib::RefPtr<Gdk::Pixbuf>(nullptr))
{
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
    Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, _size, _size);
}

void ImageToggler::get_preferred_height_vfunc(Gtk::Widget& widget, int& min_h, int& nat_h) const
{
    min_h = _size + 6;
    nat_h = _size + 8;
}

void ImageToggler::get_preferred_width_vfunc(Gtk::Widget& widget, int& min_w, int& nat_w) const
{
    min_w = _size + 12;
    nat_w = _size + 16;
}

void ImageToggler::render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 Gtk::CellRendererState flags )
{
    // Lazy/late pixbuf rendering to get access to scale factor from widget.
    if(!_property_pixbuf_on.get_value()) {
        int scale = widget.get_scale_factor();
        _property_pixbuf_on = sp_get_icon_pixbuf(_pixOnName, _size * scale);
        _property_pixbuf_off = sp_get_icon_pixbuf(_pixOffName, _size * scale);
    }

    // Hide when not being used.
    bool visible = _property_activatable.get_value()
                || _property_active.get_value();
    if (!visible) {
        return;
    }

    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    if(_property_active.get_value()) {
        pixbuf = _property_pixbuf_on.get_value();
    } else {
        pixbuf = _property_pixbuf_off.get_value();
    }

    cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(
            pixbuf->gobj(), 0, widget.get_window()->gobj());
    g_return_if_fail(surface);

    // Center the icon in the cell area
    int x = cell_area.get_x() + int((cell_area.get_width() - _size) * 0.5);
    int y = cell_area.get_y() + int((cell_area.get_height() - _size) * 0.5);

    cairo_set_source_surface(cr->cobj(), surface, x, y);
    cr->set_operator(Cairo::OPERATOR_ATOP);
    cr->rectangle(x, y, _size, _size);
    if (_property_gossamer.get_value()) {
        cr->clip();
        cr->paint_with_alpha(0.2);
    } else {
        cr->fill();
    }
    cairo_surface_destroy(surface); // free!
}

bool
ImageToggler::activate_vfunc(GdkEvent* event,
                            Gtk::Widget& /*widget*/,
                            const Glib::ustring& path,
                            const Gdk::Rectangle& /*background_area*/,
                            const Gdk::Rectangle& /*cell_area*/,
                            Gtk::CellRendererState /*flags*/)
{
    _signal_pre_toggle.emit(event);
    _signal_toggled.emit(path);

    return false;
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

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


