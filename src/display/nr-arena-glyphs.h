#ifndef __NR_ARENA_GLYPHS_H__
#define __NR_ARENA_GLYPHS_H__

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

#define NR_TYPE_ARENA_GLYPHS (nr_arena_glyphs_get_type ())
#define NR_ARENA_GLYPHS(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_GLYPHS, NRArenaGlyphs))
#define NR_IS_ARENA_GLYPHS(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_GLYPHS))

#include <libnrtype/nr-rasterfont.h>

#include <libart_lgpl/art_svp.h>

#include "helper/curve.h"
#include "forward.h"
#include "sp-paint-server.h"
#include "nr-arena-item.h"

struct _NRArenaGlyphs {
	NRArenaItem item;

	/* Glyphs data */
	SPCurve *curve;
	SPStyle *style;
	NRMatrixF transform;
	NRFont *font;
	gint glyph;

	NRRasterFont *rfont;
	float x, y;

	ArtSVP *stroke_svp;
};

struct _NRArenaGlyphsClass {
	NRArenaItemClass parent_class;
};

NRType nr_arena_glyphs_get_type (void);

void nr_arena_glyphs_set_path (NRArenaGlyphs *glyphs,
			       SPCurve *curve, unsigned int private,
			       NRFont *font, int glyph,
			       const NRMatrixF *transform);
void nr_arena_glyphs_set_style (NRArenaGlyphs *glyphs, SPStyle *style);

/* Integrated group of component glyphss */

typedef struct _NRArenaGlyphsGroup NRArenaGlyphsGroup;
typedef struct _NRArenaGlyphsGroupClass NRArenaGlyphsGroupClass;

#include "nr-arena-group.h"

#define NR_TYPE_ARENA_GLYPHS_GROUP (nr_arena_glyphs_group_get_type ())
#define NR_ARENA_GLYPHS_GROUP(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_GLYPHS_GROUP, NRArenaGlyphsGroup))
#define NR_IS_ARENA_GLYPHS_GROUP(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_GLYPHS_GROUP))

struct _NRArenaGlyphsGroup {
	NRArenaGroup group;
	SPStyle *style;
	NRRectF paintbox;
	/* State data */
	SPPainter *fill_painter;
	SPPainter *stroke_painter;
};

struct _NRArenaGlyphsGroupClass {
	NRArenaGroupClass parent_class;
};

NRType nr_arena_glyphs_group_get_type (void);

/* Utility functions */

void nr_arena_glyphs_group_clear (NRArenaGlyphsGroup *group);

void nr_arena_glyphs_group_add_component (NRArenaGlyphsGroup *group, NRFont *font, int glyph, const NRMatrixF *transform);

void nr_arena_glyphs_group_set_style (NRArenaGlyphsGroup *group, SPStyle *style);

void nr_arena_glyphs_group_set_paintbox (NRArenaGlyphsGroup *group, const ArtDRect *pbox);

#endif
