#define __SP_RECT_CONTEXT_C__

/*
 * Rectangle drawing context
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

#include "config.h"

#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-rect.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "pixmaps/cursor-rect.xpm"
#include "rect-context.h"
#include "sp-metrics.h"
#include "helper/sp-intl.h"
#include "object-edit.h"
#include "knotholder.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "prefs-utils.h"
#include "widgets/spw-utilities.h"
#include "selcue.h"

static void sp_rect_context_class_init(SPRectContextClass *klass);
static void sp_rect_context_init(SPRectContext *rect_context);
static void sp_rect_context_dispose(GObject *object);

static void sp_rect_context_setup(SPEventContext *ec);
static void sp_rect_context_set(SPEventContext *ec, gchar const *key, gchar const *val);

static gint sp_rect_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_rect_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_rect_drag(SPRectContext &rc, NR::Point const pt, guint state);
static void sp_rect_finish(SPRectContext *rc);

static SPEventContextClass *parent_class;


GtkType sp_rect_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPRectContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_rect_context_class_init,
            NULL, NULL,
            sizeof(SPRectContext),
            4,
            (GInstanceInitFunc) sp_rect_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPRectContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_rect_context_class_init(SPRectContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_rect_context_dispose;

    event_context_class->setup = sp_rect_context_setup;
    event_context_class->set = sp_rect_context_set;
    event_context_class->root_handler  = sp_rect_context_root_handler;
    event_context_class->item_handler  = sp_rect_context_item_handler;
}

static void sp_rect_context_init(SPRectContext *rect_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(rect_context);

    event_context->cursor_shape = cursor_rect_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    rect_context->item = NULL;
    rect_context->repr = NULL;
    rect_context->knot_holder = NULL;

    rect_context->rx = 0.0;
    rect_context->ry = 0.0;

    new (&rect_context->sel_changed_connection) SigC::Connection();
}

static void sp_rect_context_dispose(GObject *object)
{
    SPRectContext *rc = SP_RECT_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection.~Connection();

    /* fixme: This is necessary because we do not grab */
    if (rc->item) {
        sp_rect_finish(rc);
    }

    if (rc->knot_holder) {
        sp_knot_holder_destroy(rc->knot_holder);
        rc->knot_holder = NULL;
    }

    if (rc->repr) { // remove old listener
        sp_repr_remove_listener_by_data(rc->repr, ec);
        sp_repr_unref(rc->repr);
        rc->repr = 0;
    }

    sp_sel_cue_shutdown(&(ec->selcue));

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void shape_event_attr_changed(SPRepr *repr,
                                     gchar const *name, gchar const *old_value, gchar const *new_value,
                                     bool const is_interactive, gpointer const data)
{
    SPRectContext *rc = SP_RECT_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    if (rc->knot_holder) {
        sp_knot_holder_destroy(rc->knot_holder);
    }
    rc->knot_holder = NULL;

    SPDesktop *desktop = ec->desktop;

    SPItem *item = SP_DT_SELECTION(desktop)->singleItem();

    if (item) {
        rc->knot_holder = sp_item_knot_holder(item, desktop);
    }
}

