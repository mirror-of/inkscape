#define __SP_ARC_CONTEXT_C__

/*
 * Ellipse drawing context
 *
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Mitsuru Oka
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "helper/sp-canvas.h"
#include "inkscape.h"
#include "sp-ellipse.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "pixmaps/cursor-arc.xpm"
#include "arc-context.h"
#include "sp-metrics.h"
#include "knotholder.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "object-edit.h"
#include "prefs-utils.h"

static void sp_arc_context_class_init (SPArcContextClass *klass);
static void sp_arc_context_init (SPArcContext *arc_context);
static void sp_arc_context_dispose (GObject *object);

static void sp_arc_context_setup (SPEventContext *ec);
static gint sp_arc_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_arc_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_arc_drag(SPArcContext *ec, NR::Point pt, guint state);
static void sp_arc_finish(SPArcContext *ec);

static SPEventContextClass *parent_class;

GtkType
sp_arc_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPArcContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_arc_context_class_init,
			NULL, NULL,
			sizeof (SPArcContext),
			4,
			(GInstanceInitFunc) sp_arc_context_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPArcContext", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_arc_context_class_init (SPArcContextClass *klass)
{
	GObjectClass *object_class;
	SPEventContextClass *event_context_class;

	object_class = (GObjectClass *) klass;
	event_context_class = (SPEventContextClass *) klass;

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

	object_class->dispose = sp_arc_context_dispose;

    event_context_class->setup = sp_arc_context_setup;
	event_context_class->root_handler = sp_arc_context_root_handler;
	event_context_class->item_handler = sp_arc_context_item_handler;
}

static void sp_arc_context_init(SPArcContext *arc_context)
{
	SPEventContext *event_context = SP_EVENT_CONTEXT(arc_context);

	event_context->cursor_shape = cursor_arc_xpm;
	event_context->hot_x = 4;
	event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

	arc_context->item = NULL;
}

static void sp_arc_context_dispose(GObject *object)
{
    SPEventContext *ec = SP_EVENT_CONTEXT (object);
	SPArcContext *ac = SP_ARC_CONTEXT(object);

    if (ac->knot_holder) {
        sp_knot_holder_destroy (ac->knot_holder);
        ac->knot_holder = NULL;
    }

    if (ac->repr) { // remove old listener
        sp_repr_remove_listener_by_data (ac->repr, ec);
        sp_repr_unref (ac->repr);
        ac->repr = 0;
    }

    if (SP_EVENT_CONTEXT_DESKTOP (ac)) {
        sp_signal_disconnect_by_data (SP_DT_SELECTION (SP_EVENT_CONTEXT_DESKTOP (ac)), ac);
    }

	/* fixme: This is necessary because we do not grab */
	if (ac->item) sp_arc_finish (ac);

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void shape_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data)
{
    SPArcContext *ac;
    SPEventContext *ec;

    ac = SP_ARC_CONTEXT (data);
    ec = SP_EVENT_CONTEXT (ac);

    if (ac->knot_holder) {
        sp_knot_holder_destroy (ac->knot_holder);
    }
    ac->knot_holder = NULL;

    SPDesktop *desktop = ec->desktop;

    SPItem *item = sp_selection_item (SP_DT_SELECTION(desktop));

    if (item) {
        ac->knot_holder = sp_item_knot_holder (item, desktop);
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
void
sp_arc_context_selection_changed (SPSelection * selection, gpointer data)
{
    SPArcContext *ac;
    SPEventContext *ec;

    ac = SP_ARC_CONTEXT (data);
    ec = SP_EVENT_CONTEXT (ac);

    if (ac->knot_holder) { // desktroy knotholder
        sp_knot_holder_destroy (ac->knot_holder);
        ac->knot_holder = NULL;
    }

    if (ac->repr) { // remove old listener
        sp_repr_remove_listener_by_data (ac->repr, ec);
        sp_repr_unref (ac->repr);
        ac->repr = 0;
    }

    SPItem *item = sp_selection_item (selection);
    if (item) {
        ac->knot_holder = sp_item_knot_holder (item, ec->desktop);
        SPRepr *repr = SP_OBJECT_REPR (item);
        if (repr) {
            ac->repr = repr;
            sp_repr_ref (repr);
            sp_repr_add_listener (repr, &shape_repr_events, ec);
            sp_repr_synthesize_events (repr, &shape_repr_events, ec);
        }


    }
}

static void
sp_arc_context_setup (SPEventContext *ec)
{
   SPArcContext *ac = SP_ARC_CONTEXT (ec);

   if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup (ec);

   SPItem *item = sp_selection_item (SP_DT_SELECTION (ec->desktop));
        if (item) {
            ac->knot_holder = sp_item_knot_holder (item, ec->desktop);
            SPRepr *repr = SP_OBJECT_REPR (item);
            if (repr) {
                ac->repr = repr;
                sp_repr_ref (repr);
                sp_repr_add_listener (repr, &shape_repr_events, ec);
                sp_repr_synthesize_events (repr, &shape_repr_events, ec);
            }
        }
        g_signal_connect (G_OBJECT (SP_DT_SELECTION (ec->desktop)),
            "changed", G_CALLBACK (sp_arc_context_selection_changed), ac);

}


static gint sp_arc_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;
	gint ret;

	ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1) {

            // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

            // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);

            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

	if (((SPEventContextClass *) parent_class)->item_handler)
		ret = ((SPEventContextClass *) parent_class)->item_handler (event_context, item, event);

	return ret;
}

static gint sp_arc_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
	static gboolean dragging;

	SPDesktop *desktop = event_context->desktop;
	SPArcContext *ac = SP_ARC_CONTEXT (event_context);

    event_context->tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);

	gint ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
           // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

           // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);

			dragging = TRUE;
			/* Position center */
			ac->center = sp_desktop_w2d_xy_point (event_context->desktop, 
													NR::Point(event->button.x, event->button.y));
			/* Snap center to nearest magnetic point */
			sp_desktop_free_snap (event_context->desktop, ac->center);
			sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
						GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging && event->motion.state && GDK_BUTTON1_MASK) {

			if ( event_context->within_tolerance
					 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
					 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
				break; // do not drag if we're within tolerance from origin
			}
			// Once the user has moved farther than tolerance from the original location
			// (indicating they intend to draw, not click), then always process the
			// motion notify coordinates as given (no snapping back to origin)
			event_context->within_tolerance = false;

			NR::Point const motion_w(event->motion.x, event->motion.y);
			NR::Point const motion_dt(sp_desktop_w2d_xy_point(event_context->desktop, motion_w));
			sp_arc_drag (ac, motion_dt, event->motion.state);
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
		if (event->button.button == 1) {
			dragging = FALSE;
            if (!event_context->within_tolerance) {
                // we've been dragging, finish the arc
			sp_arc_finish (ac);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                sp_selection_set_item (SP_DT_SELECTION (desktop), event_context->item_to_select);
            } else {
                // click in an empty space
                sp_selection_empty (SP_DT_SELECTION (desktop));
            }
            event_context->xp = 0;
            event_context->yp = 0;
            event_context->item_to_select = NULL;
			ret = TRUE;
		}
		sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
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
        case GDK_Escape:
            sp_selection_empty (SP_DT_SELECTION (desktop)); // deselect
            //TODO: make dragging escapable by Esc
		default:
			break;
		}
	default:
		break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->root_handler)
			ret = ((SPEventContextClass *) parent_class)->root_handler (event_context, event);
	}

	return ret;
}

