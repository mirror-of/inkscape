#define __SP_GRADIENT_POSITION_C__

/*
 * Gradient positioning widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <string.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock.h>
#include <libart_lgpl/art_affine.h>
#include <gtk/gtksignal.h>
#include "../helper/sp-intl.h"
#include "../helper/nr-plain-stuff.h"
#include "../helper/nr-plain-stuff-gdk.h"
#include "../macros.h"
#include "gradient-position.h"

#define RADIUS 4

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

static void sp_gradient_position_class_init (SPGradientPositionClass *klass);
static void sp_gradient_position_init (SPGradientPosition *position);
static void sp_gradient_position_destroy (GtkObject *object);

static void sp_gradient_position_realize (GtkWidget *widget);
static void sp_gradient_position_unrealize (GtkWidget *widget);
static void sp_gradient_position_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void sp_gradient_position_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint sp_gradient_position_expose (GtkWidget *widget, GdkEventExpose *event);

static gint sp_gradient_position_button_press (GtkWidget *widget, GdkEventButton *event);
static gint sp_gradient_position_button_release (GtkWidget *widget, GdkEventButton *event);
static gint sp_gradient_position_motion_notify (GtkWidget *widget, GdkEventMotion *event);

static void sp_gradient_position_gradient_release (SPGradient *gr, SPGradientPosition *im);
static void sp_gradient_position_gradient_modified (SPGradient *gr, guint flags, SPGradientPosition *im);
static void sp_gradient_position_update (SPGradientPosition *img);

static void sp_gradient_position_paint (GtkWidget *widget, GdkRectangle *area);
static void sp_gradient_position_free_image_data (SPGradientPosition *pos);

void spgp_clip_line (long *c, long x0, long y0, long x1, long y1);
void spgp_draw_line (unsigned char *px, int w, int h, int rs, int x0, int y0, int x1, int y1, unsigned long c0, unsigned long c1);
void spgp_draw_rect (unsigned char *px, int w, int h, int rs, int x0, int y0, int x1, int y1, unsigned long c0, unsigned long c1);

static GtkWidgetClass *parent_class;
static guint position_signals[LAST_SIGNAL] = {0};

GtkType
sp_gradient_position_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPGradientPosition",
			sizeof (SPGradientPosition),
			sizeof (SPGradientPositionClass),
			(GtkClassInitFunc) sp_gradient_position_class_init,
			(GtkObjectInitFunc) sp_gradient_position_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_WIDGET, &info);
	}
	return type;
}

static void
sp_gradient_position_class_init (SPGradientPositionClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_WIDGET);

	position_signals[GRABBED] = gtk_signal_new ("grabbed",
						    GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPGradientPositionClass, grabbed),
						    gtk_marshal_NONE__NONE,
						    GTK_TYPE_NONE, 0);
	position_signals[DRAGGED] = gtk_signal_new ("dragged",
						    GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPGradientPositionClass, dragged),
						    gtk_marshal_NONE__NONE,
						    GTK_TYPE_NONE, 0);
	position_signals[RELEASED] = gtk_signal_new ("released",
						     GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						     GTK_CLASS_TYPE(object_class),
						     GTK_SIGNAL_OFFSET (SPGradientPositionClass, released),
						     gtk_marshal_NONE__NONE,
						     GTK_TYPE_NONE, 0);
	position_signals[CHANGED] = gtk_signal_new ("changed",
						    GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPGradientPositionClass, changed),
						    gtk_marshal_NONE__NONE,
						    GTK_TYPE_NONE, 0);

	object_class->destroy = sp_gradient_position_destroy;

	widget_class->realize = sp_gradient_position_realize;
	widget_class->unrealize = sp_gradient_position_unrealize;
	widget_class->size_request = sp_gradient_position_size_request;
	widget_class->size_allocate = sp_gradient_position_size_allocate;
/*  	widget_class->draw = sp_gradient_position_draw; */
	widget_class->expose_event = sp_gradient_position_expose;
	widget_class->button_press_event = sp_gradient_position_button_press;
	widget_class->button_release_event = sp_gradient_position_button_release;
	widget_class->motion_notify_event = sp_gradient_position_motion_notify;
}

static void
sp_gradient_position_init (SPGradientPosition *pos)
{
	/* We are widget with window */
	GTK_WIDGET_UNSET_FLAGS (pos, GTK_NO_WINDOW);

	pos->dragging = FALSE;
	pos->position = 0;

	pos->need_update = TRUE;

	pos->gradient = NULL;
	pos->bbox.x0 = pos->bbox.y0 = pos->bbox.x1 = pos->bbox.y1 = 0.0;
	pos->spread = NR_GRADIENT_SPREAD_PAD;

	nr_matrix_f_set_identity (&pos->gs2d);

	pos->gc = NULL;
	pos->px = NULL;
}

