#define __NR_ARENA_GROUP_C__

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

#include <math.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-blit.h>
#include "nr-arena-group.h"

static void nr_arena_group_class_init (NRArenaGroupClass *klass);
static void nr_arena_group_init (NRArenaGroup *group);
static void nr_arena_group_finalize (NRObject *object);

static NRArenaItem *nr_arena_group_children (NRArenaItem *item);
static void nr_arena_group_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);
static void nr_arena_group_remove_child (NRArenaItem *item, NRArenaItem *child);
static void nr_arena_group_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);

static unsigned int nr_arena_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset);
static unsigned int nr_arena_group_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static unsigned int nr_arena_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_group_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky);

static NRArenaItemClass *parent_class;

NRType
nr_arena_group_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
						"NRArenaGroup",
						sizeof (NRArenaGroupClass),
						sizeof (NRArenaGroup),
						(void (*) (NRObjectClass *)) nr_arena_group_class_init,
						(void (*) (NRObject *)) nr_arena_group_init);
	}
	return type;
}

static void
nr_arena_group_class_init (NRArenaGroupClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_group_finalize;

	item_class->children = nr_arena_group_children;
	item_class->add_child = nr_arena_group_add_child;
	item_class->set_child_position = nr_arena_group_set_child_position;
	item_class->remove_child = nr_arena_group_remove_child;
	item_class->update = nr_arena_group_update;
	item_class->render = nr_arena_group_render;
	item_class->clip = nr_arena_group_clip;
	item_class->pick = nr_arena_group_pick;
}

static void
nr_arena_group_init (NRArenaGroup *group)
{
	group->transparent = FALSE;
	group->children = NULL;
	group->last = NULL;
	nr_matrix_f_set_identity (&group->child_transform);
}

static void
nr_arena_group_finalize (NRObject *object)
{
	NRArenaItem *item;
	NRArenaGroup *group;

	item = NR_ARENA_ITEM (object);
	group = NR_ARENA_GROUP (object);

	while (group->children) {
		group->children = nr_arena_item_detach_unref (item, group->children);
	}

	((NRObjectClass *) (parent_class))->finalize (object);
}

static NRArenaItem *
nr_arena_group_children (NRArenaItem *item)
{
	NRArenaGroup *group;

	group = NR_ARENA_GROUP (item);

	return group->children;
}

static void
nr_arena_group_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	NRArenaGroup *group;

	group = NR_ARENA_GROUP (item);

	if (!ref) {
		group->children = nr_arena_item_attach_ref (item, child, NULL, group->children);
	} else {
		ref->next = nr_arena_item_attach_ref (item, child, ref, ref->next);
	}

	if (ref == group->last) group->last = child;

	nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_group_remove_child (NRArenaItem *item, NRArenaItem *child)
{
	NRArenaGroup *group;

	group = NR_ARENA_GROUP (item);

	if (child == group->last) group->last = child->prev;

	if (child->prev) {
		nr_arena_item_detach_unref (item, child);
	} else {
		group->children = nr_arena_item_detach_unref (item, child);
	}

	nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_group_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	NRArenaGroup *group;

	group = NR_ARENA_GROUP (item);

	nr_arena_item_ref (child);

	if (child == group->last) group->last = child->prev;

	if (child->prev) {
		nr_arena_item_detach_unref (item, child);
	} else {
		group->children = nr_arena_item_detach_unref (item, child);
	}

	if (!ref) {
		group->children = nr_arena_item_attach_ref (item, child, NULL, group->children);
	} else {
		ref->next = nr_arena_item_attach_ref (item, child, ref, ref->next);
	}

	if (ref == group->last) group->last = child;

	nr_arena_item_unref (child);

	nr_arena_item_request_render (child);
}

static unsigned int
nr_arena_group_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset)
{
	NRArenaGroup *group;
	NRArenaItem *child;
	unsigned int newstate, beststate;

	group = NR_ARENA_GROUP (item);

	beststate = NR_ARENA_ITEM_STATE_ALL;

	for (child = group->children; child != NULL; child = child->next) {
		NRGC cgc;
		nr_matrix_multiply_dfd (&cgc.transform, &group->child_transform, &gc->transform);
		newstate = nr_arena_item_invoke_update (child, area, &cgc, state, reset);
		beststate = beststate & newstate;
	}

	if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
		nr_rect_l_set_empty (&item->bbox);
		for (child = group->children; child != NULL; child = child->next) {
			nr_rect_l_union (&item->bbox, &item->bbox, &child->bbox);
		}
	}

	return beststate;
}

static unsigned int
nr_arena_group_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRArenaGroup *group;
	NRArenaItem *child;
	unsigned int ret;

	group = NR_ARENA_GROUP (item);

	ret = item->state;

	/* Just compose children into parent buffer */
	for (child = group->children; child != NULL; child = child->next) {
		ret = nr_arena_item_invoke_render (child, area, pb, flags);
		if (ret & NR_ARENA_ITEM_STATE_INVALID) break;
	}

	return ret;
}

static unsigned int
nr_arena_group_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	NRArenaGroup *group;
	NRArenaItem *child;
	unsigned int ret;

	group = NR_ARENA_GROUP (item);

	ret = item->state;

	/* Just compose children into parent buffer */
	for (child = group->children; child != NULL; child = child->next) {
		ret = nr_arena_item_invoke_clip (child, area, pb);
		if (ret & NR_ARENA_ITEM_STATE_INVALID) break;
	}

	return ret;
}

static NRArenaItem *
nr_arena_group_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky)
{
	NRArenaGroup *group;
	NRArenaItem *child, *picked;

	group = NR_ARENA_GROUP (item);

	for (child = group->last; child != NULL; child = child->prev) {
		picked = nr_arena_item_invoke_pick (child, x, y, delta, sticky);
		if (picked) return (group->transparent) ? picked : item;
	}

	return NULL;
}

void
nr_arena_group_set_transparent (NRArenaGroup *group, unsigned int transparent)
{
	nr_return_if_fail (group != NULL);
	nr_return_if_fail (NR_IS_ARENA_GROUP (group));

	group->transparent = transparent;
}

void
nr_arena_group_set_child_transform (NRArenaGroup *group, NRMatrixF *t)
{
	if (!t) t = &NR_MATRIX_F_IDENTITY;

	if (!NR_MATRIX_DF_TEST_CLOSE (t, &group->child_transform, NR_EPSILON_F)) {
		nr_arena_item_request_render (NR_ARENA_ITEM (group));
		group->child_transform = *t;
		nr_arena_item_request_update (NR_ARENA_ITEM (group), NR_ARENA_ITEM_STATE_ALL, TRUE);
	}
}


