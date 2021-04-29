// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Helpers for SPItem -> gdk_pixbuf related stuff
 *
 * Authors:
 *   John Cliff <simarilius@yahoo.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008 John Cliff
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <2geom/transforms.h>

#include "helper/png-write.h"
#include "display/cairo-utils.h"
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "document.h"
#include "object/sp-root.h"
#include "object/sp-defs.h"
#include "object/sp-use.h"
#include "util/units.h"
#include "inkscape.h"

#include "helper/pixbuf-ops.h"

#include <gdk/gdk.h>

// TODO look for copy-n-paste duplication of this function:
/**
 * Hide all items that are not in list, recursively, skipping groups and defs.
 */
static void hide_other_items_recursively(SPObject *object, const std::vector<SPItem*> &items, unsigned dkey)
{
    SPItem *item = dynamic_cast<SPItem *>(object);
    if (!item) {
        // <defs>, <metadata>, etc.
        return;
    }

    if (std::find (items.begin(), items.end(), item) != items.end()) {
        // It's in our list, don't hide!
        return;
    }

    if ( !dynamic_cast<SPRoot  *>(item) &&
         !dynamic_cast<SPGroup *>(item) &&
         !dynamic_cast<SPUse   *>(item) ) {
        // Hide if not container or def.
        item->invoke_hide(dkey);
    }

    for (auto& child: object->children) {
        hide_other_items_recursively(&child, items, dkey);
    }
}


/**
    generates a bitmap from given items
    the bitmap is stored in RAM and not written to file
    @param document Inkscape document.
    @param area     Export area in document units.
    @param dpi      Resolution.
    @param items    Vector of pointers to SPItems to export. Export all items if empty.
    @param opaque   Set items opacity to 1 (used by Cairo renderer for filtered objects rendered as bitmaps).
    @return The created GdkPixbuf structure or nullptr if rendering failed.
*/
Inkscape::Pixbuf *sp_generate_internal_bitmap(SPDocument *document,
                                              Geom::Rect const &area,
                                              double dpi,
                                              std::vector<SPItem *> items,
                                              bool opaque)
{
    // Geometry
    if (area.hasZeroArea()) {
        return nullptr;
    }

    Geom::Point origin = area.min();
    double scale_factor = Inkscape::Util::Quantity::convert(dpi, "px", "in");
    Geom::Affine affine = Geom::Translate(-origin) * Geom::Scale (scale_factor, scale_factor);

    int width  = std::ceil(scale_factor * area.width());
    int height = std::ceil(scale_factor * area.height());

    // Document
    document->ensureUpToDate();
    unsigned dkey = SPItem::display_key_new(1);

    // Drawing
    Inkscape::Drawing drawing; // New drawing for offscreen rendering.
    drawing.setExact(true); // Maximum quality for blurs.

    /* Create ArenaItems and set transform */
    Inkscape::DrawingItem *root = document->getRoot()->invoke_show( drawing, dkey, SP_ITEM_SHOW_DISPLAY);
    root->setTransform(affine);
    drawing.setRoot(root);

    // Hide all items we don't want, instead of showing only requested items,
    // because that would not work if the shown item references something in defs.
    if (!items.empty()) {
        hide_other_items_recursively(document->getRoot(), items, dkey);
    }

    Geom::IntRect final_area = Geom::IntRect::from_xywh(0, 0, width, height);
    drawing.update(final_area);

    if (opaque) {
        // Required by sp_asbitmap_render().
        for (auto item : items) {
            if (item->get_arenaitem(dkey)) {
                item->get_arenaitem(dkey)->setOpacity(1.0);
            }
        }
    }

    // Rendering
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    Inkscape::Pixbuf* pixbuf = nullptr;

    if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
        Inkscape::DrawingContext dc(surface, Geom::Point(0,0));

        // render items
        drawing.render(dc, final_area, Inkscape::DrawingItem::RENDER_BYPASS_CACHE);

        pixbuf = new Inkscape::Pixbuf(surface);

    } else {

        long long size =
            (long long) height *
            (long long) cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
        g_warning("sp_generate_internal_bitmap: not enough memory to create pixel buffer. Need %lld.", size);
        cairo_surface_destroy(surface);
    }

    // Return to previous state.
    document->getRoot()->invoke_hide(dkey);

    return pixbuf;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
