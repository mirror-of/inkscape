#define __NR_ARENA_ITEM_C__

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

#define noNR_ARENA_ITEM_VERBOSE
#define noNR_ARENA_ITEM_DEBUG_CASCADE

#include <string.h>
#include <assert.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-pixops.h>
#include "nr-arena.h"
#include "nr-arena-item.h"

static void nr_arena_item_class_init (NRArenaItemClass *klass);
static void nr_arena_item_init (NRArenaItem *item);
static void nr_arena_item_private_finalize (NRObject *object);

static NRObjectClass *parent_class;

NRType
nr_arena_item_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_OBJECT,
						"NRArenaItem",
						sizeof (NRArenaItemClass),
						sizeof (NRArenaItem),
						(void (*) (NRObjectClass *)) nr_arena_item_class_init,
						(void (*) (NRObject *)) nr_arena_item_init);
	}
	return type;
}

static void
nr_arena_item_class_init (NRArenaItemClass *klass)
{
	NRObjectClass *object_class;

	object_class = (NRObjectClass *) klass;

	parent_class = ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_item_private_finalize;
}

static void
nr_arena_item_init (NRArenaItem *item)
{
	item->state = 0;
	item->sensitive = TRUE;
	item->visible = TRUE;

	/* fixme: Initialize bbox */
	item->transform = NULL;
	item->opacity = 255;
}

static void
nr_arena_item_private_finalize (NRObject *object)
{
	NRArenaItem *item;

	item = (NRArenaItem *) object;

	/* Parent has to refcount children */
	assert (!item->parent);
	assert (!item->prev);
	assert (!item->next);

	if (item->clip) {
		nr_arena_item_detach_unref (item, item->clip);
	}

	if (item->mask) {
		nr_arena_item_detach_unref (item, item->mask);
	}

	/* nr_arena_remove_item (item->arena, item); */

	if (item->px) {
		nr_free (item->px);
	}

	if (item->transform) {
		nr_free (item->transform);
	}

	((NRObjectClass *) (parent_class))->finalize (object);
}

NRArenaItem *
nr_arena_item_children (NRArenaItem *item)
{
	nr_return_val_if_fail (item != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);

	if (NR_ARENA_ITEM_VIRTUAL (item, children))
		return NR_ARENA_ITEM_VIRTUAL (item, children) (item);

	return NULL;
}

void
nr_arena_item_add_child (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));
	nr_return_if_fail (child != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (child));
	nr_return_if_fail (child->parent == NULL);
	nr_return_if_fail (child->prev == NULL);
	nr_return_if_fail (child->next == NULL);
	nr_return_if_fail (child->arena == item->arena);
	nr_return_if_fail (child != ref);
	nr_return_if_fail (!ref || NR_IS_ARENA_ITEM (ref));
	nr_return_if_fail (!ref || (ref->parent == item));

	if (NR_ARENA_ITEM_VIRTUAL (item, add_child))
		NR_ARENA_ITEM_VIRTUAL (item, add_child) (item, child, ref);
}

void
nr_arena_item_remove_child (NRArenaItem *item, NRArenaItem *child)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));
	nr_return_if_fail (child != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (child));
	nr_return_if_fail (child->parent == item);

	if (NR_ARENA_ITEM_VIRTUAL (item, remove_child))
		NR_ARENA_ITEM_VIRTUAL (item, remove_child) (item, child);
}

void
nr_arena_item_set_child_position (NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));
	nr_return_if_fail (child != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (child));
	nr_return_if_fail (child->parent == item);
	nr_return_if_fail (!ref || NR_IS_ARENA_ITEM (ref));
	nr_return_if_fail (!ref || (ref->parent == item));

	if (NR_ARENA_ITEM_VIRTUAL (item, set_child_position))
		NR_ARENA_ITEM_VIRTUAL (item, set_child_position) (item, child, ref);
}

NRArenaItem *
nr_arena_item_ref (NRArenaItem *item)
{
	nr_object_ref ((NRObject *) item);

	return item;
}

NRArenaItem *
nr_arena_item_unref (NRArenaItem *item)
{
	nr_object_unref ((NRObject *) item);

	return NULL;
}

