#define __SP_KNOT_C__

/*
 * Desktop-bound visual control object
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <glib-object.h>
#include <gtk/gtksignal.h>
#include "helper/sp-marshal.h"
#include "helper/sp-canvas-util.h"
#include "helper/sodipodi-ctrl.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-events.h"
#include "desktop-affine.h"
#include "knot.h"

#include "sp-guide.h"

#define KNOT_EVENT_MASK (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | \
			 GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | \
			 GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK)

#define hypot(a,b) sqrt ((a) * (a) + (b) * (b))

static int nograb = FALSE;

enum {
	PROP_0,

	PROP_SIZE,
	PROP_ANCHOR,
	PROP_SHAPE,
	PROP_MODE,
	PROP_FILL, PROP_FILL_MOUSEOVER, PROP_FILL_DRAGGING,
	PROP_STROKE, PROP_STROKE_MOUSEOVER, PROP_STROKE_DRAGGING,
	PROP_IMAGE, PROP_IMAGE_MOUSEOVER, PROP_IMAGE_DRAGGING,
	PROP_CURSOR, PROP_CURSOR_MOUSEOVER, PROP_CURSOR_DRAGGING,
	PROP_PIXBUF,

	PROP_LAST
};

enum {
	EVENT,
	CLICKED,
	GRABBED,
	UNGRABBED,
	MOVED,
	REQUEST,
	DISTANCE,
	LAST_SIGNAL
};

static void sp_knot_class_init (SPKnotClass * klass);
static void sp_knot_init (SPKnot * knot);
static void sp_knot_dispose (GObject * object);
static void sp_knot_set_property (GObject * object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void sp_knot_get_property (GObject * object, guint prop_id, GValue *value, GParamSpec *pspec);

static int sp_knot_handler (SPCanvasItem *item, GdkEvent *event, SPKnot *knot);
static void sp_knot_set_flag (SPKnot * knot, guint flag, gboolean set);
static void sp_knot_update_ctrl (SPKnot * knot);
static void sp_knot_set_ctrl_state (SPKnot *knot);

static GObjectClass *parent_class;
static guint knot_signals[LAST_SIGNAL] = {0};

GType
sp_knot_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPKnotClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_knot_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPKnot),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_knot_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPKnot", &info, 0);
	}
	return type;
}

static void
sp_knot_class_init (SPKnotClass * klass)
{
	GObjectClass * object_class;
	const char *nograbenv;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = sp_knot_dispose;
	object_class->set_property = sp_knot_set_property;
	object_class->get_property = sp_knot_get_property;

	/* Huh :) */

	g_object_class_install_property (object_class,
					 PROP_SIZE,
					 g_param_spec_uint ("size", "Size", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_ANCHOR,
					 g_param_spec_enum ("anchor", "Anchor", "",
							    GTK_TYPE_ANCHOR_TYPE,
							    GTK_ANCHOR_CENTER,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_SHAPE,
					 g_param_spec_int ("shape", "Shape", "",
							   SP_KNOT_SHAPE_SQUARE,
							   SP_KNOT_SHAPE_IMAGE,
							   SP_KNOT_SHAPE_SQUARE,
							   G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_MODE,
					 g_param_spec_int ("mode", "Mode", "",
							   SP_KNOT_MODE_COLOR,
							   SP_KNOT_MODE_XOR,
							   SP_KNOT_MODE_COLOR,
							   G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_FILL,
					 g_param_spec_uint ("fill", "Fill", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_FILL_MOUSEOVER,
					 g_param_spec_uint ("fill_mouseover", "Fill mouse over", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_FILL_DRAGGING,
					 g_param_spec_uint ("fill_dragging", "Fill dragging", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_STROKE,
					 g_param_spec_uint ("stroke", "Stroke", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_STROKE_MOUSEOVER,
					 g_param_spec_uint ("stroke_mouseover", "Stroke mouseover", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_STROKE_DRAGGING,
					 g_param_spec_uint ("stroke_dragging", "Stroke dragging", "",
							    0,
							    0xffffffff,
							    0xff000000,
							    G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_IMAGE,
					 g_param_spec_pointer ("image", "Image", "",
							       G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_IMAGE_MOUSEOVER,
					 g_param_spec_pointer ("image_mouseover", "Image mouseover", "",
							       G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_IMAGE_DRAGGING,
					 g_param_spec_pointer ("image_dragging", "Image dragging", "",
							       G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_CURSOR,
					 g_param_spec_boxed ("cursor", "Cursor", "",
							     GDK_TYPE_CURSOR,
							     G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_CURSOR_MOUSEOVER,
					 g_param_spec_boxed ("cursor_mouseover", "Cursor mouseover", "",
							     GDK_TYPE_CURSOR,
							     G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_CURSOR_DRAGGING,
					 g_param_spec_boxed ("cursor_dragging", "Cursor dragging", "",
							     GDK_TYPE_CURSOR,
							     G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_PIXBUF,
					 g_param_spec_pointer ("pixbuf", "Pixbuf", "",
							       G_PARAM_READWRITE));

	knot_signals[EVENT] = g_signal_new ("event",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_LAST,
					    G_STRUCT_OFFSET (SPKnotClass, event),
					    NULL, NULL,
					    sp_marshal_BOOLEAN__POINTER,
					    G_TYPE_BOOLEAN, 1,
					    GDK_TYPE_EVENT);
	knot_signals[CLICKED] = g_signal_new ("clicked",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPKnotClass, clicked),
					    NULL, NULL,
					    sp_marshal_NONE__UINT,
					    G_TYPE_NONE, 1,
					    G_TYPE_UINT);
	knot_signals[GRABBED] = g_signal_new ("grabbed",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPKnotClass, grabbed),
					    NULL, NULL,
					    sp_marshal_NONE__UINT,
					    G_TYPE_NONE, 1,
					    G_TYPE_UINT);
	knot_signals[UNGRABBED] = g_signal_new ("ungrabbed",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPKnotClass, ungrabbed),
					    NULL, NULL,
					    sp_marshal_NONE__UINT,
					    G_TYPE_NONE, 1,
					    G_TYPE_UINT);
	knot_signals[MOVED] = g_signal_new ("moved",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET (SPKnotClass, moved),
					    NULL, NULL,
					    sp_marshal_NONE__POINTER_UINT,
					    G_TYPE_NONE, 2,
					    G_TYPE_POINTER, G_TYPE_UINT);
	knot_signals[REQUEST] = g_signal_new ("request",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_LAST,
					    G_STRUCT_OFFSET (SPKnotClass, request),
					    NULL, NULL,
					    sp_marshal_BOOLEAN__POINTER_UINT,
					    G_TYPE_BOOLEAN, 2,
					    G_TYPE_POINTER, G_TYPE_UINT);
	knot_signals[DISTANCE] = g_signal_new ("distance",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_LAST,
					    G_STRUCT_OFFSET (SPKnotClass, distance),
					    NULL, NULL,
					    sp_marshal_DOUBLE__POINTER_UINT,
					    G_TYPE_DOUBLE, 2,
					    G_TYPE_POINTER, G_TYPE_UINT);

	nograbenv = getenv ("SODIPODI_NO_GRAB");
	nograb = (nograbenv && *nograbenv && (*nograbenv != '0'));
}

static void
sp_knot_init (SPKnot * knot)
{
	knot->desktop = NULL;
	knot->item = NULL;
	knot->flags = 0;

	knot->size = 8;
	knot->x = knot->y = 0.0;
	knot->hx = knot->hy = 0.0;
	knot->anchor = GTK_ANCHOR_CENTER;
	knot->shape = SP_KNOT_SHAPE_SQUARE;
	knot->mode = SP_KNOT_MODE_COLOR;

	knot->fill [SP_KNOT_STATE_NORMAL] = 0x000000ff;
	knot->fill [SP_KNOT_STATE_MOUSEOVER] = 0xff0000ff;
	knot->fill [SP_KNOT_STATE_DRAGGING] = 0x0000ffff;

	knot->stroke [SP_KNOT_STATE_NORMAL] = 0x000000ff;
	knot->stroke [SP_KNOT_STATE_MOUSEOVER] = 0x000000ff;
	knot->stroke [SP_KNOT_STATE_DRAGGING] = 0x000000ff;

	knot->image [SP_KNOT_STATE_NORMAL] = NULL;
	knot->image [SP_KNOT_STATE_MOUSEOVER] = NULL;
	knot->image [SP_KNOT_STATE_DRAGGING] = NULL;

	knot->cursor [SP_KNOT_STATE_NORMAL] = NULL;
	knot->cursor [SP_KNOT_STATE_MOUSEOVER] = NULL;
	knot->cursor [SP_KNOT_STATE_DRAGGING] = NULL;

	knot->saved_cursor = NULL;
	knot->pixbuf = NULL;
}

static void
sp_knot_dispose (GObject * object)
{
	SPKnot * knot;
	gint i;

	knot = (SPKnot *) object;

	/* ungrab pointer if still grabbed by mouseover, find a different way */
	if (gdk_pointer_is_grabbed ()) gdk_pointer_ungrab (GDK_CURRENT_TIME);

	if (knot->item) {
		gtk_object_destroy (GTK_OBJECT (knot->item));
		knot->item = NULL;
	}

	for (i = 0; i < SP_KNOT_VISIBLE_STATES; i++) {
		if (knot->cursor[i]) {
			gdk_cursor_unref (knot->cursor[i]);
			knot->cursor[i] = NULL;
		}
	}

	if (((GObjectClass *) (parent_class))->dispose)
		(* ((GObjectClass *) (parent_class))->dispose) (object);
}

static void
sp_knot_set_property (GObject * object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	SPKnot * knot;
	GdkCursor * cursor;
	gint i;

	knot = SP_KNOT (object);

	switch (prop_id) {
	case PROP_SIZE:
		knot->size = g_value_get_uint (value);
		break;
	case PROP_ANCHOR:
		knot->anchor = g_value_get_enum (value);
		break;
	case PROP_SHAPE:
		knot->shape = g_value_get_int (value);
		break;
	case PROP_MODE:
		knot->mode = g_value_get_int (value);
		break;
	case PROP_FILL:
		knot->fill[SP_KNOT_STATE_NORMAL] =
		knot->fill[SP_KNOT_STATE_MOUSEOVER] =
		knot->fill[SP_KNOT_STATE_DRAGGING] = g_value_get_uint (value);
		break;
	case PROP_FILL_MOUSEOVER:
		knot->fill[SP_KNOT_STATE_MOUSEOVER] = 
		knot->fill[SP_KNOT_STATE_DRAGGING] = g_value_get_uint (value);
		break;
	case PROP_FILL_DRAGGING:
		knot->fill[SP_KNOT_STATE_DRAGGING] = g_value_get_uint (value);
		break;
	case PROP_STROKE:
		knot->stroke[SP_KNOT_STATE_NORMAL] =
		knot->stroke[SP_KNOT_STATE_MOUSEOVER] =
		knot->stroke[SP_KNOT_STATE_DRAGGING] = g_value_get_uint (value);
		break;
	case PROP_STROKE_MOUSEOVER:
		knot->stroke[SP_KNOT_STATE_MOUSEOVER] = 
		knot->stroke[SP_KNOT_STATE_DRAGGING] = g_value_get_uint (value);
		break;
	case PROP_STROKE_DRAGGING:
		knot->stroke[SP_KNOT_STATE_DRAGGING] = g_value_get_uint (value);
		break;
	case PROP_IMAGE:
		knot->image[SP_KNOT_STATE_NORMAL] =
		knot->image[SP_KNOT_STATE_MOUSEOVER] =
		knot->image[SP_KNOT_STATE_DRAGGING] = g_value_get_pointer (value);
		break;
	case PROP_IMAGE_MOUSEOVER:
		knot->image[SP_KNOT_STATE_MOUSEOVER] = g_value_get_pointer (value);
		break;
	case PROP_IMAGE_DRAGGING:
		knot->image[SP_KNOT_STATE_DRAGGING] = g_value_get_pointer (value);
		break;
	case PROP_CURSOR:
		cursor = g_value_get_boxed (value);
		for (i = 0; i < SP_KNOT_VISIBLE_STATES; i++) {
			if (knot->cursor[i]) gdk_cursor_unref (knot->cursor[i]);
			knot->cursor[i] = cursor;
			if (cursor) gdk_cursor_ref (cursor);
		}
		break;
	case PROP_CURSOR_MOUSEOVER:
		cursor = g_value_get_boxed (value);
		if (knot->cursor[SP_KNOT_STATE_MOUSEOVER]) {
			gdk_cursor_unref (knot->cursor[SP_KNOT_STATE_MOUSEOVER]);
		}
		knot->cursor[SP_KNOT_STATE_MOUSEOVER] = cursor;
		if (cursor) gdk_cursor_ref (cursor);
		break;
	case PROP_CURSOR_DRAGGING:
		cursor = g_value_get_boxed (value);
		if (knot->cursor[SP_KNOT_STATE_DRAGGING]) {
			gdk_cursor_unref (knot->cursor[SP_KNOT_STATE_DRAGGING]);
		}
		knot->cursor[SP_KNOT_STATE_DRAGGING] = cursor;
		if (cursor) gdk_cursor_ref (cursor);
		break;
	case PROP_PIXBUF:
	        knot->pixbuf = g_value_get_pointer (value);
	        break;
	default:
		g_assert_not_reached ();
		break;
	}

	sp_knot_update_ctrl (knot);
}

static void
sp_knot_get_property (GObject * object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_assert_not_reached ();
}

static int
sp_knot_handler (SPCanvasItem *item, GdkEvent *event, SPKnot *knot)
{
	gboolean consumed;
	static gboolean grabbed = FALSE;
	static gboolean moved = FALSE;

	g_assert (knot != NULL);
	g_assert (SP_IS_KNOT (knot));

	consumed = FALSE;

	/* Run client universal event handler, if present */

	g_signal_emit (G_OBJECT (knot), knot_signals[EVENT], 0, event, &consumed);

	if (consumed) return TRUE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			NRPointF p;
			sp_desktop_w2d_xy_point (knot->desktop,
				&p,
				event->button.x,
				event->button.y);
			knot->hx = p.x - knot->x;
			knot->hy = p.y - knot->y;
			if (!nograb) {
				sp_canvas_item_grab (knot->item,
						     KNOT_EVENT_MASK,
						     knot->cursor[SP_KNOT_STATE_DRAGGING],
						     event->button.time);
			}
			sp_knot_set_flag (knot, SP_KNOT_GRABBED, TRUE);
			grabbed = TRUE;
			consumed = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			sp_knot_set_flag (knot, SP_KNOT_GRABBED, FALSE);
			if (!nograb) {
				sp_canvas_item_ungrab (knot->item, event->button.time);
			}
			if (moved) {
				sp_knot_set_flag (knot,
					SP_KNOT_DRAGGING,
					FALSE);
				g_signal_emit (G_OBJECT (knot),
					       knot_signals[UNGRABBED], 0,
					       event->button.state);
			} else {
				g_signal_emit (G_OBJECT (knot),
					       knot_signals[CLICKED], 0,
					       event->button.state);
			}
			grabbed = FALSE;
			moved = FALSE;
			consumed = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (grabbed) {
			NRPointF p;
			NRPointF fp;
			if (!moved) {
				g_signal_emit (G_OBJECT (knot),
					       knot_signals[GRABBED], 0,
					       event->motion.state);
				sp_knot_set_flag (knot,
					SP_KNOT_DRAGGING,
					TRUE);
			}
			sp_desktop_w2d_xy_point (knot->desktop, &fp, event->motion.x, event->motion.y);
			p.x = fp.x - knot->hx;
			p.y = fp.y - knot->hy;
			sp_knot_request_position (knot, &p, event->motion.state);
			moved = TRUE;
			consumed = TRUE;
		}
		break;
	case GDK_ENTER_NOTIFY:
		sp_knot_set_flag (knot, SP_KNOT_MOUSEOVER, TRUE);
		consumed = TRUE;
		break;
	case GDK_LEAVE_NOTIFY:
		sp_knot_set_flag (knot, SP_KNOT_MOUSEOVER, FALSE);
		consumed = TRUE;
		break;
	default:
		break;
	}


	return consumed;
}

SPKnot *
sp_knot_new (SPDesktop * desktop)
{
	SPKnot * knot;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (SP_DT_IS_EDITABLE (desktop), NULL);

	knot = g_object_new (SP_TYPE_KNOT, 0);

	knot->desktop = desktop;
	knot->flags = SP_KNOT_VISIBLE;

	knot->item = sp_canvas_item_new (SP_DT_CONTROLS (desktop),
		SP_TYPE_CTRL,
		"anchor", GTK_ANCHOR_CENTER,
		"size", 8.0,
		"filled", TRUE,
		"fill_color", 0x000000ff,
		"stroked", TRUE,
		"stroke_color", 0x000000ff,
		NULL);

	gtk_signal_connect (GTK_OBJECT (knot->item), "event",
		GTK_SIGNAL_FUNC (sp_knot_handler), knot);

	return knot;
}

void
sp_knot_show (SPKnot * knot)
{
	g_return_if_fail (knot != NULL);
	g_return_if_fail (SP_IS_KNOT (knot));

	sp_knot_set_flag (knot, SP_KNOT_VISIBLE, TRUE);
}

void
sp_knot_hide (SPKnot * knot)
{
	g_return_if_fail (knot != NULL);
	g_return_if_fail (SP_IS_KNOT (knot));

	sp_knot_set_flag (knot, SP_KNOT_VISIBLE, FALSE);
}

void
sp_knot_request_position (SPKnot * knot, NRPointF * p, guint state)
{
	gboolean done;

	g_return_if_fail (knot != NULL);
	g_return_if_fail (SP_IS_KNOT (knot));
	g_return_if_fail (p != NULL);

	done = FALSE;

	g_signal_emit (G_OBJECT (knot),
		       knot_signals[REQUEST], 0,
		       p,
		       state,
		       &done);

	/* If user did not complete, we simply move knot to new position */

	if (!done) {
		sp_knot_set_position (knot, p, state);
	}
}

gdouble
sp_knot_distance (SPKnot * knot, NRPointF * p, guint state)
{
	gdouble distance;

	g_return_val_if_fail (knot != NULL, 1e18);
	g_return_val_if_fail (SP_IS_KNOT (knot), 1e18);
	g_return_val_if_fail (p != NULL, 1e18);

	distance = hypot (p->x - knot->x, p->y - knot->y);

	g_signal_emit (G_OBJECT (knot),
		       knot_signals[DISTANCE], 0,
		       p,
		       state,
		       &distance);

	return distance;
}

void
sp_knot_set_position (SPKnot * knot, NRPointF * p, guint state)
{
	g_return_if_fail (knot != NULL);
	g_return_if_fail (SP_IS_KNOT (knot));
	g_return_if_fail (p != NULL);

	knot->x = p->x;
	knot->y = p->y;

	if (knot->item) sp_ctrl_moveto (SP_CTRL (knot->item), p->x, p->y);

	g_signal_emit (G_OBJECT (knot),
		       knot_signals[MOVED], 0,
		       p,
		       state);
}

NRPointF *
sp_knot_position (SPKnot * knot, NRPointF * p)
{
	g_return_val_if_fail (knot != NULL, NULL);
	g_return_val_if_fail (SP_IS_KNOT (knot), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = knot->x;
	p->y = knot->y;

	return p;
}

static void
sp_knot_set_flag (SPKnot * knot, guint flag, gboolean set)
{
	g_assert (knot != NULL);
	g_assert (SP_IS_KNOT (knot));

	if (set) {
		knot->flags |= flag;
	} else {
		knot->flags &= ~flag;
	}

	switch (flag) {
	case SP_KNOT_VISIBLE:
		if (set) {
			sp_canvas_item_show (knot->item);
		} else {
			sp_canvas_item_hide (knot->item);
		}
		break;
	case SP_KNOT_MOUSEOVER:
	case SP_KNOT_DRAGGING:
		sp_knot_set_ctrl_state (knot);
		break;
	case SP_KNOT_GRABBED:
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
sp_knot_update_ctrl (SPKnot * knot)
{
	if (!knot->item) return;

	gtk_object_set (GTK_OBJECT (knot->item), "shape", knot->shape, NULL);
	gtk_object_set (GTK_OBJECT (knot->item), "mode", knot->mode, NULL);
	gtk_object_set (GTK_OBJECT (knot->item), "size", (gdouble) knot->size, NULL);
	gtk_object_set (GTK_OBJECT (knot->item), "anchor", knot->anchor, NULL);
	if (knot->pixbuf) gtk_object_set (GTK_OBJECT (knot->item), "pixbuf", knot->pixbuf, NULL);

	sp_knot_set_ctrl_state (knot);
}

static void
sp_knot_set_ctrl_state (SPKnot * knot)
{
	if (knot->flags & SP_KNOT_DRAGGING) {
		gtk_object_set (GTK_OBJECT (knot->item),
				"fill_color",
				knot->fill [SP_KNOT_STATE_DRAGGING],
				NULL);
		gtk_object_set (GTK_OBJECT (knot->item),
				"stroke_color",
				knot->stroke [SP_KNOT_STATE_DRAGGING],
				NULL);
	} else if (knot->flags & SP_KNOT_MOUSEOVER) {
		gtk_object_set (GTK_OBJECT (knot->item),
				"fill_color",
				knot->fill [SP_KNOT_STATE_MOUSEOVER],
				NULL);
		gtk_object_set (GTK_OBJECT (knot->item),
				"stroke_color",
				knot->stroke [SP_KNOT_STATE_MOUSEOVER],
				NULL);
	} else {
		gtk_object_set (GTK_OBJECT (knot->item),
				"fill_color",
				knot->fill [SP_KNOT_STATE_NORMAL],
				NULL);
		gtk_object_set (GTK_OBJECT (knot->item),
				"stroke_color",
				knot->stroke [SP_KNOT_STATE_NORMAL],
				NULL);
	}
}
