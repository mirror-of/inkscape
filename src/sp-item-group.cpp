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
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <xml/repr.h>
#include <xml/repr-get-children.h>
#include "sp-object-repr.h"
#include "svg/svg.h"
#include "document.h"
#include "style.h"
#include "attributes.h"

#include "sp-root.h"
#include "sp-use.h"
#include "sp-item-group.h"
#include "helper/sp-intl.h"

static void sp_group_class_init (SPGroupClass *klass);
static void sp_group_init (SPGroup *group);
static void sp_group_build(SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_group_release(SPObject *object);
static void sp_group_dispose (GObject *object);

static void sp_group_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_group_remove_child (SPObject * object, SPRepr * child);
static void sp_group_order_changed (SPObject * object, SPRepr * child, SPRepr * old_ref, SPRepr * new_ref);
static void sp_group_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_group_modified (SPObject *object, guint flags);
static SPRepr *sp_group_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_group_set(SPObject *object, unsigned key, char const *value);

static void sp_group_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static void sp_group_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_group_description (SPItem * item);
static NRArenaItem *sp_group_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_group_hide (SPItem * item, unsigned int key);
static void sp_group_snappoints (SPItem const *item, SnapPointsIter p);

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
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPGroup", &group_info, (GTypeFlags)0);
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

	parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);

	object_class->dispose = sp_group_dispose;

	sp_object_class->child_added = sp_group_child_added;
	sp_object_class->remove_child = sp_group_remove_child;
	sp_object_class->order_changed = sp_group_order_changed;
	sp_object_class->update = sp_group_update;
	sp_object_class->modified = sp_group_modified;
	sp_object_class->set = sp_group_set;
	sp_object_class->write = sp_group_write;
	sp_object_class->release = sp_group_release;
	sp_object_class->build = sp_group_build;

	item_class->bbox = sp_group_bbox;
	item_class->print = sp_group_print;
	item_class->description = sp_group_description;
	item_class->show = sp_group_show;
	item_class->hide = sp_group_hide;
	item_class->snappoints = sp_group_snappoints;
}

static void
sp_group_init (SPGroup *group)
{
	group->_layer_mode = SPGroup::GROUP;
	new (&group->_display_modes) std::map<unsigned int, SPGroup::LayerMode>();
}

static void sp_group_build(SPObject *object, SPDocument *document, SPRepr *repr)
{
	sp_object_read_attr(object, "inkscape:groupmode");

	if (((SPObjectClass *)parent_class)->build) {
		((SPObjectClass *)parent_class)->build(object, document, repr);
	}
}

static void sp_group_release(SPObject *object) {
	if ( SP_GROUP(object)->_layer_mode == SPGroup::LAYER ) {
		sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "layer", object);
	}
	if (((SPObjectClass *)parent_class)->release) {
		((SPObjectClass *)parent_class)->release(object);
	}
}

static void
sp_group_dispose(GObject *object)
{
	SP_GROUP(object)->_display_modes.~map();
}

