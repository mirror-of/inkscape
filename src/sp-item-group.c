#define __SP_GROUP_C__

/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <string.h>

#include "display/nr-arena-group.h"
#include "xml/repr-private.h"
#include "sp-object-repr.h"
#include "svg/svg.h"
#include "document.h"
#include "style.h"

#include "sp-root.h"
#include "sp-item-group.h"
#include "helper/sp-intl.h"

static void sp_group_class_init (SPGroupClass *klass);
static void sp_group_init (SPGroup *group);

static void sp_group_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_group_release (SPObject *object);
static void sp_group_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_group_remove_child (SPObject * object, SPRepr * child);
static void sp_group_order_changed (SPObject * object, SPRepr * child, SPRepr * old, SPRepr * new);
static void sp_group_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_group_modified (SPObject *object, guint flags);
static gint sp_group_sequence (SPObject *object, gint seq);
static SPRepr *sp_group_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_group_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags);
static void sp_group_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_group_description (SPItem * item);
static NRArenaItem *sp_group_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_group_hide (SPItem * item, unsigned int key);

static SPItemClass * parent_class;

GType
sp_group_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPGroupClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_group_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPGroup),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_group_init,
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPGroup", &group_info, 0);
	}
	return group_type;
}

static void
sp_group_class_init (SPGroupClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = g_type_class_ref (SP_TYPE_ITEM);

	sp_object_class->build = sp_group_build;
	sp_object_class->release = sp_group_release;
	sp_object_class->child_added = sp_group_child_added;
	sp_object_class->remove_child = sp_group_remove_child;
	sp_object_class->order_changed = sp_group_order_changed;
	sp_object_class->update = sp_group_update;
	sp_object_class->modified = sp_group_modified;
	sp_object_class->sequence = sp_group_sequence;
	sp_object_class->write = sp_group_write;

	item_class->bbox = sp_group_bbox;
	item_class->print = sp_group_print;
	item_class->description = sp_group_description;
	item_class->show = sp_group_show;
	item_class->hide = sp_group_hide;
}

static void
sp_group_init (SPGroup *group)
{
	group->children = NULL;
	group->transparent = FALSE;
}

static void sp_group_build (SPObject *object, SPDocument * document, SPRepr * repr)
{
	SPGroup * group;
	SPObject * last;
	SPRepr * rchild;

	group = SP_GROUP (object);

	if (((SPObjectClass *) (parent_class))->build)
		(* ((SPObjectClass *) (parent_class))->build) (object, document, repr);

	last = NULL;
	for (rchild = repr->children; rchild != NULL; rchild = rchild->next) {
		GType type;
		SPObject * child;
		type = sp_repr_type_lookup (rchild);
		child = g_object_new (type, 0);
		if (last) {
			last->next = sp_object_attach_reref (object, child, NULL);
		} else {
			group->children = sp_object_attach_reref (object, child, NULL);
		}
		sp_object_invoke_build (child, document, rchild, SP_OBJECT_IS_CLONED (object));
		last = child;
	}
}

