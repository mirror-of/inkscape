#define __SP_CHARS_C__

/*
 * SPChars - parent class for text objects
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include <libart_lgpl/art_misc.h>

#include <libnr/nr-matrix.h>
#include <libnr/nr-pixblock.h>

#include "macros.h"
#include "display/nr-arena-glyphs.h"
#include "print.h"
#include "style.h"
#include "sp-chars.h"

static void sp_chars_class_init (SPCharsClass *class);
static void sp_chars_init (SPChars *chars);

static void sp_chars_release (SPObject *object);
static void sp_chars_modified (SPObject *object, guint flags);

static void sp_chars_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags);
static NRArenaItem *sp_chars_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);

static SPItemClass *parent_class;

GType
sp_chars_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPCharsClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_chars_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPChars),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_chars_init,
		};
		type = g_type_register_static (SP_TYPE_ITEM, "SPChars", &info, 0);
	}
	return type;
}

static void
sp_chars_class_init (SPCharsClass *klass)
{
	GObjectClass *object_class;
	SPObjectClass *sp_object_class;
	SPItemClass *item_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->release = sp_chars_release;
	sp_object_class->modified = sp_chars_modified;

	item_class->bbox = sp_chars_bbox;
	item_class->show = sp_chars_show;
}

static void
sp_chars_init (SPChars *chars)
{
	chars->elements = NULL;

	chars->paintbox.x0 = chars->paintbox.y0 = 0.0;
	chars->paintbox.x1 = chars->paintbox.y1 = 1.0;
}

static void
sp_chars_release (SPObject *object)
{
	SPChars *chars;

	chars = SP_CHARS (object);

	while (chars->elements) {
		SPCharElement *el;
		el = chars->elements;
		chars->elements = el->next;
		nr_font_unref (el->font);
		g_free (el);
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_chars_modified (SPObject *object, unsigned int flags)
{
	SPChars *chars;

	chars = SP_CHARS (object);

	if (((SPObjectClass *) (parent_class))->modified)
		((SPObjectClass *) (parent_class))->modified (object, flags);

	if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
		SPItemView *v;
		for (v = SP_ITEM (chars)->display; v != NULL; v = v->next) {
			nr_arena_glyphs_group_set_style (NR_ARENA_GLYPHS_GROUP (v->arenaitem), object->style);
		}
	}
}

static void
sp_chars_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags)
{
	SPChars *chars;
	SPCharElement *el;

	chars = SP_CHARS (item);

	for (el = chars->elements; el != NULL; el = el->next) {
		NRBPath bpath;
		if (nr_font_glyph_outline_get (el->font, el->glyph, &bpath, FALSE)) {
			NRMatrixF a;
			nr_matrix_multiply_ffd (&a, &el->transform, transform);
			nr_path_matrix_f_bbox_f_union (&bpath, &a, bbox, 0.25);
		}
	}
}

static NRArenaItem *
sp_chars_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPChars *chars;
	NRArenaItem *arenaitem;
	SPCharElement *el;

	chars = SP_CHARS (item);

	arenaitem = nr_arena_item_new (arena, NR_TYPE_ARENA_GLYPHS_GROUP);

	nr_arena_glyphs_group_set_style (NR_ARENA_GLYPHS_GROUP (arenaitem), SP_OBJECT_STYLE (item));

	for (el = chars->elements; el != NULL; el = el->next) {
		nr_arena_glyphs_group_add_component (NR_ARENA_GLYPHS_GROUP (arenaitem), el->font, el->glyph, &el->transform);
	}

	nr_arena_glyphs_group_set_paintbox (NR_ARENA_GLYPHS_GROUP (arenaitem), &chars->paintbox);

	return arenaitem;
}

void
sp_chars_clear (SPChars *chars)
{
	SPItem *item;
	SPItemView *v;

	item = SP_ITEM (chars);

	while (chars->elements) {
		SPCharElement *el;
		el = chars->elements;
		chars->elements = el->next;
		nr_font_unref (el->font);
		g_free (el);
	}

	for (v = item->display; v != NULL; v = v->next) {
		nr_arena_glyphs_group_clear (NR_ARENA_GLYPHS_GROUP (v->arenaitem));
	}
}

void
sp_chars_add_element (SPChars *chars, guint glyph, NRFont *font, const NRMatrixF *transform)
{
	SPItem *item;
	SPItemView *v;
	SPCharElement * el;

	item = SP_ITEM (chars);

	el = g_new (SPCharElement, 1);

	el->glyph = glyph;
	el->font = font;
	nr_font_ref (font);

	el->transform = *transform;

	el->next = chars->elements;
	chars->elements = el;

	for (v = item->display; v != NULL; v = v->next) {
		nr_arena_glyphs_group_add_component (NR_ARENA_GLYPHS_GROUP (v->arenaitem), el->font, el->glyph, &el->transform);
	}
}

SPCurve *
sp_chars_normalized_bpath (SPChars *chars)
{
	SPCharElement *el;
	GSList *cc;
	SPCurve *curve;

	cc = NULL;
	for (el = chars->elements; el != NULL; el = el->next) {
		NRBPath bp;
		ArtBpath *abp;
		SPCurve *c;
		gdouble a[6];
		gint i;
		for (i = 0; i < 6; i++) a[i] = el->transform.c[i];
		if (nr_font_glyph_outline_get (el->font, el->glyph, &bp, FALSE)) {
			abp = art_bpath_affine_transform (bp.path, a);
			c = sp_curve_new_from_bpath (abp);
			if (c) cc = g_slist_prepend (cc, c);
		}
	}

	cc = g_slist_reverse (cc);

	curve = sp_curve_concat (cc);

	while (cc) {
		/* fixme: This is dangerous, as we are mixing art_alloc and g_new */
		sp_curve_unref ((SPCurve *) cc->data);
		cc = g_slist_remove (cc, cc->data);
	}

	return curve;
}

