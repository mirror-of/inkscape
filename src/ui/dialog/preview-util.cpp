// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Utility functions for previewing icon representation.
 */
/* Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2005,2010 Jon A. Cruz
 * Copyright (C) 2021 Anshudhar Kumar Singh
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "preview-util.h"

#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"
#include "page-manager.h"

namespace Inkscape {
namespace UI {
namespace PREVIEW {

guchar *sp_icon_doc_icon(SPDocument *doc, Inkscape::Drawing &drawing, gchar const *name, unsigned psize,
                         unsigned &stride, Geom::OptRect *dboxIn)
{
    // If no doc then return nullptr. Beyond this point we assume there is valid doc
    if (!doc) {
        return nullptr;
    }

    SPObject *object = nullptr;

    if (name) {
        // If name is provided and valid, object will not be nullptr beyond this
        object = doc->getObjectById(name);
    }

    bool const dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg");
    guchar *px = nullptr;

    Geom::OptRect dbox;
    if (dboxIn) {
        // We have dbox from function call and hence will use it
        dbox = *dboxIn;

    } else if (object && SP_ITEM(object)) {
        // Else we will check if object is provided and will use its bounding box if its doesnt have parent.
        // Dont know why we check for parent and then use page coordinates. Will probably change it to document after
        // discussing
        if (object->parent == nullptr) {
            dbox =
                Geom::Rect(Geom::Point(0, 0), Geom::Point(doc->getWidth().value("px"), doc->getHeight().value("px")));
        } else {
            SPItem *item = SP_ITEM(object);
            dbox = item->documentVisualBounds();
        }
    } else if (doc->getRoot()) {
        // If we still dont have a dbox we will use document coordinates.
        dbox = doc->getRoot()->documentVisualBounds();
    }

    if (!dbox) {
        // If we still dont have anything to render then return nullptr
        return nullptr;
    }

    /* This is in document coordinates, i.e. pixels */

    /* Update to renderable state */
    double sf = 1.0;
    drawing.root()->setTransform(Geom::Scale(sf));
    drawing.update();
    /* Item integer bbox in points */
    // NOTE: previously, each rect coordinate was rounded using floor(c + 0.5)
    Geom::IntRect ibox = dbox->roundOutwards();

    if (dump) {
        g_message("   box    --'%s'  (%f,%f)-(%f,%f)", name, (double)ibox.left(), (double)ibox.top(),
                  (double)ibox.right(), (double)ibox.bottom());
    }

    /* Find button visible area */
    int width = ibox.width();
    int height = ibox.height();

    if (dump) {
        g_message("   vis    --'%s'  (%d,%d)", name, width, height);
    }

    {
        int block = std::max(width, height);
        if (block != static_cast<int>(psize)) {
            if (dump) {
                g_message("      resizing");
            }
            sf = (double)psize / (double)block;

            drawing.root()->setTransform(Geom::Scale(sf));
            drawing.update();

            auto scaled_box = *dbox * Geom::Scale(sf);
            ibox = scaled_box.roundOutwards();
            if (dump) {
                g_message("   box2   --'%s'  (%f,%f)-(%f,%f)", name, (double)ibox.left(), (double)ibox.top(),
                          (double)ibox.right(), (double)ibox.bottom());
            }

            /* Find button visible area */
            width = ibox.width();
            height = ibox.height();
            if (dump) {
                g_message("   vis2   --'%s'  (%d,%d)", name, width, height);
            }
        }
    }

    Geom::IntPoint pdim(psize, psize);
    int dx, dy;
    // dx = (psize - width) / 2;
    // dy = (psize - height) / 2;
    dx = dy = psize;
    // watch out for psize, since 'unsigned'-'signed' can cause problems if the result is negative
    dx = (dx - width) / 2;
    dy = (dy - height) / 2;
    Geom::IntRect area = Geom::IntRect::from_xywh(ibox.min() - Geom::IntPoint(dx, dy), pdim);

    /* Actual renderable area */
    Geom::IntRect ua = *Geom::intersect(ibox, area);

    if (dump) {
        g_message("   area   --'%s'  (%f,%f)-(%f,%f)", name, (double)area.left(), (double)area.top(),
                  (double)area.right(), (double)area.bottom());
        g_message("   ua     --'%s'  (%f,%f)-(%f,%f)", name, (double)ua.left(), (double)ua.top(), (double)ua.right(),
                  (double)ua.bottom());
    }

    stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, psize);

    /* Set up pixblock */
    px = g_new(guchar, stride * psize);
    memset(px, 0x00, stride * psize);

    /* Render */
    cairo_surface_t *s = cairo_image_surface_create_for_data(px, CAIRO_FORMAT_ARGB32, psize, psize, stride);
    Inkscape::DrawingContext dc(s, ua.min());

    // In the future, this could be a different color for each page.
    guint32 default_bg = doc->getNamedView()->getPageManager()->background_color;

    float bg_r = SP_RGBA32_R_F(default_bg);
    float bg_g = SP_RGBA32_G_F(default_bg);
    float bg_b = SP_RGBA32_B_F(default_bg);
    float bg_a = SP_RGBA32_A_F(default_bg);

    cairo_t *cr = cairo_create(s);
    cairo_set_source_rgba(cr, bg_r, bg_g, bg_b, bg_a);
    cairo_rectangle(cr, 0, 0, psize, psize);
    cairo_fill(cr);
    cairo_save(cr);
    cairo_destroy(cr);

    drawing.render(dc, ua);
    cairo_surface_destroy(s);

    // convert to GdkPixbuf format
    convert_pixels_argb32_to_pixbuf(px, psize, psize, stride);

    if (Inkscape::Preferences::get()->getBool("/debug/icons/overlaySvg")) {
        overlayPixels(px, psize, psize, stride, 0x00, 0x00, 0xff);
    }

    return px;
} // end of sp_icon_doc_icon()

void overlayPixels(guchar *px, int width, int height, int stride, unsigned r, unsigned g, unsigned b)
{
    int bytesPerPixel = 4;
    int spacing = 4;
    for (int y = 0; y < height; y += spacing) {
        guchar *ptr = px + y * stride;
        for (int x = 0; x < width; x += spacing) {
            *(ptr++) = r;
            *(ptr++) = g;
            *(ptr++) = b;
            *(ptr++) = 0xff;

            ptr += bytesPerPixel * (spacing - 1);
        }
    }

    if (width > 1 && height > 1) {
        // point at the last pixel
        guchar *ptr = px + ((height - 1) * stride) + ((width - 1) * bytesPerPixel);

        if (width > 2) {
            px[4] = r;
            px[5] = g;
            px[6] = b;
            px[7] = 0xff;

            ptr[-12] = r;
            ptr[-11] = g;
            ptr[-10] = b;
            ptr[-9] = 0xff;
        }

        ptr[-4] = r;
        ptr[-3] = g;
        ptr[-2] = b;
        ptr[-1] = 0xff;

        px[0 + stride] = r;
        px[1 + stride] = g;
        px[2 + stride] = b;
        px[3 + stride] = 0xff;

        ptr[0 - stride] = r;
        ptr[1 - stride] = g;
        ptr[2 - stride] = b;
        ptr[3 - stride] = 0xff;

        if (height > 2) {
            ptr[0 - stride * 3] = r;
            ptr[1 - stride * 3] = g;
            ptr[2 - stride * 3] = b;
            ptr[3 - stride * 3] = 0xff;
        }
    }
}

} // namespace PREVIEW
} // namespace UI
} // namespace Inkscape
