// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Utility functions for generating export previews.
 */
/* Authors:
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *   Martin Owens <doctormo@gmail.com>
 *
 * Copyright (C) 2021 Anshudhar Kumar Singh
 *               2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "preview.h"

#include "desktop.h"
#include "document.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"
#include "page-manager.h"

namespace Inkscape {
namespace UI {
namespace PREVIEW {

GdkPixbuf *render_preview(SPDocument *doc, Inkscape::Drawing &drawing, SPItem *item,
                          unsigned width_in, unsigned height_in, Geom::OptRect *dboxIn)
{
    if (auto name = (item ? item->getId() : nullptr)) {
        // Get item even if it's in another document.
        if (item->document != doc) {
            item = dynamic_cast<SPItem *>(doc->getObjectById(name));
        }
    }

    Geom::OptRect dbox;
    if (dboxIn) {
        dbox = *dboxIn;
    } else if (item) {
        if (item->parent) {
            dbox = item->documentVisualBounds();
        } else {
            dbox = doc->preferredBounds();
        }
    } else if (doc->getRoot()) {
        // If we still dont have a dbox we will use document coordinates.
        dbox = doc->getRoot()->documentVisualBounds();
    }

    // If we still dont have anything to render then return
    if (!dbox) return nullptr;

    // Calculate a scaling factor for the requested bounding box.
    double sf = 1.0;
    Geom::IntRect ibox = dbox->roundOutwards();
    if (ibox.width() != width_in || ibox.height() != height_in) {
        sf = std::min((double)width_in / dbox->width(),
                      (double)height_in / dbox->height());
        auto scaled_box = *dbox * Geom::Scale(sf);
        ibox = scaled_box.roundOutwards();
    }

    // Resize the contents to the available space with a scale factor
    drawing.root()->setTransform(Geom::Scale(sf));
    drawing.update();

    Geom::IntPoint pdim(width_in, height_in);
    // The unsigned width/height can wrap around when negative.
    int dx = ((int)width_in - ibox.width()) / 2;
    int dy = ((int)height_in - ibox.height()) / 2;
    Geom::IntRect area = Geom::IntRect::from_xywh(ibox.min() - Geom::IntPoint(dx, dy), pdim);

    /* Actual renderable area */
    Geom::IntRect ua = *Geom::intersect(ibox, area);

    /* Render */
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ua.width(), ua.height());
    Inkscape::DrawingContext dc(s, ua.min());
    cairo_t *cr = cairo_create(s);
    cairo_rectangle(cr, 0, 0, ua.width(), ua.height());

    guint32 bg = doc->getPageManager().background_color;

    // We always use checkerboard to indicate transparency.
    if (SP_RGBA32_A_F(bg) < 1.0) {
        auto pattern = ink_cairo_pattern_create_checkerboard(bg, false);
        auto background = Cairo::RefPtr<Cairo::Pattern>(new Cairo::Pattern(pattern));
        cairo_set_source(cr, background->cobj());
        cairo_fill(cr);
    }
    // We always draw the background on top to indicate partial backgrounds
    auto background = Cairo::SolidPattern::create_rgba(
        SP_RGBA32_R_F(bg), SP_RGBA32_G_F(bg),
        SP_RGBA32_B_F(bg), SP_RGBA32_A_F(bg));
    cairo_set_source(cr, background->cobj());
    cairo_fill(cr);

    cairo_save(cr);
    cairo_destroy(cr);

    drawing.render(dc, ua);
    cairo_surface_flush(s);
    return ink_pixbuf_create_from_cairo_surface(s);
}

} // namespace PREVIEW
} // namespace UI
} // namespace Inkscape