static void
sp_gradient_position_destroy (GtkObject *object)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (object);

	sp_gradient_position_free_image_data (pos);

	if (pos->gradient) {
		sp_signal_disconnect_by_data (pos->gradient, pos);
		pos->gradient = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_gradient_position_realize (GtkWidget *widget)
{
	SPGradientPosition *pos;
	GdkWindowAttr attributes;
	gint attributes_mask;

	pos = SP_GRADIENT_POSITION (widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gdk_rgb_get_visual ();
	attributes.colormap = gdk_rgb_get_cmap ();
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_EXPOSURE_MASK |
				  GDK_BUTTON_PRESS_MASK |
				  GDK_BUTTON_RELEASE_MASK |
				  GDK_POINTER_MOTION_MASK |
				  GDK_ENTER_NOTIFY_MASK |
				  GDK_LEAVE_NOTIFY_MASK);
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, widget);

	pos->need_update = TRUE;
}

static void
sp_gradient_position_unrealize (GtkWidget *widget)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	if (((GtkWidgetClass *) parent_class)->unrealize)
		(* ((GtkWidgetClass *) parent_class)->unrealize) (widget);

	sp_gradient_position_free_image_data (pos);

	pos->need_update = TRUE;
}

static void
sp_gradient_position_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	requisition->width = 128;
	requisition->height = 128;
}

static void
sp_gradient_position_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED (widget)) {
		sp_gradient_position_free_image_data (pos);
		pos->need_update = TRUE;
		gdk_window_move_resize (widget->window,
					widget->allocation.x, widget->allocation.y,
					widget->allocation.width, widget->allocation.height);
	}
}

#if 0
static void
sp_gradient_position_draw (GtkWidget *widget, GdkRectangle *area)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	if (GTK_WIDGET_DRAWABLE (widget)) {
		sp_gradient_position_paint (widget, area);
	}
}
#endif

static gint
sp_gradient_position_expose (GtkWidget *widget, GdkEventExpose *event)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	if (GTK_WIDGET_DRAWABLE (widget)) {
		sp_gradient_position_paint (widget, &event->area);
	}

	return TRUE;
}

static gint
sp_gradient_position_button_press (GtkWidget *widget, GdkEventButton *event)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	if (!pos->gradient) return FALSE;

	if (pos->mode == SP_GRADIENT_POSITION_MODE_LINEAR) {
		/* Linear mode */
		if (event->button == 1) {
			if (event->state & GDK_CONTROL_MASK) {
				pos->dragging = TRUE;
				pos->changed = FALSE;
				gdk_pointer_grab (widget->window, FALSE,
						  GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
						  NULL, NULL, event->time);
			} else {
				float x1, y1;
				pos->dragging = TRUE;
				pos->changed = FALSE;
				g_signal_emit (G_OBJECT (pos), position_signals[GRABBED], 0);
				x1 = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				y1 = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);
				if (!NR_DF_TEST_CLOSE (x1, pos->gdata.linear.x1, NR_EPSILON_F) ||
				    !NR_DF_TEST_CLOSE (y1, pos->gdata.linear.y1, NR_EPSILON_F)) {
					pos->gdata.linear.x1 = x1;
					pos->gdata.linear.y1 = y1;
					g_signal_emit (G_OBJECT (pos), position_signals[DRAGGED], 0);
					pos->changed = TRUE;
					pos->need_update = TRUE;
					if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
				}
				gdk_pointer_grab (widget->window, FALSE,
						  GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
						  NULL, NULL, event->time);
			}
		}
	} else {
		/* Radial mode */
		if (event->button == 1) {
			if (event->state & GDK_CONTROL_MASK) {
				pos->dragging = TRUE;
				pos->changed = FALSE;
				gdk_pointer_grab (widget->window, FALSE,
						  GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
						  NULL, NULL, event->time);
			} else if (event->state & GDK_SHIFT_MASK) {
				float fx, fy;
				pos->dragging = TRUE;
				pos->changed = FALSE;
				g_signal_emit (G_OBJECT (pos), position_signals[GRABBED], 0);
				fx = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				fy = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);
				if (!NR_DF_TEST_CLOSE (fx, pos->gdata.radial.fx, NR_EPSILON_F) ||
				    !NR_DF_TEST_CLOSE (fy, pos->gdata.radial.fy, NR_EPSILON_F)) {
					pos->gdata.radial.fx = fx;
					pos->gdata.radial.fy = fy;
					g_signal_emit (G_OBJECT (pos), position_signals[DRAGGED], 0);
					pos->changed = TRUE;
					pos->need_update = TRUE;
					if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
				}
				gdk_pointer_grab (widget->window, FALSE,
						  GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
						  NULL, NULL, event->time);
			} else {
				float cx, cy;
				pos->dragging = TRUE;
				pos->changed = FALSE;
				g_signal_emit (G_OBJECT (pos), position_signals[GRABBED], 0);
				cx = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				cy = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);
				if (!NR_DF_TEST_CLOSE (cx, pos->gdata.radial.cx, NR_EPSILON_F) ||
				    !NR_DF_TEST_CLOSE (cy, pos->gdata.radial.cy, NR_EPSILON_F)) {
					pos->gdata.radial.cx = cx;
					pos->gdata.radial.cy = cy;
					pos->gdata.radial.fx = cx;
					pos->gdata.radial.fy = cy;
					g_signal_emit (G_OBJECT (pos), position_signals[DRAGGED], 0);
					pos->changed = TRUE;
					pos->need_update = TRUE;
					if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
				}
				gdk_pointer_grab (widget->window, FALSE,
						  GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
						  NULL, NULL, event->time);
			}
		}
	}

	return FALSE;
}

