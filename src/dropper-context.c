#define __SP_DROPPER_CONTEXT_C__

/*
 * Tool for picking colors from drawing
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-matrix.h>
#include <libnr/nr-pixblock.h>

#include "helper/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "enums.h"
#include "color.h"
#include "sodipodi-private.h"
#include "desktop-affine.h"
#include "desktop-handles.h"

#include "dropper-context.h"

#define C1 0.552
static const ArtBpath spdc_circle[] = {
	{ART_MOVETO, 0, 0, 0, 0, -1, 0},
	{ART_CURVETO, -1, C1, -C1, 1, 0, 1},
	{ART_CURVETO, C1, 1, 1, C1, 1, 0},
	{ART_CURVETO, 1, -C1, C1, -1, 0, -1},
	{ART_CURVETO, -C1, -1, -1, -C1, -1, 0},
	{ART_END, 0, 0, 0, 0, 0, 0}
};
#undef C1

static void sp_dropper_context_class_init (SPDropperContextClass *klass);
static void sp_dropper_context_init (SPDropperContext *dc);

static void sp_dropper_context_setup (SPEventContext *ec);
static void sp_dropper_context_finish (SPEventContext *ec);

static gint sp_dropper_context_root_handler (SPEventContext *ec, GdkEvent * event);

static SPEventContextClass *parent_class;

GType
sp_dropper_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPDropperContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_dropper_context_class_init,
			NULL, NULL,
			sizeof (SPDropperContext),
			4,
			(GInstanceInitFunc) sp_dropper_context_init,
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPDropperContext", &info, 0);
	}
	return type;
}

static void
sp_dropper_context_class_init (SPDropperContextClass * klass)
{
	GObjectClass *object_class;
	SPEventContextClass * ec_class;

	object_class = (GObjectClass *) klass;
	ec_class = (SPEventContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	ec_class->setup = sp_dropper_context_setup;
	ec_class->finish = sp_dropper_context_finish;
	ec_class->root_handler = sp_dropper_context_root_handler;
}

static void
sp_dropper_context_init (SPDropperContext *dc)
{
	SPEventContext *ec;
	
	ec = SP_EVENT_CONTEXT (dc);
}

static void
sp_dropper_context_setup (SPEventContext *ec)
{
	SPDropperContext *dc;
	SPCurve *c;

	dc = SP_DROPPER_CONTEXT (ec);

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	c = sp_curve_new_from_static_bpath ((ArtBpath *) spdc_circle);
	dc->area = sp_canvas_bpath_new (SP_DT_CONTROLS (ec->desktop), c);
	sp_curve_unref (c);
	sp_canvas_bpath_set_fill (SP_CANVAS_BPATH (dc->area), 0x00000000, 0);
	sp_canvas_bpath_set_stroke (SP_CANVAS_BPATH (dc->area), 0x0000007f, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
	sp_canvas_item_hide (dc->area);
}

static void
sp_dropper_context_finish (SPEventContext *ec)
{
	SPDropperContext *dc;

	dc = SP_DROPPER_CONTEXT (ec);

	if (dc->area) {
		gtk_object_destroy (GTK_OBJECT (dc->area));
		dc->area = NULL;
	}
}

static gint
sp_dropper_context_root_handler (SPEventContext *ec, GdkEvent *event)
{
	SPDropperContext *dc;
	int ret;

	dc = (SPDropperContext *) ec;

	ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			dc->centre.x = event->button.x;
			dc->centre.y = event->button.y;
			dc->dragging = TRUE;
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dc->dragging) {
			NRMatrixD w2dt, sm;
			NRPointF cd;
			float rw, scale;
			int x0, y0, x1, y1;
			rw = hypot (event->button.x - dc->centre.x, event->button.y - dc->centre.y);
			rw = MIN (rw, 32);
			sp_desktop_w2d_xy_point (ec->desktop, &cd, dc->centre.x, dc->centre.y);
			sp_desktop_w2dt_affine (ec->desktop, &w2dt);
			scale = rw * NR_MATRIX_DF_EXPANSION (&w2dt);
			nr_matrix_d_set_scale (&sm, scale, scale);
			sm.c[4] = cd.x;
			sm.c[5] = cd.y;
			sp_canvas_item_affine_absolute (dc->area, NR_MATRIX_D_TO_DOUBLE (&sm));
			sp_canvas_item_show (dc->area);
			/* Get buffer */
			x0 = (int) floor (dc->centre.x - rw);
			y0 = (int) floor (dc->centre.y - rw);
			x1 = (int) ceil (dc->centre.x + rw);
			y1 = (int) ceil (dc->centre.y + rw);
			if ((x1 > x0) && (y1 > y0)) {
				double W, R, G, B, A;
				NRPixBlock pb;
				SPColor color;
				int x, y;
				nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8A8P, x0, y0, x1, y1, TRUE);
				/* fixme: (Lauris) */
				sp_canvas_arena_render_pixblock (SP_CANVAS_ARENA (SP_DT_DRAWING (ec->desktop)), &pb);
				W = R = G = B = A = 0.0;
				for (y = y0; y < y1; y++) {
					const unsigned char *s = NR_PIXBLOCK_PX (&pb) + (y - y0) * pb.rs;
					for (x = x0; x < x1; x++) {
						double dx, dy, w;
						dx = x - dc->centre.x;
						dy = y - dc->centre.y;
						w = exp (-((dx * dx) + (dy * dy)) / (rw * rw));
						W += w;
						R += w * s[0];
						G += w * s[1];
						B += w * s[2];
						A += w * s[3];
						s += 4;
					}
				}
				nr_pixblock_release (&pb);
				R = (R + 0.001) / (255.0 * W);
				G = (G + 0.001) / (255.0 * W);
				B = (B + 0.001) / (255.0 * W);
				A = (A + 0.001) / (255.0 * W);
				R = CLAMP (R, 0.0, 1.0);
				G = CLAMP (G, 0.0, 1.0);
				B = CLAMP (B, 0.0, 1.0);
				A = CLAMP (A, 0.0, 1.0);
				sp_color_set_rgb_float (&color, R, G, B);
				sodipodi_set_color (&color, A);
			}
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			sp_canvas_item_hide (dc->area);
			dc->dragging = FALSE;
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->root_handler)
			ret = ((SPEventContextClass *) parent_class)->root_handler (ec, event);
	}

	return ret;
}

