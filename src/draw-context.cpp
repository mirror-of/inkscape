#define __SP_DRAW_CONTEXT_C__

/*
 * Generic drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define DRAW_VERBOSE

#include <config.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include "macros.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "helper/sp-intl.h"
#include "display/curve.h"
#include "display/bezier-utils.h"
#include "display/sodipodi-ctrl.h"
#include "display/sp-ctrlline.h"
#include "display/canvas-bpath.h"

#include "prefs-utils.h"
#include "enums.h"
#include "inkscape.h"
#include "document.h"
#include "sp-path.h"
#include "selection.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "style.h"
#include "draw-context.h"

#include <libnr/nr-point-fns.h>

#define SPDC_EVENT_MASK ( GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK )

/* Drawing anchors */

struct SPDrawAnchor {
    SPDrawContext *dc;
    SPCurve *curve;
    guint start : 1;
    guint active : 1;
    NR::Point dp;
    NR::Point wp;
    SPCanvasItem *ctrl;
};

static void sp_draw_context_class_init(SPDrawContextClass *klass);
static void sp_draw_context_init(SPDrawContext *dc);
static void sp_draw_context_dispose(GObject *object);

static void sp_draw_context_setup(SPEventContext *ec);
static void sp_draw_context_set(SPEventContext *ec, gchar const *key, gchar const *value);
static void sp_draw_context_finish(SPEventContext *ec);

static gint sp_draw_context_root_handler(SPEventContext *event_context, GdkEvent *event);

static void spdc_set_attach(SPDrawContext *dc, gboolean attach);

static void spdc_selection_changed(SPSelection *sel, SPDrawContext *dc);
static void spdc_selection_modified(SPSelection *sel, guint flags, SPDrawContext *dc);

static void spdc_attach_selection(SPDrawContext *dc, SPSelection *sel);
static void spdc_detach_selection(SPDrawContext *dc, SPSelection *sel);

static void spdc_concat_colors_and_flush(SPDrawContext *dc, gboolean forceclosed);
static void spdc_flush_white(SPDrawContext *dc, SPCurve *gc);

static void spdc_reset_colors(SPDrawContext *dc);
static void spdc_reset_white(SPDrawContext *dc);
static void spdc_free_colors(SPDrawContext *dc);

static SPDrawAnchor *test_inside(SPDrawContext *dc, NR::Point p);
static SPDrawAnchor *sp_draw_anchor_test(SPDrawAnchor *anchor, NR::Point w, gboolean activate);

static void fit_and_split(SPDrawContext *dc);

static SPDrawAnchor *sp_draw_anchor_new(SPDrawContext *dc, SPCurve *curve, gboolean start, NR::Point p);
static SPDrawAnchor *sp_draw_anchor_destroy(SPDrawAnchor *anchor);

static SPEventContextClass *draw_parent_class;

GtkType
sp_draw_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPDrawContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_draw_context_class_init,
            NULL, NULL,
            sizeof(SPDrawContext),
            4,
            (GInstanceInitFunc) sp_draw_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPDrawContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_draw_context_class_init(SPDrawContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *ec_class;

    object_class = (GObjectClass *)klass;
    ec_class = (SPEventContextClass *) klass;

    draw_parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_draw_context_dispose;

    ec_class->setup = sp_draw_context_setup;
    ec_class->set = sp_draw_context_set;
    ec_class->finish = sp_draw_context_finish;
    ec_class->root_handler = sp_draw_context_root_handler;
}

static void
sp_draw_context_init(SPDrawContext *dc)
{
    dc->attach = FALSE;

    dc->red_color = 0xff00007f;
    dc->blue_color = 0x0000ff7f;
    dc->green_color = 0x00ff007f;

    dc->npoints = 0;
    dc->red_curve_is_valid=0x00;

    new (&dc->sel_changed_connection) SigC::Connection();
    new (&dc->sel_modified_connection) SigC::Connection();
}

static void
sp_draw_context_dispose(GObject *object)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(object);

    dc->sel_changed_connection.~Connection();
    dc->sel_modified_connection.~Connection();

    if (dc->grab) {
        sp_canvas_item_ungrab(dc->grab, GDK_CURRENT_TIME);
        dc->grab = NULL;
    }

    if (dc->selection) {
        dc->selection = NULL;
    }

    spdc_free_colors(dc);

    G_OBJECT_CLASS(draw_parent_class)->dispose(object);
}

static void
sp_draw_context_setup(SPEventContext *ec)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);
    SPDesktop *dt = ec->desktop;

    if (((SPEventContextClass *) draw_parent_class)->setup) {
        ((SPEventContextClass *) draw_parent_class)->setup(ec);
    }

    dc->selection = SP_DT_SELECTION(dt);

    /* Connect signals to track selection changes */
    dc->sel_changed_connection = dc->selection->connectChanged(
        SigC::bind(SigC::slot(&spdc_selection_changed), dc)
    );
    dc->sel_modified_connection = dc->selection->connectModified(
        SigC::bind(SigC::slot(&spdc_selection_modified), dc)
    );

    /* Create red bpath */
    dc->red_bpath = sp_canvas_bpath_new(SP_DT_SKETCH(ec->desktop), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->red_bpath), dc->red_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* Create red curve */
    dc->red_curve = sp_curve_new_sized(4);

    /* Create blue bpath */
    dc->blue_bpath = sp_canvas_bpath_new(SP_DT_SKETCH(ec->desktop), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->blue_bpath), dc->blue_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    /* Create blue curve */
    dc->blue_curve = sp_curve_new_sized(8);

    /* Create green curve */
    dc->green_curve = sp_curve_new_sized(64);
    /* No green anchor by default */
    dc->green_anchor = NULL;

    spdc_set_attach(dc, FALSE);
}

static void
sp_draw_context_finish(SPEventContext *ec)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);

    dc->sel_changed_connection.disconnect();
    dc->sel_modified_connection.disconnect();

    if (dc->grab) {
        sp_canvas_item_ungrab(dc->grab, GDK_CURRENT_TIME);
    }

    if (dc->selection) {
        dc->selection = NULL;
    }

    spdc_free_colors(dc);
}

static void
sp_draw_context_set(SPEventContext *ec, const gchar *key, const gchar *value)
{
}

gint
sp_draw_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
    case GDK_KEY_PRESS:
        /* fixme: */
        switch (event->key.keyval) {
        case GDK_A:
        case GDK_a:
            if (dc->attach) {
                spdc_set_attach(dc, FALSE);
            } else {
                spdc_set_attach(dc, TRUE);
            }
            ret = TRUE;
            break;
        case GDK_Up: 
        case GDK_Down: 
        case GDK_KP_Up: 
        case GDK_KP_Down: 
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY) {
                ret = TRUE;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) draw_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) draw_parent_class)->root_handler(ec, event);
        }
    }

    return ret;
}

static void
spdc_set_attach(SPDrawContext *dc, gboolean attach)
{
    if (attach) {
        dc->attach = TRUE;
        spdc_attach_selection(dc, dc->selection);
        sp_view_set_statusf_flash(SP_VIEW(SP_EVENT_CONTEXT_DESKTOP(dc)),
                                  _("Appending to selection"));
    } else {
        dc->attach = FALSE;
        spdc_detach_selection(dc, dc->selection);
        sp_view_set_statusf_flash(SP_VIEW(SP_EVENT_CONTEXT_DESKTOP(dc)),
                                  _("Creating new curve"), FALSE);
    }
}

