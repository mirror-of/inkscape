#define __SP_CLIPPATH_C__

/*
 * SVG <clipPath> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include <libnr/nr-matrix.h>

#include "display/nr-arena.h"
#include "display/nr-arena-group.h"

#include "enums.h"
#include "attributes.h"
#include "document.h"
#include "sp-item.h"

#include "sp-clippath.h"

struct _SPClipPathView {
	SPClipPathView *next;
	unsigned int key;
	NRArenaItem *arenaitem;
	NRRectF bbox;
};

static void sp_clippath_class_init (SPClipPathClass *klass);
static void sp_clippath_init (SPClipPath *clippath);

static void sp_clippath_build (SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_clippath_release (SPObject * object);
static void sp_clippath_set (SPObject *object, unsigned int key, const unsigned char *value);
static void sp_clippath_child_added (SPObject *object, SPRepr *child, SPRepr *ref);
static void sp_clippath_remove_child (SPObject *object, SPRepr *child);
static void sp_clippath_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_clippath_modified (SPObject *object, guint flags);
static SPRepr *sp_clippath_write (SPObject *object, SPRepr *repr, guint flags);

SPClipPathView *sp_clippath_view_new_prepend (SPClipPathView *list, unsigned int key, NRArenaItem *arenaitem);
SPClipPathView *sp_clippath_view_list_remove (SPClipPathView *list, SPClipPathView *view);

static SPObjectGroupClass *parent_class;

GType
sp_clippath_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPClipPathClass),
			NULL, NULL,
			(GClassInitFunc) sp_clippath_class_init,
			NULL, NULL,
			sizeof (SPClipPath),
			16,
			(GInstanceInitFunc) sp_clippath_init,
		};
		type = g_type_register_static (SP_TYPE_OBJECTGROUP, "SPClipPath", &info, 0);
	}
	return type;
}

static void
sp_clippath_class_init (SPClipPathClass *klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	parent_class = g_type_class_ref (SP_TYPE_OBJECTGROUP);

	sp_object_class->build = sp_clippath_build;
	sp_object_class->release = sp_clippath_release;
	sp_object_class->set = sp_clippath_set;
	sp_object_class->child_added = sp_clippath_child_added;
	sp_object_class->remove_child = sp_clippath_remove_child;
	sp_object_class->update = sp_clippath_update;
	sp_object_class->modified = sp_clippath_modified;
	sp_object_class->write = sp_clippath_write;
}

static void
sp_clippath_init (SPClipPath *cp)
{
	cp->clipPathUnits_set = FALSE;
	cp->clipPathUnits = SP_CONTENT_UNITS_USERSPACEONUSE;

	cp->display = NULL;
}

static void
sp_clippath_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	SPClipPath *cp;

	cp = SP_CLIPPATH (object);

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "clipPathUnits");

	/* Register ourselves */
	sp_document_add_resource (document, "clipPath", object);
}

static void
sp_clippath_release (SPObject * object)
{
	SPClipPath *cp;

	cp = SP_CLIPPATH (object);

	if (SP_OBJECT_DOCUMENT (object)) {
		/* Unregister ourselves */
		sp_document_remove_resource (SP_OBJECT_DOCUMENT (object), "clipPath", object);
	}

	while (cp->display) {
		/* We simply unref and let item to manage this in handler */
		cp->display = sp_clippath_view_list_remove (cp->display, cp->display);
	}

	if (((SPObjectClass *) (parent_class))->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_clippath_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPClipPath *cp;

	cp = SP_CLIPPATH (object);

	switch (key) {
	case SP_ATTR_CLIPPATHUNITS:
		cp->clipPathUnits = SP_CONTENT_UNITS_USERSPACEONUSE;
		cp->clipPathUnits_set = FALSE;
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				cp->clipPathUnits_set = TRUE;
			} else if (!strcmp (value, "objectBoundingBox")) {
				cp->clipPathUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;
				cp->clipPathUnits_set = TRUE;
			}
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_clippath_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPClipPath *cp;
	SPObject *ochild;

	cp = SP_CLIPPATH (object);

	/* Invoke SPObjectGroup implementation */
	((SPObjectClass *) (parent_class))->child_added (object, child, ref);

	/* Show new object */
	ochild = sp_document_lookup_id (SP_OBJECT_DOCUMENT (object), sp_repr_attr (child, "id"));
	if (SP_IS_ITEM (ochild)) {
		SPClipPathView *v;
		for (v = cp->display; v != NULL; v = v->next) {
			NRArenaItem *ac;
			ac = sp_item_invoke_show (SP_ITEM (ochild), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, SP_ITEM_REFERENCE_FLAGS);
			if (ac) {
				nr_arena_item_add_child (v->arenaitem, ac, NULL);
				nr_arena_item_unref (ac);
			}
		}
	}
}

static void
sp_clippath_remove_child (SPObject *object, SPRepr *child)
{
	SPClipPath *cp;

	cp = SP_CLIPPATH (object);

	/* Invoke SPObjectGroup implementation */
	((SPObjectClass *) (parent_class))->remove_child (object, child);
}

static void
sp_clippath_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPObjectGroup *og;
	SPClipPath *cp;
	SPObject *child;
	SPClipPathView *v;
	GSList *l;

	og = SP_OBJECTGROUP (object);
	cp = SP_CLIPPATH (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = og->children; child != NULL; child = child->next) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_update (child, ctx, flags);
		}
		g_object_unref (G_OBJECT (child));
	}

	for (v = cp->display; v != NULL; v = v->next) {
		if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
			NRMatrixF t;
			nr_matrix_f_set_scale (&t, v->bbox.x1 - v->bbox.x0, v->bbox.y1 - v->bbox.y0);
			t.c[4] = v->bbox.x0;
			t.c[5] = v->bbox.y0;
			nr_arena_group_set_child_transform (NR_ARENA_GROUP (v->arenaitem), &t);
		} else {
			nr_arena_group_set_child_transform (NR_ARENA_GROUP (v->arenaitem), NULL);
		}
	}
}