static gint
sp_gradient_position_button_release (GtkWidget *widget, GdkEventButton *event)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	if ((event->button == 1) && pos->dragging) {
		g_signal_emit (G_OBJECT (pos), position_signals[RELEASED], 0);
		if (pos->changed) {
			g_signal_emit (G_OBJECT (pos), position_signals[CHANGED], 0);
			pos->changed = FALSE;
		}
		gdk_pointer_ungrab (event->time);
		pos->dragging = FALSE;
	}

	return FALSE;
}

static gint
sp_gradient_position_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
	SPGradientPosition *pos;

	pos = SP_GRADIENT_POSITION (widget);

	if (pos->mode == SP_GRADIENT_POSITION_MODE_LINEAR) {
		if (pos->dragging) {
			if (event->state & GDK_CONTROL_MASK) {
#if 0
				NRMatrixF n2gs, gs2n, n2w, w2n, s2n;
				float x1, y1, x2, y2;
				float cx, cy, ncx, ncy, nx, ny;
				x1 = pos->gdata.linear.x1;
				y1 = pos->gdata.linear.y1;
				x2 = pos->gdata.linear.x2;
				y2 = pos->gdata.linear.y2;
				n2gs.c[0] = x2 - x1;
				n2gs.c[1] = y2 - y1;
				n2gs.c[2] = y2 - y1;
				n2gs.c[3] = x1 - x2;
				n2gs.c[4] = x1;
				n2gs.c[5] = y1;
				nr_matrix_f_invert (&gs2n, &n2gs);
				nr_matrix_multiply_fff (&n2w, &n2gs, &pos->gs2w);
				nr_matrix_f_invert (&w2n, &n2w);

				cx = (x1 + x2) / 2.0;
				cy = (y1 + y2) / 2.0;
				ncx = NR_MATRIX_DF_TRANSFORM_X (&gs2n, cx, cy);
				ncy = NR_MATRIX_DF_TRANSFORM_Y (&gs2n, cx, cy);
				nx = NR_MATRIX_DF_TRANSFORM_X (&w2n, event->x, event->y);
				ny = NR_MATRIX_DF_TRANSFORM_Y (&w2n, event->x, event->y);

				/* cx + s2n[2] * cy = x + s2n[2] * y */

				s2n.c[0] = 1.0;
				s2n.c[1] = 0.0;
				s2n.c[2] = !NR_DF_TEST_CLOSE (ncy, ny, NR_EPSILON_F) ? (nx - ncx) / (ny - ncy) : 0.0;
				s2n.c[3] = 1.0;
				s2n.c[4] = 0.0;
				s2n.c[5] = 0.0;

				nr_matrix_multiply_fff (&gs2n, &gs2n, &s2n);
				nr_matrix_multiply_fff (&gs2n, &gs2n, &n2gs);
				nr_matrix_multiply_fff (&pos->gs2w, &gs2n, &pos->gs2w);

				nr_matrix_f_invert (&pos->w2gs, &pos->gs2w);
				nr_matrix_multiply_fff (&pos->gs2d, &pos->gs2w, &pos->w2d);
#else
				pos->gdata.linear.x2 = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				pos->gdata.linear.y2 = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);
#endif
			} else {
				pos->gdata.linear.x2 = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				pos->gdata.linear.y2 = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);
			}
			g_signal_emit (G_OBJECT (pos), position_signals[DRAGGED], 0);
			pos->changed = TRUE;
			pos->need_update = TRUE;
			if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
		}
	} else {
		if (pos->dragging) {
			float cx, cy, fx, fy, r;
			cx = pos->gdata.radial.cx;
			cy = pos->gdata.radial.cy;
			fx = pos->gdata.radial.fx;
			fy = pos->gdata.radial.fy;
			r = pos->gdata.radial.r;
			if (event->state & GDK_SHIFT_MASK) {
				pos->gdata.radial.fx = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				pos->gdata.radial.fy = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);
			} else {
				float x, y;
#if 0
				if (1 || !NR_DF_TEST_CLOSE (pos->w2gs.c[0], pos->w2gs.c[3], NR_EPSILON_F) ||
				    !NR_DF_TEST_CLOSE (pos->w2gs.c[1], 0.0, NR_EPSILON_F) ||
				    !NR_DF_TEST_CLOSE (pos->w2gs.c[2], 0.0, NR_EPSILON_F)) {
					double ex, cxw, cyw, fxw, fyw, dxw, dyw, s;
					ex = NR_MATRIX_DF_EXPANSION (&pos->gs2w);
					cxw = NR_MATRIX_DF_TRANSFORM_X (&pos->gs2w, cx, cy);
					cyw = NR_MATRIX_DF_TRANSFORM_Y (&pos->gs2w, cx, cy);
					fxw = NR_MATRIX_DF_TRANSFORM_X (&pos->gs2w, fx, fy);
					fyw = NR_MATRIX_DF_TRANSFORM_Y (&pos->gs2w, fx, fy);
					dxw = fabs (event->x - cxw);
					dyw = fabs (event->y - cyw);
					dxw = MAX (dxw, 1.0);
					dyw = MAX (dyw, 1.0);
					s = dxw / dyw;
					pos->w2gs.c[0] *= s;
					pos->w2gs.c[1] *= s;
					pos->w2gs.c[2] *= s;
					pos->w2gs.c[3] *= s;
					printf ("ex %f pp %f %f %f %f %f %f\n", ex,
						pos->w2gs.c[0],
						pos->w2gs.c[1],
						pos->w2gs.c[2],
						pos->w2gs.c[3],
						pos->w2gs.c[4],
						pos->w2gs.c[5]);
					nr_matrix_f_invert (&pos->gs2w, &pos->w2gs);
					nr_matrix_multiply_fff (&pos->gs2d, &pos->gs2w, &pos->w2d);
					pos->gdata.radial.cx = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, cxw, cyw);
					pos->gdata.radial.cy = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, cxw, cyw);
					pos->gdata.radial.fx = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, fxw, fyw);
					pos->gdata.radial.fy = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, fxw, fyw);
				}