static void
sp_group_release (SPObject *object)
{
	SPGroup * group;

	group = SP_GROUP (object);

	while (group->children) {
		group->children = sp_object_detach_unref (object, group->children);
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_group_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPGroup *group;
	SPItem *item;
	GType type;
	SPObject *ochild, *prev;
	gint position;

	item = SP_ITEM (object);
	group = SP_GROUP (object);

	if (((SPObjectClass *) (parent_class))->child_added)
		(* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);

	/* Search for position reference */
	prev = NULL;
	position = 0;
	if (ref != NULL) {
		prev = group->children;
		while (prev && (prev->repr != ref)) {
			if (SP_IS_ITEM (prev)) position += 1;
			prev = prev->next;
		}
		if (SP_IS_ITEM (prev)) position += 1;
	}

	type = sp_repr_type_lookup (child);
	ochild = g_object_new (type, 0);
	if (prev) {
		prev->next = sp_object_attach_reref (object, ochild, prev->next);
	} else {
		group->children = sp_object_attach_reref (object, ochild, group->children);
	}

	sp_object_invoke_build (ochild, object->document, child, SP_OBJECT_IS_CLONED (object));

	if (SP_IS_ITEM (ochild)) {
		SPItemView *v;
		NRArenaItem *ac;
		for (v = item->display; v != NULL; v = v->next) {
			ac = sp_item_invoke_show (SP_ITEM (ochild), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);
			if (ac) {
				nr_arena_item_add_child (v->arenaitem, ac, NULL);
				nr_arena_item_set_order (ac, position);
				nr_arena_item_unref (ac);
			}
		}
	}

	sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_group_remove_child (SPObject * object, SPRepr * child)
{
	SPGroup * group;
	SPObject * prev, * ochild;

	group = SP_GROUP (object);

	if (((SPObjectClass *) (parent_class))->remove_child)
		(* ((SPObjectClass *) (parent_class))->remove_child) (object, child);

	prev = NULL;
	ochild = group->children;
	while (ochild->repr != child) {
		prev = ochild;
		ochild = ochild->next;
	}

	if (prev) {
		prev->next = sp_object_detach_unref (object, ochild);
	} else {
		group->children = sp_object_detach_unref (object, ochild);
	}
	sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_group_order_changed (SPObject *object, SPRepr *child, SPRepr *old, SPRepr *new)
{
	SPGroup *group;
	SPObject *childobj, *oldobj, *newobj, *o;
	gint childpos, oldpos, newpos;

	group = SP_GROUP (object);

	if (((SPObjectClass *) (parent_class))->order_changed)
		(* ((SPObjectClass *) (parent_class))->order_changed) (object, child, old, new);

	childobj = oldobj = newobj = NULL;
	oldpos = newpos = 0;

	/* Scan children list */
	childpos = 0;
	for (o = group->children; !childobj || (old && !oldobj) || (new && !newobj); o = o->next) {
		if (o->repr == child) {
			childobj = o;
		} else {
			if (SP_IS_ITEM (o)) childpos += 1;
		}
		if (old && o->repr == old) {
			oldobj = o;
			oldpos = childpos;
		}
		if (new && o->repr == new) {
			newobj = o;
			newpos = childpos;
		}
	}

	if (oldobj) {
		oldobj->next = childobj->next;
	} else {
		group->children = childobj->next;
	}
	if (newobj) {
		childobj->next = newobj->next;
		newobj->next = childobj;
	} else {
		childobj->next = group->children;
		group->children = childobj;
	}

	if (SP_IS_ITEM (childobj)) {
		SPItemView *v;
		for (v = SP_ITEM (childobj)->display; v != NULL; v = v->next) {
			nr_arena_item_set_order (v->arenaitem, newpos);
		}
	}

	sp_object_request_modified (object, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_group_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPGroup *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;

	group = SP_GROUP (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;

	if (((SPObjectClass *) (parent_class))->update)
		((SPObjectClass *) (parent_class))->update (object, ctx, flags);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = group->children; child != NULL; child = child->next) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (child)) {
				SPItem *chi;
				chi = SP_ITEM (child);
				nr_matrix_multiply_dfd (&cctx.i2doc, &chi->transform, &ictx->i2doc);
				nr_matrix_multiply_dfd (&cctx.i2vp, &chi->transform, &ictx->i2vp);
				sp_object_invoke_update (child, (SPCtx *) &cctx, flags);
			} else {
				sp_object_invoke_update (child, ctx, flags);
			}
		}
		g_object_unref (G_OBJECT (child));
	}
}

static void
sp_group_modified (SPObject *object, guint flags)
{
	SPGroup *group;
	SPObject *child;
	GSList *l;

	group = SP_GROUP (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	l = NULL;
	for (child = group->children; child != NULL; child = child->next) {
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

static gint
sp_group_sequence (SPObject *object, gint seq)
{
	SPGroup *group;
	SPObject *child;

	group = SP_GROUP (object);

	seq += 1;

	for (child = group->children; child != NULL; child = child->next) {
		seq = sp_object_sequence (child, seq);
	}

	return seq;
}

static SPRepr *
sp_group_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPGroup *group;
	SPObject *child;
	SPRepr *crepr;

	group = SP_GROUP (object);

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

static void
sp_group_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags)
{
	SPGroup * group;
	SPItem * child;
	SPObject * o;

	group = SP_GROUP (item);

	for (o = group->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) {
			NRMatrixD ct;
			child = SP_ITEM (o);
			nr_matrix_multiply_dfd (&ct, &child->transform, transform);
			sp_item_invoke_bbox_full (child, bbox, &ct, flags, FALSE);
		}
	}
}

static void
sp_group_print (SPItem * item, SPPrintContext *ctx)
{
	SPGroup * group;
	SPItem * child;
	SPObject * o;

	group = SP_GROUP (item);

	for (o = group->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) {
			child = SP_ITEM (o);
			sp_item_invoke_print (SP_ITEM (o), ctx);
		}
	}
}

static gchar * sp_group_description (SPItem * item)
{
	SPGroup * group;
	SPObject * o;
	gint len;
	static char c[128];

	group = SP_GROUP (item);

	len = 0;
	for (o = group->children; o != NULL; o = o->next) len += 1;

	g_snprintf (c, 128, _("Group of %d objects"), len);

	return g_strdup (c);
}

static NRArenaItem *
sp_group_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPGroup *group;
	NRArenaItem *ai, *ac, *ar;
	SPItem * child;
	SPObject * o;

	group = (SPGroup *) item;

	ai = nr_arena_item_new (arena, NR_TYPE_ARENA_GROUP);
	nr_arena_group_set_transparent (NR_ARENA_GROUP (ai), group->transparent);

	ar = NULL;
	for (o = group->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) {
			child = SP_ITEM (o);
			ac = sp_item_invoke_show (child, arena, key, flags);
			if (ac) {
				nr_arena_item_add_child (ai, ac, ar);
				ar = ac;
				nr_arena_item_unref (ac);
			}
		}
	}

	return ai;
}