static void
sp_clippath_modified (SPObject *object, guint flags)
{
	SPObjectGroup *og;
	SPClipPath *cp;
	SPObject *child;
	GSList *l;

	og = SP_OBJECTGROUP (object);
	cp = SP_CLIPPATH (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = og->children; child != NULL; child = child->next) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			sp_object_invoke_modified (child, flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}

static SPRepr *
sp_clippath_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPClipPath *cp;

	cp = SP_CLIPPATH (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("clipPath");
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

NRArenaItem *
sp_clippath_show (SPClipPath *cp, NRArena *arena, unsigned int key)
{
	NRArenaItem *ai, *ac;
	SPObject *child;

	g_return_val_if_fail (cp != NULL, NULL);
	g_return_val_if_fail (SP_IS_CLIPPATH (cp), NULL);
	g_return_val_if_fail (arena != NULL, NULL);
	g_return_val_if_fail (NR_IS_ARENA (arena), NULL);

	ai = nr_arena_item_new (arena, NR_TYPE_ARENA_GROUP);
	cp->display = sp_clippath_view_new_prepend (cp->display, key, ai);

	for (child = SP_OBJECTGROUP (cp)->children; child != NULL; child = child->next) {
		if (SP_IS_ITEM (child)) {
			ac = sp_item_invoke_show (SP_ITEM (child), arena, key, SP_ITEM_REFERENCE_FLAGS);
			if (ac) {
				/* The order is not important in clippath */
				nr_arena_item_add_child (ai, ac, NULL);
				nr_arena_item_unref (ac);
			}
		}
	}

	if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
		NRMatrixF t;
		nr_matrix_f_set_scale (&t, cp->display->bbox.x1 - cp->display->bbox.x0, cp->display->bbox.y1 - cp->display->bbox.y0);
		t.c[4] = cp->display->bbox.x0;
		t.c[5] = cp->display->bbox.y0;
		nr_arena_group_set_child_transform (NR_ARENA_GROUP (ai), &t);
	}

	return ai;
}

void
sp_clippath_hide (SPClipPath *cp, unsigned int key)
{
	SPClipPathView *v;
	SPObject *child;

	g_return_if_fail (cp != NULL);
	g_return_if_fail (SP_IS_CLIPPATH (cp));

	for (child = SP_OBJECTGROUP (cp)->children; child != NULL; child = child->next) {
		if (SP_IS_ITEM (child)) {
			sp_item_invoke_hide (SP_ITEM (child), key);
		}
	}

	for (v = cp->display; v != NULL; v = v->next) {
		if (v->key == key) {
			/* We simply unref and let item to manage this in handler */
			cp->display = sp_clippath_view_list_remove (cp->display, v);
			return;
		}
	}

	g_assert_not_reached ();
}

void
sp_clippath_set_bbox (SPClipPath *cp, unsigned int key, NRRectF *bbox)
{
	SPClipPathView *v;

	for (v = cp->display; v != NULL; v = v->next) {
		if (v->key == key) {
			if (!NR_DF_TEST_CLOSE (v->bbox.x0, bbox->x0, NR_EPSILON_F) ||
			    !NR_DF_TEST_CLOSE (v->bbox.y0, bbox->y0, NR_EPSILON_F) ||
			    !NR_DF_TEST_CLOSE (v->bbox.x1, bbox->x1, NR_EPSILON_F) ||
			    !NR_DF_TEST_CLOSE (v->bbox.y1, bbox->y1, NR_EPSILON_F)) {
				v->bbox = *bbox;
				sp_object_request_update (SP_OBJECT (cp), SP_OBJECT_MODIFIED_FLAG);
			}
			break;
		}
	}
}

/* ClipPath views */

SPClipPathView *
sp_clippath_view_new_prepend (SPClipPathView *list, unsigned int key, NRArenaItem *arenaitem)
{
	SPClipPathView *new;

	new = g_new (SPClipPathView, 1);

	new->next = list;
	new->key = key;
	new->arenaitem = nr_arena_item_ref (arenaitem);
	new->bbox.x0 = new->bbox.x1 = 0.0;
	new->bbox.y0 = new->bbox.y1 = 0.0;

	return new;
}

SPClipPathView *
sp_clippath_view_list_remove (SPClipPathView *list, SPClipPathView *view)
{
	if (view == list) {
		list = list->next;
	} else {
		SPClipPathView *prev;
		prev = list;
		while (prev->next != view) prev = prev->next;
		prev->next = view->next;
	}

	nr_arena_item_unref (view->arenaitem);
	g_free (view);

	return list;
}

