/** \file Pen event context. */

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

#include "pen-context.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "desktop-handles.h"
#include "draw-anchor.h"
#include "draw-context.h"
#include "prefs-utils.h"
#include "display/canvas-bpath.h"
#include "display/sp-canvas.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include "helper/sp-intl.h"
#include "libnr/n-art-bpath.h"

static void sp_pen_context_class_init(SPPenContextClass *klass);
static void sp_pen_context_init(SPPenContext *pc);
static void sp_pen_context_dispose(GObject *object);

static void sp_pen_context_setup(SPEventContext *ec);
static void sp_pen_context_finish(SPEventContext *ec);
static void sp_pen_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_pen_context_root_handler(SPEventContext *ec, GdkEvent *event);

static void spdc_pen_set_initial_point(SPPenContext *pc, NR::Point const p);
static void spdc_pen_set_subsequent_point(SPPenContext *pc, NR::Point const p);
static void spdc_pen_set_ctrl(SPPenContext *pc, NR::Point const p, guint state);
static void spdc_pen_finish_segment(SPPenContext *pc, NR::Point p, guint state);

static void spdc_pen_finish(SPPenContext *pc, gboolean closed);

static gint pen_handle_button_press(SPPenContext *const pc, GdkEventButton const &bevent);
static gint pen_handle_motion_notify(SPPenContext *const pc, GdkEventMotion const &mevent);
static gint pen_handle_button_release(SPPenContext *const pc, GdkEventButton const &revent);
static gint pen_handle_2button_press(SPPenContext *const pc);
static gint pen_handle_key_press(SPPenContext *const pc, guint const keyval);
static void spdc_reset_colors(SPDrawContext *dc);


static NR::Point pen_drag_origin_w(0, 0);
static bool pen_within_tolerance = false;

static SPDrawContextClass *pen_parent_class;


GType
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

    if (prefs_get_int_attribute("tools.freehand.pen", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }
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
sp_pen_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
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


/** Snaps new node's handle relative to the new node. */
static void
spdc_endpoint_snap_handle(SPPenContext *pc, NR::Point &p, guint const state)
{
    g_return_if_fail(( pc->npoints == 2 ||
                       pc->npoints == 5   ));

    spdc_endpoint_snap_internal(pc, p, pc->p[pc->npoints - 2], state);
}

gint
sp_pen_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    SPPenContext *const pc = SP_PEN_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = pen_handle_button_press(pc, event->button);
            break;

        case GDK_MOTION_NOTIFY:
            ret = pen_handle_motion_notify(pc, event->motion);
            break;

        case GDK_BUTTON_RELEASE:
            ret = pen_handle_button_release(pc, event->button);
            break;

        case GDK_2BUTTON_PRESS:
            ret = pen_handle_2button_press(pc);
            break;

        case GDK_KEY_PRESS:
            ret = pen_handle_key_press(pc, event->key.keyval);
            break;

        default:
            break;
    }

    if (!ret) {
        gint (*const parent_root_handler)(SPEventContext *, GdkEvent *)
            = ((SPEventContextClass *) pen_parent_class)->root_handler;
        if (parent_root_handler) {
            ret = parent_root_handler(ec, event);
        }
    }

    return ret;
}

