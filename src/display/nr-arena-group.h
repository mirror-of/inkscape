#ifndef __NR_ARENA_GROUP_H__
#define __NR_ARENA_GROUP_H__

/*
 * RGBA display list system for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#define NR_TYPE_ARENA_GROUP (nr_arena_group_get_type ())
#define NR_ARENA_GROUP(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ARENA_GROUP, NRArenaGroup))
#define NR_IS_ARENA_GROUP(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ARENA_GROUP))

#include "nr-arena-item.h"

struct _NRArenaGroup {
	NRArenaItem item;
	unsigned int transparent : 1;
	NRArenaItem *children;
	NRArenaItem *last;
	NRMatrixF child_transform;
};

struct _NRArenaGroupClass {
	NRArenaItemClass parent_class;
};

NRType nr_arena_group_get_type (void);

void nr_arena_group_set_transparent (NRArenaGroup *group, unsigned int transparent);

void nr_arena_group_set_child_transform (NRArenaGroup *group, NRMatrixF *t);

#endif