/* This is completely unrelated to SPItem::print */

static void
sp_chars_print_bpath (SPPrintContext *ctx, const NRBPath *bpath, const SPStyle *style, const NRMatrixF *ctm,
		      const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	if (style->fill.type != SP_PAINT_TYPE_NONE) {
		sp_print_fill (ctx, bpath, ctm, style, pbox, dbox, bbox);
	}

	if (style->stroke.type != SP_PAINT_TYPE_NONE) {
		sp_print_stroke (ctx, bpath, ctm, style, pbox, dbox, bbox);
	}
}

/*
 * pbox is bbox for paint server (user coordinates)
 * dbox is the whole display area
 * bbox is item bbox on desktop
 */

void
sp_chars_do_print (SPChars *chars, SPPrintContext *ctx, const NRMatrixF *ctm, const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPCharElement *el;

	for (el = chars->elements; el != NULL; el = el->next) {
		gdouble chela[6];
		NRBPath bpath;
		gint i;

		for (i = 0; i < 6; i++) chela[i] = el->transform.c[i];
		if (nr_font_glyph_outline_get (el->font, el->glyph, &bpath, FALSE)) {
			NRBPath abp;
			abp.path = art_bpath_affine_transform (bpath.path, chela);
			sp_chars_print_bpath (ctx, &abp, SP_OBJECT_STYLE (chars), ctm, pbox, dbox, bbox);
			art_free (abp.path);
		}
	}
}

void
sp_chars_set_paintbox (SPChars *chars, NRRectF *paintbox)
{
	SPItemView *v;

	chars->paintbox.x0 = paintbox->x0;
	chars->paintbox.y0 = paintbox->y0;
	chars->paintbox.x1 = paintbox->x1;
	chars->paintbox.y1 = paintbox->y1;

	for (v = SP_ITEM (chars)->display; v != NULL; v = v->next) {
		nr_arena_glyphs_group_set_paintbox (NR_ARENA_GLYPHS_GROUP (v->arenaitem), &chars->paintbox);
	}
}

