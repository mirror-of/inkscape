// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef __UI_DIALOG_SHAPEICON_H__
#define __UI_DIALOG_SHAPEICON_H__
/*
 * Authors:
 *   Martin Owens
 *
 * Copyright (C) 2020 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm/iconinfo.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/widget.h>
#include <glibmm/property.h>

namespace Inkscape {
namespace UI {
namespace Widget {

// Object overlay states usually modify the icon and indicate
// That there may be non-item children under this item (e.g. clip)
using OverlayState = int;
enum OverlayStates : OverlayState {
    OVERLAY_NONE = 0,     // Nothing special about the object.
    OVERLAY_CLIP = 1,     // Object has a clip
    OVERLAY_MASK = 2      // Object has a mask
};

/* Custom cell renderer for type icon */
class CellRendererItemIcon : public Gtk::CellRenderer {
public:
  
    CellRendererItemIcon() :
        Glib::ObjectBase(typeid(CellRenderer)),
        Gtk::CellRenderer(),
        _property_shape_type(*this, "shape_type", "unknown"),
        _property_color(*this, "color", 0),
        _property_clipmask(*this, "clipmask", 0),
        _clip_overlay(nullptr),
        _mask_overlay(nullptr)
    {
        Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, _size, _size);
    } 
    ~CellRendererItemIcon() override = default;
     
    Glib::PropertyProxy<std::string> property_shape_type() {
        return _property_shape_type.get_proxy();
    }
    Glib::PropertyProxy<unsigned int> property_color() {
        return _property_color.get_proxy();
    }
    Glib::PropertyProxy<unsigned int> property_clipmask() {
        return _property_clipmask.get_proxy();
    }
  
protected:
    void render_vfunc(const Cairo::RefPtr<Cairo::Context>& cr, 
                      Gtk::Widget& widget,
                      const Gdk::Rectangle& background_area,
                      const Gdk::Rectangle& cell_area,
                      Gtk::CellRendererState flags) override;
    void paint_icon(const Cairo::RefPtr<Cairo::Context>& cr,
                    Gtk::Widget& widget,
                    Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                    int x, int y);

    void get_preferred_width_vfunc(Gtk::Widget& widget, int& min_w, int& nat_w) const override;
    void get_preferred_height_vfunc(Gtk::Widget& widget, int& min_h, int& nat_h) const override;

private:
  
    int _size;
    Glib::Property<std::string> _property_shape_type;
    Glib::Property<unsigned int> _property_color;
    Glib::Property<unsigned int> _property_clipmask;
    std::map<const std::string, Glib::RefPtr<Gdk::Pixbuf> > _icon_cache;

    // Overlay indicators
    Glib::RefPtr<Gdk::Pixbuf> _mask_overlay;
    Glib::RefPtr<Gdk::Pixbuf> _clip_overlay;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif /* __UI_DIALOG_SHAPEICON_H__ */

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