#endif

				x = NR_MATRIX_DF_TRANSFORM_X (&pos->w2gs, event->x, event->y);
				y = NR_MATRIX_DF_TRANSFORM_Y (&pos->w2gs, event->x, event->y);

				pos->gdata.radial.r = hypot (x - cx, y - cy);
			}
			g_signal_emit (G_OBJECT (pos), position_signals[DRAGGED], 0);
			pos->changed = TRUE;
			pos->need_update = TRUE;
			if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
		}
	}

	return FALSE;
}

GtkWidget *
sp_gradient_position_new (SPGradient *gradient)
{
	SPGradientPosition *position;

	position = gtk_type_new (SP_TYPE_GRADIENT_POSITION);

	sp_gradient_position_set_gradient (position, gradient);

	return (GtkWidget *) position;
}

void
sp_gradient_position_set_gradient (SPGradientPosition *pos, SPGradient *gradient)
{
	if (pos->gradient) {
		sp_signal_disconnect_by_data (pos->gradient, pos);
	}

	pos->gradient = gradient;

	if (gradient) {
		g_signal_connect (G_OBJECT (gradient), "release", G_CALLBACK (sp_gradient_position_gradient_release), pos);
		g_signal_connect (G_OBJECT (gradient), "modified", G_CALLBACK (sp_gradient_position_gradient_modified), pos);
	}

	pos->need_update = TRUE;
	if (GTK_WIDGET_DRAWABLE (pos)) {
		gtk_widget_queue_draw (GTK_WIDGET (pos));
	}
}

static void
sp_gradient_position_gradient_release (SPGradient *gradient, SPGradientPosition *pos)
{
	sp_gradient_position_set_gradient (pos, NULL);
}

static void
sp_gradient_position_gradient_modified (SPGradient *gradient, guint flags, SPGradientPosition *pos)
{
	pos->need_update = TRUE;
	if (GTK_WIDGET_DRAWABLE (pos)) {
		gtk_widget_queue_draw (GTK_WIDGET (pos));
	}
}

void
sp_gradient_position_set_mode (SPGradientPosition *pos, guint mode)
{
	if (pos->mode != mode) {
		pos->mode = mode;
		if (pos->mode == SP_GRADIENT_POSITION_MODE_LINEAR) {
			pos->gdata.linear.x1 = 0.0;
			pos->gdata.linear.y1 = 0.0;
			pos->gdata.linear.x2 = 1.0;
			pos->gdata.linear.y2 = 0.0;
		} else {
			pos->gdata.radial.cx = 0.5;
			pos->gdata.radial.cy = 0.5;
			pos->gdata.radial.fx = 0.5;
			pos->gdata.radial.fy = 0.5;
			pos->gdata.radial.r = 0.5;
		}
		pos->need_update = TRUE;
		if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
	}
}

void
sp_gradient_position_set_bbox (SPGradientPosition *pos, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
	g_return_if_fail (x1 > x0);
	g_return_if_fail (y1 > y0);

	pos->bbox.x0 = x0;
	pos->bbox.y0 = y0;
	pos->bbox.x1 = x1;
	pos->bbox.y1 = y1;

	pos->need_update = TRUE;
	if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
}

