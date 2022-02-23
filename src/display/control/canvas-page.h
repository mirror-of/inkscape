// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * 
 *//*
 * Authors:
 *   Martin Owens 2021
 * 
 * Copyright (C) 2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_CANVAS_PAGE_H
#define SEEN_CANVAS_PAGE_H

#include <2geom/rect.h>
#include <glib.h>
#include <vector>

#include "canvas-item.h"

namespace Inkscape {
    namespace UI {
        namespace Widget {
            class Canvas;
        };
    };

    class CanvasItemGroup;

class CanvasPage
{
public:
    CanvasPage() = default;
    ~CanvasPage();

    void update(Geom::Rect size, const char *txt, bool outline = false);
    void add(Geom::Rect size, CanvasItemGroup *background_group, CanvasItemGroup *foreground_group);
    void remove(UI::Widget::Canvas *canvas);
    void show();
    void hide();

    bool setAttributes(bool on_top, guint32 border, guint32 bg, int shadow, bool checkerboard);
    void setOutline(bool outline);

    bool is_selected = false;
private:
    // This may make this look like a CanvasItemGroup, but it's not one. This
    // isn't a collection of items, but a set of items in multiple Canvases.
    // Each item can belong in either a foreground or background group.
    std::vector<Inkscape::CanvasItem *> canvas_items;

    int _shadow_size = 0;
    bool _border_on_top = true;
    guint32 _background_color = 0xffffff00;
    guint32 _border_color = 0x00000040;
    bool _checkerboard = false;
};

};

#endif // SEEN_CANVAS_PAGE_H

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
