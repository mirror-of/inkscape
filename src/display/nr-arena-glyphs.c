#define __NR_ARENA_GLYPHS_C__

/*
 * RGBA display list system for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 *
 */


#include <math.h>
#include <string.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-blit.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_svp_point.h>
#include <libart_lgpl/art_gray_svp.h>
#include "../style.h"
#include "nr-arena.h"
#include "nr-arena-glyphs.h"

static void nr_arena_glyphs_class_init (NRArenaGlyphsClass *klass);
static void nr_arena_glyphs_init (NRArenaGlyphs *glyphs);
static void nr_arena_glyphs_finalize (NRObject *object);

static guint nr_arena_glyphs_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static guint nr_arena_glyphs_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_glyphs_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky);

static NRArenaItemClass *glyphs_parent_class;

NRType
nr_arena_glyphs_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
						"NRArenaGlyphs",
						sizeof (NRArenaGlyphsClass),
						sizeof (NRArenaGlyphs),
						(void (*) (NRObjectClass *)) nr_arena_glyphs_class_init,
						(void (*) (NRObject *)) nr_arena_glyphs_init);
	}
	return type;
}

static void
nr_arena_glyphs_class_init (NRArenaGlyphsClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	glyphs_parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_glyphs_finalize;

	item_class->update = nr_arena_glyphs_update;
	item_class->clip = nr_arena_glyphs_clip;
	item_class->pick = nr_arena_glyphs_pick;
}

static void
nr_arena_glyphs_init (NRArenaGlyphs *glyphs)
{
	glyphs->curve = NULL;
	glyphs->style = NULL;

	glyphs->stroke_svp = NULL;
}

static void
nr_arena_glyphs_finalize (NRObject *object)
{
	NRArenaGlyphs *glyphs;

	glyphs = NR_ARENA_GLYPHS (object);

	if (glyphs->stroke_svp) {
		art_svp_free (glyphs->stroke_svp);
		glyphs->stroke_svp = NULL;
	}

	if (glyphs->rfont) {
		glyphs->rfont = nr_rasterfont_unref (glyphs->rfont);
	}

	if (glyphs->font) {
		glyphs->font = nr_font_unref (glyphs->font);
	}

	if (glyphs->style) {
		sp_style_unref (glyphs->style);
		glyphs->style = NULL;
	}

	if (glyphs->curve) {
		glyphs->curve = sp_curve_unref (glyphs->curve);
	}

	((NRObjectClass *) glyphs_parent_class)->finalize (object);
}

static guint
nr_arena_glyphs_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
	NRArenaGlyphs *glyphs;
	NRRasterFont *rfont;
	NRMatrixF t;
	ArtBpath *abp;
	ArtVpath *vp, *pvp;
	ArtDRect bbox;

	glyphs = NR_ARENA_GLYPHS (item);

	/* Request repaint old area if needed */
	/* fixme: Think about it a bit (Lauris) */
	if (!nr_rect_l_test_empty (&item->bbox)) {
		nr_arena_request_render_rect (item->arena, &item->bbox);
		nr_rect_l_set_empty (&item->bbox);
	}

	/* Release state data */
	if (glyphs->stroke_svp) {
		art_svp_free (glyphs->stroke_svp);
		glyphs->stroke_svp = NULL;
	}

	if (!glyphs->font || !glyphs->curve || !glyphs->style) return NR_ARENA_ITEM_STATE_ALL;
	if ((glyphs->style->fill.type == SP_PAINT_TYPE_NONE) && (glyphs->style->stroke.type == SP_PAINT_TYPE_NONE)) return NR_ARENA_ITEM_STATE_ALL;

	bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;

	if (glyphs->style->fill.type != SP_PAINT_TYPE_NONE) {
		NRRectF area;
		nr_matrix_multiply_ffd (&t, &glyphs->transform, &gc->transform);
		rfont = nr_rasterfont_new (glyphs->font, &t);
		if (glyphs->rfont) glyphs->rfont = nr_rasterfont_unref (glyphs->rfont);
		glyphs->rfont = rfont;
		glyphs->x = t.c[4];
		glyphs->y = t.c[5];
		nr_rasterfont_glyph_area_get (rfont, glyphs->glyph, &area);
		bbox.x0 = area.x0 + glyphs->x;
		bbox.y0 = area.y0 + glyphs->y;
		bbox.x1 = area.x1 + glyphs->x;
		bbox.y1 = area.y1 + glyphs->y;
	}

	if (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE) {
		/* Build state data */
		abp = art_bpath_affine_transform (glyphs->curve->bpath, NR_MATRIX_D_TO_DOUBLE (&gc->transform));
		vp = art_bez_path_to_vec (abp, 0.25);
		art_free (abp);
		pvp = art_vpath_perturb (vp);
		art_free (vp);

		if (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE) {
			gdouble width;
			width = glyphs->style->stroke_width.computed * NR_MATRIX_DF_EXPANSION (&gc->transform);
			width = MAX (0.125, width);
			glyphs->stroke_svp = art_svp_vpath_stroke (pvp,
								   glyphs->style->stroke_linejoin.value,
								   glyphs->style->stroke_linecap.value,
								   width,
								   glyphs->style->stroke_miterlimit.value, 0.25);
		}

		art_free (pvp);
	}

	if (glyphs->stroke_svp) art_drect_svp_union (&bbox, glyphs->stroke_svp);
	if (art_drect_empty (&bbox)) return NR_ARENA_ITEM_STATE_ALL;

	item->bbox.x0 = bbox.x0 - 1.0;
	item->bbox.y0 = bbox.y0 - 1.0;
	item->bbox.x1 = bbox.x1 + 1.0;
	item->bbox.y1 = bbox.y1 + 1.0;
	nr_arena_request_render_rect (item->arena, &item->bbox);

	return NR_ARENA_ITEM_STATE_ALL;
}