/*
 * Selection handlers
 */

static void
spdc_selection_changed(SPSelection *sel, SPDrawContext *dc)
{
    if (dc->attach) {
        spdc_attach_selection(dc, sel);
    }
}

/* fixme: We have to ensure this is not delayed (Lauris) */

static void
spdc_selection_modified(SPSelection *sel, guint flags, SPDrawContext *dc)
{
    if (dc->attach) {
        spdc_attach_selection(dc, sel);
    }
}

static void
spdc_attach_selection(SPDrawContext *dc, SPSelection *sel)
{
    /* We reset white and forget white/start/end anchors */
    spdc_reset_white(dc);
    dc->sa = NULL;
    dc->ea = NULL;

    SPItem *item = sp_selection_item(dc->selection);

    if ( item && SP_IS_PATH(item) ) {
        NRMatrix i2dt;
        /* Create new white data */
        /* Item */
        dc->white_item = item;
        /* Curve list */
        /* We keep it in desktop coordinates to eliminate calculation errors */
        SPCurve *norm = sp_shape_get_curve(SP_SHAPE(item));
        sp_item_i2d_affine(dc->white_item, &i2dt);
        norm = sp_curve_transform(norm, NR_MATRIX_D_TO_DOUBLE(&i2dt));
        g_return_if_fail( norm != NULL );
        dc->white_curves = sp_curve_split(norm);
        sp_curve_unref(norm);
        /* Anchor list */
        for (GSList *l = dc->white_curves; l != NULL; l = l->next) {
            SPCurve *c;
            c = (SPCurve*)l->data;
            g_return_if_fail( c->end > 1 );
            if ( c->bpath->code == NR_MOVETO_OPEN ) {
                NArtBpath *s, *e;
                SPDrawAnchor *a;
                s = sp_curve_first_bpath(c);
                e = sp_curve_last_bpath(c);
                a = sp_draw_anchor_new(dc, c, TRUE, NR::Point(s->x3, s->y3));
                dc->white_anchors = g_slist_prepend(dc->white_anchors, a);
                a = sp_draw_anchor_new(dc, c, FALSE, NR::Point(e->x3, e->y3));
                dc->white_anchors = g_slist_prepend(dc->white_anchors, a);
            }
        }
        /* fixme: recalculate active anchor? */
    }
}

static void
spdc_detach_selection(SPDrawContext *dc, SPSelection *sel)
{
    /* We reset white and forget white/start/end anchors */
    spdc_reset_white(dc);
    dc->sa = NULL;
    dc->ea = NULL;
}

/**
\brief  Snaps node or handle to PI/rotationsnapsperpi degree increments
\param dc  draw context
\param p  cursor point (to be changed by snapping)
\param o  origin point 
\param state  keyboard state to check if ctrl was pressed
*/
static void
spdc_endpoint_snap_internal(SPDrawContext *dc, NR::Point &p, NR::Point const o, guint state)
{
    if ( state & GDK_CONTROL_MASK ) {
        /* Constrained motion */

        unsigned snaps = abs(prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12));
        /* 0 means no snapping. */

        /* mirrored by fabs, so this corresponds to 15 degrees */
        NR::Point best; /* best solution */
        double bn = 1e18; /* best normal */
        double bdot = 0;
        NR::Point v = NR::Point(0, 1);
        double const r00 = cos( M_PI / snaps ), r01 = sin( M_PI / snaps );
        double const r10 = -r01, r11 = r00;

        NR::Point delta = p - o;

        for (unsigned i = 0; i < snaps; i++) {
            double const ndot = fabs(dot(v,NR::rot90(delta)));
            NR::Point t(r00*v[NR::X] + r01*v[NR::Y],
                        r10*v[NR::X] + r11*v[NR::Y]);
            if ( ndot < bn ) { 
                /* I think it is better numerically to use the normal, rather than the dot product
                 * to assess solutions, but I haven't proven it. */
                bn = ndot;
                best = v;
                bdot = dot(v, delta);
            }
            v = t;
        }

        if ( fabs(bdot) > 0 ) {
            p = o + bdot * best;

            /* Snap it along best vector */
            sp_desktop_vector_snap(SP_EVENT_CONTEXT_DESKTOP(dc), p, best);
        }
    } else {
        /* Free */
        sp_desktop_free_snap(SP_EVENT_CONTEXT_DESKTOP(dc), p);
    }
}

/**
\brief  Snaps new node relative to the previous node
*/
static void
spdc_endpoint_snap(SPDrawContext *dc, NR::Point &p, guint state)
{
    spdc_endpoint_snap_internal(dc, p, dc->p[0], state);
}

/**
\brief  Snaps new node's handle relative to the new node
*/
static void
spdc_endpoint_snap_handle(SPDrawContext *dc, NR::Point &p, guint state)
{
    if ( dc->npoints == 5 ) {
        spdc_endpoint_snap_internal(dc, p, dc->p[3], state);
    } else {
        spdc_endpoint_snap_internal(dc, p, dc->p[0], state);
    }
}


/*
 * Concats red, blue and green
 * If any anchors are defined, process these, optionally removing curves from white list
 * Invoke _flush_white to write result back to object
 *
 */

static void
spdc_concat_colors_and_flush(SPDrawContext *dc, gboolean forceclosed)
{
    /* Concat RBG */
    SPCurve *c = dc->green_curve;

    /* Green */
    dc->green_curve = sp_curve_new_sized(64);
    while (dc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }
    /* Blue */
    sp_curve_append_continuous(c, dc->blue_curve, 0.0625);
    sp_curve_reset(dc->blue_curve);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->blue_bpath), NULL);
    /* Red */
    if ( dc->red_curve_is_valid ) {
        sp_curve_append_continuous(c, dc->red_curve, 0.0625);
    }
    sp_curve_reset(dc->red_curve);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);

    /* Step A - test, whether we ended on green anchor */
    if ( forceclosed || ( dc->green_anchor && dc->green_anchor->active ) ) {
        // We hit green anchor, closing Green-Blue-Red
        sp_view_set_statusf_flash(SP_VIEW(SP_EVENT_CONTEXT_DESKTOP(dc)), "Path is closed.");
        sp_curve_closepath_current(c);
        /* Closed path, just flush */
        spdc_flush_white(dc, c);
        sp_curve_unref(c);
        return;
    }

    /* Step B - both start and end anchored to same curve */
    if ( dc->sa && dc->ea && ( dc->sa->curve == dc->ea->curve ) ) {
      bool doItLad=true;
      if ( dc->sa == dc->ea ) {
        doItLad=false;
        // ouch: anchored to same point
        if ( dc->sa->curve->closed ) {
          doItLad=true;
        } else {
        }
      }
      if ( doItLad ) {
        // We hit bot start and end of single curve, closing paths
          sp_view_set_statusf_flash(SP_VIEW(SP_EVENT_CONTEXT_DESKTOP(dc)), "Closing path.");
        if (dc->sa->start && !(dc->sa->curve->closed) ) {
            SPCurve *r;
            // Reversing curve
            r = sp_curve_reverse(c);
            sp_curve_unref(c);
            c = r;
        }
        sp_curve_append_continuous(dc->sa->curve, c, 0.0625);
        sp_curve_unref(c);
        sp_curve_closepath_current(dc->sa->curve);
        spdc_flush_white(dc, NULL);
        return;
       }
     }

    /* Step C - test start */
    if (dc->sa) {
        SPCurve *s;
        g_print("Curve start hit anchor\n");
        s = dc->sa->curve;
        dc->white_curves = g_slist_remove(dc->white_curves, s);
        if (dc->sa->start) {
            SPCurve *r;
            g_print("Reversing curve\n");
            r = sp_curve_reverse(s);
            sp_curve_unref(s);
            s = r;
        }
        sp_curve_append_continuous(s, c, 0.0625);
        sp_curve_unref(c);
        c = s;
    } else /* Step D - test end */ if (dc->ea) {
        SPCurve *e;
        g_print("Curve end hit anchor\n");
        e = dc->ea->curve;
        dc->white_curves = g_slist_remove(dc->white_curves, e);
        if (!dc->ea->start) {
            SPCurve *r;
            g_print("Reversing curve\n");
            r = sp_curve_reverse(e);
            sp_curve_unref(e);
            e = r;
        }
        sp_curve_append_continuous(c, e, 0.0625);
        sp_curve_unref(e);
    }


    spdc_flush_white(dc, c);

    sp_curve_unref(c);
}