static void
sp_group_hide (SPItem *item, unsigned int key)
{
	SPGroup * group;
	SPItem * child;
	SPObject * o;

	group = (SPGroup *) item;

	for (o = group->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) {
			child = SP_ITEM (o);
			sp_item_invoke_hide (child, key);
		}
	}

	if (((SPItemClass *) parent_class)->hide)
		((SPItemClass *) parent_class)->hide (item, key);
}

void
sp_item_group_ungroup (SPGroup *group, GSList **children)
{
	SPDocument *doc;
	SPItem *gitem, *pitem;
	SPRepr *grepr, *prepr, *lrepr;
	SPObject *root, *defs, *child;
	GSList *items, *objects;

	g_return_if_fail (group != NULL);
	g_return_if_fail (SP_IS_GROUP (group));

	doc = SP_OBJECT_DOCUMENT (group);
	root = SP_DOCUMENT_ROOT (doc);
	defs = SP_OBJECT (SP_ROOT (root)->defs);

	gitem = SP_ITEM (group);
	grepr = SP_OBJECT_REPR (gitem);
	pitem = SP_ITEM (SP_OBJECT_PARENT (gitem));
	prepr = SP_OBJECT_REPR (pitem);

	g_return_if_fail (!strcmp (sp_repr_name (grepr), "g") || !strcmp (sp_repr_name (grepr), "a"));

	/* Step 1 - generate lists of children objects */
	items = NULL;
	objects = NULL;
	for (child = group->children; child != NULL; child = child->next) {
		SPRepr *nrepr;
		nrepr = sp_repr_duplicate (SP_OBJECT_REPR (child));
		if (SP_IS_ITEM (child)) {
			SPItem *citem;
			NRMatrixF ctrans;
			gchar affinestr[80];
			guchar *ss;

			citem = SP_ITEM (child);

			nr_matrix_multiply_fff (&ctrans, &citem->transform, &gitem->transform);
			if (sp_svg_transform_write (affinestr, 79, &ctrans)) {
				sp_repr_set_attr (nrepr, "transform", affinestr);
			} else {
				sp_repr_set_attr (nrepr, "transform", NULL);
			}

			/* Merging of style */
			/* fixme: We really should respect presentation attributes too */
			ss = sp_style_write_difference (SP_OBJECT_STYLE (citem), SP_OBJECT_STYLE (pitem));
			sp_repr_set_attr (nrepr, "style", ss);
			g_free (ss);

			items = g_slist_prepend (items, nrepr);
		} else {
			objects = g_slist_prepend (objects, nrepr);
		}
	}

	items = g_slist_reverse (items);
	objects = g_slist_reverse (objects);

	/* Step 2 - clear group */
	while (group->children) {
		/* Now it is time to remove original */
		sp_repr_remove_child (grepr, SP_OBJECT_REPR (group->children));
	}

	/* Step 3 - add nonitems */
	while (objects) {
		sp_repr_append_child (SP_OBJECT_REPR (defs), (SPRepr *) objects->data);
		sp_repr_unref ((SPRepr *) objects->data);
		objects = g_slist_remove (objects, objects->data);
	}

	/* Step 4 - add items */
	lrepr = grepr;
	while (items) {
		SPItem *nitem;
		sp_repr_add_child (prepr, (SPRepr *) items->data, lrepr);
		lrepr = (SPRepr *) items->data;
		nitem = (SPItem *) sp_document_lookup_id (doc, sp_repr_attr ((SPRepr *) items->data, "id"));
		sp_repr_unref ((SPRepr *) items->data);
		if (children && SP_IS_ITEM (nitem)) *children = g_slist_prepend (*children, nitem);
		items = g_slist_remove (items, items->data);
	}

	sp_repr_unparent (grepr);
	sp_document_done (doc);
}

/*
 * some API for list aspect of SPGroup
 */

GSList * 
sp_item_group_item_list (SPGroup * group)
{
        GSList *s;
	SPObject *o;

	g_return_val_if_fail (group != NULL, NULL);
	g_return_val_if_fail (SP_IS_GROUP (group), NULL);

	s = NULL;

	for (o = group->children; o != NULL; o = o->next) {
		if (SP_IS_ITEM (o)) s = g_slist_prepend (s, o);
	}

	return g_slist_reverse (s);
}

SPObject *
sp_item_group_get_child_by_name (SPGroup *group, SPObject *ref, const unsigned char *name)
{
	SPObject *child;
	child = (ref) ? ref->next : group->children;
	while (child && strcmp (sp_repr_name (child->repr), name)) child = child->next;
	return child;
}