static guint
nr_arena_glyphs_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaGlyphs *glyphs;

	glyphs = NR_ARENA_GLYPHS (item);

	if (!glyphs->font || !glyphs->curve) return item->state;

#if 0
	/* fixme: Implement */
	if (glyphs->fill_svp) {
		art_gray_svp_aa (glyphs->fill_svp, area->x0, area->y0, area->x1, area->y1, b->px, b->rs);
		b->empty = FALSE;
	}
#endif

	return item->state;
}

static NRArenaItem *
nr_arena_glyphs_pick (NRArenaItem *item, gdouble x, gdouble y, gdouble delta, unsigned int sticky)
{
	NRArenaGlyphs *glyphs;

	glyphs = NR_ARENA_GLYPHS (item);

	if (!glyphs->font || !glyphs->curve) return NULL;
	if (!glyphs->style) return NULL;

	/* fixme: */
	if ((x >= item->bbox.x0) && (y >= item->bbox.y0) && (x < item->bbox.x1) && (y < item->bbox.y1)) return item;

	if (glyphs->stroke_svp && (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE)) {
		if (art_svp_point_wind (glyphs->stroke_svp, x, y)) return item;
	}
	if (delta > 1e-3) {
		if (glyphs->stroke_svp && (glyphs->style->stroke.type != SP_PAINT_TYPE_NONE)) {
			if (art_svp_point_dist (glyphs->stroke_svp, x, y) <= delta) return item;
		}
	}

	return NULL;
}