/*
 * Flushes white curve(s) and additional curve into object
 *
 * No cleaning of colored curves - this has to be done by caller
 * No rereading of white data, so if you cannot rely on ::modified, do it in caller
 *
 */

static void
spdc_flush_white(SPDrawContext *dc, SPCurve *gc)
{
    SPCurve *c;

    if (dc->white_curves) {
        g_assert(dc->white_item);
        c = sp_curve_concat(dc->white_curves);
        g_slist_free(dc->white_curves);
        dc->white_curves = NULL;
        if (gc) {
            sp_curve_append(c, gc, FALSE);
        }
    } else if (gc) {
        c = gc;
        sp_curve_ref(c);
    } else {
        return;
    }

    /* Now we have to go back to item coordinates at last */
    if (dc->white_item) {
        NRMatrix d2item;
        sp_item_dt2i_affine(dc->white_item, SP_EVENT_CONTEXT_DESKTOP(dc), &d2item);
        c = sp_curve_transform(c, NR_MATRIX_D_TO_DOUBLE(&d2item));
    } else {
        gdouble d2item[6];
        sp_desktop_dt2root_affine(SP_EVENT_CONTEXT_DESKTOP(dc), (NRMatrix *) d2item);
        c = sp_curve_transform(c, d2item);
    }

    if ( c && !sp_curve_empty(c) ) {
        SPDesktop *dt;
        SPDocument *doc;
        SPRepr *repr;
        gchar *str;

        /* We actually have something to write */

        dt = SP_EVENT_CONTEXT_DESKTOP(dc);
        doc = SP_DT_DOCUMENT(dt);

        if (dc->white_item) {
            repr = SP_OBJECT_REPR(dc->white_item);
        } else {
            SPRepr *style;
            repr = sp_repr_new("path");
            /* fixme: Pen and pencil need separate style (Lauris) */
            style = inkscape_get_repr(INKSCAPE, "tools.freehand");
            if (style) {
                SPCSSAttr *css;
                css = sp_repr_css_attr_inherited(style, "style");
                sp_repr_css_set(repr, css, "style");
                sp_repr_css_attr_unref(css);
            }
        }

        str = sp_svg_write_path(SP_CURVE_BPATH(c));
        g_assert( str != NULL );
        sp_repr_set_attr(repr, "d", str);
        g_free(str);

        if (!dc->white_item) {
            /* Attach repr */
            sp_document_add_repr(SP_DT_DOCUMENT(dt), repr);
            sp_selection_set_repr(dc->selection, repr);
            sp_repr_unref(repr);
        }

        sp_document_done(doc);
    }

    sp_curve_unref(c);

    /* Flush pending updates */
    sp_document_ensure_up_to_date(SP_DT_DOCUMENT(SP_EVENT_CONTEXT_DESKTOP(dc)));
}

/*
 * Returns FIRST active anchor (the activated one)
 */

static SPDrawAnchor *test_inside(SPDrawContext *dc, NR::Point p)
{
    SPDrawAnchor *active = NULL;

    /* Test green anchor */
    if (dc->green_anchor) {
        active = sp_draw_anchor_test(dc->green_anchor, p, TRUE);
    }

    for (GSList *l = dc->white_anchors; l != NULL; l = l->next) {
        SPDrawAnchor *na = sp_draw_anchor_test((SPDrawAnchor *) l->data, p, !active);
        if ( !active && na ) {
            active = na;
        }
    }

    return active;
}

static inline double square(double const x)
{
    return x * x;
}

static void fit_and_split(SPDrawContext *dc)
{
    g_assert( dc->npoints > 1 );

    double const tolerance_sq = square( NR::expansion(SP_EVENT_CONTEXT(dc)->desktop->w2d)
                                        * prefs_get_double_attribute_limited("tools.freehand.pencil",
                                                                             "tolerance", 10.0, 1.0, 100.0) );

    NR::Point b[4];
    int const n_segs = sp_bezier_fit_cubic(b, dc->p, dc->npoints, tolerance_sq);
    if ( n_segs > 0
         && dc->npoints < SP_DRAW_POINTS_MAX )
    {
        /* Fit and draw and reset state */
        sp_curve_reset(dc->red_curve);
        sp_curve_moveto(dc->red_curve, b[0]);
        sp_curve_curveto(dc->red_curve, b[1], b[2], b[3]);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), dc->red_curve);
        dc->red_curve_is_valid=0x01;
    } else {
        /* Fit and draw and copy last point */

        /* todo: This isn't what we want.  We really want to do the curve fit with say a 50%
         * overlap with the previous points, but such that the curve matches the tangent at the
         * exact meeting point.  Probably we want 2nd order continuity. */

        g_assert(!sp_curve_empty(dc->red_curve));
        sp_curve_append_continuous(dc->green_curve, dc->red_curve, 0.0625);
        SPCurve *curve = sp_curve_copy(dc->red_curve);

        /* fixme: */
        SPCanvasItem *cshape = sp_canvas_bpath_new(SP_DT_SKETCH(SP_EVENT_CONTEXT(dc)->desktop), curve);
        sp_curve_unref(curve);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), dc->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

        dc->green_bpaths = g_slist_prepend(dc->green_bpaths, cshape);

        int const continuity = std::min(2, dc->npoints);
        for (int i = 0; i < continuity; i++) {
            dc->p[i] = dc->p[dc->npoints - continuity + i];
        }
        dc->npoints = continuity;
        dc->red_curve_is_valid=0x00;
    }
}

