#ifndef __NR_ARENA_GLYPHS_H__
#define __NR_ARENA_GLYPHS_H__

/*
 * RGBA display list system for inkscape
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

#include <display/curve.h>
#include <forward.h>
#include <sp-paint-server.h>
#include <display/nr-arena-item.h>

#define test_glyph_liv

class Shape;

struct NRArenaGlyphs : public NRArenaItem{
	/* Glyphs data */
	SPCurve *curve;
	SPStyle *style;
	NRMatrix transform;
	NRFont *font;
	gint glyph;

	NRRasterFont *rfont;
	float x, y;

	NRMatrix cached_tr;
	Shape  *cached_shp;
	bool   cached_shp_dirty;
	bool   cached_style_dirty;
	
	Shape  *stroke_shp;
};

struct NRArenaGlyphsClass {
	NRArenaItemClass parent_class;
};

NRType nr_arena_glyphs_get_type (void);

void nr_arena_glyphs_set_path (NRArenaGlyphs *glyphs,
			       SPCurve *curve, unsigned int lieutenant,
			       NRFont *font, int glyph,
			       const NRMatrix *transform);
void nr_arena_glyphs_set_style (NRArenaGlyphs *glyphs, SPStyle *style);

/* Integrated group of component glyphss */

typedef struct NRArenaGlyphsGroup NRArenaGlyphsGroup;
typedef struct NRArenaGlyphsGroupClass NRArenaGlyphsGroupClass;

#include "nr-arena-group.h"

#define NR_TYPE_ARENA_GLYPHS_GROUP (nr_arena_glyphs_group_get_type ())
#define NR_ARENA_GLYPHS_GROUP(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_GLYPHS_GROUP, NRArenaGlyphsGroup))
#define NR_IS_ARENA_GLYPHS_GROUP(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_GLYPHS_GROUP))

struct NRArenaGlyphsGroup : public NRArenaGroup{
	SPStyle *style;
	NRRect paintbox;
	/* State data */
	SPPainter *fill_painter;
	SPPainter *stroke_painter;
};

struct NRArenaGlyphsGroupClass {
	NRArenaGroupClass parent_class;
};

NRType nr_arena_glyphs_group_get_type (void);

/* Utility functions */

void nr_arena_glyphs_group_clear (NRArenaGlyphsGroup *group);

void nr_arena_glyphs_group_add_component (NRArenaGlyphsGroup *group, NRFont *font, int glyph, const NRMatrix *transform);

void nr_arena_glyphs_group_set_style (NRArenaGlyphsGroup *group, SPStyle *style);

void nr_arena_glyphs_group_set_paintbox (NRArenaGlyphsGroup *group, const NRRect *pbox);

#endif