unsigned int
nr_arena_item_invoke_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset)
{
	NRGC childgc;

	nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NR_ARENA_ITEM_STATE_INVALID);
	nr_return_val_if_fail (!(state & NR_ARENA_ITEM_STATE_INVALID), NR_ARENA_ITEM_STATE_INVALID);

#ifdef NR_ARENA_ITEM_DEBUG_CASCADE
	printf ("Update %s:%p %x %x %x\n", nr_type_name_from_instance ((GTypeInstance *) item), item, state, item->state, reset);
#endif

	/* return if in error */
	if (item->state & NR_ARENA_ITEM_STATE_INVALID) return item->state;
	/* Set reset flags according to propagation status */
	if (item->propagate) {
		reset |= ~item->state;
		item->propagate = FALSE;
	}
	/* Reset our state */
	item->state &= ~reset;
	/* Return if NOP */
	if (!(~item->state & state)) return item->state;
	/* Test whether to return immediately */
	if (area && (item->state & NR_ARENA_ITEM_STATE_BBOX)) {
		if (!nr_rect_l_test_intersect (area, &item->bbox)) return item->state;
	}

	/* Reset image cache, if not to be kept */
	if (!(item->state & NR_ARENA_ITEM_STATE_IMAGE) && (item->px)) {
		nr_free (item->px);
		item->px = NULL;
	}

	/* Set up local gc */
	childgc = *gc;
	if (item->transform) {
		nr_matrix_multiply_dfd (&childgc.transform, item->transform, &childgc.transform);
	}

	/* Invoke the real method */
	item->state = NR_ARENA_ITEM_VIRTUAL (item, update) (item, area, &childgc, state, reset);
	if (item->state & NR_ARENA_ITEM_STATE_INVALID) return item->state;
	/* Clipping */
	if (item->clip) {
		unsigned int newstate;
		newstate = nr_arena_item_invoke_update (item->clip, area, &childgc, state, reset);
		if (newstate & NR_ARENA_ITEM_STATE_INVALID) {
			item->state |= NR_ARENA_ITEM_STATE_INVALID;
			return item->state;
		}
		nr_rect_l_intersect (&item->bbox, &item->bbox, &item->clip->bbox);
	}
	/* Masking */
	if (item->mask) {
		unsigned int newstate;
		newstate = nr_arena_item_invoke_update (item->mask, area, &childgc, state, reset);
		if (newstate & NR_ARENA_ITEM_STATE_INVALID) {
			item->state |= NR_ARENA_ITEM_STATE_INVALID;
			return item->state;
		}
		nr_rect_l_intersect (&item->bbox, &item->bbox, &item->mask->bbox);
	}

	return item->state;
}

unsigned int
nr_arena_item_invoke_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRRectL carea;
	NRPixBlock *dpb;
	NRPixBlock cpb;
	unsigned int state;

	nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NR_ARENA_ITEM_STATE_INVALID);
	nr_return_val_if_fail (item->state & NR_ARENA_ITEM_STATE_BBOX, item->state);

#ifdef NR_ARENA_ITEM_VERBOSE
	printf ("Invoke render %p: %d %d - %d %d\n", item, area->x0, area->y0, area->x1, area->y1);
#endif

	/* If we are outside bbox just return successfully */
#if 0
	/* Shouldn't be needed anymore with new rect logic (Lauris) */
	if (NR_RECT_DFLS_TEST_EMPTY (&item->bbox)) return item->state | NR_ARENA_ITEM_STATE_RENDER;