static void
spdc_reset_colors(SPDrawContext *dc)
{
    /* Red */
    sp_curve_reset(dc->red_curve);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);
    /* Blue */
    sp_curve_reset(dc->blue_curve);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->blue_bpath), NULL);
    /* Green */
    while (dc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }
    sp_curve_reset(dc->green_curve);
    if (dc->green_anchor) {
        dc->green_anchor = sp_draw_anchor_destroy(dc->green_anchor);
    }
    dc->sa = NULL;
    dc->ea = NULL;
    dc->npoints = 0;
    dc->red_curve_is_valid=0x00;
}

static void
spdc_reset_white(SPDrawContext *dc)
{
    if (dc->white_item) {
        /* We do not hold refcount */
        dc->white_item = NULL;
    }
    while (dc->white_curves) {
        sp_curve_unref((SPCurve *) dc->white_curves->data);
        dc->white_curves = g_slist_remove(dc->white_curves, dc->white_curves->data);
    }
    while (dc->white_anchors) {
        sp_draw_anchor_destroy((SPDrawAnchor *) dc->white_anchors->data);
        dc->white_anchors = g_slist_remove(dc->white_anchors, dc->white_anchors->data);
    }
}

static void
spdc_free_colors(SPDrawContext *dc)
{
    /* Red */
    if (dc->red_bpath) {
        gtk_object_destroy(GTK_OBJECT(dc->red_bpath));
        dc->red_bpath = NULL;
    }
    if (dc->red_curve) {
        dc->red_curve = sp_curve_unref(dc->red_curve);
    }
    /* Blue */
    if (dc->blue_bpath) {
        gtk_object_destroy(GTK_OBJECT(dc->blue_bpath));
        dc->blue_bpath = NULL;
    }
    if (dc->blue_curve) {
        dc->blue_curve = sp_curve_unref(dc->blue_curve);
    }
    /* Green */
    while (dc->green_bpaths) {
        gtk_object_destroy(GTK_OBJECT(dc->green_bpaths->data));
        dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
    }
    if (dc->green_curve) {
        dc->green_curve = sp_curve_unref(dc->green_curve);
    }
    if (dc->green_anchor) {
        dc->green_anchor = sp_draw_anchor_destroy(dc->green_anchor);
    }
    /* White */
    if (dc->white_item) {
        /* We do not hold refcount */
        dc->white_item = NULL;
    }
    while (dc->white_curves) {
        sp_curve_unref((SPCurve *) dc->white_curves->data);
        dc->white_curves = g_slist_remove(dc->white_curves, dc->white_curves->data);
    }
    while (dc->white_anchors) {
        sp_draw_anchor_destroy((SPDrawAnchor *) dc->white_anchors->data);
        dc->white_anchors = g_slist_remove(dc->white_anchors, dc->white_anchors->data);
    }
}

/*
 * Anchors
 */

static SPDrawAnchor *
sp_draw_anchor_new(SPDrawContext *dc, SPCurve *curve, gboolean start, NR::Point delta)
{
    SPDesktop *dt=SP_EVENT_CONTEXT_DESKTOP(SP_EVENT_CONTEXT(dc));

    sp_view_set_statusf_flash(SP_VIEW(dt), _("Creating anchor at (%g,%g)"), delta[NR::X], delta[NR::Y]);

    SPDrawAnchor *a = g_new(SPDrawAnchor, 1);

    a->dc = dc;
    a->curve = curve;
    a->start = start;
    a->active = FALSE;
    a->dp = delta;
    a->wp = sp_desktop_d2w_xy_point(dt, delta);
    a->ctrl = sp_canvas_item_new(SP_DT_CONTROLS(dt), SP_TYPE_CTRL,
                                 "size", 4.0,
                                 "filled", 0,
                                 "fill_color", 0xff00007f,
                                 "stroked", 1,
                                 "stroke_color", 0x000000ff,
                                 NULL);

    sp_ctrl_moveto(SP_CTRL(a->ctrl), delta);

    return a;
}

static SPDrawAnchor *
sp_draw_anchor_destroy(SPDrawAnchor *anchor)
{
    if (anchor->ctrl) {
        gtk_object_destroy(GTK_OBJECT(anchor->ctrl));
    }
    g_free(anchor);
    return NULL;
}

#define A_SNAP 4.0

static SPDrawAnchor *
sp_draw_anchor_test(SPDrawAnchor *anchor, NR::Point w, gboolean activate)
{
    if ( activate && ( NR::LInfty( w - anchor->wp ) <= A_SNAP ) ) {
        if (!anchor->active) {
            sp_canvas_item_set((GtkObject *) anchor->ctrl, "filled", TRUE, NULL);
            anchor->active = TRUE;
        }
        return anchor;
    }

    if (anchor->active) {
        sp_canvas_item_set((GtkObject *) anchor->ctrl, "filled", FALSE, NULL);
        anchor->active = FALSE;
    }
    return NULL;
}

/* Pencil context */

static void sp_pencil_context_class_init(SPPencilContextClass *klass);
static void sp_pencil_context_init(SPPencilContext *dc);
static void sp_pencil_context_dispose(GObject *object);

static gint sp_pencil_context_root_handler(SPEventContext *event_context, GdkEvent *event);

static void spdc_set_startpoint(SPPencilContext *dc, NR::Point p, guint state);
static void spdc_set_endpoint(SPPencilContext *dc, NR::Point p, guint state);
static void spdc_finish_endpoint(SPPencilContext *dc, NR::Point p, gboolean snap, guint state);
static void spdc_add_freehand_point(SPPencilContext *dc, NR::Point p, guint state);

static SPDrawContextClass *pencil_parent_class;

GtkType
sp_pencil_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPPencilContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_pencil_context_class_init,
            NULL, NULL,
            sizeof(SPPencilContext),
            4,
            (GInstanceInitFunc) sp_pencil_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_DRAW_CONTEXT, "SPPencilContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_pencil_context_class_init(SPPencilContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *event_context_class;

    object_class = (GObjectClass *) klass;
    event_context_class = (SPEventContextClass *) klass;

    pencil_parent_class = (SPDrawContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_pencil_context_dispose;

    event_context_class->root_handler = sp_pencil_context_root_handler;
}

static void
sp_pencil_context_init(SPPencilContext *pc)
{
    pc->state = SP_PENCIL_CONTEXT_IDLE;
}

static void
sp_pencil_context_dispose(GObject *object)
{
    SPPencilContext *pc;

    pc = SP_PENCIL_CONTEXT(object);

    G_OBJECT_CLASS(pencil_parent_class)->dispose(object);
}

