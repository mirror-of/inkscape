#define __SP_RECT_C__

/*
 * SVG <rect> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <math.h>
#include <string.h>

#include <display/curve.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-div.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-fns.h>

#include "svg/svg.h"
#include "attributes.h"
#include "style.h"
#include "document.h"
#include "dialogs/object-attributes.h"
#include "sp-root.h"
#include "sp-shape.h"
#include "sp-rect.h"
#include "helper/sp-intl.h"
#include "xml/repr.h"

#define noRECT_VERBOSE

static void sp_rect_class_init(SPRectClass *klass);
static void sp_rect_init(SPRect *rect);

static void sp_rect_build(SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_rect_set(SPObject *object, unsigned key, gchar const *value);
static void sp_rect_update(SPObject *object, SPCtx *ctx, guint flags);
static SPRepr *sp_rect_write(SPObject *object, SPRepr *repr, guint flags);

static gchar *sp_rect_description(SPItem *item);
static NR::Matrix sp_rect_set_transform(SPItem *item, NR::Matrix const &xform);

static void sp_rect_set_shape(SPShape *shape);

static SPShapeClass *parent_class;

GType
sp_rect_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPRectClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_rect_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPRect),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_rect_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_SHAPE, "SPRect", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_rect_class_init(SPRectClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;
    SPShapeClass *shape_class = (SPShapeClass *) klass;

    parent_class = (SPShapeClass *)g_type_class_ref(SP_TYPE_SHAPE);

    sp_object_class->build = sp_rect_build;
    sp_object_class->write = sp_rect_write;
    sp_object_class->set = sp_rect_set;
    sp_object_class->update = sp_rect_update;

    item_class->description = sp_rect_description;
    item_class->set_transform = sp_rect_set_transform;

    shape_class->set_shape = sp_rect_set_shape;
}

static void
sp_rect_init(SPRect *rect)
{
    /* Initializing to zero is automatic */
    /* sp_svg_length_unset(&rect->x, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->y, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->width, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->height, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->rx, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->ry, SP_SVG_UNIT_NONE, 0.0, 0.0); */
}

static void
sp_rect_build(SPObject *object, SPDocument *document, SPRepr *repr)
{
    SPRect *rect = SP_RECT(object);

    if (((SPObjectClass *) parent_class)->build)
        ((SPObjectClass *) parent_class)->build(object, document, repr);

    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "width");
    sp_object_read_attr(object, "height");
    sp_object_read_attr(object, "rx");
    sp_object_read_attr(object, "ry");

    SPVersion const version = sp_object_get_sodipodi_version(object);

    if ( version.major == 0 && version.minor == 29 ) {
        if (rect->rx.set && rect->ry.set) {
            /* 0.29 treated 0.0 radius as missing value */
            if ((rect->rx.value != 0.0) && (rect->ry.value == 0.0)) {
                sp_repr_set_attr(repr, "ry", NULL);
                sp_object_read_attr(object, "ry");
            } else if ((rect->ry.value != 0.0) && (rect->rx.value == 0.0)) {
                sp_repr_set_attr(repr, "rx", NULL);
                sp_object_read_attr(object, "rx");
            }
        }
    }
}

