#define __NR_ARENA_C__

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

#include "nr-arena-item.h"
#include "nr-arena.h"
#include <libnr/nr-rect.h>

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
	NRObjectClass *object_class = (NRObjectClass *) klass;

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
	NRArena *arena = NR_ARENA (object);

	((NRObjectClass *) (parent_class))->finalize (object);
}

void
nr_arena_request_update (NRArena *arena, NRArenaItem *item)
{
	NRActiveObject *aobject = (NRActiveObject *) arena;

	nr_return_if_fail (arena != NULL);
	nr_return_if_fail (NR_IS_ARENA (arena));
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	if (aobject->callbacks) {
		for (unsigned int i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener = aobject->callbacks->listeners + i;
			NRArenaEventVector *avector = (NRArenaEventVector *) listener->vector;
			if ((listener->size >= sizeof (NRArenaEventVector)) && avector->request_update) {
				avector->request_update (arena, item, listener->data);
			}
		}
	}
}

void
nr_arena_request_render_rect (NRArena *arena, NRRectL *area)
{
	NRActiveObject *aobject = (NRActiveObject *) arena;

	nr_return_if_fail (arena != NULL);
	nr_return_if_fail (NR_IS_ARENA (arena));
	nr_return_if_fail (area != NULL);

	if (aobject->callbacks && area && !nr_rect_l_test_empty (area)) {
		for (unsigned int i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener = aobject->callbacks->listeners + i;
			NRArenaEventVector *avector = (NRArenaEventVector *) listener->vector;
			if ((listener->size >= sizeof (NRArenaEventVector)) && avector->request_render) {
				avector->request_render (arena, area, listener->data);
			}
		}
	}
}