#endif
	if (!item->visible) return item->state | NR_ARENA_ITEM_STATE_RENDER;
	nr_rect_l_intersect (&carea, area, &item->bbox);
	if (nr_rect_l_test_empty (&carea)) return item->state | NR_ARENA_ITEM_STATE_RENDER;

	if (item->px) {
		/* Has cache pixblock, render this and return */
		nr_pixblock_setup_extern (&cpb, NR_PIXBLOCK_MODE_R8G8B8A8P,
					  /* fixme: This probably cannot overflow, because we render only if visible */
					  /* fixme: and pixel cache is there only for small items */
					  /* fixme: But this still needs extra check (Lauris) */
					  item->bbox.x0, item->bbox.y0,
					  item->bbox.x1, item->bbox.y1,
					  item->px, 4 * (item->bbox.x1 - item->bbox.x0), FALSE, FALSE);
		nr_blit_pixblock_pixblock (pb, &cpb);
		nr_pixblock_release (&cpb);
		cpb.empty = FALSE;
		return item->state | NR_ARENA_ITEM_STATE_RENDER;
	}

	dpb = pb;
	/* Setup cache if we can */
	if ((!(flags & NR_ARENA_ITEM_RENDER_NO_CACHE)) &&
	    (carea.x0 <= item->bbox.x0) && (carea.y0 <= item->bbox.y0) &&
	    (carea.x1 >= item->bbox.x1) && (carea.y1 >= item->bbox.y1) &&
	    (((item->bbox.x1 - item->bbox.x0) * (item->bbox.y1 - item->bbox.y0)) <= 4096)) {
		/* Item bbox is fully in renderable area and size is acceptable */
		carea.x0 = item->bbox.x0;
		carea.y0 = item->bbox.y0;
		carea.x1 = item->bbox.x1;
		carea.y1 = item->bbox.y1;
		item->px = nr_new (unsigned char, 4 * (carea.x1 - carea.x0) * (carea.y1 - carea.y0));
		nr_pixblock_setup_extern (&cpb, NR_PIXBLOCK_MODE_R8G8B8A8P,
					  carea.x0, carea.y0, carea.x1, carea.y1,
					  item->px, 4 * (carea.x1 - carea.x0), TRUE, TRUE);
		dpb = &cpb;
		/* Set nocache flag for downstream rendering */
		flags |= NR_ARENA_ITEM_RENDER_NO_CACHE;
	}

	/* Determine, whether we need temporary buffer */
	if (item->clip || item->mask || ((item->opacity != 255) && !item->render_opacity)) {
		NRPixBlock ipb, mpb;

		/* Setup and render item buffer */
		nr_pixblock_setup_fast (&ipb, NR_PIXBLOCK_MODE_R8G8B8A8P, carea.x0, carea.y0, carea.x1, carea.y1, TRUE);
		state = NR_ARENA_ITEM_VIRTUAL (item, render) (item, &carea, &ipb, flags);
		if (state & NR_ARENA_ITEM_STATE_INVALID) {
			/* Clean up and return error */
			nr_pixblock_release (&ipb);
			if (dpb != pb) nr_pixblock_release (dpb);
			item->state |= NR_ARENA_ITEM_STATE_INVALID;
			return item->state;
		}
		ipb.empty = FALSE;

		if (item->clip || item->mask) {
			/* Setup mask pixblock */
			nr_pixblock_setup_fast (&mpb, NR_PIXBLOCK_MODE_A8, carea.x0, carea.y0, carea.x1, carea.y1, TRUE);
			/* Do clip if needed */
			if (item->clip) {
				state = nr_arena_item_invoke_clip (item->clip, &carea, &mpb);
				if (state & NR_ARENA_ITEM_STATE_INVALID) {
					/* Clean up and return error */
					nr_pixblock_release (&mpb);
					nr_pixblock_release (&ipb);
					if (dpb != pb) nr_pixblock_release (dpb);
					item->state |= NR_ARENA_ITEM_STATE_INVALID;
					return item->state;
				}
				mpb.empty = FALSE;
			}
			/* Do mask if needed */
			if (item->mask) {
				NRPixBlock tpb;
				/* Set up yet another temporary pixblock */
				nr_pixblock_setup_fast (&tpb, NR_PIXBLOCK_MODE_R8G8B8A8N, carea.x0, carea.y0, carea.x1, carea.y1, TRUE);
				state = NR_ARENA_ITEM_VIRTUAL (item->mask, render) (item->mask, &carea, &tpb, flags);
				if (state & NR_ARENA_ITEM_STATE_INVALID) {
					/* Clean up and return error */
					nr_pixblock_release (&tpb);
					nr_pixblock_release (&mpb);
					nr_pixblock_release (&ipb);
					if (dpb != pb) nr_pixblock_release (dpb);
					item->state |= NR_ARENA_ITEM_STATE_INVALID;
					return item->state;
				}
				/* Composite with clip */
				if (item->clip) {
					int x, y;
					for (y = carea.y0; y < carea.y1; y++) {
						unsigned char *s, *d;
						s = NR_PIXBLOCK_PX (&tpb) + (y - carea.y0) * tpb.rs;
						d = NR_PIXBLOCK_PX (&mpb) + (y - carea.y0) * mpb.rs;
						for (x = carea.x0; x < carea.x1; x++) {
							unsigned int m;
							m = ((s[0] + s[1] + s[2]) * s[3] + 127) / (3 * 255);
							d[0] = NR_PREMUL (d[0], m);
							s += 4;
							d += 1;
						}
					}
				} else {
					int x, y;
					for (y = carea.y0; y < carea.y1; y++) {
						unsigned char *s, *d;
						s = NR_PIXBLOCK_PX (&tpb) + (y - carea.y0) * tpb.rs;
						d = NR_PIXBLOCK_PX (&mpb) + (y - carea.y0) * mpb.rs;
						for (x = carea.x0; x < carea.x1; x++) {
							unsigned int m;
							m = ((s[0] + s[1] + s[2]) * s[3] + 127) / (3 * 255);
							d[0] = m;
							s += 4;
							d += 1;
						}
					}
					mpb.empty = FALSE;
				}
				nr_pixblock_release (&tpb);
			}
			/* Multiply with opacity if needed */
			if ((item->opacity != 255) && !item->render_opacity) {
				int x, y;
				unsigned int a;
				a = item->opacity;
				for (y = carea.y0; y < carea.y1; y++) {
					unsigned char *d;
					d = NR_PIXBLOCK_PX (&mpb) + (y - carea.y0) * mpb.rs;
					for (x = carea.x0; x < carea.x1; x++) {
						d[0] = NR_PREMUL (d[0], a);
						d += 1;
					}
				}
			}
			/* Compose rendering pixblock int destination */
			nr_blit_pixblock_pixblock_mask (dpb, &ipb, &mpb);
			nr_pixblock_release (&mpb);
		} else {
			/* Opacity only */
			nr_blit_pixblock_pixblock_alpha (dpb, &ipb, item->opacity);
		}
		nr_pixblock_release (&ipb);
		dpb->empty = FALSE;
	} else {
		/* Just render */
		state = NR_ARENA_ITEM_VIRTUAL (item, render) (item, &carea, dpb, flags);
		if (state & NR_ARENA_ITEM_STATE_INVALID) {
			/* Clean up and return error */
			if (dpb != pb) nr_pixblock_release (dpb);
			item->state |= NR_ARENA_ITEM_STATE_INVALID;
			return item->state;
		}
		dpb->empty = FALSE;
	}

	if (dpb != pb) {
		/* Have to blit from cache */
		nr_blit_pixblock_pixblock (pb, dpb);
		nr_pixblock_release (dpb);
		pb->empty = FALSE;
		item->state |= NR_ARENA_ITEM_STATE_IMAGE;
	}

	return item->state | NR_ARENA_ITEM_STATE_RENDER;
}