static void
sp_rect_set(SPObject *object, unsigned key, gchar const *value)
{
    SPRect *rect = SP_RECT(object);

    /* fixme: We need real error processing some time */

    switch (key) {
        case SP_ATTR_X:
            if (!sp_svg_length_read(value, &rect->x)) {
                sp_svg_length_unset(&rect->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            if (!sp_svg_length_read(value, &rect->y)) {
                sp_svg_length_unset(&rect->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            if (!sp_svg_length_read(value, &rect->width) || (rect->width.value < 0.0)) {
                sp_svg_length_unset(&rect->width, SP_SVG_UNIT_NONE, 0.0, 0.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            if (!sp_svg_length_read(value, &rect->height) || (rect->height.value < 0.0)) {
                sp_svg_length_unset(&rect->height, SP_SVG_UNIT_NONE, 0.0, 0.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_RX:
            if (!sp_svg_length_read(value, &rect->rx) || (rect->rx.value < 0.0)) {
                sp_svg_length_unset(&rect->rx, SP_SVG_UNIT_NONE, 0.0, 0.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_RY:
            if (!sp_svg_length_read(value, &rect->ry) || (rect->ry.value < 0.0)) {
                sp_svg_length_unset(&rect->ry, SP_SVG_UNIT_NONE, 0.0, 0.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) parent_class)->set)
                ((SPObjectClass *) parent_class)->set(object, key, value);
            break;
    }
}

static void
sp_rect_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SPRect *rect = (SPRect *) object;
        SPStyle *style = object->style;
        SPItemCtx *ictx = (SPItemCtx *) ctx;
        double const d = 1.0 / NR_MATRIX_DF_EXPANSION(&ictx->i2vp);
        double const w = d * (ictx->vp.x1 - ictx->vp.x0);
        double const h = d * (ictx->vp.y1 - ictx->vp.y0);
        sp_svg_length_update(&rect->x, style->font_size.computed, style->font_size.computed * 0.5, w);
        sp_svg_length_update(&rect->y, style->font_size.computed, style->font_size.computed * 0.5, h);
        sp_svg_length_update(&rect->width, style->font_size.computed, style->font_size.computed * 0.5, w);
        sp_svg_length_update(&rect->height, style->font_size.computed, style->font_size.computed * 0.5, h);
        sp_svg_length_update(&rect->rx, style->font_size.computed, style->font_size.computed * 0.5, w);
        sp_svg_length_update(&rect->ry, style->font_size.computed, style->font_size.computed * 0.5, h);
        sp_shape_set_shape((SPShape *) object);
        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
    }

    if (((SPObjectClass *) parent_class)->update)
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
}

static SPRepr *
sp_rect_write(SPObject *object, SPRepr *repr, guint flags)
{
    SPRect *rect = SP_RECT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("rect");
    }

    sp_repr_set_double(repr, "width", rect->width.computed);
    sp_repr_set_double(repr, "height", rect->height.computed);
    if (rect->rx.set) sp_repr_set_double(repr, "rx", rect->rx.computed);
    if (rect->ry.set) sp_repr_set_double(repr, "ry", rect->ry.computed);
    sp_repr_set_double(repr, "x", rect->x.computed);
    sp_repr_set_double(repr, "y", rect->y.computed);

    if (((SPObjectClass *) parent_class)->write)
        ((SPObjectClass *) parent_class)->write(object, repr, flags);

    return repr;
}

static gchar *
sp_rect_description(SPItem *item)
{
    g_return_val_if_fail(SP_IS_RECT(item), NULL);

    return g_strdup(_("<b>Rectangle</b>"));
}

#define C1 0.554

static void
sp_rect_set_shape(SPShape *shape)
{
    SPRect *rect = (SPRect *) shape;

    if ((rect->height.computed < 1e-18) || (rect->width.computed < 1e-18)) return;

    SPCurve *c = sp_curve_new();

    double const x = rect->x.computed;
    double const y = rect->y.computed;
    double const w = rect->width.computed;
    double const h = rect->height.computed;
    double const w2 = w / 2;
    double const h2 = h / 2;
    double const rx = std::min(( rect->rx.set
                                 ? rect->rx.computed
                                 : ( rect->ry.set
                                     ? rect->ry.computed
                                     : 0.0 ) ),
                               .5 * rect->width.computed);
    double const ry = std::min(( rect->ry.set
                                 ? rect->ry.computed
                                 : ( rect->rx.set
                                     ? rect->rx.computed
                                     : 0.0 ) ),
                               .5 * rect->height.computed);
    /* TODO: Handle negative rx or ry as per
     * http://www.w3.org/TR/SVG11/shapes.html#RectElementRXAttribute once Inkscape has proper error
     * handling (see http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing).
     */

    /* We don't use proper circular/elliptical arcs, but bezier curves can approximate a 90-degree
     * arc fairly well.
     */
    if ((rx > 1e-18) && (ry > 1e-18)) {
        sp_curve_moveto(c, x + rx, y + 0.0);
        sp_curve_curveto(c, x + rx * (1 - C1), y + 0.0, x + 0.0, y + ry * (1 - C1), x + 0.0, y + ry);
        if (ry < h2) sp_curve_lineto(c, x + 0.0, y + h - ry);
        sp_curve_curveto(c, x + 0.0, y + h - ry * (1 - C1), x + rx * (1 - C1), y + h, x + rx, y + h);
        if (rx < w2) sp_curve_lineto(c, x + w - rx, y + h);
        sp_curve_curveto(c, x + w - rx * (1 - C1), y + h, x + w, y + h - ry * (1 - C1), x + w, y + h - ry);
        if (ry < h2) sp_curve_lineto(c, x + w, y + ry);
        sp_curve_curveto(c, x + w, y + ry * (1 - C1), x + w - rx * (1 - C1), y + 0.0, x + w - rx, y + 0.0);
        if (rx < w2) sp_curve_lineto(c, x + rx, y + 0.0);
    } else {
        sp_curve_moveto(c, x + 0.0, y + 0.0);
        sp_curve_lineto(c, x + 0.0, y + h);
        sp_curve_lineto(c, x + w, y + h);
        sp_curve_lineto(c, x + w, y + 0.0);
        sp_curve_lineto(c, x + 0.0, y + 0.0);
    }

    sp_curve_closepath_current(c);
    sp_shape_set_curve_insync(SP_SHAPE(rect), c, TRUE);
    sp_curve_unref(c);
}

/* fixme: Think (Lauris) */

void
sp_rect_position_set(SPRect *rect, gdouble x, gdouble y, gdouble width, gdouble height)
{
    g_return_if_fail(rect != NULL);
    g_return_if_fail(SP_IS_RECT(rect));

    rect->x.computed = x;
    rect->y.computed = y;
    rect->width.computed = width;
    rect->height.computed = height;

    SP_OBJECT(rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
sp_rect_set_rx(SPRect *rect, gboolean set, gdouble value)
{
    g_return_if_fail(rect != NULL);
    g_return_if_fail(SP_IS_RECT(rect));

    rect->rx.set = set;
    if (set) rect->rx.computed = value;

    SP_OBJECT(rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
sp_rect_set_ry(SPRect *rect, gboolean set, gdouble value)
{
    g_return_if_fail(rect != NULL);
    g_return_if_fail(SP_IS_RECT(rect));

    rect->ry.set = set;
    if (set) rect->ry.computed = value;

    SP_OBJECT(rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * Initially we'll do:
 * Transform x, y, set x, y, clear translation
 */

/* fixme: Use preferred units somehow (Lauris) */
/* fixme: Alternately preserve whatever units there are (lauris) */

static NR::Matrix
sp_rect_set_transform(SPItem *item, NR::Matrix const &xform)
{
    SPRect *rect = SP_RECT(item);

    /* Calculate rect start in parent coords. */
    NR::Point pos( NR::Point(rect->x.computed, rect->y.computed) * xform );

    /* This function takes care of translation and scaling, we return whatever parts we can't
       handle. */
    NR::Matrix ret(NR::transform(xform));
    gdouble const sw = hypot(ret[0], ret[1]);
    gdouble const sh = hypot(ret[2], ret[3]);
    if (sw > 1e-9) {
        ret[0] /= sw;
        ret[1] /= sw;
    } else {
        ret[0] = 1.0;
        ret[1] = 0.0;
    }
    if (sh > 1e-9) {
        ret[2] /= sh;
        ret[3] /= sh;
    } else {
        ret[2] = 0.0;
        ret[3] = 1.0;
    }

    /* fixme: Would be nice to preserve units here */
    rect->width = rect->width.computed * sw;
    rect->height = rect->height.computed * sh;
    if (rect->rx.set) {
        rect->rx = rect->rx.computed * sw;
    }
    if (rect->ry.set) {
        rect->ry = rect->ry.computed * sh;
    }

    /* Find start in item coords */
    pos = pos * ret.inverse();
    rect->x = pos[NR::X];
    rect->y = pos[NR::Y];

    sp_rect_set_shape(rect);

    // Adjust stroke width
    sp_shape_adjust_stroke(item, sqrt(fabs(sw * sh)));

    // Adjust pattern fill
    sp_shape_adjust_pattern(item, xform / ret);

    // Adjust gradient fill
    sp_shape_adjust_gradient(item, xform / ret);

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    return ret;
}


/**
Returns the ratio in which the vector from p0 to p1 is stretched by transform
 */
static gdouble
vector_stretch(NR::Point p0, NR::Point p1, NR::Matrix xform)
{
    if (p0 == p1)
        return 0;
    return (NR::distance(p0 * xform, p1 * xform) / NR::distance(p0, p1));
}

void
sp_rect_set_visible_rx(SPRect *rect, gdouble rx)
{
    if (rx == 0) {
        rect->rx.computed = 0;
        rect->rx.set = FALSE;
    } else {
        rect->rx.computed = rx / vector_stretch(
            NR::Point(rect->x.computed + 1, rect->y.computed),
            NR::Point(rect->x.computed, rect->y.computed),
            SP_ITEM(rect)->transform);
        rect->rx.set = TRUE;
    }
    SP_OBJECT(rect)->updateRepr();
}

void
sp_rect_set_visible_ry(SPRect *rect, gdouble ry)
{
    if (ry == 0) {
        rect->ry.computed = 0;
        rect->ry.set = FALSE;
    } else {
        rect->ry.computed = ry / vector_stretch(
            NR::Point(rect->x.computed, rect->y.computed + 1),
            NR::Point(rect->x.computed, rect->y.computed),
            SP_ITEM(rect)->transform);
        rect->ry.set = TRUE;
    }
    SP_OBJECT(rect)->updateRepr();
}

gdouble
sp_rect_get_visible_rx(SPRect *rect)
{
    if (!rect->rx.set)
        return 0;
    return rect->rx.computed * vector_stretch(
        NR::Point(rect->x.computed + 1, rect->y.computed),
        NR::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
}

gdouble
sp_rect_get_visible_ry(SPRect *rect)
{
    if (!rect->ry.set)
        return 0;
    return rect->ry.computed * vector_stretch(
        NR::Point(rect->x.computed, rect->y.computed + 1),
        NR::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
}

void
sp_rect_compensate_rxry(SPRect *rect, NR::Matrix xform)
{
    if (rect->rx.computed == 0 && rect->ry.computed == 0)
        return; // nothing to compensate

    // test unit vectors to find out compensation:
    NR::Point c(rect->x.computed, rect->y.computed);
    NR::Point cx = c + NR::Point(1, 0);
    NR::Point cy = c + NR::Point(0, 1);

    // apply previous transform if any
    c *= SP_ITEM(rect)->transform;
    cx *= SP_ITEM(rect)->transform;
    cy *= SP_ITEM(rect)->transform;

    // find out stretches that we need to compensate
    gdouble eX = vector_stretch(cx, c, xform);
    gdouble eY = vector_stretch(cy, c, xform);

    // If only one of the radii is set, set both radii so they have the same visible length
    // This is needed because if we just set them the same length in SVG, they might end up unequal because of transform
    if ((rect->rx.set && !rect->ry.set) || (rect->ry.set && !rect->rx.set)) {
        gdouble r = MAX(rect->rx.computed, rect->ry.computed);
        rect->rx.computed = r / eX;
        rect->ry.computed = r / eY;
    } else {
        rect->rx.computed = rect->rx.computed / eX;
        rect->ry.computed = rect->ry.computed / eY;
    }

    // Note that a radius may end up larger than half-side if the rect is scaled down;
    // that's ok because this preserves the intended radii in case the rect is enlarged again,
    // and set_shape will take care of trimming too large radii when generating d=

    rect->rx.set = rect->ry.set = TRUE;
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
