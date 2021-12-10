// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * SVG to Pixbuf renderer
 *
 * Author:
 *   Michael Kowalski
 *
 * Copyright (C) 2020-2021 Michael Kowalski
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "svg-renderer.h"
#include "io/file.h"
#include "xml/repr.h"
#include "object/sp-root.h"
#include "display/cairo-utils.h"
#include "helper/pixbuf-ops.h"
#include "util/units.h"

namespace Inkscape {

Glib::ustring rgba_to_css_color(double r, double g, double b) {
    char buffer[16];
    sprintf(buffer, "#%02x%02x%02x",
        static_cast<int>(r * 0xff + 0.5),
        static_cast<int>(g * 0xff + 0.5),
        static_cast<int>(b * 0xff + 0.5)
    );
    return Glib::ustring(buffer);
}

Glib::ustring rgba_to_css_color(const Gdk::RGBA& color) {
    return rgba_to_css_color(color.get_red(), color.get_green(), color.get_blue());
}

Glib::ustring rgba_to_css_color(const SPColor& color) {
    float rgb[3];
    color.get_rgb_floatv(rgb);
    return rgba_to_css_color(rgb[0], rgb[1], rgb[2]);
}

Glib::ustring double_to_css_value(double value) {
    char buffer[32];
    // arbitrarily chosen precision
    sprintf(buffer, "%.4f", value);
    return Glib::ustring(buffer);
}

svg_renderer::svg_renderer(const char* svg_file_path) {

    auto file = Gio::File::create_for_path(svg_file_path);

    _document.reset(ink_file_open(file, nullptr));

    if (_document) {
        _root = _document->getRoot();
    }

    if (!_root) throw std::runtime_error("Cannot find root element in svg document");
}

size_t svg_renderer::set_style(const Glib::ustring& selector, const char* name, const Glib::ustring& value) {
    auto objects = _document->getObjectsBySelector(selector);
    for (auto el : objects) {
        if (SPCSSAttr* css = sp_repr_css_attr(el->getRepr(), "style")) {
            sp_repr_css_set_property(css, name, value.c_str());
            el->changeCSS(css, "style");
            sp_repr_css_attr_unref(css);
        }
    }
    return objects.size();
}

double svg_renderer::get_width_px() const {
    return _document->getWidth().value("px");
}

double svg_renderer::get_height_px() const {
    return _document->getHeight().value("px");
}

Inkscape::Pixbuf* svg_renderer::do_render(double scale) {
    auto w = _document->getWidth().value("px");
    auto h = _document->getHeight().value("px");
    auto dpi = 96 * scale;
    auto area = Geom::Rect(0, 0, w, h);

    return sp_generate_internal_bitmap(_document.get(), area, dpi);
}

Glib::RefPtr<Gdk::Pixbuf> svg_renderer::render(double scale) {
    auto pixbuf = do_render(scale);
    if (!pixbuf) return Glib::RefPtr<Gdk::Pixbuf>();

    // ref it
    auto raw = Glib::wrap(pixbuf->getPixbufRaw(), true);
    delete pixbuf;
    return raw;
}

Cairo::RefPtr<Cairo::Surface> svg_renderer::render_surface(double scale) {
    auto pixbuf = do_render(scale);
    if (!pixbuf) return Cairo::RefPtr<Cairo::Surface>();

    // ref it by saying that we have no reference
    auto surface = Cairo::RefPtr<Cairo::Surface>(new Cairo::Surface(pixbuf->getSurfaceRaw(), false));
    delete pixbuf;
    return surface;
}

} // namespace
