#ifndef __NR_RASTERFONT_H__
#define __NR_RASTERFONT_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

typedef struct _NRRasterFont NRRasterFont;
typedef struct _NRRFGlyphSlot NRRFGlyphSlot;

#include <libnr/nr-pixblock.h>
#include <libnrtype/nr-font.h>

struct _NRRasterFont {
	unsigned int refcount;
	NRRasterFont *next;
	NRFont *font;
	NRMatrixF transform;
	unsigned int nglyphs;
	NRRFGlyphSlot **pages;
};

#define NR_RASTERFONT_FONT(rf) (((NRRasterFont *) rf)->font)
#define NR_RASTERFONT_TYPEFACE(rf) (((NRRasterFont *) rf)->font->face)

NRRasterFont *nr_rasterfont_ref (NRRasterFont *rf);
NRRasterFont *nr_rasterfont_unref (NRRasterFont *rf);

NRPointF *nr_rasterfont_glyph_advance_get (NRRasterFont *rf, int glyph, NRPointF *adv);
NRRectF *nr_rasterfont_glyph_area_get (NRRasterFont *rf, int glyph, NRRectF *area);

void nr_rasterfont_glyph_mask_render (NRRasterFont *rf, int glyph, NRPixBlock *mask, float x, float y);

/* Generic implementation */

NRRasterFont *nr_rasterfont_generic_new (NRFont *font, NRMatrixF *transform);
void nr_rasterfont_generic_free (NRRasterFont *rf);
NRPointF *nr_rasterfont_generic_glyph_advance_get (NRRasterFont *rf, unsigned int glyph, NRPointF *adv);
NRRectF *nr_rasterfont_generic_glyph_area_get (NRRasterFont *rf, unsigned int glyph, NRRectF *area);
void nr_rasterfont_generic_glyph_mask_render (NRRasterFont *rf, unsigned int glyph, NRPixBlock *m, float x, float y);

#endif