gint
sp_pencil_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);
    SPPencilContext *pc = SP_PENCIL_CONTEXT(ec);
    SPDesktop *dt = ec->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {
            NR::Point const button_w(event->button.x,
                                     event->button.y);
            /* Find desktop coordinates */
            NR::Point p = sp_desktop_w2d_xy_point(dt, button_w);

            /* Test whether we hit any anchor. */
            SPDrawAnchor *anchor = test_inside(dc, button_w);

            switch (pc->state) {
            case SP_PENCIL_CONTEXT_ADDLINE:
                /* Current segment will be finished with release */
                ret = TRUE;
                break;
            default:
                /* Set first point of sequence */
                if (anchor) {
                    p = anchor->dp;
                }
                dc->sa = anchor;
                spdc_set_startpoint(pc, p, event->button.state);
                ret = TRUE;
                break;
            }
        }
        break;
    case GDK_MOTION_NOTIFY:
    {

        if ( ( event->motion.state & GDK_BUTTON1_MASK ) && !dc->grab ) {
            /* Grab mouse, so release will not pass unnoticed */
            dc->grab = SP_CANVAS_ITEM(dt->acetate);
            sp_canvas_item_grab(dc->grab, SPDC_EVENT_MASK, NULL, event->button.time);
        }

        /* Find desktop coordinates */
        NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(event->motion.x, event->motion.y));

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = test_inside(dc, NR::Point(event->button.x, event->button.y));

        switch (pc->state) {
        case SP_PENCIL_CONTEXT_ADDLINE:
            /* Set red endpoint */
            if (anchor) {
                p = anchor->dp;
            }
            spdc_set_endpoint(pc, p, event->motion.state);
            ret = TRUE;
            break;
        default:
            /* We may be idle or already freehand */
            if ( event->motion.state & GDK_BUTTON1_MASK ) {
                pc->state = SP_PENCIL_CONTEXT_FREEHAND;
                if ( !dc->sa && !dc->green_anchor ) {
                    /* Create green anchor */
                    dc->green_anchor = sp_draw_anchor_new(dc, dc->green_curve, TRUE, dc->p[0]);
                }
                /* fixme: I am not sure whether we want to snap to anchors in middle of freehand (Lauris) */
                if (anchor) {
                    p = anchor->dp;
                } else {
                    sp_desktop_free_snap(dt, p);
                }
                if ( dc->npoints != 0 ) { // buttonpress may have happened before we entered draw context!
                    spdc_add_freehand_point(pc, p, event->motion.state);
                    ret = TRUE;
                }
            }
            break;
        }
        break;
    }
    case GDK_BUTTON_RELEASE:
        if ( event->button.button == 1 ) {
            /* Find desktop coordinates */
            NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(event->motion.x, event->motion.y));

            /* Test whether we hit any anchor. */
            SPDrawAnchor *anchor = test_inside(dc, NR::Point(event->button.x,
                                                             event->button.y));

            switch (pc->state) {
            case SP_PENCIL_CONTEXT_IDLE:
                /* Releasing button in idle mode means single click */
                /* We have already set up start point/anchor in button_press */
                pc->state = SP_PENCIL_CONTEXT_ADDLINE;
                ret = TRUE;
                break;
            case SP_PENCIL_CONTEXT_ADDLINE:
                /* Finish segment now */
                if (anchor) {
                    p = anchor->dp;
                }
                dc->ea = anchor;
                spdc_finish_endpoint(pc, p, !anchor, event->button.state);
                pc->state = SP_PENCIL_CONTEXT_IDLE;
                ret = TRUE;
                break;
            case SP_PENCIL_CONTEXT_FREEHAND:
                /* Finish segment now */
                /* fixme: Clean up what follows (Lauris) */
                if (anchor) {
                    p = anchor->dp;
                }
                dc->ea = anchor;
                /* Write curves to object */

                sp_view_set_statusf_flash(SP_VIEW(dt), _("Finishing freehand"));

                spdc_concat_colors_and_flush(dc, FALSE);
                dc->sa = NULL;
                dc->ea = NULL;
                if (dc->green_anchor) {
                    dc->green_anchor = sp_draw_anchor_destroy(dc->green_anchor);
                }
                pc->state = SP_PENCIL_CONTEXT_IDLE;
                ret = TRUE;
                break;
            default:
                break;
            }

            if (dc->grab) {
                /* Release grab now */
                sp_canvas_item_ungrab(dc->grab, event->button.time);
                dc->grab = NULL;
            }

            dc->grab = NULL;
            ret = TRUE;
        }
        break;
    case GDK_KEY_PRESS:
        switch (event->key.keyval) {
        case GDK_Up: 
        case GDK_Down: 
        case GDK_KP_Up: 
        case GDK_KP_Down: 
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;
        default:
            break;
        }
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) pencil_parent_class)->root_handler)
            ret = ((SPEventContextClass *) pencil_parent_class)->root_handler(ec, event);
    }

    return ret;
}

/*
 * Reset points and set new starting point
 */

static void
spdc_set_startpoint(SPPencilContext *pc, NR::Point p, guint state)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    sp_desktop_free_snap(SP_EVENT_CONTEXT_DESKTOP(dc), p);

    dc->npoints = 0;
    dc->p[dc->npoints++] = p;
    dc->red_curve_is_valid=0x00;
}

/*
 * Change moving enpoint position
 *
 * - Ctrl constraints moving to H/V direction, snapping in given direction
 * - otherwise we snap freely to whatever attractors are available
 *
 * Number of points is (re)set to 2 always, 2nd point is modified
 * We change RED curve
 */

static void
spdc_set_endpoint(SPPencilContext *pc, NR::Point p, guint state)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    g_assert( dc->npoints > 0 );

    spdc_endpoint_snap(dc, p, state);

    dc->p[1] = p;
    dc->npoints = 2;

    sp_curve_reset(dc->red_curve);
    sp_curve_moveto(dc->red_curve, dc->p[0]);
    sp_curve_lineto(dc->red_curve, dc->p[1]);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), dc->red_curve);
    dc->red_curve_is_valid=0x01;
}

/*
 * Set endpoint final position and end addline mode
 * fixme: I'd like remove red reset from concat colors (lauris)
 * fixme: Still not sure, how it will make most sense
 */

static void
spdc_finish_endpoint(SPPencilContext *pc, NR::Point p, gboolean snap, guint state)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    if ( SP_CURVE_LENGTH(dc->red_curve) < 2 ) {
        /* Just a click, reset red curve and continue */
        g_print("Finishing empty red curve\n");
        sp_curve_reset(dc->red_curve);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);
    } else if ( SP_CURVE_LENGTH(dc->red_curve) > 2 ) {
        g_warning("Red curve length is %d", SP_CURVE_LENGTH(dc->red_curve));
        sp_curve_reset(dc->red_curve);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);
    } else {
        NArtBpath *s, *e;
        /* We have actual line */
        if (snap) {
            /* Do (bogus?) snap */
            spdc_endpoint_snap(dc, p, state);
        }
        /* fixme: We really should test start and end anchors instead */
        s = SP_CURVE_SEGMENT(dc->red_curve, 0);
        e = SP_CURVE_SEGMENT(dc->red_curve, 1);
        if ( ( e->x3 == s->x3 ) && ( e->y3 == s->y3 ) ) {
            /* Empty line, reset red curve and continue */
            g_print("Finishing zero length red curve\n");
            sp_curve_reset(dc->red_curve);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);
        } else {
            /* Write curves to object */
            g_print("Finishing real red curve\n");
            spdc_concat_colors_and_flush(dc, FALSE);
            dc->sa = NULL;
            dc->ea = NULL;
        }
    }
}

static void
spdc_add_freehand_point(SPPencilContext *pc, NR::Point p, guint state)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    g_assert( dc->npoints > 0 );
    if ( p != dc->p[ dc->npoints - 1 ] ) {
        dc->p[dc->npoints++] = p;
        fit_and_split(dc);
    }
}

