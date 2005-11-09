#define __SP_ITEM_TRANSFORM_C__

/*
 * Transformations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include "libnr/nr-matrix-rotate-ops.h"
#include "libnr/nr-matrix-scale-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "svg/svg.h"
#include "sp-item-transform.h"
#include "sp-item.h"

static NR::translate inverse(NR::translate const m)
{
	/* TODO: Move this to nr-matrix-fns.h or the like. */
	return NR::translate(-m[0], -m[1]);
}

void 
sp_item_rotate_rel(SPItem *item, NR::rotate const &rotation)
{
	NR::translate const s(sp_item_bbox_desktop(item).midpoint());

	// Rotate item.
	sp_item_set_i2d_affine(item,
			       sp_item_i2d_affine(item) * inverse(s) * rotation * s);

	// Use each item's own transform writer, consistent with sp_selection_apply_affine()
	sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
}

void
sp_item_scale_rel (SPItem *item, NR::scale const &scale)
{
	NR::translate const s(sp_item_bbox_desktop(item).midpoint());

	sp_item_set_i2d_affine(item,
			       sp_item_i2d_affine(item) * inverse(s) * scale * s);
	sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
} 

void
sp_item_skew_rel (SPItem *item, double skewX, double skewY)
{
	NR::Rect bbox(sp_item_bbox_desktop(item));

	NR::translate const s(bbox.midpoint());

	NR::Matrix const skew(1, skewY, skewX, 1, 0, 0);

	sp_item_set_i2d_affine(item,
			       sp_item_i2d_affine(item) * inverse(s) * skew * s);
	sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
} 

void sp_item_move_rel(SPItem *item, NR::translate const &tr)
{
	sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * tr);

	sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
}