static void
sp_group_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPItem *item;

	item = SP_ITEM (object);

	if (((SPObjectClass *) (parent_class))->child_added)
		(* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);

	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	if ( ochild && SP_IS_ITEM(ochild) ) {
		/* TODO: this should be moved into SPItem somehow */
		SPItemView *v;
		NRArenaItem *ac;

		unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));

		for (v = item->display; v != NULL; v = v->next) {
			ac = sp_item_invoke_show (SP_ITEM (ochild), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);

			if (ac) {
				nr_arena_item_add_child (v->arenaitem, ac, NULL);
				nr_arena_item_set_order (ac, position);
				nr_arena_item_unref (ac);
			}
		}
	}

	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_group_remove_child (SPObject * object, SPRepr * child)
{
	if (((SPObjectClass *) (parent_class))->remove_child)
		(* ((SPObjectClass *) (parent_class))->remove_child) (object, child);

	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_group_order_changed (SPObject *object, SPRepr *child, SPRepr *old_ref, SPRepr *new_ref)
{
	if (((SPObjectClass *) (parent_class))->order_changed)
		(* ((SPObjectClass *) (parent_class))->order_changed) (object, child, old_ref, new_ref);

	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	if ( ochild && SP_IS_ITEM(ochild) ) {
		/* TODO: this should be moved into SPItem somehow */
		SPItemView *v;
		unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));
		for ( v = SP_ITEM (ochild)->display ; v != NULL ; v = v->next ) {
			nr_arena_item_set_order (v->arenaitem, position);
		}
	}

	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
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
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (child)) {
				SPItem const &chi = *SP_ITEM(child);
				cctx.i2doc = chi.transform * ictx->i2doc;
				cctx.i2vp = chi.transform * ictx->i2vp;
				child->updateDisplay((SPCtx *)&cctx, flags);
			} else {
				child->updateDisplay(ctx, flags);
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
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
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
		for (child = sp_object_first_child(object); child != NULL; child = SP_OBJECT_NEXT(child) ) {
			crepr = child->updateRepr(NULL, flags);
			if (crepr) l = g_slist_prepend (l, crepr);
		}
		while (l) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (child = sp_object_first_child(object) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
			child->updateRepr(flags);
		}
	}

	if ( flags & SP_OBJECT_WRITE_EXT ) {
		const char *value;
		if ( group->_layer_mode == SPGroup::LAYER ) {
			value = "layer";
		} else if ( flags & SP_OBJECT_WRITE_ALL ) {
			value = "group";
		} else {
			value = NULL;
		}
		sp_repr_set_attr(repr, "inkscape:groupmode", NULL);
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_group_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags)
{
	for (SPObject *o = sp_object_first_child(SP_OBJECT(item)); o != NULL; o = SP_OBJECT_NEXT(o)) {
		if (SP_IS_ITEM(o)) {
			SPItem *child = SP_ITEM(o);
			NR::Matrix const ct(child->transform * transform);
			sp_item_invoke_bbox_full(child, bbox, ct, flags, FALSE);
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

	for (o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
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

	group = SP_GROUP (item);

	len = 0;
	for ( o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (SP_IS_ITEM(o)) {
			len += 1;
		}
	}

	return g_strdup_printf(_("Group of %d objects"), len);
}

static void sp_group_set(SPObject *object, unsigned key, char const *value) {
	SPGroup *group=SP_GROUP(object);

	switch (key) {
		case SP_ATTR_INKSCAPE_GROUPMODE: {
			if (!strcmp(value, "layer")) {
				group->setLayerMode(SPGroup::LAYER);
			} else {
				group->setLayerMode(SPGroup::GROUP);
			}
		} break;
		default: {
			if (((SPObjectClass *) (parent_class))->set) {
				(* ((SPObjectClass *) (parent_class))->set)(object, key, value);
			}
		}
	}
}

static NRArenaItem *
sp_group_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPGroup *group;
	NRArenaItem *ai, *ac, *ar;
	SPItem * child;
	SPObject * o;

	group = (SPGroup *) item;

	ai = NRArenaGroup::create(arena);
	nr_arena_group_set_transparent(NR_ARENA_GROUP (ai),
	                               group->effectiveLayerMode(key) ==
				         SPGroup::LAYER);

	ar = NULL;

	for (o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL; o = SP_OBJECT_NEXT(o) ) {
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

	for (o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL; o = SP_OBJECT_NEXT(o) ) {
		if (SP_IS_ITEM (o)) {
			child = SP_ITEM (o);
			sp_item_invoke_hide (child, key);
		}
	}

	if (((SPItemClass *) parent_class)->hide)
		((SPItemClass *) parent_class)->hide (item, key);
}

static void sp_group_snappoints (SPItem const *item, SnapPointsIter p)
{
	for (SPObject const *o = sp_object_first_child(SP_OBJECT(item));
	     o != NULL;
	     o = SP_OBJECT_NEXT(o))
	{
		if (SP_IS_ITEM(o)) {
			sp_item_snappoints(SP_ITEM(o), p);
		}
	}
}


void
sp_item_group_ungroup (SPGroup *group, GSList **children, bool do_done)
{
	g_return_if_fail (group != NULL);
	g_return_if_fail (SP_IS_GROUP (group));

	SPDocument *doc = SP_OBJECT_DOCUMENT (group);
	SPObject *root = SP_DOCUMENT_ROOT (doc);
	SPObject *defs = SP_OBJECT (SP_ROOT (root)->defs);

	SPItem *gitem = SP_ITEM (group);
	SPRepr *grepr = SP_OBJECT_REPR (gitem);
	SPCSSAttr *gstyle = sp_css_attr_from_style (SP_OBJECT (gitem));

	SPItem *pitem = SP_ITEM (SP_OBJECT_PARENT (gitem));
	SPRepr *prepr = SP_OBJECT_REPR (pitem);

	g_return_if_fail (!strcmp (sp_repr_name (grepr), "g") || !strcmp (sp_repr_name (grepr), "a"));

	/* Step 1 - generate lists of children objects */
	GSList *items = NULL;
	GSList *objects = NULL;
	for (SPObject *child = sp_object_first_child(SP_OBJECT(group)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {

		SPRepr *nrepr = sp_repr_duplicate (SP_OBJECT_REPR (child));

		if (SP_IS_ITEM (child)) {
			gchar affinestr[80];

			SPItem *citem = SP_ITEM (child);

			NR::Matrix ctrans;
			if (SP_IS_USE(citem) && (SP_OBJECT_PARENT (sp_use_get_original (SP_USE(citem))) == SP_OBJECT(group))) {
				// make sure a clone's effective transform is the same as was under group
				NR::Matrix const g(gitem->transform);
				ctrans = g.inverse() * citem->transform * g;
			} else {
				ctrans = citem->transform * gitem->transform;
			}

			// FIXME: cannot call sp_item_write_transform here - the repr is unattached at this point, has no item. rethink!
			if (sp_svg_transform_write(affinestr, 79, ctrans)) {
				sp_repr_set_attr (nrepr, "transform", affinestr);
			} else {
				sp_repr_set_attr (nrepr, "transform", NULL);
			}

			/* Merging of style */
			// we do this by merging SPCSSAttrs, because there's no easy way to do this with SPStyle
			// perhaps we need to program some sort of sp_style_combine_with_parent (SPStyle *, SPStyle *)
			SPCSSAttr *cstyle = sp_repr_css_attr_new ();
			sp_repr_css_merge (cstyle, gstyle);
			sp_repr_css_merge (cstyle, sp_css_attr_from_style (SP_OBJECT (citem)));
			sp_repr_css_change (nrepr, cstyle, "style");

			items = g_slist_prepend (items, nrepr);
		} else {
			objects = g_slist_prepend (objects, nrepr);
		}
	}

	/* Step 2 - clear group */
	// remember the position of the group
	SPRepr *g_prev_sibling = sp_repr_prev_sibling(SP_OBJECT_REPR(group));

	// the group is leaving forever, no heir, clones should take note; its children however are going to reemerge
	SP_OBJECT (group)->deleteObject(true, false);

	/* Step 3 - add nonitems */
	if (objects) {
	    SPRepr *last_def = sp_repr_last_child(SP_OBJECT_REPR(defs));
	    while (objects) {
		sp_repr_add_child(SP_OBJECT_REPR(defs), (SPRepr *) objects->data, last_def);
		sp_repr_unref ((SPRepr *) objects->data);
		objects = g_slist_remove (objects, objects->data);
	    }
	}

	/* Step 4 - add items */
	while (items) {
		SPRepr *repr = (SPRepr *) items->data;
		// add item
		sp_repr_add_child(prepr, repr, g_prev_sibling);

		// fill in the children list if non-null
		SPItem *nitem = (SPItem *) doc->getObjectByRepr(repr);
		sp_repr_unref (repr);
		if (children && SP_IS_ITEM (nitem)) 
			*children = g_slist_prepend (*children, nitem);

		items = g_slist_remove (items, items->data);
	}

	if (do_done) sp_document_done (doc);
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

	for ( o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
		if (SP_IS_ITEM (o)) {
			s = g_slist_prepend (s, o);
		}
	}

	return g_slist_reverse (s);
}

SPObject *
sp_item_group_get_child_by_name (SPGroup *group, SPObject *ref, const gchar *name)
{
	SPObject *child;
	child = (ref) ? SP_OBJECT_NEXT(ref) : sp_object_first_child(SP_OBJECT(group));
	while ( child && strcmp (sp_repr_name (SP_OBJECT_REPR(child)), name) ) {
		child = SP_OBJECT_NEXT(child);
	}
	return child;
}

void SPGroup::setLayerMode(LayerMode mode) {
	if ( _layer_mode != mode ) {
		if ( mode == LAYER ) {
			sp_document_add_resource(SP_OBJECT_DOCUMENT(this), "layer", this);
		} else {
			sp_document_remove_resource(SP_OBJECT_DOCUMENT(this), "layer", this);
		}
		_layer_mode = mode;
		_updateLayerMode();
	}
}

SPGroup::LayerMode SPGroup::layerDisplayMode(unsigned int dkey) const {
	std::map<unsigned int, LayerMode>::const_iterator iter;
	iter = _display_modes.find(dkey);
	if ( iter != _display_modes.end() ) {
		return (*iter).second;
	} else {
		return GROUP;
	}
}

void SPGroup::setLayerDisplayMode(unsigned int dkey, SPGroup::LayerMode mode) {
	if ( layerDisplayMode(dkey) != mode ) {
		_display_modes[dkey] = mode;
		_updateLayerMode(dkey);
	}
}

void SPGroup::_updateLayerMode(unsigned int display_key) {
	SPItemView *view;
	for ( view = this->display ; view ; view = view->next ) {
		if ( !display_key || view->key == display_key ) {
			NRArenaGroup *arena_group=NR_ARENA_GROUP(view->arenaitem);
			if (arena_group) {
				nr_arena_group_set_transparent(arena_group, effectiveLayerMode(view->key) == SPGroup::LAYER);
			}
		}
	}
}

