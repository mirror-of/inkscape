// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape pages implementation
 *
 * Authors:
 *   Martin Owens <doctormo@geek-2.com>
 *
 * Copyright (C) 2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "canvas-page.h"
#include "canvas-item-rect.h"
#include "canvas-item-text.h"

namespace Inkscape {

CanvasPage::~CanvasPage()
{
    for (auto item : canvas_items) {
        delete item;
    }
    canvas_items.clear();
}

/**
 * Add the page canvas to the given canvas item groups (canvas view is implicit)
 */
void CanvasPage::add(Geom::Rect size, CanvasItemGroup *background_group, CanvasItemGroup *border_group)
{
    // Foreground 'border'
    if (auto item = new CanvasItemRect(border_group, size)) {
        item->set_name("foreground");
        canvas_items.push_back(item);
    }

    // Background rectangle 'fill'
    if (auto item = new CanvasItemRect(background_group, size)) {
        item->set_name("background");
        item->set_dashed(false);
        item->set_inverted(false);
        item->set_stroke(0x00000000);
        canvas_items.push_back(item);
    }

    if (auto label = new CanvasItemText(border_group, Geom::Point(0, 0), "{Page Label}")) {
        label->set_fontsize(10.0);
        label->set_fill(0xffffffff);
        label->set_background(0x00000099);
        label->set_bg_radius(1.0);
        label->set_anchor(Geom::Point(-1.0, -1.5));
        label->set_adjust(Geom::Point(-3, 0));
        canvas_items.push_back(label);
    }
}
/**
 * Hide the page in the given canvas widget.
 */
void CanvasPage::remove(UI::Widget::Canvas *canvas)
{
    g_assert(canvas != nullptr);
    for (auto it = canvas_items.begin(); it != canvas_items.end();) {
        if (canvas == (*it)->get_canvas()) {
            delete (*it);
            it = canvas_items.erase(it);
        } else {
            ++it;
        }
    }
}

void CanvasPage::show()
{
    for (auto item : canvas_items) {
        item->show();
    }
}

void CanvasPage::hide()
{
    for (auto item : canvas_items) {
        item->hide();
    }
}

/**
 * Update the visual representation of a page on screen.
 *
 * @param size - The size of the page in desktop units
 * @param txt - An optional label for the page
 * @param outline - Disable normal rendering and show as an outline.
 */
void CanvasPage::update(Geom::Rect size, const char *txt, bool outline)
{
    // Put these in the preferences?
    bool border_on_top = _border_on_top;
    guint32 shadow_color = _border_color; // there's no separate shadow color in the UI, border color is used
    guint32 select_color = 0xff0000cc;
    guint32 border_color = _border_color;

    // This is used when showing the viewport as *not a page* it's mostly
    // never used as the first page is normally the viewport too.
    if (outline) {
        border_on_top = false;
        _shadow_size = 0;
        border_color = select_color;
    }

    for (auto item : canvas_items) {
        if (auto rect = dynamic_cast<CanvasItemRect *>(item)) {
            rect->set_rect(size);

            bool is_foreground = (rect->get_name() == "foreground");
            // This will put the border on the background OR foreground layer as needed.
            if (is_foreground == border_on_top) {
                rect->show();
                rect->set_shadow(shadow_color, _shadow_size);
                rect->set_stroke(is_selected ? select_color : border_color);
            } else {
                rect->hide();
                rect->set_shadow(0x0, 0);
                rect->set_stroke(0x0);
            }
            // This undoes the hide for the background rect, but that's ok
            if (!is_foreground) {
                rect->show();
                if (_checkerboard) {
                    rect->set_background_checkerboard(_background_color, true);
                }
                else {
    // TODO: This ignores the requested transparency to paint the background.
    // there is disagreement between developers about this feature.
                    rect->set_background(_background_color | 0xff);
                }
            }
        }
        if (auto label = dynamic_cast<CanvasItemText *>(item)) {
            if (txt) {
                auto corner = size.corner(0);
                label->set_coord(corner);
                label->set_text(txt);
                label->show();
            } else {
                label->set_text("");
                label->hide();
            }
        }
    }
}

bool CanvasPage::setAttributes(bool on_top, guint32 border, guint32 bg, int shadow, bool checkerboard)
{
    if (on_top != _border_on_top || border != _border_color || bg != _background_color || shadow != _shadow_size || checkerboard != _checkerboard) {
        this->_border_on_top = on_top;
        this->_border_color = border;
        this->_background_color = bg;
        _shadow_size = shadow;
        _checkerboard = checkerboard;
        return true;
    }
    return false;
}

};

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
