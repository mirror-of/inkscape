#ifndef __NR_ARENA_SHAPE_H__
#define __NR_ARENA_SHAPE_H__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define NR_TYPE_ARENA_SHAPE (nr_arena_shape_get_type ())
#define NR_ARENA_SHAPE(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_SHAPE, NRArenaShape))
#define NR_IS_ARENA_SHAPE(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_SHAPE))

#include <libnr/nr-svp.h>

#include "helper/curve.h"
#include "helper/canvas-bpath.h"
#include "forward.h"
#include "sp-paint-server.h"
#include "nr-arena-item.h"

#include "../livarot/Shape.h"

#define test_liv

struct _NRArenaShape {
	NRArenaItem item;
	/* Shape data */
	SPCurve *curve;
	SPStyle *style;
	NRRect paintbox;
	/* State data */
	NRMatrix ctm;
	SPPainter *fill_painter;
	SPPainter *stroke_painter;
#ifndef test_liv
	NRSVP *fill_svp;
	NRSVP *stroke_svp;
#else
  // the 2 cached polygons, for rasterizations uses
  Shape *fill_shp;
  Shape *stroke_shp;
  // delayed_shp=true means the *_shp polygons are not computed yet
  // they'll be computed on demand in *_render(), *_pick() or *_clip()
  // the goal is to not uncross polygons that are outside the viewing region
  bool    delayed_shp;
  // approximate bounding box, for the case when the polygons have been delayed
  NRRectL approx_bbox;
  // cache for transformations: cached_fill and cached_stroke are polygons computed for the cached_fctm and cache_sctm respectively
  // when the transforametion changes interactively (tracked by the SP_OBJECT_USER_MODIFIED_FLAG_B), we check if it's an isometry wrt
  // the cached ctm. if it's an isommetry, just apply it to the cached polygon to get the *_shp polygon. otherwise recompute
  // so this works fine for translation and rotation, but not scaling and skewing
  NRMatrix cached_fctm;
  NRMatrix cached_sctm;
  Shape    *cached_fill;
  Shape    *cached_stroke;
#endif
	/* Markers */
	NRArenaItem *markers;
};

struct _NRArenaShapeClass {
	NRArenaItemClass parent_class;
};

NRType nr_arena_shape_get_type (void);

void nr_arena_shape_set_path(NRArenaShape *shape, SPCurve *curve,bool justTrans);
void nr_arena_shape_set_style (NRArenaShape *shape, SPStyle *style);
void nr_arena_shape_set_paintbox (NRArenaShape *shape, const NRRect *pbox);

#endif
