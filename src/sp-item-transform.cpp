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
#include "svg/svg.h"
#include "sp-item-transform.h"
#include "sp-item.h"

static NR::translate inverse(NR::translate const m)
{
	/* TODO: Move this to nr-matrix-fns.h or the like. */
	return NR::translate(-m[0], -m[1]);
}

void sp_item_rotate_rel(SPItem *item, NR::rotate const &rotation)
{
	NR::translate const s(sp_item_bbox_desktop(item).midpoint());

	// Rotate item.
	sp_item_set_i2d_affine(item,
			       sp_item_i2d_affine(item) * inverse(s) * rotation * s);

	// this method is consistent with sp_selection_apply_affine()
	// it uses each item's own transform writer 
	// (so e.g. no transform= is ever added to a path, but its nodes always have final coordinates)
	sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
}

void sp_item_move_rel(SPItem *item, NR::translate const &tr)
{
	sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * tr);

	sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
}