void
sp_gradient_position_set_gs2d_matrix_f (SPGradientPosition *pos, NRMatrixF *gs2d)
{
	pos->gs2d = *gs2d;

	pos->need_update = TRUE;
	if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
}

void
sp_gradient_position_get_gs2d_matrix_f (SPGradientPosition *pos, NRMatrixF *gs2d)
{
	*gs2d = pos->gs2d;
}

void
sp_gradient_position_set_linear_position (SPGradientPosition *pos, float x1, float y1, float x2, float y2)
{
	pos->gdata.linear.x1 = x1;
	pos->gdata.linear.y1 = y1;
	pos->gdata.linear.x2 = x2;
	pos->gdata.linear.y2 = y2;

	pos->need_update = TRUE;
	if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
}

void
sp_gradient_position_set_radial_position (SPGradientPosition *pos, float cx, float cy, float fx, float fy, float r)
{
	pos->gdata.radial.cx = cx;
	pos->gdata.radial.cy = cy;
	pos->gdata.radial.fx = fx;
	pos->gdata.radial.fy = fy;
	pos->gdata.radial.r = r;

	pos->need_update = TRUE;
	if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
}

void
sp_gradient_position_set_spread (SPGradientPosition *pos, unsigned int spread)
{
	if (spread != pos->spread) {
		pos->spread = spread;
		pos->need_update = TRUE;
		if (GTK_WIDGET_DRAWABLE (pos)) gtk_widget_queue_draw (GTK_WIDGET (pos));
	}
}

void
sp_gradient_position_get_linear_position_floatv (SPGradientPosition *gp, float *pos)
{
	pos[0] = gp->gdata.linear.x1;
	pos[1] = gp->gdata.linear.y1;
	pos[2] = gp->gdata.linear.x2;
	pos[3] = gp->gdata.linear.y2;
}

void
sp_gradient_position_get_radial_position_floatv (SPGradientPosition *gp, float *pos)
{
	pos[0] = gp->gdata.radial.cx;
	pos[1] = gp->gdata.radial.cy;
	pos[2] = gp->gdata.radial.fx;
	pos[3] = gp->gdata.radial.fy;
	pos[4] = gp->gdata.radial.r;
}

static void
sp_gradient_position_update (SPGradientPosition *pos)
{
	GtkWidget *widget;
	int width, height;
	gdouble xs, ys;

	widget = GTK_WIDGET (pos);
	width = widget->allocation.width;
	height = widget->allocation.height;

	pos->need_update = FALSE;

	/* Create image data */
	if (!pos->px) pos->px = gdk_pixmap_new (widget->window, 64, 64, -1);
	if (!pos->gc) pos->gc = gdk_gc_new (widget->window);

	/* Calculate bbox */
	xs = width / (pos->bbox.x1 - pos->bbox.x0);
	ys = height / (pos->bbox.y1 - pos->bbox.y0);

	if (xs > ys) {
		pos->vbox.x0 = (short) floor (width * (1 - ys / xs) / 2.0);
		pos->vbox.y0 = 0;
	} else if (xs < ys) {
		pos->vbox.x0 = 0;
		pos->vbox.y0 = (short) floor (height * (1 - xs / ys) / 2.0);
	} else {
		pos->vbox.x0 = 0;
		pos->vbox.y0 = 0;
	}
	pos->vbox.x1 = widget->allocation.width - pos->vbox.x0;
	pos->vbox.y1 = widget->allocation.height - pos->vbox.y0;

	/* Calculate w2d */
	pos->w2d.c[0] = (pos->bbox.x1 - pos->bbox.x0) / (pos->vbox.x1 - pos->vbox.x0);
	pos->w2d.c[1] = 0.0;
	pos->w2d.c[2] = 0.0;
	pos->w2d.c[3] = (pos->bbox.y1 - pos->bbox.y0) / (pos->vbox.y1 - pos->vbox.y0);
	pos->w2d.c[4] = pos->bbox.x0 - (pos->vbox.x0 * pos->w2d.c[0]);
	pos->w2d.c[5] = pos->bbox.y0 - (pos->vbox.y0 * pos->w2d.c[3]);
	/* Calculate d2w */
	nr_matrix_f_invert (&pos->d2w, &pos->w2d);
	/* Calculate wbox */
	pos->wbox.x0 = pos->w2d.c[4];
	pos->wbox.x0 = pos->w2d.c[5];
	pos->wbox.x1 = pos->wbox.x0 + pos->w2d.c[0] * width;
	pos->wbox.y1 = pos->wbox.y0 + pos->w2d.c[1] * height;
	/* w2gs and gs2w */
	nr_matrix_multiply_fff (&pos->gs2w, &pos->gs2d, &pos->d2w);
	nr_matrix_f_invert (&pos->w2gs, &pos->gs2w);

	if (!pos->cv) pos->cv = g_new (guchar, 4 * NR_GRADIENT_VECTOR_LENGTH);
	sp_gradient_render_vector_line_rgba (pos->gradient, pos->cv, NR_GRADIENT_VECTOR_LENGTH, 0 , NR_GRADIENT_VECTOR_LENGTH);

	if (pos->mode == SP_GRADIENT_POSITION_MODE_LINEAR) {
		nr_lgradient_renderer_setup (&pos->renderer.lgr, pos->cv, pos->spread, &pos->gs2w,
					     pos->gdata.linear.x1, pos->gdata.linear.y1,
					     pos->gdata.linear.x2, pos->gdata.linear.y2);
	} else {
		nr_rgradient_renderer_setup (&pos->renderer.rgr, pos->cv, pos->spread, &pos->gs2w,
					     pos->gdata.radial.cx, pos->gdata.radial.cy,
					     pos->gdata.radial.fx, pos->gdata.radial.fy,
					     pos->gdata.radial.r);
	}
}

