// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Authors:
 *   Martin Owens
 *
 * Copyright (C) 2020 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */


#include "color.h"
#include "ui/widget/shapeicon.h"
#include "ui/icon-loader.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/*
 * This is a type of CellRenderer which you might expect to inherit from the
 * pixbuf CellRenderer, but we actually need to write a Cairo surface directly
 * in order to maintain HiDPI sharpness in icons. Upstream Gtk have made it clear
 * that CellRenderers are going away in Gtk4 so they aren't interested in fixing
 * rendering problems like the one in CellRendererPixbuf.
 *
 * See: https://gitlab.gnome.org/GNOME/gtk/-/issues/613
 */

void CellRendererItemIcon::render_vfunc(const Cairo::RefPtr<Cairo::Context>& cr, 
                                      Gtk::Widget& widget,
                                      const Gdk::Rectangle& background_area,
                                      const Gdk::Rectangle& cell_area,
                                      Gtk::CellRendererState flags)
{
    std::string shape_type = _property_shape_type.get_value();
    std::string highlight = SPColor(_property_color.get_value()).toString();
    std::string cache_id = shape_type + "-" + highlight;

    // if the icon isn't cached, render it to a pixbuf
    int scale = widget.get_scale_factor();
    if ( !_icon_cache[cache_id] ) { 
        _icon_cache[cache_id] = sp_get_shape_icon(shape_type, Gdk::RGBA(highlight), _size, scale);
    }
    g_return_if_fail(_icon_cache[cache_id]);
  
    // Center the icon in the cell area
    int x = cell_area.get_x() + int((cell_area.get_width() - _size) * 0.5);
    int y = cell_area.get_y() + int((cell_area.get_height() - _size) * 0.5);

    // Paint the pixbuf to a cairo surface to get HiDPI support
    paint_icon(cr, widget, _icon_cache[cache_id], x, y);

    // Create an overlay icon
    int clipmask = _property_clipmask.get_value();
    if (clipmask > 0) {
        if (!_clip_overlay) {
            _clip_overlay = sp_get_icon_pixbuf("overlay-clip", Gtk::ICON_SIZE_MENU, scale);
        }
        if (!_mask_overlay) {
            _mask_overlay = sp_get_icon_pixbuf("overlay-mask", Gtk::ICON_SIZE_MENU, scale);
        }

        if (clipmask == OVERLAY_CLIP && _clip_overlay) {
            paint_icon(cr, widget, _clip_overlay, x, y);
        }
        if (clipmask == OVERLAY_MASK && _mask_overlay) {
            paint_icon(cr, widget, _mask_overlay, x, y);
        }
    }

}

void CellRendererItemIcon::paint_icon(const Cairo::RefPtr<Cairo::Context>& cr,
                                      Gtk::Widget& widget,
                                      Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                                      int x, int y)
{
    cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(
            pixbuf->gobj(), 0, widget.get_window()->gobj());
    if (!surface) return;
    cairo_set_source_surface(cr->cobj(), surface, x, y);
    cr->set_operator(Cairo::OPERATOR_ATOP);
    cr->rectangle(x, y, _size, _size);
    cr->fill();
    cairo_surface_destroy(surface); // free!
}

void CellRendererItemIcon::get_preferred_height_vfunc(Gtk::Widget& widget, int& min_h, int& nat_h) const
{
    min_h = _size;
    nat_h = _size + 4;
}   

void CellRendererItemIcon::get_preferred_width_vfunc(Gtk::Widget& widget, int& min_w, int& nat_w) const
{
    min_w = _size;
    nat_w = _size + 4;
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


