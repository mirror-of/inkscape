#define __NR_ARENA_SHAPE_C__

/*
 * RGBA display list system for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <math.h>
#include <string.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-path.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-stroke.h>
#include <libnr/nr-svp-render.h>

#include <libnr/nr-svp-private.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>

#include "../style.h"
#include "nr-arena.h"
#include "nr-arena-shape.h"

static void nr_arena_shape_class_init (NRArenaShapeClass *klass);
static void nr_arena_shape_init (NRArenaShape *shape);
static void nr_arena_shape_finalize (NRObject *object);

static NRArenaItem *nr_arena_shape_children (NRArenaItem *item);
static void nr_arena_shape_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);
static void nr_arena_shape_remove_child (NRArenaItem *item, NRArenaItem *child);
static void nr_arena_shape_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);

static guint nr_arena_shape_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static unsigned int nr_arena_shape_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static guint nr_arena_shape_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_shape_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky);

static NRArenaItemClass *shape_parent_class;

NRType
nr_arena_shape_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
						"NRArenaShape",
						sizeof (NRArenaShapeClass),
						sizeof (NRArenaShape),
						(void (*) (NRObjectClass *)) nr_arena_shape_class_init,
						(void (*) (NRObject *)) nr_arena_shape_init);
	}
	return type;
}

static void
nr_arena_shape_class_init (NRArenaShapeClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	shape_parent_class = (NRArenaItemClass *)  ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_shape_finalize;

	item_class->children = nr_arena_shape_children;
	item_class->add_child = nr_arena_shape_add_child;
	item_class->set_child_position = nr_arena_shape_set_child_position;
	item_class->remove_child = nr_arena_shape_remove_child;
	item_class->update = nr_arena_shape_update;
	item_class->render = nr_arena_shape_render;
	item_class->clip = nr_arena_shape_clip;
	item_class->pick = nr_arena_shape_pick;
}

static void
nr_arena_shape_init (NRArenaShape *shape)
{
	shape->curve = NULL;
	shape->style = NULL;
	shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
	shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;

	nr_matrix_d_set_identity (&shape->ctm);
	shape->fill_painter = NULL;
	shape->stroke_painter = NULL;
	shape->fill_svp = NULL;
	shape->stroke_svp = NULL;
}

static void
nr_arena_shape_finalize (NRObject *object)
{
	NRArenaItem *item;
	NRArenaShape *shape;

	item = (NRArenaItem *) object;
	shape = (NRArenaShape *) (object);

	while (shape->markers) {
		shape->markers = nr_arena_item_detach_unref (item, shape->markers);
	}

	if (shape->fill_svp) nr_svp_free (shape->fill_svp);
	if (shape->stroke_svp) nr_svp_free (shape->stroke_svp);
	if (shape->fill_painter) sp_painter_free (shape->fill_painter);
	if (shape->stroke_painter) sp_painter_free (shape->stroke_painter);
	if (shape->style) sp_style_unref (shape->style);
	if (shape->curve) sp_curve_unref (shape->curve);

	((NRObjectClass *) shape_parent_class)->finalize (object);
}

static NRArenaItem *
nr_arena_shape_children (NRArenaItem *item)
{
	NRArenaShape *shape;

	shape = (NRArenaShape *) item;

	return shape->markers;
}

static void
nr_arena_shape_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	NRArenaShape *shape;

	shape = (NRArenaShape *) item;

	if (!ref) {
		shape->markers = nr_arena_item_attach_ref (item, child, NULL, shape->markers);
	} else {
		ref->next = nr_arena_item_attach_ref (item, child, ref, ref->next);
	}

	nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_shape_remove_child (NRArenaItem *item, NRArenaItem *child)
{
	NRArenaShape *shape;

	shape = (NRArenaShape *) item;

	if (child->prev) {
		nr_arena_item_detach_unref (item, child);
	} else {
		shape->markers = nr_arena_item_detach_unref (item, child);
	}

	nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_shape_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	NRArenaShape *shape;

	shape = (NRArenaShape *) item;

	nr_arena_item_ref (child);

	if (child->prev) {
		nr_arena_item_detach_unref (item, child);
	} else {
		shape->markers = nr_arena_item_detach_unref (item, child);
	}

	if (!ref) {
		shape->markers = nr_arena_item_attach_ref (item, child, NULL, shape->markers);
	} else {
		ref->next = nr_arena_item_attach_ref (item, child, ref, ref->next);
	}

	nr_arena_item_unref (child);

	nr_arena_item_request_render (child);
}

#include "enums.h"

static guint
nr_arena_shape_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
	NRArenaShape *shape;
	NRArenaItem *child;
	SPStyle *style;
	NRRectF bbox;
	unsigned int newstate, beststate;

	shape = NR_ARENA_SHAPE (item);
	style = shape->style;

	beststate = NR_ARENA_ITEM_STATE_ALL;

	for (child = shape->markers; child != NULL; child = child->next) {
		newstate = nr_arena_item_invoke_update (child, area, gc, state, reset);
		beststate = beststate & newstate;
	}

	if (!(state & NR_ARENA_ITEM_STATE_RENDER)) {
		/* We do not have to create rendering structures */
		shape->ctm = gc->transform;
		if (state & NR_ARENA_ITEM_STATE_BBOX) {
			if (shape->curve) {
				NRMatrixF ctm;
				NRBPath bp;
				/* fixme: */
				bbox.x0 = bbox.y0 = NR_HUGE_F;
				bbox.x1 = bbox.y1 = -NR_HUGE_F;
				nr_matrix_f_from_d (&ctm, &gc->transform);
				bp.path = shape->curve->bpath;
				nr_path_matrix_f_bbox_f_union (&bp, &ctm, &bbox, 1.0);
				item->bbox.x0 = bbox.x0 - 1.0F;
				item->bbox.y0 = bbox.y0 - 1.0F;
				item->bbox.x1 = bbox.x1 + 1.9999F;
				item->bbox.y1 = bbox.y1 + 1.9999F;
			}
			if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
				for (child = shape->markers; child != NULL; child = child->next) {
					nr_rect_l_union (&item->bbox, &item->bbox, &child->bbox);
				}
			}
		}
		return (state | item->state);
	}

	/* Request repaint old area if needed */
	/* fixme: Think about it a bit (Lauris) */
	/* fixme: Thios is only needed, if actually rendered/had svp (Lauris) */
	if (!nr_rect_l_test_empty (&item->bbox)) {
		nr_arena_request_render_rect (item->arena, &item->bbox);
		nr_rect_l_set_empty (&item->bbox);
	}

	/* Release state data */
	if (TRUE || !nr_matrix_d_test_transform_equal (&gc->transform, &shape->ctm, NR_EPSILON_D)) {
		/* Concept test */
		if (shape->fill_svp) {
			nr_svp_free (shape->fill_svp);
			shape->fill_svp = NULL;
		}
	}
	if (shape->stroke_svp) {
		nr_svp_free (shape->stroke_svp);
		shape->stroke_svp = NULL;
	}
	if (shape->fill_painter) {
		sp_painter_free (shape->fill_painter);
		shape->fill_painter = NULL;
	}
	if (shape->stroke_painter) {
		sp_painter_free (shape->stroke_painter);
		shape->stroke_painter = NULL;
	}

	if (!shape->curve || !shape->style) return NR_ARENA_ITEM_STATE_ALL;
	if (sp_curve_is_empty (shape->curve)) return NR_ARENA_ITEM_STATE_ALL;
	if ((shape->style->fill.type == SP_PAINT_TYPE_NONE) && (shape->style->stroke.type == SP_PAINT_TYPE_NONE)) return NR_ARENA_ITEM_STATE_ALL;

	/* Build state data */
	if (shape->style->fill.type != SP_PAINT_TYPE_NONE) {
		if ((shape->curve->end > 2) || (shape->curve->bpath[1].code == ART_CURVETO)) {
			if (TRUE || !shape->fill_svp) {
				NRMatrixF ctmf;
				NRSVL *svl;
				unsigned int windrule;
				nr_matrix_f_from_d (&ctmf, &gc->transform);
				windrule = (shape->style->fill_rule.value == SP_WIND_RULE_EVENODD) ? NR_WIND_RULE_EVENODD : NR_WIND_RULE_NONZERO;
				svl = nr_svl_from_art_bpath (shape->curve->bpath, &ctmf, windrule, TRUE, 0.25);
				shape->fill_svp = nr_svp_from_svl (svl, NULL);
				nr_svl_free_list (svl);
			} else if (!NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (&gc->transform, &NR_MATRIX_D_IDENTITY, NR_EPSILON_D)) {
#if 0
				ArtSVP *svpa;
				/* Concept test */
				svpa = art_svp_translate (shape->fill_svp,
							  gc->transform.c[4] - shape->ctm.c[4],
							  gc->transform.c[5] - shape->ctm.c[5]);
				art_svp_free (shape->fill_svp);
				shape->fill_svp = svpa;
#endif
			}
			shape->ctm = gc->transform;
		}
	}

	if (style->stroke.type != SP_PAINT_TYPE_NONE) {

		NRBPath bp;
		float width, scale;
		NRSVL *svl;
		scale = NR_MATRIX_DF_EXPANSION (&gc->transform);
		width = MAX (0.125, style->stroke_width.computed * scale);
		bp.path = art_bpath_affine_transform (shape->curve->bpath, NR_MATRIX_D_TO_DOUBLE (&gc->transform));
		if (!style->stroke_dash.n_dash) {
			svl = nr_bpath_stroke (&bp, NULL, width,
					       shape->style->stroke_linecap.value,
					       shape->style->stroke_linejoin.value,
					       shape->style->stroke_miterlimit.value * M_PI / 180.0,
					       0.25);
		} else {
			double dlen;
			int i;
			ArtVpath *vp, *pvp;
			ArtSVP *asvp;
			vp = art_bez_path_to_vec (bp.path, 0.25);
			pvp = art_vpath_perturb (vp);
			art_free (vp);
			dlen = 0.0;
			for (i = 0; i < style->stroke_dash.n_dash; i++) dlen += style->stroke_dash.dash[i] * scale;
			if (dlen >= 1.0) {
				ArtVpathDash dash;
				int i;
				dash.offset = style->stroke_dash.offset * scale;
				dash.n_dash = style->stroke_dash.n_dash;
				dash.dash = g_new (double, dash.n_dash);
				for (i = 0; i < dash.n_dash; i++) {
					dash.dash[i] = style->stroke_dash.dash[i] * scale;
				}
				vp = art_vpath_dash (pvp, &dash);
				art_free (pvp);
				pvp = vp;
				g_free (dash.dash);
			}
			asvp = art_svp_vpath_stroke (pvp,
						     shape->style->stroke_linejoin.value,
						     shape->style->stroke_linecap.value,
						     width,
						     shape->style->stroke_miterlimit.value, 0.25);
			art_free (pvp);
			svl = nr_svl_from_art_svp (asvp);
			art_svp_free (asvp);
		}
		shape->stroke_svp = nr_svp_from_svl (svl, NULL);
		nr_svl_free_list (svl);
		art_free (bp.path);
	}

	bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
	if (shape->stroke_svp && shape->stroke_svp->length > 0) {
		nr_svp_bbox (shape->stroke_svp, &bbox, FALSE);
	}
	if (shape->fill_svp && shape->fill_svp->length > 0) {
		nr_svp_bbox (shape->fill_svp, &bbox, FALSE);
	}
	if (nr_rect_f_test_empty (&bbox)) return NR_ARENA_ITEM_STATE_ALL;

	item->bbox.x0 = bbox.x0 - 1.0F;
	item->bbox.y0 = bbox.y0 - 1.0F;
	item->bbox.x1 = bbox.x1 + 1.0F;
	item->bbox.y1 = bbox.y1 + 1.0F;
	nr_arena_request_render_rect (item->arena, &item->bbox);

	item->render_opacity = TRUE;
	if (shape->style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
		shape->fill_painter = sp_paint_server_painter_new (SP_STYLE_FILL_SERVER (shape->style),
								   NR_MATRIX_D_TO_DOUBLE (&gc->transform), &shape->paintbox);
		item->render_opacity = FALSE;
	}
	if (shape->style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) {
		shape->stroke_painter = sp_paint_server_painter_new (SP_STYLE_STROKE_SERVER (shape->style),
								     NR_MATRIX_D_TO_DOUBLE (&gc->transform), &shape->paintbox);
		item->render_opacity = FALSE;
	}

	if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
		for (child = shape->markers; child != NULL; child = child->next) {
			nr_rect_l_union (&item->bbox, &item->bbox, &child->bbox);
		}
	}

	return NR_ARENA_ITEM_STATE_ALL;
}

