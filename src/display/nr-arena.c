#define __NR_ARENA_C__

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

#include "nr-arena-item.h"
#include "nr-arena.h"
#include "../libnr/nr-rect.h"

static void nr_arena_class_init (NRArenaClass *klass);
static void nr_arena_init (NRArena *arena);
static void nr_arena_finalize (NRObject *object);

static NRActiveObjectClass *parent_class;

NRType
nr_arena_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ACTIVE_OBJECT,
						"NRArena",
						sizeof (NRArenaClass),
						sizeof (NRArena),
						(void (*) (NRObjectClass *)) nr_arena_class_init,
						(void (*) (NRObject *)) nr_arena_init);
	}
	return type;
}

static void
nr_arena_class_init (NRArenaClass *klass)
{
	NRObjectClass * object_class;

	object_class = (NRObjectClass *) klass;

	parent_class = (NRActiveObjectClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = nr_arena_finalize;
}

static void
nr_arena_init (NRArena *arena)
{
}

static void
nr_arena_finalize (NRObject *object)
{
	NRArena *arena;

	arena = NR_ARENA (object);

	((NRObjectClass *) (parent_class))->finalize (object);
}

#if 0
void
nr_arena_item_added (NRArena *arena, NRArenaItem *item)
{
	g_return_if_fail (arena != NULL);
	g_return_if_fail (NR_IS_ARENA (arena));
	g_return_if_fail (item != NULL);
	g_return_if_fail (NR_IS_ARENA_ITEM (item));

	g_signal_emit (G_OBJECT (arena), signals [ITEM_ADDED], 0, item);
}

void
nr_arena_remove_item (NRArena *arena, NRArenaItem *item)
{
	g_return_if_fail (arena != NULL);
	g_return_if_fail (NR_IS_ARENA (arena));
	g_return_if_fail (item != NULL);
	g_return_if_fail (NR_IS_ARENA_ITEM (item));

	g_signal_emit (G_OBJECT (arena), signals [REMOVE_ITEM], 0, item);
}
#endif

void
nr_arena_request_update (NRArena *arena, NRArenaItem *item)
{
	NRActiveObject *aobject;

	aobject = (NRActiveObject *) arena;

	nr_return_if_fail (arena != NULL);
	nr_return_if_fail (NR_IS_ARENA (arena));
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	if (aobject->callbacks) {
		int i;
		for (i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener;
			NRArenaEventVector *avector;
			listener = aobject->callbacks->listeners + i;
			avector = (NRArenaEventVector *) listener->vector;
			if ((listener->size >= sizeof (NRArenaEventVector)) && avector->request_update) {
				avector->request_update (arena, item, listener->data);
			}
		}
	}
}

void
nr_arena_request_render_rect (NRArena *arena, NRRectL *area)
{
	NRActiveObject *aobject;

	aobject = (NRActiveObject *) arena;

	nr_return_if_fail (arena != NULL);
	nr_return_if_fail (NR_IS_ARENA (arena));
	nr_return_if_fail (area != NULL);

	if (aobject->callbacks && area && !nr_rect_l_test_empty (area)) {
		int i;
		for (i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener;
			NRArenaEventVector *avector;
			listener = aobject->callbacks->listeners + i;
			avector = (NRArenaEventVector *) listener->vector;
			if ((listener->size >= sizeof (NRArenaEventVector)) && avector->request_render) {
				avector->request_render (arena, area, listener->data);
			}
		}
	}
}