unsigned int
nr_arena_item_invoke_clip (NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
	nr_return_val_if_fail (item != NULL, NR_ARENA_ITEM_STATE_INVALID);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NR_ARENA_ITEM_STATE_INVALID);
#if 0
	nr_return_val_if_fail (item->state & NR_ARENA_ITEM_STATE_CLIP, item->state);
#endif
	nr_return_val_if_fail ((pb->area.x1 - pb->area.x0) >= (area->x1 - area->x0), NR_ARENA_ITEM_STATE_INVALID);
	nr_return_val_if_fail ((pb->area.y1 - pb->area.y0) >= (area->y1 - area->y0), NR_ARENA_ITEM_STATE_INVALID);

#ifdef NR_ARENA_ITEM_VERBOSE
	printf ("Invoke render %p: %d %d - %d %d\n", item, area->x0, area->y0, area->x1, area->y1);
#endif

	if (item->visible && nr_rect_l_test_intersect (area, &item->bbox)) {
		/* Need render that item */
		if (((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->clip)
			return ((NRArenaItemClass *) NR_OBJECT_GET_CLASS(item))->clip (item, area, pb);
	}

	return item->state;
}

NRArenaItem *
nr_arena_item_invoke_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky)
{
	int ix, iy;

	nr_return_val_if_fail (item != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);
	nr_return_val_if_fail (item->state & NR_ARENA_ITEM_STATE_BBOX, NULL);
	nr_return_val_if_fail (item->state & NR_ARENA_ITEM_STATE_PICK, NULL);

	if (!sticky && !item->sensitive) return NULL;

	ix = (int) x;
	iy = (int) y;

	if (((x + delta) >= item->bbox.x0) &&
	    ((x - delta) <  item->bbox.x1) &&
	    ((y + delta) >= item->bbox.y0) &&
	    ((y - delta) <  item->bbox.y1)) {
		if (((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->pick)
			return ((NRArenaItemClass *) NR_OBJECT_GET_CLASS (item))->pick (item, x, y, delta, sticky);
	}

	return NULL;
}

void
nr_arena_item_request_update (NRArenaItem *item, unsigned int reset, unsigned int propagate)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));
	nr_return_if_fail (!(reset & NR_ARENA_ITEM_STATE_INVALID));

	if (propagate && !item->propagate) item->propagate = TRUE;

	if (item->state & reset) {
		item->state &= ~reset;
#if 0
		if ((reset & NR_ARENA_ITEM_STATE_IMAGE) && item->px) {
			/* Concept test */
			/* Clear buffer */
			nr_free (item->px);
			item->px = NULL;
		}
#endif
		if (item->parent) {
			nr_arena_item_request_update (item->parent, reset, FALSE);
		} else {
			nr_arena_request_update (item->arena, item);
		}
	}
}

