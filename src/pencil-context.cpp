/** \file Pencil event context. */

/*
 * Initial author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdkkeysyms.h>

#include "pencil-context.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "desktop-handles.h"
#include "draw-anchor.h"
#include "draw-context.h"
#include "modifier-fns.h"
#include "prefs-utils.h"
#include "snap.h"
#include "display/bezier-utils.h"
#include "display/canvas-bpath.h"
#include "display/sp-canvas.h"
#include "helper/sp-intl.h"
#include "libnr/n-art-bpath.h"

static void sp_pencil_context_class_init(SPPencilContextClass *klass);
static void sp_pencil_context_init(SPPencilContext *pc);
static void sp_pencil_context_setup(SPEventContext *ec);
static void sp_pencil_context_dispose(GObject *object);

static gint sp_pencil_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint pencil_handle_button_press(SPPencilContext *const pc, GdkEventButton const &bevent);
static gint pencil_handle_motion_notify(SPPencilContext *const pc, GdkEventMotion const &mevent);
static gint pencil_handle_button_release(SPPencilContext *const pc, GdkEventButton const &revent);
static gint pencil_handle_key_press(SPPencilContext *const pc, guint const keyval, guint const state);

static void spdc_set_startpoint(SPPencilContext *pc, NR::Point p, guint state);
static void spdc_set_endpoint(SPPencilContext *pc, NR::Point p, guint state);
static void spdc_finish_endpoint(SPPencilContext *pc, NR::Point p, gboolean snap, guint state);
static void spdc_add_freehand_point(SPPencilContext *pc, NR::Point p, guint state);
static void fit_and_split(SPPencilContext *pc);


static SPDrawContextClass *pencil_parent_class;


GType
sp_pencil_context_get_type()
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

    event_context_class->setup = sp_pencil_context_setup;
    event_context_class->root_handler = sp_pencil_context_root_handler;
}

static void
sp_pencil_context_init(SPPencilContext *pc)
{
    pc->npoints = 0;
    pc->state = SP_PENCIL_CONTEXT_IDLE;
}

static void
sp_pencil_context_setup(SPEventContext *ec)
{
    if (prefs_get_int_attribute("tools.freehand.pencil", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    if (((SPEventContextClass *) pencil_parent_class)->setup) {
        ((SPEventContextClass *) pencil_parent_class)->setup(ec);
    }
}


static void
sp_pencil_context_dispose(GObject *object)
{
    G_OBJECT_CLASS(pencil_parent_class)->dispose(object);
}

gint
sp_pencil_context_root_handler(SPEventContext *const ec, GdkEvent *event)
{
    SPPencilContext *const pc = SP_PENCIL_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = pencil_handle_button_press(pc, event->button);
            break;

        case GDK_MOTION_NOTIFY:
            ret = pencil_handle_motion_notify(pc, event->motion);
            break;

        case GDK_BUTTON_RELEASE:
            ret = pencil_handle_button_release(pc, event->button);
            break;

        case GDK_KEY_PRESS:
            ret = pencil_handle_key_press(pc,
                                          event->key.keyval,
                                          event->key.state);
            break;

        default:
            break;
    }

    if (!ret) {
        gint (*const parent_root_handler)(SPEventContext *, GdkEvent *)
            = ((SPEventContextClass *) pencil_parent_class)->root_handler;
        if (parent_root_handler) {
            ret = parent_root_handler(ec, event);
        }
    }

    return ret;
}

static gint
pencil_handle_button_press(SPPencilContext *const pc, GdkEventButton const &bevent)
{
    gint ret = FALSE;
    if ( bevent.button == 1 ) {
        NR::Point const button_w(bevent.x,
                                 bevent.y);
        /* Find desktop coordinates */
        NR::Point p = sp_desktop_w2d_xy_point(pc->desktop, button_w);

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, button_w);

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
                pc->sa = anchor;
                spdc_set_startpoint(pc, p, bevent.state);
                ret = TRUE;
                break;
        }
    }
    return ret;
}