/* Pen context */

static void sp_pen_context_class_init(SPPenContextClass *klass);
static void sp_pen_context_init(SPPenContext *pc);
static void sp_pen_context_dispose(GObject *object);

static void sp_pen_context_setup(SPEventContext *ec);
static void sp_pen_context_finish(SPEventContext *ec);
static void sp_pen_context_set(SPEventContext *ec, const gchar *key, const gchar *val);
static gint sp_pen_context_root_handler(SPEventContext *ec, GdkEvent *event);

static void spdc_pen_set_initial_point(SPPenContext *pc, NR::Point const p);
static void spdc_pen_set_subsequent_point(SPPenContext *pc, NR::Point const p);
static void spdc_pen_set_ctrl(SPPenContext *pc, NR::Point const p, guint state);
static void spdc_pen_finish_segment(SPPenContext *pc, NR::Point p, guint state);

static void spdc_pen_finish(SPPenContext *pc, gboolean closed);

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

static SPDrawContextClass *pen_parent_class;

GtkType
sp_pen_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPPenContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_pen_context_class_init,
            NULL, NULL,
            sizeof(SPPenContext),
            4,
            (GInstanceInitFunc) sp_pen_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_DRAW_CONTEXT, "SPPenContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_pen_context_class_init(SPPenContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *event_context_class;

    object_class = (GObjectClass *) klass;
    event_context_class = (SPEventContextClass *) klass;

    pen_parent_class = (SPDrawContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_pen_context_dispose;

    event_context_class->setup = sp_pen_context_setup;
    event_context_class->finish = sp_pen_context_finish;
    event_context_class->set = sp_pen_context_set;
    event_context_class->root_handler = sp_pen_context_root_handler;
}

static void
sp_pen_context_init(SPPenContext *pc)
{
    pc->mode = SP_PEN_CONTEXT_MODE_CLICK;

    pc->state = SP_PEN_CONTEXT_POINT;

    pc->c0 = NULL;
    pc->c1 = NULL;
    pc->cl0 = NULL;
    pc->cl1 = NULL;
}

static void
sp_pen_context_dispose(GObject *object)
{
    SPPenContext *pc;

    pc = SP_PEN_CONTEXT(object);

    if (pc->c0) {
        gtk_object_destroy(GTK_OBJECT(pc->c0));
        pc->c0 = NULL;
    }
    if (pc->c1) {
        gtk_object_destroy(GTK_OBJECT(pc->c1));
        pc->c1 = NULL;
    }
    if (pc->cl0) {
        gtk_object_destroy(GTK_OBJECT(pc->cl0));
        pc->cl0 = NULL;
    }
    if (pc->cl1) {
        gtk_object_destroy(GTK_OBJECT(pc->cl1));
        pc->cl1 = NULL;
    }

    G_OBJECT_CLASS(pen_parent_class)->dispose(object);
}

