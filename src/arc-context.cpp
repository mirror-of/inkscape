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

static void sp_arc_context_class_init (SPArcContextClass *klass);
static void sp_arc_context_init (SPArcContext *arc_context);
static void sp_arc_context_dispose (GObject *object);

static gint sp_arc_context_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_arc_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

static void sp_arc_drag (SPArcContext * ec, double x, double y, guint state);
static void sp_arc_finish (SPArcContext * ec);

static SPEventContextClass * parent_class;

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

	event_context_class->root_handler = sp_arc_context_root_handler;
	event_context_class->item_handler = sp_arc_context_item_handler;
}

static void
sp_arc_context_init (SPArcContext * arc_context)
{
	SPEventContext * event_context;
	
	event_context = SP_EVENT_CONTEXT (arc_context);

	event_context->cursor_shape = cursor_arc_xpm;
	event_context->hot_x = 4;
	event_context->hot_y = 4;

	arc_context->item = NULL;
}

static void
sp_arc_context_dispose (GObject *object)
{
	SPArcContext * ac;

	ac = SP_ARC_CONTEXT (object);

	/* fixme: This is necessary because we do not grab */
	if (ac->item) sp_arc_finish (ac);

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gint
sp_arc_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event)
{
	gint ret;

	ret = FALSE;

	if (((SPEventContextClass *) parent_class)->item_handler)
		ret = ((SPEventContextClass *) parent_class)->item_handler (event_context, item, event);

	return ret;
}

static gint
sp_arc_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
	static gboolean dragging;
	SPArcContext * ac;
	gint ret;
	SPDesktop * desktop;

	desktop = event_context->desktop;
	ac = SP_ARC_CONTEXT (event_context);
	ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			NRPoint fp;
			dragging = TRUE;
			/* Position center */
			sp_desktop_w2d_xy_point (event_context->desktop, &fp,
									 (float) event->button.x, (float) event->button.y);
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
			NRPoint p;
			sp_desktop_w2d_xy_point (event_context->desktop, &p,
					(float) event->motion.x, (float) event->motion.y);
			sp_arc_drag (ac, p.x, p.y, event->motion.state);
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			dragging = FALSE;
			sp_arc_finish (ac);
			ret = TRUE;
		}
		sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
		break;
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_Up: 
		case GDK_Down: 
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
		if (((SPEventContextClass *) parent_class)->root_handler)
			ret = ((SPEventContextClass *) parent_class)->root_handler (event_context, event);
	}

	return ret;
}

static void
sp_arc_drag (SPArcContext * ac, double xx, double yy, guint state)
{
	NR::Point p0, p1;
	const NR::Point pt(xx, yy);

	SPDesktop * desktop = SP_EVENT_CONTEXT (ac)->desktop;

	if (!ac->item) {
		SPRepr * repr, * style;
		SPCSSAttr * css;
		/* Create object */
		repr = sp_repr_new ("path");
		sp_repr_set_attr (repr, "sodipodi:type", "arc");
		/* Set style */
		style = inkscape_get_repr (INKSCAPE, "tools.shapes.arc");
		if (style) {
			css = sp_repr_css_attr_inherited (style, "style");
			sp_repr_css_set (repr, css, "style");
			sp_repr_css_attr_unref (css);
		}
		ac->item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
	}

	/* This is bit ugly, but so we are */

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
		const NR::Coord p0h = sp_desktop_horizontal_snap (desktop, &p0);
		const NR::Coord p0v = sp_desktop_vertical_snap (desktop, &p0);
		const NR::Coord p1h = sp_desktop_horizontal_snap (desktop, &p1);
		const NR::Coord p1v = sp_desktop_vertical_snap (desktop, &p1);
		if (p0h < p1h) {
			/* Use Point 0 horizontal position */
			p1[NR::X] = 2 * ac->center[NR::X] - p0[NR::X];
		} else {
			p0[NR::X] = 2 * ac->center[NR::X] - p1[NR::X];
		}
		if (p0v < p1v) {
			/* Use Point 0 vertical position */
			p1[NR::Y] = 2 * ac->center[NR::Y] - p0[NR::Y];
		} else {
			p0[NR::Y] = 2 * ac->center[NR::Y] - p1[NR::Y];
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
	GString * xs, * ys;
	gchar status[80];
	xs = SP_PT_TO_METRIC_STRING (fabs(x1-x0), SP_DEFAULT_METRIC);
	ys = SP_PT_TO_METRIC_STRING (fabs(y1-y0), SP_DEFAULT_METRIC);
	sprintf (status, "Draw arc  %s x %s", xs->str, ys->str);
	sp_view_set_status (SP_VIEW (desktop), status, FALSE);
	g_string_free (xs, FALSE);
	g_string_free (ys, FALSE);
}

static void
sp_arc_finish (SPArcContext * ac)
{
	if (ac->item != NULL) {
		SPDesktop *desktop = SP_EVENT_CONTEXT (ac)->desktop;

		sp_object_invoke_write (SP_OBJECT (ac->item), SP_OBJECT_REPR (ac->item), SP_OBJECT_WRITE_EXT);

		sp_selection_set_item (SP_DT_SELECTION (desktop), ac->item);
		sp_document_done (SP_DT_DOCUMENT (desktop));

		ac->item = NULL;
	}
}

