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
  Shape *fill_shp;
  Shape *stroke_shp;
  
  bool    delayed_shp;
  NRRectL approx_bbox;
  
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