static gint
pencil_handle_motion_notify(SPPencilContext *const pc, GdkEventMotion const &mevent)
{
    gint ret = FALSE;
    SPDesktop *const dt = pc->desktop;
    if ( ( mevent.state & GDK_BUTTON1_MASK ) && !pc->grab ) {
        /* Grab mouse, so release will not pass unnoticed */
        pc->grab = SP_CANVAS_ITEM(dt->acetate);
        sp_canvas_item_grab(pc->grab, ( GDK_BUTTON_PRESS_MASK   |
                                        GDK_BUTTON_RELEASE_MASK |
                                        GDK_POINTER_MOTION_MASK  ),
                            NULL, mevent.time);
    }

    /* Find desktop coordinates */
    NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(mevent.x, mevent.y));

    /* Test whether we hit any anchor. */
    SPDrawAnchor *anchor = spdc_test_inside(pc, NR::Point(mevent.x, mevent.y));

    switch (pc->state) {
        case SP_PENCIL_CONTEXT_ADDLINE:
            /* Set red endpoint */
            if (anchor) {
                p = anchor->dp;
            }
            spdc_set_endpoint(pc, p, mevent.state);
            ret = TRUE;
            break;
        default:
            /* We may be idle or already freehand */
            if ( mevent.state & GDK_BUTTON1_MASK ) {
                pc->state = SP_PENCIL_CONTEXT_FREEHAND;
                if ( !pc->sa && !pc->green_anchor ) {
                    /* Create green anchor */
                    pc->green_anchor = sp_draw_anchor_new(pc, pc->green_curve, TRUE, pc->p[0]);
                }
                /* fixme: I am not sure whether we want to snap to anchors in middle of freehand (Lauris) */
                if (anchor) {
                    p = anchor->dp;
                } else if ((mevent.state & GDK_SHIFT_MASK) == 0) {
                    namedview_free_snap_all_types(dt->namedview, p);
                }
                if ( pc->npoints != 0 ) { // buttonpress may have happened before we entered draw context!
                    spdc_add_freehand_point(pc, p, mevent.state);
                    ret = TRUE;
                }
            }
            break;
    }
    return ret;
}

static gint
pencil_handle_button_release(SPPencilContext *const pc, GdkEventButton const &revent)
{
    gint ret = FALSE;
    if ( revent.button == 1 ) {
        SPDesktop *const dt = pc->desktop;

        /* Find desktop coordinates */
        NR::Point p = sp_desktop_w2d_xy_point(dt, NR::Point(revent.x,
                                                            revent.y));

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, NR::Point(revent.x,
                                                              revent.y));

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
                pc->ea = anchor;
                spdc_finish_endpoint(pc, p, !anchor, revent.state);
                pc->state = SP_PENCIL_CONTEXT_IDLE;
                ret = TRUE;
                break;
            case SP_PENCIL_CONTEXT_FREEHAND:
                /* Finish segment now */
                /* fixme: Clean up what follows (Lauris) */
                if (anchor) {
                    p = anchor->dp;
                }
                pc->ea = anchor;
                /* Write curves to object */

                dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Finishing freehand"));

                spdc_concat_colors_and_flush(pc, FALSE);
                pc->sa = NULL;
                pc->ea = NULL;
                if (pc->green_anchor) {
                    pc->green_anchor = sp_draw_anchor_destroy(pc->green_anchor);
                }
                pc->state = SP_PENCIL_CONTEXT_IDLE;
                ret = TRUE;
                break;
            default:
                break;
        }

        if (pc->grab) {
            /* Release grab now */
            sp_canvas_item_ungrab(pc->grab, revent.time);
            pc->grab = NULL;
        }

        pc->grab = NULL;
        ret = TRUE;
    }
    return ret;
}

static gint
pencil_handle_key_press(SPPencilContext *const pc, guint const keyval, guint const state)
{
    gint ret = FALSE;
    switch (keyval) {
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // Prevent the zoom field from activation.
            if (!mod_ctrl_only(state)) {
                ret = TRUE;
            }
            break;
        default:
            break;
    }
    return ret;
}

/** Snaps new node relative to the previous node. */
static void
spdc_endpoint_snap(SPPencilContext const *pc, NR::Point &p, guint const state)
{
    spdc_endpoint_snap_internal(pc, p, pc->p[0], state);
}

/**
 * Reset points and set new starting point.
 */
static void
spdc_set_startpoint(SPPencilContext *const pc, NR::Point p, guint const state)
{
    if ((state & GDK_SHIFT_MASK) == 0) {
        namedview_free_snap_all_types(SP_EVENT_CONTEXT_DESKTOP(pc)->namedview, p);
    }

    pc->npoints = 0;
    pc->p[pc->npoints++] = p;
    pc->red_curve_is_valid = false;
}