static void
sp_gradient_position_free_image_data (SPGradientPosition *pos)
{
	if (pos->px) {
		gdk_pixmap_unref (pos->px);
		pos->px = NULL;
	}
	if (pos->gc) {
		gdk_gc_unref (pos->gc);
		pos->gc = NULL;
	}
	if (pos->cv) {
		g_free (pos->cv);
		pos->cv = NULL;
	}
}

static void
sp_gradient_position_paint (GtkWidget *widget, GdkRectangle *area)
{
	SPGradientPosition *gp;
	gint x, y;

	gp = SP_GRADIENT_POSITION (widget);

	if (!gp->gradient ||
	    (gp->bbox.x0 >= gp->bbox.x1) ||
	    (gp->bbox.y0 >= gp->bbox.y1) ||
	    (widget->allocation.width < 1) ||
	    (widget->allocation.height < 1)) {
		/* Draw empty thing */
		if (!gp->gc) gp->gc = gdk_gc_new (widget->window);
		nr_gdk_draw_gray_garbage (widget->window, gp->gc, area->x, area->y, area->width, area->height);
		return;
	}

	if (gp->need_update) {
		sp_gradient_position_update (gp);
	}

	if (gp->mode == SP_GRADIENT_POSITION_MODE_LINEAR) {
		float x1, y1, x2, y2, cx, cy, dx, dy;
		long wx1, wy1, wx2, wy2, c[4];

		x1 = gp->gdata.linear.x1;
		y1 = gp->gdata.linear.y1;
		x2 = gp->gdata.linear.x2;
		y2 = gp->gdata.linear.y2;

		wx1 = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, x1, y1) + 0.5);
		wy1 = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, x1, y1) + 0.5);
		wx2 = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, x2, y2) + 0.5);
		wy2 = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, x2, y2) + 0.5);

		cx = (x1 + x2) / 2.0 + y1 - y2;
		cy = (y1 + y2) / 2.0 + x2 - x1;
		c[0] = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, cx, cy) + 0.5);
		c[1] = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, cx, cy) + 0.5);
		dx = (x1 + x2) / 2.0 - y1 + y2;
		dy = (y1 + y2) / 2.0 - x2 + x1;
		c[2] = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, dx, dy) + 0.5);
		c[3] = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, dx, dy) + 0.5);

		spgp_clip_line (c, area->x + 4, area->y + 4, area->x + area->width - 4, area->y + area->height - 4);

		for (y = area->y; y < area->y + area->height; y += 64) {
			for (x = area->x; x < area->x + area->width; x += 64) {
				int w, h;
				NRPixBlock pb;

				w = MIN (x + 64, area->x + area->width) - x;
				h = MIN (y + 64, area->y + area->height) - y;

				/* Set up pixblock */
				nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8, x, y, x + w, y + h, FALSE);
				/* Draw checkerboard */
				nr_render_checkerboard_rgb (NR_PIXBLOCK_PX (&pb), w, h, pb.rs, x, y);
				pb.empty = FALSE;

				/* Render gradient */
				nr_render ((NRRenderer *) &gp->renderer.lgr, &pb, NULL);

				/* Draw controls */
				spgp_draw_line (NR_PIXBLOCK_PX (&pb), w, h, pb.rs, wx1 - x, wy1 - y, wx2 - x, wy2 - y,
						0xfff7f77f, 0x7f7f7f7f);
				spgp_draw_line (NR_PIXBLOCK_PX (&pb), w, h, pb.rs,
						c[0] - x, c[1] - y,
						c[2] - x, c[3] - y,
						0x7f7fffff, 0x7f7f7fff);

				spgp_draw_rect (NR_PIXBLOCK_PX (&pb), w, h, pb.rs,
						wx1 - RADIUS - x, wy1 - RADIUS - y,
						wx1 + RADIUS - x, wy1 + RADIUS - y,
						0xff7f7f7f, 0x7f7f7f7f);
				spgp_draw_rect (NR_PIXBLOCK_PX (&pb), w, h, pb.rs,
						wx2 - RADIUS - x, wy2 - RADIUS - y,
						wx2 + RADIUS - x, wy2 + RADIUS - y,
						0xff7f7f7f, 0x7f7f7f7f);

				/* Draw pixmap */
				gdk_gc_set_function (gp->gc, GDK_COPY);
				gdk_draw_rgb_image (gp->px, gp->gc, 0, 0, w, h, GDK_RGB_DITHER_MAX, NR_PIXBLOCK_PX (&pb), pb.rs);

				/* Draw bbox */
				gdk_draw_rectangle (gp->px, gp->gc, FALSE,
						    gp->vbox.x0 - x, gp->vbox.y0 - y,
						    gp->vbox.x1 - gp->vbox.x0 - 1, gp->vbox.y1 - gp->vbox.y0 - 1);
				/* Copy to window */
				gdk_gc_set_function (gp->gc, GDK_COPY);
				gdk_draw_pixmap (widget->window, gp->gc, gp->px, 0, 0, x, y, w, h);

				/* Release pixblock */
				nr_pixblock_release (&pb);
			}
		}
	} else {
		float cx, cy, fx, fy, r;
		long wcx, wcy, wdx, wdy, wfx, wfy, c[4];

		cx = gp->gdata.radial.cx;
		cy = gp->gdata.radial.cy;
		fx = gp->gdata.radial.fx;
		fy = gp->gdata.radial.fy;
		r = gp->gdata.radial.r;

		wcx = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, cx, cy) + 0.5);
		wcy = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, cx, cy) + 0.5);
		wdx = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, cx + r, cy) + 0.5);
		wdy = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, cx + r, cy) + 0.5);
		wfx = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, fx, fy) + 0.5);
		wfy = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, fx, fy) + 0.5);

		c[0] = wcx;
		c[1] = wcy;
		c[2] = (short) (NR_MATRIX_DF_TRANSFORM_X (&gp->gs2w, cx, cy + r) + 0.5);
		c[3] = (short) (NR_MATRIX_DF_TRANSFORM_Y (&gp->gs2w, cx, cy + r) + 0.5);

		spgp_clip_line (c, area->x + 4, area->y + 4, area->x + area->width - 4, area->y + area->height - 4);

		for (y = area->y; y < area->y + area->height; y += 64) {
			for (x = area->x; x < area->x + area->width; x += 64) {
				int w, h;
				NRPixBlock pb;

				w = MIN (x + 64, area->x + area->width) - x;
				h = MIN (y + 64, area->y + area->height) - y;

				/* Set up pixblock */
				nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8, x, y, x + w, y + h, FALSE);
				/* Draw checkerboard */
				nr_render_checkerboard_rgb (NR_PIXBLOCK_PX (&pb), w, h, pb.rs, x, y);
				pb.empty = FALSE;

				/* Render gradient */
				nr_render ((NRRenderer *) &gp->renderer.rgr, &pb, NULL);

				/* Draw controls */
				spgp_draw_line (NR_PIXBLOCK_PX (&pb), w, h, pb.rs, wcx - x, wcy - y, wdx - x, wdy - y,
						0xfff7f77f, 0x7f7f7f7f);
				spgp_draw_line (NR_PIXBLOCK_PX (&pb), w, h, pb.rs,
						c[0] - x, c[1] - y,
						c[2] - x, c[3] - y,
						0x7f7fffff, 0x7f7f7fff);

				spgp_draw_rect (NR_PIXBLOCK_PX (&pb), w, h, pb.rs,
						wcx - RADIUS - x, wcy - RADIUS - y,
						wcx + RADIUS - x, wcy + RADIUS - y,
						0xff7f7f7f, 0x7f7f7f7f);
				spgp_draw_rect (NR_PIXBLOCK_PX (&pb), w, h, pb.rs,
						wfx - RADIUS - x, wfy - RADIUS - y,
						wfx + RADIUS - x, wfy + RADIUS - y,
						0xff7f7f7f, 0x7f7f7f7f);


				/* Draw pixmap */
				gdk_gc_set_function (gp->gc, GDK_COPY);
				gdk_draw_rgb_image (gp->px, gp->gc, 0, 0, w, h, GDK_RGB_DITHER_MAX, NR_PIXBLOCK_PX (&pb), pb.rs);

				/* Draw bbox */
				gdk_draw_rectangle (gp->px, gp->gc, FALSE,
						    gp->vbox.x0 - x, gp->vbox.y0 - y,
						    gp->vbox.x1 - gp->vbox.x0 - 1, gp->vbox.y1 - gp->vbox.y0 - 1);
				/* Copy to window */
				gdk_gc_set_function (gp->gc, GDK_COPY);
				gdk_draw_pixmap (widget->window, gp->gc, gp->px, 0, 0, x, y, w, h);

				/* Release pixblock */
				nr_pixblock_release (&pb);
			}
		}
	}
}