static unsigned int
nr_arena_shape_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRArenaShape *shape;
	NRArenaItem *child;
	SPStyle *style;

	shape = NR_ARENA_SHAPE (item);

	if (!shape->curve) return item->state;
	if (!shape->style) return item->state;

	style = shape->style;

	if (shape->fill_svp) {
		NRPixBlock m;
		guint32 rgba;

		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		nr_pixblock_render_svp_mask_or (&m, shape->fill_svp);
		m.empty = FALSE;

		switch (style->fill.type) {
		case SP_PAINT_TYPE_COLOR:
			rgba = sp_color_get_rgba32_falpha (&style->fill.value.color,
							   SP_SCALE24_TO_FLOAT (style->fill_opacity.value) *
							   SP_SCALE24_TO_FLOAT (style->opacity.value));
			nr_blit_pixblock_mask_rgba32 (pb, &m, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (shape->fill_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				shape->fill_painter->fill (shape->fill_painter, &cb);
				cb.empty = FALSE;
				/* Composite */
				nr_blit_pixblock_pixblock_mask (pb, &cb, &m);
				pb->empty = FALSE;
				nr_pixblock_release (&cb);
			}
			break;
		default:
			break;
		}
		nr_pixblock_release (&m);
	}

	if (shape->stroke_svp) {
		NRPixBlock m;
		guint32 rgba;

		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		nr_pixblock_render_svp_mask_or (&m, shape->stroke_svp);
		m.empty = FALSE;

		switch (style->stroke.type) {
		case SP_PAINT_TYPE_COLOR:
			rgba = sp_color_get_rgba32_falpha (&style->stroke.value.color,
							   SP_SCALE24_TO_FLOAT (style->stroke_opacity.value) *
							   SP_SCALE24_TO_FLOAT (style->opacity.value));
			nr_blit_pixblock_mask_rgba32 (pb, &m, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (shape->stroke_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				shape->stroke_painter->fill (shape->stroke_painter, &cb);
				cb.empty = FALSE;
				/* Composite */
				nr_blit_pixblock_pixblock_mask (pb, &cb, &m);
				pb->empty = FALSE;
				nr_pixblock_release (&cb);
			}
			break;
		default:
			break;
		}
		nr_pixblock_release (&m);
	}

	/* Just compose children into parent buffer */
	for (child = shape->markers; child != NULL; child = child->next) {
		unsigned int ret;
		ret = nr_arena_item_invoke_render (child, area, pb, flags);
		if (ret & NR_ARENA_ITEM_STATE_INVALID) return ret;
	}

	return item->state;
}

static guint
nr_arena_shape_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaShape *shape;

	shape = NR_ARENA_SHAPE (item);

	if (!shape->curve) return item->state;

	if (shape->fill_svp) {
		NRPixBlock m;
		int x, y;

		/* fixme: We can OR in one step (Lauris) */
		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		nr_pixblock_render_svp_mask_or (&m, shape->fill_svp);

		for (y = area->y0; y < area->y1; y++) {
			unsigned char *s, *d;
			s = NR_PIXBLOCK_PX (&m) + (y - area->y0) * m.rs;
			d = NR_PIXBLOCK_PX (pb) + (y - area->y0) * pb->rs;
			for (x = area->x0; x < area->x1; x++) {
				*d = (NR_A7 (*s, *d) + 127) / 255;
				d += 1;
				s += 1;
			}
		}
		nr_pixblock_release (&m);
		pb->empty = FALSE;
	}

	return item->state;
}