/**
 * Change moving endpoint position.
 * <ul>
 * <li>Ctrl constrains to moving to H/V direction, snapping in given direction.
 * <li>Otherwise we snap freely to whatever attractors are available.
 * </ul>
 *
 * Number of points is (re)set to 2 always, 2nd point is modified.
 * We change RED curve.
 */
static void
spdc_set_endpoint(SPPencilContext *const pc, NR::Point p, guint const state)
{
    g_assert( pc->npoints > 0 );

    spdc_endpoint_snap(pc, p, state);

    pc->p[1] = p;
    pc->npoints = 2;

    sp_curve_reset(pc->red_curve);
    sp_curve_moveto(pc->red_curve, pc->p[0]);
    sp_curve_lineto(pc->red_curve, pc->p[1]);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);
    pc->red_curve_is_valid = true;
}

/*
 * Set endpoint final position and end addline mode
 * fixme: I'd like remove red reset from concat colors (lauris)
 * fixme: Still not sure, how it will make most sense
 */

static void
spdc_finish_endpoint(SPPencilContext *const pc, NR::Point p, gboolean const snap,
                     guint const state)
{
    if ( SP_CURVE_LENGTH(pc->red_curve) < 2 ) {
        /* Just a click, reset red curve and continue */
        sp_curve_reset(pc->red_curve);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);
    } else if ( SP_CURVE_LENGTH(pc->red_curve) > 2 ) {
        sp_curve_reset(pc->red_curve);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);
    } else {
        NArtBpath *s, *e;
        /* We have actual line */
        if (snap) {
            /* Do (bogus?) snap */
            spdc_endpoint_snap(pc, p, state);
        }
        /* fixme: We really should test start and end anchors instead */
        s = SP_CURVE_SEGMENT(pc->red_curve, 0);
        e = SP_CURVE_SEGMENT(pc->red_curve, 1);
        if ( ( e->x3 == s->x3 ) && ( e->y3 == s->y3 ) ) {
            /* Empty line, reset red curve and continue */
            sp_curve_reset(pc->red_curve);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), NULL);
        } else {
            /* Write curves to object */
            spdc_concat_colors_and_flush(pc, FALSE);
            pc->sa = NULL;
            pc->ea = NULL;
        }
    }
}

static void
spdc_add_freehand_point(SPPencilContext *pc, NR::Point p, guint state)
{
    g_assert( pc->npoints > 0 );
    if ( p != pc->p[ pc->npoints - 1 ] ) {
        pc->p[pc->npoints++] = p;
        fit_and_split(pc);
    }
}

static inline double
square(double const x)
{
    return x * x;
}

static void
fit_and_split(SPPencilContext *pc)
{
    g_assert( pc->npoints > 1 );

    double const tolerance_sq = square( NR::expansion(pc->desktop->w2d)
                                        * prefs_get_double_attribute_limited("tools.freehand.pencil",
                                                                             "tolerance", 10.0, 1.0, 100.0) );

    NR::Point b[4];
    int const n_segs = sp_bezier_fit_cubic(b, pc->p, pc->npoints, tolerance_sq);
    if ( n_segs > 0
         && unsigned(pc->npoints) < G_N_ELEMENTS(pc->p) )
    {
        /* Fit and draw and reset state */
        sp_curve_reset(pc->red_curve);
        sp_curve_moveto(pc->red_curve, b[0]);
        sp_curve_curveto(pc->red_curve, b[1], b[2], b[3]);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);
        pc->red_curve_is_valid = true;
    } else {
        /* Fit and draw and copy last point */

        /* todo: This isn't what we want.  We really want to do the curve fit with say a 50%
         * overlap with the previous points, but such that the curve matches the tangent at the
         * exact meeting point.  Probably we want 2nd order continuity. */

        g_assert(!sp_curve_empty(pc->red_curve));
        sp_curve_append_continuous(pc->green_curve, pc->red_curve, 0.0625);
        SPCurve *curve = sp_curve_copy(pc->red_curve);

        /* fixme: */
        SPCanvasItem *cshape = sp_canvas_bpath_new(SP_DT_SKETCH(pc->desktop), curve);
        sp_curve_unref(curve);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), pc->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

        pc->green_bpaths = g_slist_prepend(pc->green_bpaths, cshape);

        g_assert( 3 <= pc->npoints );
        for (int i = 0; i < 3; i++) {
            pc->p[i] = pc->p[pc->npoints - 3 + i];
        }
        pc->npoints = 3;
        pc->red_curve_is_valid = false;
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