void
nr_arena_glyphs_set_path (NRArenaGlyphs *glyphs, SPCurve *curve, unsigned int private, NRFont *font, gint glyph, const NRMatrixF *transform)
{
	nr_return_if_fail (glyphs != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS (glyphs));

	nr_arena_item_request_render (NR_ARENA_ITEM (glyphs));

	if (glyphs->curve) {
		sp_curve_unref (glyphs->curve);
		glyphs->curve = NULL;
	}

	if (curve) {
		if (transform) {
			ArtBpath *abp;
			double a[6];
			a[0] = transform->c[0];
			a[1] = transform->c[1];
			a[2] = transform->c[2];
			a[3] = transform->c[3];
			a[4] = transform->c[4];
			a[5] = transform->c[5];
			abp = art_bpath_affine_transform (curve->bpath, a);
			curve = sp_curve_new_from_bpath (abp);
			glyphs->curve = curve;
			glyphs->transform = *transform;
		} else {
			glyphs->curve = curve;
			sp_curve_ref (curve);
			nr_matrix_f_set_identity (&glyphs->transform);
		}
	}

	if (glyphs->font) glyphs->font = nr_font_unref (glyphs->font);
	if (font) glyphs->font = nr_font_ref (font);
	glyphs->glyph = glyph;

	nr_arena_item_request_update (NR_ARENA_ITEM (glyphs), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_glyphs_set_style (NRArenaGlyphs *glyphs, SPStyle *style)
{
	nr_return_if_fail (glyphs != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS (glyphs));

	if (style) sp_style_ref (style);
	if (glyphs->style) sp_style_unref (glyphs->style);
	glyphs->style = style;

	nr_arena_item_request_update (NR_ARENA_ITEM (glyphs), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static guint
nr_arena_glyphs_fill_mask (NRArenaGlyphs *glyphs, NRRectL *area, NRPixBlock *m)
{
	NRArenaItem *item;

	/* fixme: area == m->area, so merge these */

	item = NR_ARENA_ITEM (glyphs);

	if (glyphs->rfont && nr_rect_l_test_intersect (area, &item->bbox)) {
		nr_rasterfont_glyph_mask_render (glyphs->rfont, glyphs->glyph, m, glyphs->x, glyphs->y);
	}

	return item->state;
}

static guint
nr_arena_glyphs_stroke_mask (NRArenaGlyphs *glyphs, NRRectL *area, NRPixBlock *m)
{
	NRArenaItem *item;

	item = NR_ARENA_ITEM (glyphs);

	if (glyphs->stroke_svp && nr_rect_l_test_intersect (area, &item->bbox)) {
		NRPixBlock gb;
		gint x, y;
		nr_pixblock_setup_fast (&gb, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		art_gray_svp_aa (glyphs->stroke_svp, area->x0, area->y0, area->x1, area->y1, NR_PIXBLOCK_PX (&gb), gb.rs);
		for (y = area->y0; y < area->y1; y++) {
			guchar *d, *s;
			d = NR_PIXBLOCK_PX (m) + (y - area->y0) * m->rs;
			s = NR_PIXBLOCK_PX (&gb) + (y - area->y0) * gb.rs;
			for (x = area->x0; x < area->x1; x++) {
				*d = (*d) + ((255 - *d) * (*s) / 255);
				d += 1;
				s += 1;
			}
		}
		nr_pixblock_release (&gb);
		m->empty = FALSE;
	}

	return item->state;
}

static void nr_arena_glyphs_group_class_init (NRArenaGlyphsGroupClass *klass);
static void nr_arena_glyphs_group_init (NRArenaGlyphsGroup *group);
static void nr_arena_glyphs_group_finalize (NRObject *object);

static guint nr_arena_glyphs_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static unsigned int nr_arena_glyphs_group_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static unsigned int nr_arena_glyphs_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_glyphs_group_pick (NRArenaItem *item, gdouble x, gdouble y, gdouble delta, unsigned int sticky);

static NRArenaGroupClass *group_parent_class;

NRType
nr_arena_glyphs_group_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_GROUP,
						"NRArenaGlyphsGroup",
						sizeof (NRArenaGlyphsGroupClass),
						sizeof (NRArenaGlyphsGroup),
						(void (*) (NRObjectClass *)) nr_arena_glyphs_group_class_init,
						(void (*) (NRObject *)) nr_arena_glyphs_group_init);
	}
	return type;
}

static void
nr_arena_glyphs_group_class_init (NRArenaGlyphsGroupClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	group_parent_class = (NRArenaGroupClass *) ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_glyphs_group_finalize;

	item_class->update = nr_arena_glyphs_group_update;
	item_class->render = nr_arena_glyphs_group_render;
	item_class->clip = nr_arena_glyphs_group_clip;
	item_class->pick = nr_arena_glyphs_group_pick;
}

static void
nr_arena_glyphs_group_init (NRArenaGlyphsGroup *group)
{
	group->style = NULL;
	group->paintbox.x0 = group->paintbox.y0 = 0.0F;
	group->paintbox.x1 = group->paintbox.y1 = 1.0F;

	group->fill_painter = NULL;
	group->stroke_painter = NULL;
}

static void
nr_arena_glyphs_group_finalize (NRObject *object)
{
	NRArenaGlyphsGroup *group;

	group = NR_ARENA_GLYPHS_GROUP (object);

	if (group->fill_painter) {
		sp_painter_free (group->fill_painter);
		group->fill_painter = NULL;
	}

	if (group->stroke_painter) {
		sp_painter_free (group->stroke_painter);
		group->stroke_painter = NULL;
	}

	if (group->style) {
		sp_style_unref (group->style);
		group->style = NULL;
	}

		((NRObjectClass *) group_parent_class)->finalize (object);
}

static guint
nr_arena_glyphs_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
	NRArenaGlyphsGroup *group;

	group = NR_ARENA_GLYPHS_GROUP (item);

	if (group->fill_painter) {
		sp_painter_free (group->fill_painter);
		group->fill_painter = NULL;
	}

	if (group->stroke_painter) {
		sp_painter_free (group->stroke_painter);
		group->stroke_painter = NULL;
	}

	item->render_opacity = TRUE;
	if (group->style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
		group->fill_painter = sp_paint_server_painter_new (SP_STYLE_FILL_SERVER (group->style),
								   NR_MATRIX_D_TO_DOUBLE (&gc->transform), &group->paintbox);
	item->render_opacity = FALSE;
	}

	if (group->style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) {
		group->stroke_painter = sp_paint_server_painter_new (SP_STYLE_STROKE_SERVER (group->style),
								     NR_MATRIX_D_TO_DOUBLE (&gc->transform), &group->paintbox);
	item->render_opacity = FALSE;
	}

	if (((NRArenaItemClass *) group_parent_class)->update)
		return ((NRArenaItemClass *) group_parent_class)->update (item, area, gc, state, reset);

	return NR_ARENA_ITEM_STATE_ALL;
}

/* This sucks - as soon, as we have inheritable renderprops, do something with that opacity */

static unsigned int
nr_arena_glyphs_group_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRArenaGroup *group;
	NRArenaGlyphsGroup *ggroup;
	NRArenaItem *child;
	SPStyle *style;
	guint ret;

	group = NR_ARENA_GROUP (item);
	ggroup = NR_ARENA_GLYPHS_GROUP (item);
	style = ggroup->style;

	ret = item->state;

	/* Fill */
	if (style->fill.type != SP_PAINT_TYPE_NONE) {
		NRPixBlock mb;
		guint32 rgba;
		nr_pixblock_setup_fast (&mb, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		/* Render children fill mask */
		for (child = group->children; child != NULL; child = child->next) {
			ret = nr_arena_glyphs_fill_mask (NR_ARENA_GLYPHS (child), area, &mb);
			if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) {
				nr_pixblock_release (&mb);
				return ret;
			}
		}
		/* Composite into buffer */
		switch (style->fill.type) {
		case SP_PAINT_TYPE_COLOR:
			rgba = sp_color_get_rgba32_falpha (&style->fill.value.color,
							   SP_SCALE24_TO_FLOAT (style->fill_opacity.value) *
							   SP_SCALE24_TO_FLOAT (style->opacity.value));
			nr_blit_pixblock_mask_rgba32 (pb, &mb, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (ggroup->fill_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				ggroup->fill_painter->fill (ggroup->fill_painter, &cb);
				cb.empty = FALSE;
				/* Composite */
				nr_blit_pixblock_pixblock_mask (pb, &cb, &mb);
				pb->empty = FALSE;
				nr_pixblock_release (&cb);
			}
			break;
		default:
			break;
		}
		nr_pixblock_release (&mb);
	}

	/* Stroke */
	if (style->stroke.type != SP_PAINT_TYPE_NONE) {
		NRPixBlock m;
		guint32 rgba;
		nr_pixblock_setup_fast (&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
		/* Render children stroke mask */
		for (child = group->children; child != NULL; child = child->next) {
			ret = nr_arena_glyphs_stroke_mask (NR_ARENA_GLYPHS (child), area, &m);
			if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) {
				nr_pixblock_release (&m);
				return ret;
			}
		}
		/* Composite into buffer */
		switch (style->stroke.type) {
		case SP_PAINT_TYPE_COLOR:
			rgba = sp_color_get_rgba32_falpha (&style->stroke.value.color,
							   SP_SCALE24_TO_FLOAT (style->stroke_opacity.value) *
							   SP_SCALE24_TO_FLOAT (style->opacity.value));
			nr_blit_pixblock_mask_rgba32 (pb, &m, rgba);
			pb->empty = FALSE;
			break;
		case SP_PAINT_TYPE_PAINTSERVER:
			if (ggroup->stroke_painter) {
				NRPixBlock cb;
				/* Need separate gradient buffer */
				nr_pixblock_setup_fast (&cb, NR_PIXBLOCK_MODE_R8G8B8A8N, area->x0, area->y0, area->x1, area->y1, TRUE);
				ggroup->stroke_painter->fill (ggroup->stroke_painter, &cb);
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

	return ret;
}

static unsigned int
nr_arena_glyphs_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaGroup *group;
	NRArenaGlyphsGroup *ggroup;
	NRArenaItem *child;
	guint ret;

	group = NR_ARENA_GROUP (item);
	ggroup = NR_ARENA_GLYPHS_GROUP (item);

	ret = item->state;

	/* Render children fill mask */
	for (child = group->children; child != NULL; child = child->next) {
		ret = nr_arena_glyphs_fill_mask (NR_ARENA_GLYPHS (child), area, pb);
		if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) return ret;
	}

	return ret;
}

static NRArenaItem *
nr_arena_glyphs_group_pick (NRArenaItem *item, gdouble x, gdouble y, gdouble delta, unsigned int sticky)
{
	NRArenaGroup *group;
	NRArenaItem *picked;

	group = NR_ARENA_GROUP (item);

	picked = NULL;

	if (((NRArenaItemClass *) group_parent_class)->pick)
		picked = ((NRArenaItemClass *) group_parent_class)->pick (item, x, y, delta, sticky);

	if (picked) picked = item;

	return picked;
}

void
nr_arena_glyphs_group_clear (NRArenaGlyphsGroup *sg)
{
	NRArenaGroup *group;

	group = NR_ARENA_GROUP (sg);

	nr_arena_item_request_render (NR_ARENA_ITEM (group));

	while (group->children) {
		nr_arena_item_remove_child (NR_ARENA_ITEM (group), group->children);
	}
}

void
nr_arena_glyphs_group_add_component (NRArenaGlyphsGroup *sg, NRFont *font, int glyph, const NRMatrixF *transform)
{
	NRArenaGroup *group;
	NRBPath bpath;

	group = NR_ARENA_GROUP (sg);

	if (nr_font_glyph_outline_get (font, glyph, &bpath, FALSE) && bpath.path) {
		SPCurve *curve;

		nr_arena_item_request_render (NR_ARENA_ITEM (group));

		curve = sp_curve_new_from_foreign_bpath (bpath.path);
		if (curve) {
			NRArenaItem *new;
			new = nr_arena_item_new (NR_ARENA_ITEM (group)->arena, NR_TYPE_ARENA_GLYPHS);
			nr_arena_item_append_child (NR_ARENA_ITEM (group), new);
			nr_arena_item_unref (new);
			nr_arena_glyphs_set_path (NR_ARENA_GLYPHS (new), curve, FALSE, font, glyph, transform);
			nr_arena_glyphs_set_style (NR_ARENA_GLYPHS (new), sg->style);
			sp_curve_unref (curve);
		}
	}

}

void
nr_arena_glyphs_group_set_style (NRArenaGlyphsGroup *sg, SPStyle *style)
{
	NRArenaGroup *group;
	NRArenaItem *child;

	nr_return_if_fail (sg != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS_GROUP (sg));

	group = NR_ARENA_GROUP (sg);

	if (style) sp_style_ref (style);
	if (sg->style) sp_style_unref (sg->style);
	sg->style = style;

	for (child = group->children; child != NULL; child = child->next) {
		nr_return_if_fail (NR_IS_ARENA_GLYPHS (child));
		nr_arena_glyphs_set_style (NR_ARENA_GLYPHS (child), sg->style);
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (sg), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_glyphs_group_set_paintbox (NRArenaGlyphsGroup *gg, const ArtDRect *pbox)
{
	nr_return_if_fail (gg != NULL);
	nr_return_if_fail (NR_IS_ARENA_GLYPHS_GROUP (gg));
	nr_return_if_fail (pbox != NULL);

	if ((pbox->x0 < pbox->x1) && (pbox->y0 < pbox->y1)) {
		gg->paintbox.x0 = pbox->x0;
		gg->paintbox.y0 = pbox->y0;
		gg->paintbox.x1 = pbox->x1;
		gg->paintbox.y1 = pbox->y1;
	} else {
		/* fixme: We kill warning, although not sure what to do here (Lauris) */
		gg->paintbox.x0 = gg->paintbox.y0 = 0.0F;
		gg->paintbox.x1 = gg->paintbox.y1 = 256.0F;
	}

	nr_arena_item_request_update (NR_ARENA_ITEM (gg), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

