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

#include "FontInstance.h"
#include "RasterFont.h"
#include "../livarot/Path.h"

/**
 * Increments the reference count for the font.
 */
NRFont *
nr_font_ref (NRFont *font)
{
	if ( font == NULL ) return NULL;
	((font_instance*)font)->Ref();
	return font;
#if 0
	font->refcount += 1;

	return font;
#endif
}

/**
 * Decrements the reference count for the font, and if it hits zero, frees it.
 */
NRFont *
nr_font_unref (NRFont *font)
{
	if ( font == NULL ) return NULL;
	((font_instance*)font)->Unref();
	return NULL;
#if 0
	font->refcount -= 1;

	if (font->refcount < 1) {
		((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_free (font);
	}

	return NULL;
#endif
}

/**
 * Retrieves the outline path for the glyph in the given font.
 */
NRBPath *
nr_font_glyph_outline_get (NRFont *font, unsigned int glyph, NRBPath *path, unsigned int ref)
{
	if ( font == NULL ) return NULL;
	font_instance* f=(font_instance*)font;
	path->path=(NArtBpath*)f->ArtBPath(glyph);
	return path;
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_outline_get (font, glyph, path, ref);
#endif
}

/**
 * Dereferences the font outline for the glyph
 */
void
nr_font_glyph_outline_unref (NRFont *font, unsigned int glyph)
{
#if 0
	((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_outline_unref (font, glyph);
#endif
}

/**
 * Retrieves the horizontal positional advancement for the glyph in the
 * given font.
 */
NR::Point nr_font_glyph_advance_get (NRFont *font, unsigned int glyph,unsigned int metrics)
{
	return nr_typeface_glyph_advance_get((NRTypeFace*)font,glyph,metrics);
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_advance_get (font, glyph);
#endif
}

/**
 * Retrieves the NRRect area for the given font and glyph
 */
NRRect *
nr_font_glyph_area_get (NRFont *font, unsigned int glyph, NRRect *area)
{
	area->x0=area->y0=area->x1=area->y1=0;
	if ( font == NULL ) return area;
	font_instance* f=(font_instance*)font;
	NR::Rect res=f->BBox(glyph);
	area->x0=(res.min())[0];
	area->y0=(res.min())[1];
	area->x1=(res.max())[0];
	area->y1=(res.max())[1];
	return area;
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->font_glyph_area_get (font, glyph, area);
#endif
}

/**
 * Creates a new rasterfont object from the given font object plus transform
 */
NRRasterFont *
nr_rasterfont_new (NRFont *font, NR::Matrix transform)
{
	if ( font == NULL ) return NULL;
	return (NRRasterFont*) ((font_instance*)font)->RasterFont(transform,0.0);
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->rasterfont_new (font, transform);
#endif
}
NRRasterFont *
nr_rasterfont_new (NRFont *font, font_style const &styl)
{
	if ( font == NULL ) return NULL;
	return (NRRasterFont*) ((font_instance*)font)->RasterFont(styl);
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->rasterfont_new (font, transform);
#endif
}

/* Generic implementation */

/**
 * Generic font structure consisting of a font, list of raster fonts, 
 * and list of font BPath outlines.
 */
struct NRFontGeneric {
	NRFont font;

	NRRasterFont *rfonts;

	NRBPath *outlines;
};

/**
 * Creates a new generic font object for the given typeface, metrics, and transform
 */
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

/**
 * Frees the given font object, including its outlines and typeface
 */
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

/**
 * Gets an outline path for the given font and glyph
 */
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
			double const s = font->size / 1000.0;
			NR::Matrix const scale(NR::scale(s, s));
			nr_path_duplicate_transform (&fg->outlines[glyph], &tfgol, scale);
		}
	}

	*d = fg->outlines[glyph];

	return d;
}

/**
 * No op
 */
void
nr_font_generic_glyph_outline_unref (NRFont *font, unsigned int glyph)
{
	/* NOP by now */
}

/**
 * Retrieves the horizontal positional advancement for the glyph in the
 * given font.
 */
NR::Point nr_font_generic_glyph_advance_get (NRFont *font, unsigned int glyph)
{
	return (font->size / 1000.0) * ((NRTypeFaceClass *) ((NRObject *) font->face)->klass)->glyph_advance_get (font->face, glyph, font->metrics);
}

/**
 * Gets the rectangular area that the glyph requires in the given font
 */
NRRect *
nr_font_generic_glyph_area_get (NRFont *font, unsigned int glyph, NRRect *area)
{
	NRBPath bpath;

	if (!nr_font_glyph_outline_get (font, glyph, &bpath, 0)) return NULL;
	area->x0 = area->y0 = NR_HUGE;
	area->x1 = area->y1 = -NR_HUGE;
	nr_path_matrix_bbox_union(&bpath, NR::identity(), area);

	return !nr_rect_d_test_empty (area) ? area : NULL;
}

/**
 * Creates a new generic raster font object for the given font and 
 * transformation matrix.
 */
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

/**
 * Frees the given list of raster font objects
 */
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
