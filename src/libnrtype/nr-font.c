#define __NR_FONT_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <string.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include "nr-font.h"

NRFont *
nr_font_ref (NRFont *font)
{
	font->refcount += 1;

	return font;
}

NRFont *
nr_font_unref (NRFont *font)
{
	font->refcount -= 1;

	if (font->refcount < 1) {
		((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_free (font);
	}

	return NULL;
}

NRBPath *
nr_font_glyph_outline_get (NRFont *font, unsigned int glyph, NRBPath *path, unsigned int ref)
{
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_outline_get (font, glyph, path, ref);
}

void
nr_font_glyph_outline_unref (NRFont *font, unsigned int glyph)
{
	((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_outline_unref (font, glyph);
}

NRPointF *
nr_font_glyph_advance_get (NRFont *font, unsigned int glyph, NRPointF *adv)
{
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_advance_get (font, glyph, adv);
}

NRRectF *
nr_font_glyph_area_get (NRFont *font, unsigned int glyph, NRRectF *area)
{
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_area_get (font, glyph, area);
}

NRRasterFont *
nr_rasterfont_new (NRFont *font, NRMatrixF *transform)
{
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->rasterfont_new (font, transform);
}

/* Generic implementation */

typedef struct _NRFontGeneric NRFontGeneric;

struct _NRFontGeneric {
	NRFont font;

	NRRasterFont *rfonts;

	NRBPath *outlines;
};

NRFont *
nr_font_generic_new (NRTypeFace *tf, unsigned int metrics, NRMatrixF *transform)
{
	NRFontGeneric *fg;

	fg = nr_new (NRFontGeneric, 1);

	fg->font.refcount = 1;
	fg->font.next = NULL;
	fg->font.face = nr_typeface_ref (tf);
	fg->font.metrics = metrics;
	fg->font.size = NR_MATRIX_DF_EXPANSION (transform);

	fg->rfonts = NULL;
	fg->outlines = NULL;

	return (NRFont *) fg;
}

void
nr_font_generic_free (NRFont *font)
{
	NRFontGeneric *fg;
	int i;

	fg = (NRFontGeneric *) font;

	if (fg->outlines) {
		for (i = 0; i < font->face->nglyphs; i++) {
			if (fg->outlines[i].path) nr_free (fg->outlines[i].path);
		}
		nr_free (fg->outlines);
	}

	nr_typeface_unref (font->face);
	nr_free (font);
}

NRBPath *
nr_font_generic_glyph_outline_get (NRFont *font, unsigned int glyph, NRBPath *d, unsigned int ref)
{
	NRFontGeneric *fg;

	fg = (NRFontGeneric *) font;

	if (!fg->outlines) {
		fg->outlines = nr_new (NRBPath, font->face->nglyphs);
		memset (fg->outlines, 0x0, font->face->nglyphs * sizeof (NRBPath));
	}

	if (!fg->outlines[glyph].path) {
		NRBPath tfgol;
		if (nr_typeface_glyph_outline_get (font->face, glyph, font->metrics, &tfgol, 0)) {
			NRMatrixF scale;
			nr_matrix_f_set_scale (&scale, font->size / 1000.0, font->size / 1000.0);
			nr_path_duplicate_transform (&fg->outlines[glyph], &tfgol, &scale);
		}
	}

	*d = fg->outlines[glyph];

	return d;
}

void
nr_font_generic_glyph_outline_unref (NRFont *font, unsigned int glyph)
{
	/* NOP by now */
}

NRPointF *
nr_font_generic_glyph_advance_get (NRFont *font, unsigned int glyph, NRPointF *adv)
{
	((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->glyph_advance_get (font->face, glyph, font->metrics, adv);

	adv->x *= (font->size / 1000.0);
	adv->y *= (font->size / 1000.0);

	return adv;
}

NRRectF *
nr_font_generic_glyph_area_get (NRFont *font, unsigned int glyph, NRRectF *area)
{
	NRBPath bpath;

	if (!nr_font_glyph_outline_get (font, glyph, &bpath, 0)) return NULL;
	area->x0 = area->y0 = NR_HUGE_F;
	area->x1 = area->y1 = -NR_HUGE_F;
	nr_path_matrix_f_bbox_f_union (&bpath, NULL, area, 0.25);

	return !nr_rect_f_test_empty (area) ? area : NULL;
}

NRRasterFont *
nr_font_generic_rasterfont_new (NRFont *font, NRMatrixF *transform)
{
	NRFontGeneric *fg;
	NRRasterFont *rf;
	double mexp;

	fg = (NRFontGeneric *) font;

	mexp = NR_MATRIX_DF_EXPANSION (transform);
	rf = fg->rfonts;
	while (rf != NULL) {
		double fmexp;
		fmexp = NR_MATRIX_DF_EXPANSION (&rf->transform);
		if (NR_DF_TEST_CLOSE (mexp, fmexp, 0.001 * mexp)) {
			double rmexp;
			if (NR_DF_TEST_CLOSE (mexp, 0.0, 0.0001)) return nr_rasterfont_ref (rf);
			rmexp = mexp * 0.001;
			if (NR_DF_TEST_CLOSE (transform->c[0], rf->transform.c[0], rmexp) &&
			    NR_DF_TEST_CLOSE (transform->c[1], rf->transform.c[1], rmexp) &&
			    NR_DF_TEST_CLOSE (transform->c[2], rf->transform.c[2], rmexp) &&
			    NR_DF_TEST_CLOSE (transform->c[3], rf->transform.c[3], rmexp)) {
				return nr_rasterfont_ref (rf);
			}
		}
		rf = rf->next;
	}

	rf = nr_rasterfont_generic_new (font, transform);

	rf->next = fg->rfonts;
	fg->rfonts = rf;

	return rf;
}

void
nr_font_generic_rasterfont_free (NRRasterFont *rf)
{
	NRFontGeneric *fg;

	fg = (NRFontGeneric *) rf->font;

	if (fg->rfonts == rf) {
		fg->rfonts = rf->next;
	} else {
		NRRasterFont *ref;
		ref = fg->rfonts;
		while (ref->next != rf) ref = ref->next;
		ref->next = rf->next;
	}

	rf->next = NULL;

	nr_rasterfont_generic_free (rf);
}
