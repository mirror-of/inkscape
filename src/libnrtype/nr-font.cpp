#define __NR_FONT_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "config.h"

#if HAVE_STRING_H
#include <string.h>
#endif

#include <libnr/nr-macros.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
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

NR::Point nr_font_glyph_advance_get (NRFont *font, unsigned int glyph)
{
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_advance_get (font, glyph);
}

NRRect *
nr_font_glyph_area_get (NRFont *font, unsigned int glyph, NRRect *area)
{
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_area_get (font, glyph, area);
}

NRRasterFont *
nr_rasterfont_new (NRFont *font, NR::Matrix transform)
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
nr_font_generic_new (NRTypeFace *tf, unsigned int metrics, NR::Matrix const transform)
{
	NRFontGeneric *fg = nr_new (NRFontGeneric, 1);

	fg->font.refcount = 1;
	fg->font.next = NULL;
	fg->font.face = nr_typeface_ref (tf);
	fg->font.metrics = metrics;
	fg->font.size = NR::expansion(transform);

	fg->rfonts = NULL;
	fg->outlines = NULL;

	return (NRFont *) fg;
}

void
nr_font_generic_free (NRFont *font)
{
	NRFontGeneric *fg = (NRFontGeneric *) font;

	if (fg->outlines) {
		for (unsigned int i = 0; i < font->face->nglyphs; i++) {
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
	NRFontGeneric *fg = (NRFontGeneric *) font;

	if (!fg->outlines) {
		fg->outlines = nr_new (NRBPath, font->face->nglyphs);
		memset (fg->outlines, 0x0, font->face->nglyphs * sizeof (NRBPath));
	}

	if (!fg->outlines[glyph].path) {
		NRBPath tfgol;
		if (nr_typeface_glyph_outline_get (font->face, glyph, font->metrics, &tfgol, 0)) {
			NR::Matrix scale = NR::scale(NR::Point(font->size / 1000.0, font->size / 1000.0));
			nr_path_duplicate_transform (&fg->outlines[glyph], &tfgol, scale);
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

NR::Point nr_font_generic_glyph_advance_get (NRFont *font, unsigned int glyph)
{
	return (font->size / 1000.0) * ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->glyph_advance_get (font->face, glyph, font->metrics);
}

NRRect *
nr_font_generic_glyph_area_get (NRFont *font, unsigned int glyph, NRRect *area)
{
	NRBPath bpath;

	if (!nr_font_glyph_outline_get (font, glyph, &bpath, 0)) return NULL;
	area->x0 = area->y0 = NR_HUGE;
	area->x1 = area->y1 = -NR_HUGE;
	nr_path_matrix_f_bbox_f_union (&bpath, NULL, area, 0.25);

	return !nr_rect_f_test_empty (area) ? area : NULL;
}

NRRasterFont *
nr_font_generic_rasterfont_new (NRFont *font, NR::Matrix const transform)
{
	NRFontGeneric *fg = (NRFontGeneric *) font;

	double mexp = NR::expansion(transform);
	NRRasterFont *rf = fg->rfonts;
	while (rf != NULL) {
		double fmexp = NR::expansion(rf->transform);
		if (NR_DF_TEST_CLOSE (mexp, fmexp, 0.001 * mexp)) {
			if (NR_DF_TEST_CLOSE (mexp, 0.0, 0.0001))
				return nr_rasterfont_ref (rf);
			double rmexp = mexp * 0.001;
			if (NR_DF_TEST_CLOSE (transform[0], rf->transform[0], rmexp) &&
			    NR_DF_TEST_CLOSE (transform[1], rf->transform[1], rmexp) &&
			    NR_DF_TEST_CLOSE (transform[2], rf->transform[2], rmexp) &&
			    NR_DF_TEST_CLOSE (transform[3], rf->transform[3], rmexp)) {
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
	NRFontGeneric *fg = (NRFontGeneric *) rf->font;

	if (fg->rfonts == rf) {
		fg->rfonts = rf->next;
	} else {
		NRRasterFont *ref = fg->rfonts;
		while (ref->next != rf)
			ref = ref->next;
		ref->next = rf->next;
	}

	rf->next = NULL;

	nr_rasterfont_generic_free (rf);
}