static void
sp_pen_context_setup(SPEventContext *ec)
{
    SPPenContext *pc;

    pc = SP_PEN_CONTEXT(ec);

    if (((SPEventContextClass *) pen_parent_class)->setup) {
        ((SPEventContextClass *) pen_parent_class)->setup(ec);
    }

    /* Pen indicators */
    pc->c0 = sp_canvas_item_new(SP_DT_CONTROLS(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRL, "shape", SP_CTRL_SHAPE_CIRCLE,
                                "size", 4.0, "filled", 0, "fill_color", 0xff00007f, "stroked", 1, "stroke_color", 0x0000ff7f, NULL);
    pc->c1 = sp_canvas_item_new(SP_DT_CONTROLS(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRL, "shape", SP_CTRL_SHAPE_CIRCLE,
                                "size", 4.0, "filled", 0, "fill_color", 0xff00007f, "stroked", 1, "stroke_color", 0x0000ff7f, NULL);
    pc->cl0 = sp_canvas_item_new(SP_DT_CONTROLS(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_rgba32(SP_CTRLLINE(pc->cl0), 0x0000007f);
    pc->cl1 = sp_canvas_item_new(SP_DT_CONTROLS(SP_EVENT_CONTEXT_DESKTOP(ec)), SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_rgba32(SP_CTRLLINE(pc->cl1), 0x0000007f);

    sp_canvas_item_hide(pc->c0);
    sp_canvas_item_hide(pc->c1);
    sp_canvas_item_hide(pc->cl0);
    sp_canvas_item_hide(pc->cl1);

    sp_event_context_read(ec, "mode");
}

static void
sp_pen_context_finish(SPEventContext *ec)
{
    spdc_pen_finish(SP_PEN_CONTEXT(ec), FALSE);

    if (((SPEventContextClass *) pen_parent_class)->finish) {
        ((SPEventContextClass *) pen_parent_class)->finish(ec);
    }
}

static void
sp_pen_context_set(SPEventContext *ec, const gchar *key, const gchar *val)
{
    SPPenContext *pc = SP_PEN_CONTEXT(ec);

    if (!strcmp(key, "mode")) {
        if ( val && !strcmp(val, "drag") ) {
            pc->mode = SP_PEN_CONTEXT_MODE_DRAG;
        } else {
            pc->mode = SP_PEN_CONTEXT_MODE_CLICK;
        }
    }
}

gint
sp_pen_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(ec);
    SPPenContext *pc = SP_PEN_CONTEXT(ec);
    SPDesktop *dt = ec->desktop;

    gint ret = FALSE;

    tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100);
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {

            // save drag origin
            xp = (gint) event->button.x; 
            yp = (gint) event->button.y;
            within_tolerance = true;

            /* Find desktop coordinates */
            NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(event->button.x, event->button.y));

            /* Test whether we hit any anchor. */
            SPDrawAnchor *anchor = test_inside(dc, NR::Point(event->button.x, event->button.y));

            switch (pc->mode) {
            case SP_PEN_CONTEXT_MODE_CLICK:
                /* In click mode we add point on release */
                switch (pc->state) {
                case SP_PEN_CONTEXT_POINT:
                case SP_PEN_CONTEXT_CONTROL:
                case SP_PEN_CONTEXT_CLOSE:
                    break;
                case SP_PEN_CONTEXT_STOP:
                    /* This is allowed, if we just cancelled curve */
                    pc->state = SP_PEN_CONTEXT_POINT;
                    break;
                default:
                    break;
                }
                break;
            case SP_PEN_CONTEXT_MODE_DRAG:
                switch (pc->state) {
                case SP_PEN_CONTEXT_STOP:
                    /* This is allowed, if we just cancelled curve */
                case SP_PEN_CONTEXT_POINT:
                    if ( dc->npoints == 0 ) {
                        /* Set start anchor */
                        dc->sa = anchor;
                        if (anchor) {
                            /* Adjust point to anchor if needed */
                            p = anchor->dp;
                        } else {
                            /* Create green anchor */
                            spdc_endpoint_snap(dc, p, event->motion.state);
                            dc->green_anchor = sp_draw_anchor_new(dc, dc->green_curve, TRUE, p);
                        }
                        spdc_pen_set_initial_point(pc, p);
                    } else {
                        /* Set end anchor */
                        dc->ea = anchor;
                        if (!anchor) {   /* Snap node only if not hitting anchor */
                            spdc_endpoint_snap(dc, p, event->motion.state);
                        } else {
                            p = anchor->dp;
                        }
                        spdc_pen_set_subsequent_point(pc, p);
                        if ( dc->green_anchor && dc->green_anchor->active ) {
                            pc->state = SP_PEN_CONTEXT_CLOSE;
                            ret = TRUE;
                            break;
                        }
                    }
                    pc->state = SP_PEN_CONTEXT_CONTROL;
                    ret = TRUE;
                    break;
                case SP_PEN_CONTEXT_CONTROL:
                    g_warning("Button down in CONTROL state");
                    break;
                case SP_PEN_CONTEXT_CLOSE:
                    g_warning("Button down in CLOSE state");
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        } else if (event->button.button == 3) {
            if (dc->npoints != 0) {
                spdc_pen_finish(pc, FALSE);
                ret = TRUE;
            }
        }
        break;
    case GDK_MOTION_NOTIFY:
    {
        if ( within_tolerance
             && ( abs( (gint) event->motion.x - xp ) < tolerance )
             && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
            break; // do not drag if we're within tolerance from origin
        }
        // Once the user has moved farther than tolerance from the original location 
        // (indicating they intend to move the object, not click), then always process the 
        // motion notify coordinates as given (no snapping back to origin)
        within_tolerance = false; 

        if ( ( event->motion.state & GDK_BUTTON1_MASK ) && !dc->grab ) {
            /* Grab mouse, so release will not pass unnoticed */
            dc->grab = SP_CANVAS_ITEM(dt->acetate);
            sp_canvas_item_grab(dc->grab, SPDC_EVENT_MASK, NULL, event->button.time);
        }

        /* Find desktop coordinates */
        NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(event->motion.x, event->motion.y));

        /* Test, whether we hit any anchor */
        SPDrawAnchor *anchor = test_inside(dc, NR::Point(event->button.x, event->button.y));

        switch (pc->mode) {
        case SP_PEN_CONTEXT_MODE_CLICK:
            switch (pc->state) {
            case SP_PEN_CONTEXT_POINT:
                if ( dc->npoints != 0 ) {
                    /* Only set point, if we are already appending */
                    spdc_endpoint_snap(dc, p, event->motion.state);
                    spdc_pen_set_subsequent_point(pc, p);
                    ret = TRUE;
                }
                break;
            case SP_PEN_CONTEXT_CONTROL:
            case SP_PEN_CONTEXT_CLOSE:
                /* Placing controls is last operation in CLOSE state */
                spdc_endpoint_snap(dc, p, event->motion.state);
                spdc_pen_set_ctrl(pc, p, event->motion.state);
                ret = TRUE;
                break;
            case SP_PEN_CONTEXT_STOP:
                /* This is perfectly valid */
                break;
            default:
                break;
            }
            break;
        case SP_PEN_CONTEXT_MODE_DRAG:
            switch (pc->state) {
            case SP_PEN_CONTEXT_POINT:
                if ( dc->npoints > 0 ) {
                    /* Only set point, if we are already appending */

                    if (!anchor) {   /* Snap node only if not hitting anchor */
                        spdc_endpoint_snap(dc, p, event->motion.state);
                    }

                    spdc_pen_set_subsequent_point(pc, p);
                    ret = TRUE;
                }
                break;
            case SP_PEN_CONTEXT_CONTROL:
            case SP_PEN_CONTEXT_CLOSE:
                /* Placing controls is last operation in CLOSE state */

                // snap the handle
                spdc_endpoint_snap_handle(dc, p, event->motion.state);

                spdc_pen_set_ctrl(pc, p, event->motion.state);
                ret = TRUE;
                break;
            case SP_PEN_CONTEXT_STOP:
                /* This is perfectly valid */
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case GDK_BUTTON_RELEASE:
        xp = yp = 0;
        if ( event->button.button == 1 ) {
            /* Find desktop coordinates */
            NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(event->motion.x, event->motion.y));

            /* Test whether we hit any anchor. */
            SPDrawAnchor *anchor = test_inside(dc, NR::Point(event->button.x, event->button.y));

            switch (pc->mode) {
            case SP_PEN_CONTEXT_MODE_CLICK:
                switch (pc->state) {
                case SP_PEN_CONTEXT_POINT:
                    if ( dc->npoints == 0 ) {
                        /* Start new thread only with button release */
                        if (anchor) {
                            p = anchor->dp;
                        }
                        dc->sa = anchor;
                        spdc_endpoint_snap(dc, p, event->motion.state);
                        spdc_pen_set_initial_point(pc, p);
                    } else {
                        /* Set end anchor here */
                        dc->ea = anchor;
                         if (anchor) {
                            p = anchor->dp;
                        }
                    }
                    pc->state = SP_PEN_CONTEXT_CONTROL;
                    ret = TRUE;
                    break;
                case SP_PEN_CONTEXT_CONTROL:
                    /* End current segment */
                    spdc_endpoint_snap(dc, p, event->motion.state);
                    spdc_pen_finish_segment(pc, p, event->button.state);
                    pc->state = SP_PEN_CONTEXT_POINT;
                    ret = TRUE;
                    break;
                case SP_PEN_CONTEXT_CLOSE:
                    /* End current segment */
                    if (!anchor) {   /* Snap node only if not hitting anchor */
                        spdc_endpoint_snap(dc, p, event->motion.state);
                    }
                    spdc_pen_finish_segment(pc, p, event->button.state);
                    spdc_pen_finish(pc, TRUE);
                    pc->state = SP_PEN_CONTEXT_POINT;
                    ret = TRUE;
                    break;
                case SP_PEN_CONTEXT_STOP:
                    /* This is allowed, if we just cancelled curve */
                    pc->state = SP_PEN_CONTEXT_POINT;
                    ret = TRUE;
                    break;
                default:
                    break;
                }
                break;
            case SP_PEN_CONTEXT_MODE_DRAG:
                switch (pc->state) {
                case SP_PEN_CONTEXT_POINT:
                case SP_PEN_CONTEXT_CONTROL:
                    spdc_endpoint_snap(dc, p, event->motion.state);
                    spdc_pen_finish_segment(pc, p, event->button.state);
                    break;
                case SP_PEN_CONTEXT_CLOSE:
                    spdc_endpoint_snap(dc, p, event->motion.state);
                    spdc_pen_finish_segment(pc, p, event->button.state);
                    spdc_pen_finish(pc, TRUE);
                    break;
                case SP_PEN_CONTEXT_STOP:
                    /* This is allowed, if we just cancelled curve */
                    break;
                default:
                    break;
                }
                pc->state = SP_PEN_CONTEXT_POINT;
                ret = TRUE;
                break;
            default:
                break;
            }

            if (dc->grab) {
                /* Release grab now */
                sp_canvas_item_ungrab(dc->grab, event->button.time);
                dc->grab = NULL;
            }

            ret = TRUE;
        }
        break;
    case GDK_2BUTTON_PRESS:
        if (dc->npoints != 0) {
            spdc_pen_finish(pc, FALSE);
            ret = TRUE;
        }
        break;
    case GDK_KEY_PRESS:
        /* fixme: */
        switch (event->key.keyval) {
        case GDK_Return:
            if (dc->npoints != 0) {
                spdc_pen_finish(pc, FALSE);
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            pc->state = SP_PEN_CONTEXT_STOP;
            spdc_reset_colors(dc);
            sp_canvas_item_hide(pc->c0);
            sp_canvas_item_hide(pc->c1);
            sp_canvas_item_hide(pc->cl0);
            sp_canvas_item_hide(pc->cl1);
            ret = TRUE;
            break;
        case GDK_BackSpace:
            if (sp_curve_is_empty(dc->green_curve)) {
                /* Same as cancel */
                pc->state = SP_PEN_CONTEXT_STOP;
                spdc_reset_colors(dc);
                sp_canvas_item_hide(pc->c0);
                sp_canvas_item_hide(pc->c1);
                sp_canvas_item_hide(pc->cl0);
                sp_canvas_item_hide(pc->cl1);
                ret = TRUE;
                break;
            } else {
                NR::Point pt;
                NArtBpath *p;
                gint e;
                /* Reset red curve */
                sp_curve_reset(dc->red_curve);
                /* Destroy topmost green bpath */
                gtk_object_destroy(GTK_OBJECT(dc->green_bpaths->data));
                dc->green_bpaths = g_slist_remove(dc->green_bpaths, dc->green_bpaths->data);
                /* Get last segment */
                p = SP_CURVE_BPATH(dc->green_curve);
                e = SP_CURVE_LENGTH(dc->green_curve);
                if ( e < 2 ) {
                    g_warning("Green curve length is %d", e);
                    break;
                }
                dc->p[0] = NR::Point(p[e - 2].x3, p[e - 2].y3);
                dc->p[1] = NR::Point(p[e - 1].x1, p[e - 1].y1);
                if ( dc->npoints < 4 ) {
                    pt = NR::Point(p[e - 1].x3, p[e - 1].y3);
                } else {
                    pt = dc->p[3];
                }
                dc->npoints = 2;
                sp_curve_backspace(dc->green_curve);
                sp_canvas_item_hide(pc->c0);
                sp_canvas_item_hide(pc->c1);
                sp_canvas_item_hide(pc->cl0);
                sp_canvas_item_hide(pc->cl1);
                pc->state = SP_PEN_CONTEXT_POINT;
                spdc_pen_set_subsequent_point(pc, pt);
                ret = TRUE;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) pen_parent_class)->root_handler)
            return ((SPEventContextClass *) pen_parent_class)->root_handler(ec, event);
    }

    return ret;
}

static void
spdc_pen_set_initial_point(SPPenContext *pc, NR::Point const p)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);
    g_assert( dc->npoints == 0 );

    dc->p[0] = p;
    dc->p[1] = p;
    dc->npoints = 2;
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), NULL);
}

