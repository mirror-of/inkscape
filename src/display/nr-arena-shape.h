#ifndef __NR_ARENA_SHAPE_H__
#define __NR_ARENA_SHAPE_H__

/*
 * RGBA display list system for sodipodi
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
#include "forward.h"
#include "sp-paint-server.h"
#include "nr-arena-item.h"

struct _NRArenaShape {
	NRArenaItem item;
	/* Shape data */
	SPCurve *curve;
	SPStyle *style;
	NRRectF paintbox;
	/* State data */
	NRMatrixD ctm;
	SPPainter *fill_painter;
	SPPainter *stroke_painter;
	NRSVP *fill_svp;
	NRSVP *stroke_svp;
	/* Markers */
	NRArenaItem *markers;
};

struct _NRArenaShapeClass {
	NRArenaItemClass parent_class;
};

NRType nr_arena_shape_get_type (void);

void nr_arena_shape_set_path (NRArenaShape *shape, SPCurve *curve, unsigned int private, const double *affine);
void nr_arena_shape_set_style (NRArenaShape *shape, SPStyle *style);
void nr_arena_shape_set_paintbox (NRArenaShape *shape, const NRRectF *pbox);

#endif