static NRArenaItem *
nr_arena_shape_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky)
{
	NRArenaShape *shape;

	shape = NR_ARENA_SHAPE (item);

	if (!shape->curve) return NULL;
	if (!shape->style) return NULL;

	if (item->state & NR_ARENA_ITEM_STATE_RENDER) {
		if (shape->fill_svp && (shape->style->fill.type != SP_PAINT_TYPE_NONE)) {
			if (nr_svp_point_wind (shape->fill_svp, (float) x, (float) y)) return item;
		}
		if (shape->stroke_svp && (shape->style->stroke.type != SP_PAINT_TYPE_NONE)) {
			if (nr_svp_point_wind (shape->stroke_svp, (float) x, (float) y)) return item;
		}
		if (delta > 1e-3) {
			if (shape->fill_svp && (shape->style->fill.type != SP_PAINT_TYPE_NONE)) {
				if (nr_svp_point_distance (shape->fill_svp, (float) x, (float) y) <= delta) return item;
			}
			if (shape->stroke_svp && (shape->style->stroke.type != SP_PAINT_TYPE_NONE)) {
				if (nr_svp_point_distance (shape->stroke_svp, (float) x, (float) y) <= delta) return item;
			}
		}
	} else {
		NRMatrixF t;
		NRPointF pt;
		NRBPath bp;
		float dist;
		int wind;
		pt.x = (float) x;
		pt.y = (float) y;
		nr_matrix_f_from_d (&t, &shape->ctm);
		bp.path = shape->curve->bpath;
		dist = NR_HUGE_F;
		wind = 0;
		nr_path_matrix_f_point_f_bbox_wind_distance (&bp, &t, &pt, NULL, &wind, &dist, NR_EPSILON_F);
		if (shape->style->fill.type != SP_PAINT_TYPE_NONE) {
			if (!shape->style->fill_rule.value) {
				if (wind != 0) return item;
			} else {
				if (wind & 0x1) return item;
			}
		}
		if (shape->style->stroke.type != SP_PAINT_TYPE_NONE) {
			/* fixme: We do not take stroke width into account here (Lauris) */
			if (dist < delta) return item;
		}
	}

	return NULL;
}