static gint
pen_handle_button_press(SPPenContext *const pc, GdkEventButton const &bevent)
{
    gint ret = FALSE;
    if ( bevent.button == 1 ) {
        NR::Point const event_w(bevent.x,
                                bevent.y);
        pen_drag_origin_w = event_w;
        pen_within_tolerance = true;

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, event_w);

        NR::Point const event_dt(sp_desktop_w2d_xy_point(pc->desktop, event_w));
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
                        if ( pc->npoints == 0 ) {
                            /* Set start anchor */
                            pc->sa = anchor;
                            NR::Point p;
                            if (anchor) {
                                /* Adjust point to anchor if needed */
                                p = anchor->dp;
                            } else {
                                /* Create green anchor */
                                p = event_dt;
                                spdc_endpoint_snap(pc, p, bevent.state);
                                pc->green_anchor = sp_draw_anchor_new(pc, pc->green_curve, TRUE, p);
                            }
                            spdc_pen_set_initial_point(pc, p);
                        } else {
                            /* Set end anchor */
                            pc->ea = anchor;
                            NR::Point p;
                            if (anchor) {   /* Snap node only if not hitting anchor. */
                                p = anchor->dp;
                            } else {
                                p = event_dt;
                                spdc_endpoint_snap(pc, p, bevent.state);
                            }
                            spdc_pen_set_subsequent_point(pc, p);
                            if ( pc->green_anchor && pc->green_anchor->active ) {
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
    } else if (bevent.button == 3) {
        if (pc->npoints != 0) {
            spdc_pen_finish(pc, FALSE);
            ret = TRUE;
        }
    }
    return ret;
}

static gint
pen_handle_motion_notify(SPPenContext *const pc, GdkEventMotion const &mevent)
{
    gint ret = FALSE;
    NR::Point const event_w(mevent.x,
                            mevent.y);
    if (pen_within_tolerance) {
        gint const tolerance = prefs_get_int_attribute_limited("options.dragtolerance",
                                                               "value", 0, 0, 100);
        if ( NR::LInfty( event_w - pen_drag_origin_w ) < tolerance ) {
            return FALSE;   // Do not drag if we're within tolerance from origin.
        }
    }
    // Once the user has moved farther than tolerance from the original location 
    // (indicating they intend to move the object, not click), then always process the 
    // motion notify coordinates as given (no snapping back to origin)
    pen_within_tolerance = false; 

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
    NR::Point p = sp_desktop_w2d_xy_point(dt, event_w);

    /* Test, whether we hit any anchor */
    SPDrawAnchor *anchor = spdc_test_inside(pc, event_w);

    switch (pc->mode) {
        case SP_PEN_CONTEXT_MODE_CLICK:
            switch (pc->state) {
                case SP_PEN_CONTEXT_POINT:
                    if ( pc->npoints != 0 ) {
                        /* Only set point, if we are already appending */
                        spdc_endpoint_snap(pc, p, mevent.state);
                        spdc_pen_set_subsequent_point(pc, p);
                        ret = TRUE;
                    }
                    break;
                case SP_PEN_CONTEXT_CONTROL:
                case SP_PEN_CONTEXT_CLOSE:
                    /* Placing controls is last operation in CLOSE state */
                    spdc_endpoint_snap(pc, p, mevent.state);
                    spdc_pen_set_ctrl(pc, p, mevent.state);
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
                    if ( pc->npoints > 0 ) {
                        /* Only set point, if we are already appending */

                        if (!anchor) {   /* Snap node only if not hitting anchor */
                            spdc_endpoint_snap(pc, p, mevent.state);
                        }

                        spdc_pen_set_subsequent_point(pc, p);
                        ret = TRUE;
                    }
                    break;
                case SP_PEN_CONTEXT_CONTROL:
                case SP_PEN_CONTEXT_CLOSE:
                    /* Placing controls is last operation in CLOSE state */

                    // snap the handle
                    spdc_endpoint_snap_handle(pc, p, mevent.state);

                    spdc_pen_set_ctrl(pc, p, mevent.state);
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
    return ret;
}

static gint
pen_handle_button_release(SPPenContext *const pc, GdkEventButton const &revent)
{
    gint ret = FALSE;
    if ( revent.button == 1 ) {
        NR::Point const event_w(revent.x,
                                revent.y);
        /* Find desktop coordinates */
        NR::Point p = sp_desktop_w2d_xy_point(pc->desktop, event_w);

        /* Test whether we hit any anchor. */
        SPDrawAnchor *anchor = spdc_test_inside(pc, event_w);

        switch (pc->mode) {
            case SP_PEN_CONTEXT_MODE_CLICK:
                switch (pc->state) {
                    case SP_PEN_CONTEXT_POINT:
                        if ( pc->npoints == 0 ) {
                            /* Start new thread only with button release */
                            if (anchor) {
                                p = anchor->dp;
                            }
                            pc->sa = anchor;
                            spdc_endpoint_snap(pc, p, revent.state);
                            spdc_pen_set_initial_point(pc, p);
                        } else {
                            /* Set end anchor here */
                            pc->ea = anchor;
                            if (anchor) {
                                p = anchor->dp;
                            }
                        }
                        pc->state = SP_PEN_CONTEXT_CONTROL;
                        ret = TRUE;
                        break;
                    case SP_PEN_CONTEXT_CONTROL:
                        /* End current segment */
                        spdc_endpoint_snap(pc, p, revent.state);
                        spdc_pen_finish_segment(pc, p, revent.state);
                        pc->state = SP_PEN_CONTEXT_POINT;
                        ret = TRUE;
                        break;
                    case SP_PEN_CONTEXT_CLOSE:
                        /* End current segment */
                        if (!anchor) {   /* Snap node only if not hitting anchor */
                            spdc_endpoint_snap(pc, p, revent.state);
                        }
                        spdc_pen_finish_segment(pc, p, revent.state);
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
                        spdc_endpoint_snap(pc, p, revent.state);
                        spdc_pen_finish_segment(pc, p, revent.state);
                        break;
                    case SP_PEN_CONTEXT_CLOSE:
                        spdc_endpoint_snap(pc, p, revent.state);
                        spdc_pen_finish_segment(pc, p, revent.state);
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

        if (pc->grab) {
            /* Release grab now */
            sp_canvas_item_ungrab(pc->grab, revent.time);
            pc->grab = NULL;
        }

        ret = TRUE;
    }
    return ret;
}

static gint
pen_handle_2button_press(SPPenContext *const pc)
{
    gint ret = FALSE;
    if (pc->npoints != 0) {
        spdc_pen_finish(pc, FALSE);
        ret = TRUE;
    }
    return ret;
}

static gint
pen_handle_key_press(SPPenContext *const pc, guint const keyval)
{
    gint ret = FALSE;
    /* fixme: */
    switch (keyval) {
        case GDK_Return:
            if (pc->npoints != 0) {
                spdc_pen_finish(pc, FALSE);
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            pc->state = SP_PEN_CONTEXT_STOP;
            spdc_reset_colors(pc);
            sp_canvas_item_hide(pc->c0);
            sp_canvas_item_hide(pc->c1);
            sp_canvas_item_hide(pc->cl0);
            sp_canvas_item_hide(pc->cl1);
            ret = TRUE;
            break;
        case GDK_BackSpace:
            if (sp_curve_is_empty(pc->green_curve)) {
                /* Same as cancel */
                pc->state = SP_PEN_CONTEXT_STOP;
                spdc_reset_colors(pc);
                sp_canvas_item_hide(pc->c0);
                sp_canvas_item_hide(pc->c1);
                sp_canvas_item_hide(pc->cl0);
                sp_canvas_item_hide(pc->cl1);
                ret = TRUE;
                break;
            } else {
                /* Reset red curve */
                sp_curve_reset(pc->red_curve);
                /* Destroy topmost green bpath */
                gtk_object_destroy(GTK_OBJECT(pc->green_bpaths->data));
                pc->green_bpaths = g_slist_remove(pc->green_bpaths, pc->green_bpaths->data);

                /* Get last segment */
                NArtBpath const *const p = SP_CURVE_BPATH(pc->green_curve);
                gint const e = SP_CURVE_LENGTH(pc->green_curve);
                if ( e < 2 ) {
                    g_warning("Green curve length is %d", e);
                    break;
                }
                pc->p[0] = p[e - 2].c(3);
                pc->p[1] = p[e - 1].c(1);
                NR::Point const pt(( pc->npoints < 4
                                     ? p[e - 1].c(3)
                                     : pc->p[3] ));
                pc->npoints = 2;
                sp_curve_backspace(pc->green_curve);
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
    return ret;
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
    dc->red_curve_is_valid = false;
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
spdc_pen_set_subsequent_point(SPPenContext *const pc, NR::Point const p)
{
    g_assert( pc->npoints != 0 );
    /* todo: Check callers to see whether 2 <= npoints is guaranteed. */

    pc->p[2] = p;
    pc->p[3] = p;
    pc->p[4] = p;
    pc->npoints = 5;
    sp_curve_reset(pc->red_curve);
    sp_curve_moveto(pc->red_curve, pc->p[0]);
    if ( (pc->onlycurves)
         || ( pc->p[1] != pc->p[0] ) )
    {
        sp_curve_curveto(pc->red_curve, pc->p[1], p, p);
    } else {
        sp_curve_lineto(pc->red_curve, p);
    }
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(pc->red_bpath), pc->red_curve);
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
        SP_CTRL(pc->c1)->moveto(dc->p[1]);
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
        SP_CTRL(pc->c0)->moveto(dc->p[2]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl0), dc->p[3], dc->p[2]);
        SP_CTRL(pc->c1)->moveto(dc->p[4]);
        sp_ctrlline_set_coords(SP_CTRLLINE(pc->cl1), dc->p[3], dc->p[4]);
    } else {
        g_warning("Something bad happened - npoints is %d", dc->npoints);
    }
}

static void
spdc_pen_finish_segment(SPPenContext *const pc, NR::Point const p, guint const state)
{
    SPDrawContext *const dc = SP_DRAW_CONTEXT(pc);

    if (!sp_curve_empty(dc->red_curve)) {
        sp_curve_append_continuous(dc->green_curve, dc->red_curve, 0.0625);
        SPCurve *curve = sp_curve_copy(dc->red_curve);
        /* fixme: */
        SPCanvasItem *cshape = sp_canvas_bpath_new(SP_DT_SKETCH(pc->desktop), curve);
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
spdc_pen_finish(SPPenContext *const pc, gboolean const closed)
{
    SPDrawContext *const dc = SP_DRAW_CONTEXT(pc);
    SPDesktop *const desktop = pc->desktop;
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Finishing pen"));

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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
