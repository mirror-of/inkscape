// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Icon Loader
 *
 * Icon Loader management code
 *
 * Authors:
 *  Jabiertxo Arraiza <jabier.arraiza@marker.es>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */


#include "icon-loader.h"
#include "inkscape.h"
#include "io/resource.h"
#include "svg/svg-color.h"
#include "widgets/toolbox.h"
#include <fstream>
#include <gdkmm/display.h>
#include <gdkmm/screen.h>
#include <gtkmm/iconinfo.h>
#include <gtkmm/icontheme.h>

Gtk::Image *sp_get_icon_image(Glib::ustring icon_name, gint size)
{
    Gtk::Image *icon = new Gtk::Image();
    icon->set_from_icon_name(icon_name, Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
    icon->set_pixel_size(size);
    return icon;
}

Gtk::Image *sp_get_icon_image(Glib::ustring icon_name, Gtk::IconSize icon_size)
{
    Gtk::Image *icon = new Gtk::Image();
    icon->set_from_icon_name(icon_name, icon_size);
    return icon;
}

Gtk::Image *sp_get_icon_image(Glib::ustring icon_name, Gtk::BuiltinIconSize icon_size)
{
    Gtk::Image *icon = new Gtk::Image();
    icon->set_from_icon_name(icon_name, icon_size);
    return icon;
}

Gtk::Image *sp_get_icon_image(Glib::ustring icon_name, gchar const *prefs_size)
{
    Gtk::IconSize icon_size = Inkscape::UI::ToolboxFactory::prefToSize_mm(prefs_size);
    return sp_get_icon_image(icon_name, icon_size);
}

GtkWidget *sp_get_icon_image(Glib::ustring icon_name, GtkIconSize icon_size)
{
    return gtk_image_new_from_icon_name(icon_name.c_str(), icon_size);
}

Glib::RefPtr<Gdk::Pixbuf> sp_get_icon_pixbuf(Glib::ustring icon_name, gint size)
{
    Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
    Glib::RefPtr<Gdk::Screen>  screen = display->get_default_screen();
    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_for_screen(screen);
    auto prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/theme/symbolicIcons", false) && icon_name.find("-symbolic") == Glib::ustring::npos) {
        icon_name += Glib::ustring("-symbolic");
    }
    Gtk::IconInfo iconinfo = icon_theme->lookup_icon(icon_name, size, Gtk::ICON_LOOKUP_FORCE_SIZE);
    Glib::RefPtr<Gdk::Pixbuf> _icon_pixbuf;
    if (prefs->getBool("/theme/symbolicIcons", false)) {
        Gtk::Window *window = SP_ACTIVE_DESKTOP->getToplevel();
        if (window) {
            Glib::RefPtr<Gtk::StyleContext> stylecontext = window->get_style_context();
            bool was_symbolic = false;
            _icon_pixbuf = iconinfo.load_symbolic(stylecontext, was_symbolic);
        } else {
            // we never go here
            _icon_pixbuf = iconinfo.load_icon();
        }
    } else {
        _icon_pixbuf = iconinfo.load_icon();
    }
    return _icon_pixbuf;
}

Glib::RefPtr<Gdk::Pixbuf> sp_get_icon_pixbuf(Glib::ustring icon_name, Gtk::IconSize icon_size)
{
    int width, height;
    Gtk::IconSize::lookup(icon_size, width, height);
    return sp_get_icon_pixbuf(icon_name, width);
}

Glib::RefPtr<Gdk::Pixbuf> sp_get_icon_pixbuf(Glib::ustring icon_name, Gtk::BuiltinIconSize icon_size, int scale)
{
    int width, height;
    Gtk::IconSize::lookup(Gtk::IconSize(icon_size), width, height);
    return sp_get_icon_pixbuf(icon_name, width * scale);
}

Glib::RefPtr<Gdk::Pixbuf> sp_get_icon_pixbuf(Glib::ustring icon_name, GtkIconSize icon_size, int scale)
{
    gint width, height;
    gtk_icon_size_lookup(icon_size, &width, &height);
    return sp_get_icon_pixbuf(icon_name, width * scale);
}

Glib::RefPtr<Gdk::Pixbuf> sp_get_icon_pixbuf(Glib::ustring icon_name, gchar const *prefs_size, int scale)
{
    // Load icon based in preference size defined allowed values are:
    //"/toolbox/tools/small" Toolbox icon size
    //"/toolbox/small" Control bar icon size
    //"/toolbox/secondary" Secondary toolbar icon size
    GtkIconSize icon_size = Inkscape::UI::ToolboxFactory::prefToSize(prefs_size);
    return sp_get_icon_pixbuf(icon_name, icon_size, scale);
}

/**
 * Get the shape icon for this named shape type. For example 'rect'. These icons
 * are always symbolic icons no matter the theme in order to be coloured by the highlight
 * color.
 *
 * @param shape_type - A string id for the shape from SPItem->typeName()
 * @param color - The fg color of the shape icon
 * @param size - The icon size to generate
 */
Glib::RefPtr<Gdk::Pixbuf> sp_get_shape_icon(Glib::ustring shape_type, Gdk::RGBA color, gint size, int scale)
{
    Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
    Glib::RefPtr<Gdk::Screen>  screen = display->get_default_screen();
    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_for_screen(screen);

    Gtk::IconInfo iconinfo = icon_theme->lookup_icon("shape-" + shape_type + "-symbolic",
                                                     size * scale, Gtk::ICON_LOOKUP_FORCE_SIZE);
    if (!iconinfo) {
        iconinfo = icon_theme->lookup_icon("shape-unknown-symbolic", size * scale, Gtk::ICON_LOOKUP_FORCE_SIZE);
        // We know this could fail, but it should exist, so persist.
    }
    // Gtkmm requires all colours, even though gtk does not
    auto other = Gdk::RGBA("black");
    bool was_symbolic = false;
    return iconinfo.load_symbolic(color, other, other, other, was_symbolic);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