void
nr_arena_shape_set_path (NRArenaShape *shape, SPCurve *curve, unsigned int private, const double *affine)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (NR_IS_ARENA_SHAPE (shape));

	nr_arena_item_request_render (NR_ARENA_ITEM (shape));

	if (shape->curve) {
		sp_curve_unref (shape->curve);
		shape->curve = NULL;
	}

	if (curve) {
		if (affine) {
			ArtBpath *abp;
			abp = art_bpath_affine_transform (curve->bpath, affine);
			curve = sp_curve_new_from_bpath (abp);
			shape->curve = curve;
		} else {
			shape->curve = curve;
			sp_curve_ref (curve);
		}
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_shape_set_style (NRArenaShape *shape, SPStyle *style)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (NR_IS_ARENA_SHAPE (shape));

	if (style) sp_style_ref (style);
	if (shape->style) sp_style_unref (shape->style);
	shape->style = style;

	nr_arena_item_request_update (NR_ARENA_ITEM (shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_shape_set_paintbox (NRArenaShape *shape, const NRRectF *pbox)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (NR_IS_ARENA_SHAPE (shape));
	g_return_if_fail (pbox != NULL);

	if ((pbox->x0 < pbox->x1) && (pbox->y0 < pbox->y1)) {
		shape->paintbox = *pbox;
	} else {
		/* fixme: We kill warning, although not sure what to do here (Lauris) */
		shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
		shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

