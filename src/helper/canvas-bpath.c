#define __SP_CANVAS_BPATH_C__

/*
 * Simple bezier bpath CanvasItem for sodipodi
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_point.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_rgb_svp.h>
#include "sp-canvas.h"
#include "sp-canvas-util.h"
#include "canvas-bpath.h"

static void sp_canvas_bpath_class_init (SPCanvasBPathClass *klass);
static void sp_canvas_bpath_init (SPCanvasBPath *path);
static void sp_canvas_bpath_destroy (GtkObject *object);

static void sp_canvas_bpath_update (SPCanvasItem *item, double *affine, unsigned int flags);
static void sp_canvas_bpath_render (SPCanvasItem *item, SPCanvasBuf *buf);
static double sp_canvas_bpath_point (SPCanvasItem *item, double x, double y, SPCanvasItem **actual_item);
 
static SPCanvasItemClass *parent_class;

GtkType
sp_canvas_bpath_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPCanvasBPath",
			sizeof (SPCanvasBPath),
			sizeof (SPCanvasBPathClass),
			(GtkClassInitFunc) sp_canvas_bpath_class_init,
			(GtkObjectInitFunc) sp_canvas_bpath_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
	}
	return type;
}

static void
sp_canvas_bpath_class_init (SPCanvasBPathClass *klass)
{
	GtkObjectClass *object_class;
	SPCanvasItemClass *item_class;

	object_class = GTK_OBJECT_CLASS (klass);
	item_class = (SPCanvasItemClass *) klass;

	parent_class = gtk_type_class (SP_TYPE_CANVAS_ITEM);

	object_class->destroy = sp_canvas_bpath_destroy;

	item_class->update = sp_canvas_bpath_update;
	item_class->render = sp_canvas_bpath_render;
	item_class->point = sp_canvas_bpath_point;
}

static void
sp_canvas_bpath_init (SPCanvasBPath * bpath)
{
	bpath->fill_rgba = 0x000000ff;
	bpath->fill_rule = ART_WIND_RULE_ODDEVEN;

	bpath->stroke_rgba = 0x00000000;
	bpath->stroke_width = 1.0;
	bpath->stroke_linejoin = ART_PATH_STROKE_JOIN_MITER;
	bpath->stroke_linecap = ART_PATH_STROKE_CAP_BUTT;
	bpath->stroke_miterlimit = 11.0;

	bpath->fill_svp = NULL;
	bpath->stroke_svp = NULL;
}

static void
sp_canvas_bpath_destroy (GtkObject *object)
{
	SPCanvasBPath *cbp;

	cbp = SP_CANVAS_BPATH (object);

	if (cbp->fill_svp) {
		art_svp_free (cbp->fill_svp);
		cbp->fill_svp = NULL;
	}

	if (cbp->stroke_svp) {
		art_svp_free (cbp->stroke_svp);
		cbp->stroke_svp = NULL;
	}

	if (cbp->curve) {
		cbp->curve = sp_curve_unref (cbp->curve);
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_canvas_bpath_update (SPCanvasItem *item, double *affine, unsigned int flags)
{
	SPCanvasBPath *cbp;
	ArtDRect dbox, pbox;
	ArtIRect ibox;

	cbp = SP_CANVAS_BPATH (item);

	sp_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2, item->y2);

	if (((SPCanvasItemClass *) parent_class)->update)
		((SPCanvasItemClass *) parent_class)->update (item, affine, flags);

	sp_canvas_item_reset_bounds (item);

	if (cbp->fill_svp) {
		art_svp_free (cbp->fill_svp);
		cbp->fill_svp = NULL;
	}

	if (cbp->stroke_svp) {
		art_svp_free (cbp->stroke_svp);
		cbp->stroke_svp = NULL;
	}

	if (!cbp->curve) return;

	dbox.x0 = dbox.y0 = 0.0;
	dbox.x1 = dbox.y1 = -1.0;

	if ((cbp->fill_rgba & 0xff) || (cbp->stroke_rgba & 0xff)) {
		ArtBpath *bp;
		ArtVpath *vp, *pp;
		bp = art_bpath_affine_transform (cbp->curve->bpath, affine);
		vp = art_bez_path_to_vec (bp, 0.25);
		art_free (bp);
		pp = art_vpath_perturb (vp);
		art_free (vp);

		if ((cbp->fill_rgba & 0xff) && (cbp->curve->end > 2)) {
			ArtSVP *svpa, *svpb;
			svpa = art_svp_from_vpath (pp);
			svpb = art_svp_uncross (svpa);
			art_svp_free (svpa);
			cbp->fill_svp = art_svp_rewind_uncrossed (svpb, cbp->fill_rule);
			art_svp_free (svpb);
			art_drect_svp (&pbox, cbp->fill_svp);
			art_drect_union (&dbox, &dbox, &pbox);
		}

		if ((cbp->stroke_rgba & 0xff) && (cbp->curve->end > 1)) {
			cbp->stroke_svp = art_svp_vpath_stroke (pp, cbp->stroke_linejoin, cbp->stroke_linecap,
								cbp->stroke_width, cbp->stroke_miterlimit, 0.25);
			art_drect_svp (&pbox, cbp->stroke_svp);
			art_drect_union (&dbox, &dbox, &pbox);
		}

		art_free (pp);
	}


	art_drect_to_irect (&ibox, &dbox);

	item->x1 = ibox.x0;
	item->y1 = ibox.y0;
	item->x2 = ibox.x1;
	item->y2 = ibox.y1;

	sp_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2, item->y2);
}

static void
sp_canvas_bpath_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
	SPCanvasBPath *cbp;

	cbp = SP_CANVAS_BPATH (item);

	if (buf->is_bg) {
		sp_canvas_clear_buffer (buf);
		buf->is_bg = FALSE;
		buf->is_buf = TRUE;
	}

	if (cbp->fill_svp) {
		art_rgb_svp_alpha (cbp->fill_svp, buf->rect.x0, buf->rect.y0, buf->rect.x1, buf->rect.y1, cbp->fill_rgba,
				   buf->buf, buf->buf_rowstride,
				   NULL);
	}

	if (cbp->stroke_svp) {
		art_rgb_svp_alpha (cbp->stroke_svp, buf->rect.x0, buf->rect.y0, buf->rect.x1, buf->rect.y1, cbp->stroke_rgba,
				   buf->buf, buf->buf_rowstride,
				   NULL);
	}
}

#define BIGVAL 1e18

static double
sp_canvas_bpath_point (SPCanvasItem *item, double x, double y, SPCanvasItem **actual_item)
{
	SPCanvasBPath *cbp;
	gint wind;
	gdouble dist;

	cbp = SP_CANVAS_BPATH (item);

	if (cbp->fill_svp) {
		wind = art_svp_point_wind (cbp->fill_svp, x, y);
		if (wind) {
			*actual_item = item;
			return 0.0;
		}
	}

	if (cbp->stroke_svp) {
		wind = art_svp_point_wind (cbp->stroke_svp, x, y);
		if (wind) {
			*actual_item = item;
			return 0.0;
		}
		dist = art_svp_point_dist (cbp->stroke_svp, x, y);
		return dist;
	}

	if (cbp->fill_svp) {
		dist = art_svp_point_dist (cbp->fill_svp, x, y);
		return dist;
	}

	return BIGVAL;
}

SPCanvasItem *
sp_canvas_bpath_new (SPCanvasGroup *parent, SPCurve *curve)
{
	SPCanvasItem *item;

	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);

	item = sp_canvas_item_new (parent, SP_TYPE_CANVAS_BPATH, NULL);

	sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (item), curve);

	return item;
}

void
sp_canvas_bpath_set_bpath (SPCanvasBPath *cbp, SPCurve *curve)
{
	g_return_if_fail (cbp != NULL);
	g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

	if (cbp->curve) {
		cbp->curve = sp_curve_unref (cbp->curve);
	}

	if (curve) {
		cbp->curve = sp_curve_ref (curve);
	}

	sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

void
sp_canvas_bpath_set_fill (SPCanvasBPath *cbp, guint32 rgba, ArtWindRule rule)
{
	g_return_if_fail (cbp != NULL);
	g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

	cbp->fill_rgba = rgba;
	cbp->fill_rule = rule;

	sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

void
sp_canvas_bpath_set_stroke (SPCanvasBPath *cbp, guint32 rgba, gdouble width, ArtPathStrokeJoinType join, ArtPathStrokeCapType cap)
{
	g_return_if_fail (cbp != NULL);
	g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

	cbp->stroke_rgba = rgba;
	cbp->stroke_width = MAX (width, 0.1);
	cbp->stroke_linejoin = join;
	cbp->stroke_linecap = cap;

	sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

