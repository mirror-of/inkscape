#define __SP_OBJECTGROUP_C__

/*
 * Abstract base class for non-item groups
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/repr-private.h"
#include "sp-object-group.h"
#include "sp-object-repr.h"

static void sp_objectgroup_class_init (SPObjectGroupClass *klass);
static void sp_objectgroup_init (SPObjectGroup *objectgroup);

static void sp_objectgroup_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_objectgroup_release (SPObject *object);
static void sp_objectgroup_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_objectgroup_remove_child (SPObject * object, SPRepr * child);
static void sp_objectgroup_order_changed (SPObject * object, SPRepr * child, SPRepr * old, SPRepr * new);
static SPRepr *sp_objectgroup_write (SPObject *object, SPRepr *repr, guint flags);

static SPObject *sp_objectgroup_get_le_child_by_repr (SPObjectGroup *og, SPRepr *ref);

static SPObjectClass *parent_class;

GType
sp_objectgroup_get_type (void)
{
	static GType objectgroup_type = 0;
	if (!objectgroup_type) {
		GTypeInfo objectgroup_info = {
			sizeof (SPObjectGroupClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_objectgroup_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPObjectGroup),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_objectgroup_init,
		};
		objectgroup_type = g_type_register_static (SP_TYPE_OBJECT, "SPObjectGroup", &objectgroup_info, 0);
	}
	return objectgroup_type;
}

static void
sp_objectgroup_class_init (SPObjectGroupClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;

	parent_class = g_type_class_ref (SP_TYPE_OBJECT);

	sp_object_class->build = sp_objectgroup_build;
	sp_object_class->release = sp_objectgroup_release;
	sp_object_class->child_added = sp_objectgroup_child_added;
	sp_object_class->remove_child = sp_objectgroup_remove_child;
	sp_object_class->order_changed = sp_objectgroup_order_changed;
	sp_object_class->write = sp_objectgroup_write;
}

static void
sp_objectgroup_init (SPObjectGroup *objectgroup)
{
	objectgroup->children = NULL;
}

static void sp_objectgroup_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	SPObjectGroup *og;
	SPObject *last;
	SPRepr *rchild;

	og = SP_OBJECTGROUP (object);

	if (((SPObjectClass *) (parent_class))->build)
		(* ((SPObjectClass *) (parent_class))->build) (object, document, repr);

	last = NULL;
	for (rchild = repr->children; rchild != NULL; rchild = rchild->next) {
		GType type;
		SPObject *child;
		type = sp_repr_type_lookup (rchild);
		if (g_type_is_a (type, SP_TYPE_OBJECT)) {
			child = g_object_new (type, 0);
			if (last) {
				last->next = sp_object_attach_reref (object, child, NULL);
			} else {
				og->children = sp_object_attach_reref (object, child, NULL);
			}
			sp_object_invoke_build (child, document, rchild, SP_OBJECT_IS_CLONED (object));
			last = child;
		}
	}
}

static void
sp_objectgroup_release (SPObject *object)
{
	SPObjectGroup *og;

	og = SP_OBJECTGROUP (object);

	while (og->children) {
		og->children = sp_object_detach_unref (object, og->children);
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_objectgroup_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPObjectGroup * og;
	GType type;

	og = SP_OBJECTGROUP (object);

	if (((SPObjectClass *) (parent_class))->child_added)
		(* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);

	type = sp_repr_type_lookup (child);
	if (g_type_is_a (type, SP_TYPE_OBJECT)) {
		SPObject *ochild, *prev;
		ochild = g_object_new (type, 0);
		prev = sp_objectgroup_get_le_child_by_repr (og, ref);
		if (prev) {
			prev->next = sp_object_attach_reref (object, ochild, prev->next);
		} else {
			og->children = sp_object_attach_reref (object, ochild, og->children);
		}
		sp_object_invoke_build (ochild, object->document, child, SP_OBJECT_IS_CLONED (object));
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
	}
}

static void
sp_objectgroup_remove_child (SPObject *object, SPRepr *child)
{
	SPObjectGroup *og;
	SPObject *ref, *oc;

	og = SP_OBJECTGROUP (object);

	if (((SPObjectClass *) (parent_class))->remove_child)
		(* ((SPObjectClass *) (parent_class))->remove_child) (object, child);

	ref = NULL;
	oc = og->children;
	while (oc && (SP_OBJECT_REPR (oc) != child)) {
		ref = oc;
		oc = oc->next;
	}

	if (oc) {
		if (ref) {
			ref->next = sp_object_detach_unref (object, oc);
		} else {
			og->children = sp_object_detach_unref (object, oc);
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
	}
}

static void
sp_objectgroup_order_changed (SPObject *object, SPRepr *child, SPRepr *old, SPRepr *new)
{
	SPObjectGroup *og;
	SPObject *ochild, *oold, *onew;

	og = SP_OBJECTGROUP (object);

	if (((SPObjectClass *) (parent_class))->order_changed)
		(* ((SPObjectClass *) (parent_class))->order_changed) (object, child, old, new);

	ochild = sp_objectgroup_get_le_child_by_repr (og, child);
	oold = sp_objectgroup_get_le_child_by_repr (og, old);
	onew = sp_objectgroup_get_le_child_by_repr (og, new);

	if (ochild) {
		if (oold) {
			oold->next = sp_object_detach (object, ochild);
		} else {
			og->children = sp_object_detach (object, ochild);
		}
		if (onew) {
			onew->next = sp_object_attach_reref (object, ochild, (onew) ? onew->next : og->children);
		} else {
			og->children = sp_object_attach_reref (object, ochild, (onew) ? onew->next : og->children);
		}
		sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
	}
}

static SPRepr *
sp_objectgroup_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPObjectGroup *group;
	SPObject *child;
	SPRepr *crepr;

	group = SP_OBJECTGROUP (object);

	if (flags & SP_OBJECT_WRITE_BUILD) {
		GSList *l;
		if (!repr) repr = sp_repr_new ("g");
		l = NULL;
		for (child = group->children; child != NULL; child = child->next) {
			crepr = sp_object_invoke_write (child, NULL, flags);
			if (crepr) l = g_slist_prepend (l, crepr);
		}
		while (l) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (child = group->children; child != NULL; child = child->next) {
			sp_object_invoke_write (child, SP_OBJECT_REPR (child), flags);
		}
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static SPObject *
sp_objectgroup_get_le_child_by_repr (SPObjectGroup *og, SPRepr *ref)
{
	SPObject *o, *oc;
	SPRepr *r, *rc;

	if (!ref) return NULL;

	o = NULL;
	r = SP_OBJECT_REPR (og);
	rc = r->children;
	oc = og->children;

	while (rc) {
		if (rc == ref) return o;
		if (oc && (SP_OBJECT_REPR (oc) == rc)) {
			/* Rewing object */
			o = oc;
			oc = oc->next;
		}
		/* Rewind repr */
		rc = rc->next;
	}

	g_assert_not_reached ();

	return NULL;
}