/* fixme: We may want to adjust it to integers (Lauris) */

void
spgp_clip_line (long *c, long x0, long y0, long x1, long y1)
{
	float px, py, vx, vy;
	float s, e, t0, t1;

	px = c[0];
	py = c[1];
	vx = c[2] - c[0];
	vy = c[3] - c[1];

	s = -NR_HUGE_F;
	e = NR_HUGE_F;

	if (!NR_DF_TEST_CLOSE (vx, 0.0, NR_EPSILON_F)) {
		t0 = (x0 - px) / vx;
		t1 = (x1 - px) / vx;
		s = MAX (s, MIN (t0, t1));
		e = MIN (e, MAX (t0, t1));
	}
	if (!NR_DF_TEST_CLOSE (vy, 0.0, NR_EPSILON_F)) {
		t0 = (y0 - py) / vy;
		t1 = (y1 - py) / vy;
		s = MAX (s, MIN (t0, t1));
		e = MIN (e, MAX (t0, t1));
	}

	c[0] = px + s * vx;
	c[1] = py + s * vy;
	c[2] = px + e * vx;
	c[3] = py + e * vy;
}

void
spgp_draw_line (unsigned char *px, int w, int h, int rs, int x0, int y0, int x1, int y1, unsigned long c0, unsigned long c1)
{
	long deltax, deltay, xinc1, xinc2, yinc1, yinc2;
	long den, num, numadd, numpixels;
	long x, y, curpixel;
	unsigned char c[8];

	if ((x0 < 0) && (x1 < 0)) return;
	if ((x0 >= w) && (x1 >= w)) return;
	if ((y0 < 0) && (y1 < 0)) return;
	if ((y0 >= h) && (y1 >= h)) return;

	if (x1 >= x0) {
		deltax = x1 - x0;
		xinc1 = 1;
		xinc2 = 1;
	} else {
		deltax = x0 - x1;
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y1 >= y0) {
		deltay = y1 - y0;
		yinc1 = 1;
		yinc2 = 1;
	} else {
		deltay = y0 - y1;
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) {
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	c[0] = NR_RGBA32_R (c0);
	c[1] = NR_RGBA32_G (c0);
	c[2] = NR_RGBA32_B (c0);
	c[3] = NR_RGBA32_A (c0);
	c[4] = NR_RGBA32_R (c1);
	c[5] = NR_RGBA32_G (c1);
	c[6] = NR_RGBA32_B (c1);
	c[7] = NR_RGBA32_A (c1);

	x = x0;
	y = y0;

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		if ((x >= 0) && (y >= 0) && (x < w) && (y < h)) {
			unsigned char *d, *s;
			d = px + y * rs + 3 * x;
			s = &c[4 * (curpixel & 0x1)];
			d[0] = NR_COMPOSEN11 (s[0], s[3], d[0]);
			d[1] = NR_COMPOSEN11 (s[1], s[3], d[1]);
			d[2] = NR_COMPOSEN11 (s[2], s[3], d[2]);
		}
		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void
spgp_draw_rect (unsigned char *px, int w, int h, int rs, int x0, int y0, int x1, int y1, unsigned long c0, unsigned long c1)
{
	int sx, sy, ex, ey, x, y;
	unsigned char s[4];
	unsigned char f[4];
	unsigned char *p;

	if (x0 >= w) return;
	if (y0 >= h) return;
	if (x1 < 0) return;
	if (y1 < 0) return;
 
	s[0] = NR_RGBA32_R (c0);
	s[1] = NR_RGBA32_G (c0);
	s[2] = NR_RGBA32_B (c0);
	s[3] = NR_RGBA32_A (c0);
	f[0] = NR_RGBA32_R (c1);
	f[1] = NR_RGBA32_G (c1);
	f[2] = NR_RGBA32_B (c1);
	f[3] = NR_RGBA32_A (c1);

	sx = MAX (x0, 0);
	sy = MAX (y0, 0);
	ex = MIN (x1, w - 1);
	ey = MIN (y1, h - 1);

	for (y = sy; y <= ey; y++) {
		p = px + y * rs + sx * 3;
		if ((y == y0) || (y == y1)) {
			for (x = sx; x <= ex; x++) {
				p[0] = NR_COMPOSEN11 (s[0], s[3], p[0]);
				p[1] = NR_COMPOSEN11 (s[0], s[3], p[0]);
				p[2] = NR_COMPOSEN11 (s[0], s[3], p[0]);
				p += 3;
			}
		} else {
			for (x = sx; x <= ex; x++) {
				if ((x == x0) || (x == x1)) {
					p[0] = NR_COMPOSEN11 (s[0], s[3], p[0]);
					p[1] = NR_COMPOSEN11 (s[0], s[3], p[0]);
					p[2] = NR_COMPOSEN11 (s[0], s[3], p[0]);
					p += 3;
				} else {
					p[0] = NR_COMPOSEN11 (f[0], f[3], p[0]);
					p[1] = NR_COMPOSEN11 (f[0], f[3], p[0]);
					p[2] = NR_COMPOSEN11 (f[0], f[3], p[0]);
					p += 3;
				}
			}
		}
	}
}