void
nr_arena_item_request_render (NRArenaItem *item)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	nr_arena_request_render_rect (item->arena, &item->bbox);
}

/* Public */

NRArenaItem *
nr_arena_item_new (NRArena *arena, unsigned int type)
{
	NRArenaItem *item;

	nr_return_val_if_fail (arena != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA (arena), NULL);
	nr_return_val_if_fail (nr_type_is_a (type, NR_TYPE_ARENA_ITEM), NULL);

	item = (NRArenaItem *) nr_object_new (type);

	item->arena = arena;

	/* nr_arena_item_added (arena, item); */

	return item;
}

NRArenaItem *
nr_arena_item_unparent (NRArenaItem *item)
{
	nr_return_val_if_fail (item != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (item), NULL);

	nr_arena_item_request_render (item);

	if (item->parent) {
		nr_arena_item_remove_child (item->parent, item);
	}

	return NULL;
}

void
nr_arena_item_append_child (NRArenaItem *parent, NRArenaItem *child)
{
	NRArenaItem *ref;

	nr_return_if_fail (parent != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (parent));
	nr_return_if_fail (child != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (child));
	nr_return_if_fail (parent->arena == child->arena);
	nr_return_if_fail (child->parent == NULL);
	nr_return_if_fail (child->prev == NULL);
	nr_return_if_fail (child->next == NULL);

	ref = nr_arena_item_children (parent);
	if (ref) while (ref->next) ref = ref->next;

	nr_arena_item_add_child (parent, child, ref);
}

void
nr_arena_item_set_transform (NRArenaItem *item, const NRMatrixF *transform)
{
	const NRMatrixF *ms, *md;

	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	if (!transform && !item->transform) return;

	md = (item->transform) ? item->transform : &NR_MATRIX_F_IDENTITY;
	ms = (transform) ? transform : &NR_MATRIX_F_IDENTITY;

	if (!NR_MATRIX_DF_TEST_CLOSE (md, ms, NR_EPSILON_F)) {
		nr_arena_item_request_render (item);
		if (!transform || nr_matrix_f_test_identity (transform, NR_EPSILON_F)) {
			/* Set to identity affine */
			if (item->transform) nr_free (item->transform);
			item->transform = NULL;
		} else {
			if (!item->transform) item->transform = nr_new (NRMatrixF, 1);
			*item->transform = *transform;
		}
		nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, TRUE);
	}
}