static SPReprEventVector shape_repr_events = {
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    shape_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void sp_rect_context_selection_changed(SPSelection *selection, gpointer data)
{
    SPRectContext *rc = SP_RECT_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    if (rc->knot_holder) { // destroy knotholder
        sp_knot_holder_destroy(rc->knot_holder);
        rc->knot_holder = NULL;
    }

    if (rc->repr) { // remove old listener
        sp_repr_remove_listener_by_data(rc->repr, ec);
        sp_repr_unref(rc->repr);
        rc->repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        rc->knot_holder = sp_item_knot_holder(item, ec->desktop);
        SPRepr *repr = SP_OBJECT_REPR(item);
        if (repr) {
            rc->repr = repr;
            sp_repr_ref(repr);
            sp_repr_add_listener(repr, &shape_repr_events, ec);
            sp_repr_synthesize_events(repr, &shape_repr_events, ec);
        }
    }
}

static void sp_rect_context_setup(SPEventContext *ec)
{
    SPRectContext *rc = SP_RECT_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    SPItem *item = SP_DT_SELECTION(ec->desktop)->singleItem();
    if (item) {
        rc->knot_holder = sp_item_knot_holder(item, ec->desktop);
        SPRepr *repr = SP_OBJECT_REPR(item);
        if (repr) {
            rc->repr = repr;
            sp_repr_ref(repr);
            sp_repr_add_listener(repr, &shape_repr_events, ec);
            sp_repr_synthesize_events(repr, &shape_repr_events, ec);
        }
    }

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection = SP_DT_SELECTION(ec->desktop)->connectChanged(
        SigC::bind(SigC::slot(&sp_rect_context_selection_changed), (gpointer)rc)
    );

    sp_event_context_read(ec, "rx");
    sp_event_context_read(ec, "ry");

    if (prefs_get_int_attribute("tools.shapes", "selcue", 0) != 0)
		sp_sel_cue_init(&(ec->selcue), ec->desktop);
}

static void sp_rect_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    SPRectContext *rc = SP_RECT_CONTEXT(ec);

    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    if ( strcmp(key, "rx") == 0 ) {
        rc->rx = ( val
                         ? g_ascii_strtod (val, NULL)
                         : 0.0 );
    } else if ( strcmp(key, "ry") == 0 ) {
        rc->ry = ( val
                         ? g_ascii_strtod (val, NULL)
                         : 0.0 );
    }
}

static gint sp_rect_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {

            // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point(desktop, NR::Point(event->button.x, event->button.y), TRUE);

            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_rect_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;

    SPDesktop *desktop = event_context->desktop;
    SPRectContext *rc = SP_RECT_CONTEXT(event_context);

    event_context->tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100);

    gint ret = FALSE;
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {
            NR::Point const button_w(event->button.x,
                                     event->button.y);

            // save drag origin
            event_context->xp = (gint) button_w[NR::X];
            event_context->yp = (gint) button_w[NR::Y];
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point(desktop, button_w, TRUE);

            dragging = true;
            /* Position center */
            NR::Point const button_dt(sp_desktop_w2d_xy_point(event_context->desktop, button_w));
            /* Snap center to nearest magnetic point */
            rc->center = button_dt;
            sp_desktop_free_snap(event_context->desktop, rc->center);
            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                ( GDK_BUTTON_RELEASE_MASK       |
                                  GDK_POINTER_MOTION_MASK       |
                                  GDK_POINTER_MOTION_HINT_MASK  |
                                  GDK_BUTTON_PRESS_MASK ),
                                NULL, event->button.time);
            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && ( event->motion.state & GDK_BUTTON1_MASK ) )
        {
            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            event_context->within_tolerance = false;

            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point const motion_dt(sp_desktop_w2d_xy_point(event_context->desktop, motion_w));
            sp_rect_drag(*rc, motion_dt, event->motion.state);
            ret = TRUE;
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if ( event->button.button == 1 ) {
            dragging = false;

            if (!event_context->within_tolerance) {
                // we've been dragging, finish the rect
                sp_rect_finish(rc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                SP_DT_SELECTION(desktop)->setItem(event_context->item_to_select);
            } else {
                // click in an empty space
                SP_DT_SELECTION(desktop)->clear();
            }

            event_context->item_to_select = NULL;
            ret = TRUE;
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                  event->button.time);
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

        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                gpointer hb = sp_search_by_data_recursive (desktop->owner->aux_toolbox, (gpointer) "altx-rect");
                if (hb && GTK_IS_WIDGET(hb)) {
                    gtk_widget_grab_focus (GTK_WIDGET (hb));
                }
                ret = TRUE;
            }
            break;

        case GDK_Escape:
            SP_DT_SELECTION(desktop)->clear();
            //TODO: make dragging escapable by Esc
        default:
            break;
        }
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