static void
spdc_pen_set_subsequent_point(SPPenContext *pc, NR::Point const p)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);
    g_assert( dc->npoints != 0 );

    dc->p[2] = p;
    dc->p[3] = p;
    dc->p[4] = p;
    dc->npoints = 5;
    sp_curve_reset(dc->red_curve);
    sp_curve_moveto(dc->red_curve, dc->p[0]);
    if ( (pc->onlycurves)
         || ( dc->p[1][NR::X] != dc->p[0][NR::X] )
         || ( dc->p[1][NR::Y] != dc->p[0][NR::Y] ) )
    {
        sp_curve_curveto(dc->red_curve, dc->p[1], dc->p[2], dc->p[3]);
    } else {
        sp_curve_lineto(dc->red_curve, dc->p[3]);
    }
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), dc->red_curve);
}

static void
spdc_pen_set_ctrl(SPPenContext *pc, NR::Point const p, guint state)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    sp_canvas_item_show(pc->c1);
    sp_canvas_item_show(pc->cl1);

    if ( dc->npoints == 2 ) {
        dc->p[1] = p;
        sp_canvas_item_hide(pc->c0);
        sp_canvas_item_hide(pc->cl0);
        sp_ctrl_moveto(SP_CTRL(pc->c1), dc->p[1]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl1), dc->p[0], dc->p[1]);
    } else if ( dc->npoints == 5 ) {
        dc->p[4] = p;
        sp_canvas_item_show(pc->c0);
        sp_canvas_item_show(pc->cl0);
        if ( ( ( pc->mode == SP_PEN_CONTEXT_MODE_CLICK ) && ( state & GDK_CONTROL_MASK ) ) ||
             ( ( pc->mode == SP_PEN_CONTEXT_MODE_DRAG ) &&  !( state & GDK_SHIFT_MASK  ) ) ) {
            NR::Point delta = p - dc->p[3];
            dc->p[2] = dc->p[3] - delta;
            sp_curve_reset(dc->red_curve);
            sp_curve_moveto(dc->red_curve, dc->p[0]);
            sp_curve_curveto(dc->red_curve, dc->p[1], dc->p[2], dc->p[3]);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(dc->red_bpath), dc->red_curve);
        }
        sp_ctrl_moveto(SP_CTRL(pc->c0), dc->p[2]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl0), dc->p[3], dc->p[2]);
        sp_ctrl_moveto(SP_CTRL(pc->c1), dc->p[4]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl1), dc->p[3], dc->p[4]);
    } else {
        g_warning("Something bad happened - npoints is %d", dc->npoints);
    }
}

static void
spdc_pen_finish_segment(SPPenContext *pc, NR::Point p, guint state)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    if (!sp_curve_empty(dc->red_curve)) {
        sp_curve_append_continuous(dc->green_curve, dc->red_curve, 0.0625);
        SPCurve *curve = sp_curve_copy(dc->red_curve);
        /* fixme: */
        SPCanvasItem *cshape = sp_canvas_bpath_new(SP_DT_SKETCH(SP_EVENT_CONTEXT(dc)->desktop), curve);
        sp_curve_unref(curve);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), dc->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

        dc->green_bpaths = g_slist_prepend(dc->green_bpaths, cshape);

        dc->p[0] = dc->p[3];
        dc->p[1] = dc->p[4];
        dc->npoints = 2;

        sp_curve_reset(dc->red_curve);
    }
}

static void
spdc_pen_finish(SPPenContext *pc, gboolean closed)
{
    SPDrawContext *dc = SP_DRAW_CONTEXT(pc);

    sp_view_set_statusf_flash(SP_VIEW(SP_EVENT_CONTEXT(dc)->desktop), _("Finishing pen"));

    sp_curve_reset(dc->red_curve);
    spdc_concat_colors_and_flush(dc, closed);
    dc->sa = NULL;
    dc->ea = NULL;

    dc->npoints = 0;
    pc->state = SP_PEN_CONTEXT_POINT;

    sp_canvas_item_hide(pc->c0);
    sp_canvas_item_hide(pc->c1);
    sp_canvas_item_hide(pc->cl0);
    sp_canvas_item_hide(pc->cl1);

    if (dc->green_anchor) {
        dc->green_anchor = sp_draw_anchor_destroy(dc->green_anchor);
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