void
nr_arena_item_set_opacity (NRArenaItem *item, double opacity)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	nr_arena_item_request_render (item);

	item->opacity = (unsigned int) (opacity * 255.9999);
}

void
nr_arena_item_set_sensitive (NRArenaItem *item, unsigned int sensitive)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	/* fixme: mess with pick/repick... */

	item->sensitive = sensitive;
}

void
nr_arena_item_set_visible (NRArenaItem *item, unsigned int visible)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	item->visible = visible;

	nr_arena_item_request_render (item);
}

void
nr_arena_item_set_clip (NRArenaItem *item, NRArenaItem *clip)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));
	nr_return_if_fail (!clip || NR_IS_ARENA_ITEM (clip));

	if (clip != item->clip) {
		nr_arena_item_request_render (item);
		if (item->clip) item->clip = nr_arena_item_detach_unref (item, item->clip);
		if (clip) item->clip = nr_arena_item_attach_ref (item, clip, NULL, NULL);
		nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, TRUE);
	}
}

void
nr_arena_item_set_mask (NRArenaItem *item, NRArenaItem *mask)
{
	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));
	nr_return_if_fail (!mask || NR_IS_ARENA_ITEM (mask));

	if (mask != item->mask) {
		nr_arena_item_request_render (item);
		if (item->mask) item->mask = nr_arena_item_detach_unref (item, item->mask);
		if (mask) item->mask = nr_arena_item_attach_ref (item, mask, NULL, NULL);
		nr_arena_item_request_update (item, NR_ARENA_ITEM_STATE_ALL, TRUE);
	}
}

void
nr_arena_item_set_order (NRArenaItem *item, int order)
{
	NRArenaItem *children, *child, *ref;
	int pos;

	nr_return_if_fail (item != NULL);
	nr_return_if_fail (NR_IS_ARENA_ITEM (item));

	if (!item->parent) return;

	children = nr_arena_item_children (item->parent);

	ref = NULL;
	pos = 0;
	for (child = children; child != NULL; child = child->next) {
		if (pos >= order) break;
		if (child != item) {
			ref = child;
			pos += 1;
		}
	}

	nr_arena_item_set_child_position (item->parent, item, ref);
}

/* Helpers */

NRArenaItem *
nr_arena_item_attach_ref (NRArenaItem *parent, NRArenaItem *child, NRArenaItem *prev, NRArenaItem *next)
{
	nr_return_val_if_fail (parent != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (parent), NULL);
	nr_return_val_if_fail (child != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (child), NULL);
	nr_return_val_if_fail (child->parent == NULL, NULL);
	nr_return_val_if_fail (child->prev == NULL, NULL);
	nr_return_val_if_fail (child->next == NULL, NULL);
	nr_return_val_if_fail (!prev || NR_IS_ARENA_ITEM (prev), NULL);
	nr_return_val_if_fail (!prev || (prev->parent == parent), NULL);
	nr_return_val_if_fail (!prev || (prev->next == next), NULL);
	nr_return_val_if_fail (!next || NR_IS_ARENA_ITEM (next), NULL);
	nr_return_val_if_fail (!next || (next->parent == parent), NULL);
	nr_return_val_if_fail (!next || (next->prev == prev), NULL);

	nr_arena_item_ref (child);

	child->parent = parent;
	child->prev = prev;
	child->next = next;

	if (prev) prev->next = child;
	if (next) next->prev = child;

	return child;
}

NRArenaItem *
nr_arena_item_detach_unref (NRArenaItem *parent, NRArenaItem *child)
{
	NRArenaItem *prev, *next;

	nr_return_val_if_fail (parent != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (parent), NULL);
	nr_return_val_if_fail (child != NULL, NULL);
	nr_return_val_if_fail (NR_IS_ARENA_ITEM (child), NULL);
	nr_return_val_if_fail (child->parent == parent, NULL);

	prev = child->prev;
	next = child->next;

	child->parent = NULL;
	child->prev = NULL;
	child->next = NULL;

	nr_arena_item_unref (child);

	if (prev) prev->next = next;
	if (next) next->prev = prev;

	return next;
}