static void sp_rect_drag(SPRectContext &rc, NR::Point const pt, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(&rc)->desktop;

    if (!rc.item) {
        /* Create object */
        SPRepr *repr = sp_repr_new("rect");
        /* Set style */
        SPRepr *style = inkscape_get_repr(INKSCAPE, "tools.shapes.rect");
        if (style) {
            SPCSSAttr *css = sp_repr_css_attr_inherited(style, "style");
            sp_repr_css_set(repr, css, "style");
            sp_repr_css_attr_unref(css);
        }
        rc.item = (SPItem *) sp_document_add_repr(SP_DT_DOCUMENT(desktop), repr);
        sp_repr_unref(repr);
    }

    NR::Point p0, p1;
    if ( state & GDK_CONTROL_MASK ) {
        NR::Point delta = pt - rc.center;
        /* fixme: Snapping */
        if ( ( fabs(delta[0]) > fabs(delta[1]) )
             && ( delta[1] != 0.0) )
        {
            delta[0] = floor( delta[0] / delta[1] + 0.5 ) * delta[1];
        } else if ( delta[0] != 0.0 ) {
            delta[1] = floor( delta[1] / delta[0] + 0.5 ) * delta[0];
        }
        p1 = rc.center + delta;
        if ( state & GDK_SHIFT_MASK ) {
            p0 = rc.center - delta;
            NR::Coord const l0 = sp_desktop_vector_snap(desktop, p0, p0 - p1);
            NR::Coord const l1 = sp_desktop_vector_snap(desktop, p1, p1 - p0);

            if (l0 < l1) {
                p1 = 2 * rc.center - p0;
            } else {
                p0 = 2 * rc.center - p1;
            }
        } else {
            p0 = rc.center;
            sp_desktop_vector_snap(desktop, p1,
                                   p1 - p0);
        }
    } else if ( state & GDK_SHIFT_MASK ) {
        /* Corner point movements are bound */
        p1 = pt;
        p0 = 2 * rc.center - p1;
        for (unsigned d = 0 ; d < 2 ; ++d) {
            double snap_movement[2];
            snap_movement[0] = sp_desktop_dim_snap(desktop, p0, NR::Dim2(d));
            snap_movement[1] = sp_desktop_dim_snap(desktop, p1, NR::Dim2(d));
            if ( snap_movement[0] <
                 snap_movement[1] )
            {
                /* Use point 0 position. */
                p1[d] = 2 * rc.center[d] - p0[d];
            } else {
                p0[d] = 2 * rc.center[d] - p1[d];
            }
        }
    } else {
        /* Free movement for corner point */
        p0 = rc.center;
        p1 = pt;
        sp_desktop_free_snap(desktop, p1);
    }

    p0 = sp_desktop_dt2root_xy_point(desktop, p0);
    p1 = sp_desktop_dt2root_xy_point(desktop, p1);

    // TODO: use NR::Rect
    using NR::X;
    using NR::Y;
    NR::Coord const x0 = MIN(p0[X], p1[X]);
    NR::Coord const y0 = MIN(p0[Y], p1[Y]);
    NR::Coord const x1 = MAX(p0[X], p1[X]);
    NR::Coord const y1 = MAX(p0[Y], p1[Y]);
    NR::Coord const w  = x1 - x0;
    NR::Coord const h  = y1 - y0;

    sp_rect_position_set(SP_RECT(rc.item), x0, y0, w, h);
    if ( rc.rx != 0.0 ) {
        sp_rect_set_rx (SP_RECT(rc.item), TRUE, rc.rx);
    }
    if ( rc.ry != 0.0 ) {
        if (rc.rx == 0.0)
            sp_rect_set_ry (SP_RECT(rc.item), TRUE, CLAMP(rc.ry, 0, MIN(w, h)/2));
        else 
            sp_rect_set_ry (SP_RECT(rc.item), TRUE, CLAMP(rc.ry, 0, h));
    }

    // status text
    gchar status[80];
    GString *xs = SP_PT_TO_METRIC_STRING(fabs( x1 - x0 ), SP_DEFAULT_METRIC);
    GString *ys = SP_PT_TO_METRIC_STRING(fabs( y1 - y0 ), SP_DEFAULT_METRIC);
    g_snprintf(status, 80, _("Draw rectangle: %s x %s"), xs->str, ys->str);
    sp_view_set_status(SP_VIEW(desktop), status, FALSE);
    /* FIXME: I'd guess that arg2 should be TRUE below, that otherwise we'd have mem leak.
     * Check with valgrind. */
    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);
}

static void sp_rect_finish(SPRectContext *rc)
{
    if ( rc->item != NULL ) {
        SPDesktop * dt;

        dt = SP_EVENT_CONTEXT_DESKTOP(rc);

        SP_OBJECT(rc->item)->updateRepr();

        SP_DT_SELECTION(dt)->setItem(rc->item);
        sp_document_done(SP_DT_DOCUMENT(dt));

        rc->item = NULL;
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