static void sp_arc_drag(SPArcContext *ac, NR::Point pt, guint state)
{
	SPDesktop *desktop = SP_EVENT_CONTEXT(ac)->desktop;

	if (!ac->item) {
		/* Create object */
		SPRepr *repr = sp_repr_new("path");
		sp_repr_set_attr (repr, "sodipodi:type", "arc");
		/* Set style */
		SPRepr *style = inkscape_get_repr(INKSCAPE, "tools.shapes.arc");
		if (style) {
			SPCSSAttr *css = sp_repr_css_attr_inherited(style, "style");
			sp_repr_css_set (repr, css, "style");
			sp_repr_css_attr_unref (css);
		}
		ac->item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
	}

	/* This is bit ugly, but so we are */

	NR::Point p0, p1;
	if (state & GDK_CONTROL_MASK) {
		NR::Point delta = pt - ac->center;
		/* fixme: Snapping */
		if ((fabs (delta[0]) > fabs (delta[1])) && (delta[1] != 0.0)) {
			delta[0] = floor (delta[0]/delta[1] + 0.5) * delta[1];
		} else if (delta[0] != 0.0) {
			delta[1] = floor (delta[1]/delta[0] + 0.5) * delta[0];
		}
		p1 = ac->center + delta;
		if (state & GDK_SHIFT_MASK) {
			p0 = ac->center - delta;
			const NR::Coord l0 = sp_desktop_vector_snap (desktop, p0, p0 - p1);
			const NR::Coord l1 = sp_desktop_vector_snap (desktop, p1, p1 - p0);
			
			if (l0 < l1) {
				p1 = 2 * ac->center - p0;
			} else {
				p0 = 2 * ac->center - p1;
			}
		} else {
			p0 = ac->center;
			sp_desktop_vector_snap (desktop, p1, 
									p1 - p0);
		}
	} else if (state & GDK_SHIFT_MASK) {
		/* Corner point movements are bound */
		p1 = pt;
		p0 = 2 * ac->center - p1;
		for (unsigned d = 0 ; d < 2 ; ++d) {
			double snap_movement[2];
			snap_movement[0] = sp_desktop_dim_snap(desktop, p0, d);
			snap_movement[1] = sp_desktop_dim_snap(desktop, p1, d);
			if ( snap_movement[0] <
			     snap_movement[1] ) {
				/* Use point 0 position. */
				p1[d] = 2 * ac->center[d] - p0[d];
			} else {
				p0[d] = 2 * ac->center[d] - p1[d];
			}
		}
	} else {
		/* Free movement for corner point */
		p0 = ac->center;
		p1 = pt;
		sp_desktop_free_snap (desktop, p1);
	}

	p0 = sp_desktop_dt2root_xy_point (desktop, p0);
	p1 = sp_desktop_dt2root_xy_point (desktop, p1);
	
	// FIXME: use NR::Rect
	const NR::Coord x0 = MIN (p0[NR::X], p1[NR::X]);
	const NR::Coord y0 = MIN (p0[NR::Y], p1[NR::Y]);
	const NR::Coord x1 = MAX (p0[NR::X], p1[NR::X]);
	const NR::Coord y1 = MAX (p0[NR::Y], p1[NR::Y]);

	sp_arc_position_set (SP_ARC (ac->item), (x0 + x1) / 2, (y0 + y1) / 2, (x1 - x0) / 2, (y1 - y0) / 2);

	// status text
	gchar status[80];
	GString *xs = SP_PT_TO_METRIC_STRING (fabs(x1-x0), SP_DEFAULT_METRIC);
	GString *ys = SP_PT_TO_METRIC_STRING (fabs(y1-y0), SP_DEFAULT_METRIC);
	sprintf (status, "Draw arc  %s x %s", xs->str, ys->str);
	sp_view_set_status (SP_VIEW (desktop), status, FALSE);
	g_string_free (xs, FALSE);
	g_string_free (ys, FALSE);
	/* FIXME: The above looks like a memory leak: I think arg2 should be TRUE. */
}

static void sp_arc_finish(SPArcContext *ac)
{
	if (ac->item != NULL) {
		SPDesktop *desktop = SP_EVENT_CONTEXT (ac)->desktop;

		sp_object_invoke_write (SP_OBJECT (ac->item), SP_OBJECT_REPR (ac->item), SP_OBJECT_WRITE_EXT);

		sp_selection_set_item (SP_DT_SELECTION (desktop), ac->item);
		sp_document_done (SP_DT_DOCUMENT (desktop));

		ac->item = NULL;
	}
}

