#define __SP_OBJECT_C__
/*
 * Abstract base class for all nodes
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdlib.h>
#include <string.h>

#include "helper/sp-marshal.h"
#include "xml/repr-private.h"
#include "attributes.h"
#include "document.h"
#include "style.h"
#include "sp-object-repr.h"
#include "sp-root.h"

#include "sp-object.h"

#define noSP_OBJECT_DEBUG
#define noSP_OBJECT_DEBUG_CASCADE

static void sp_object_class_init (SPObjectClass * klass);
static void sp_object_init (SPObject * object);
static void sp_object_finalize (GObject * object);

static void sp_object_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_object_remove_child (SPObject * object, SPRepr * child);
static void sp_object_order_changed (SPObject * object, SPRepr * child, SPRepr * old_ref, SPRepr * new_ref);

static void sp_object_release(SPObject *object);
static void sp_object_build (SPObject * object, SPDocument * document, SPRepr * repr);

static void sp_object_private_set (SPObject *object, unsigned int key, const gchar *value);
static SPRepr *sp_object_private_write (SPObject *object, SPRepr *repr, guint flags);

/* Real handlers of repr signals */

static unsigned int sp_object_repr_change_attr (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, gpointer data);
static void sp_object_repr_attr_changed (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, gpointer data);

static void sp_object_repr_content_changed (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, gpointer data);

static void sp_object_repr_child_added (SPRepr *repr, SPRepr *child, SPRepr *ref, gpointer data);
static void sp_object_repr_child_removed (SPRepr *repr, SPRepr *child, SPRepr *ref, void *data);

static void sp_object_repr_order_changed (SPRepr *repr, SPRepr *child, SPRepr *old, SPRepr *newer, gpointer data);

static gchar * sp_object_get_unique_id (SPObject * object, const gchar * defid);

enum {RELEASE, MODIFIED, LAST_SIGNAL};

SPReprEventVector object_event_vector = {
	NULL, /* Destroy */
	NULL, /* Add child */
	sp_object_repr_child_added,
	NULL, /* Remove child */
	sp_object_repr_child_removed,
	sp_object_repr_change_attr,
	sp_object_repr_attr_changed,
	NULL, /* Change content */
	sp_object_repr_content_changed,
	NULL, /* change_order */
	sp_object_repr_order_changed
};

static GObjectClass *parent_class;
static guint object_signals[LAST_SIGNAL] = {0};

GType
sp_object_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPObjectClass),
			NULL, NULL,
			(GClassInitFunc) sp_object_class_init,
			NULL, NULL,
			sizeof (SPObject),
			16,
			(GInstanceInitFunc) sp_object_init,
			NULL
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPObject", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_object_class_init (SPObjectClass * klass)
{
	GObjectClass * object_class;

	object_class = (GObjectClass *) klass;

	parent_class = (GObjectClass *) g_type_class_ref (G_TYPE_OBJECT);

	object_signals[RELEASE] =  g_signal_new ("release",
						 G_TYPE_FROM_CLASS (klass),
						 (GSignalFlags)(G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
						 G_STRUCT_OFFSET (SPObjectClass, release),
						 NULL, NULL,
						 sp_marshal_VOID__VOID,
						 G_TYPE_NONE, 0);
	object_signals[MODIFIED] = g_signal_new ("modified",
                                                 G_TYPE_FROM_CLASS (klass),
                                                 G_SIGNAL_RUN_FIRST,
                                                 G_STRUCT_OFFSET (SPObjectClass, modified),
                                                 NULL, NULL,
                                                 sp_marshal_NONE__UINT,
                                                 G_TYPE_NONE, 1, G_TYPE_UINT);

	object_class->finalize = sp_object_finalize;

	klass->child_added = sp_object_child_added;
	klass->remove_child = sp_object_remove_child;
	klass->order_changed = sp_object_order_changed;

	klass->release = sp_object_release;

	klass->build = sp_object_build;

	klass->set = sp_object_private_set;
	klass->write = sp_object_private_write;
}

static void
sp_object_init (SPObject * object)
{
#ifdef SP_OBJECT_DEBUG
	g_print("sp_object_init: id=%x, typename=%s\n", object, g_type_name_from_instance((GTypeInstance*)object));
#endif

	object->hrefcount = 0;
	object->_total_hrefcount = 0;
	object->document = NULL;
	object->children = NULL;
	object->parent = object->next = NULL;
	object->repr = NULL;
	object->id = NULL;
	object->style = NULL;

	object->_collection_policy = SPObject::COLLECT_WITH_PARENT;

	new (&object->_delete_signal) SigC::Signal1<void, SPObject *>();
	object->_successor = NULL;
}

static void
sp_object_finalize (GObject * object)
{
	SPObject *spobject = (SPObject *)object;

	if (spobject->_successor) {
		sp_object_unref(spobject->_successor, NULL);
		spobject->_successor = NULL;
	}

	if (((GObjectClass *) (parent_class))->finalize) {
		(* ((GObjectClass *) (parent_class))->finalize) (object);
	}

	spobject->_delete_signal.~Signal1();
}

/*
 * Refcounting
 *
 * Owner is here for debug reasons, you can set it to NULL safely
 * Ref should return object, NULL is error, unref return always NULL
 */

SPObject *
sp_object_ref (SPObject *object, SPObject *owner)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (SP_IS_OBJECT (object), NULL);
	g_return_val_if_fail (!owner || SP_IS_OBJECT (owner), NULL);

	g_object_ref (G_OBJECT (object));

	return object;
}

SPObject *
sp_object_unref (SPObject *object, SPObject *owner)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (SP_IS_OBJECT (object), NULL);
	g_return_val_if_fail (!owner || SP_IS_OBJECT (owner), NULL);

	g_object_unref (G_OBJECT (object));

	return NULL;
}

SPObject *
sp_object_href (SPObject *object, gpointer owner)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (SP_IS_OBJECT (object), NULL);

	object->hrefcount++;
	object->_updateTotalHRefCount(1);

	return object;
}

SPObject *
sp_object_hunref (SPObject *object, gpointer owner)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (SP_IS_OBJECT (object), NULL);
	g_return_val_if_fail (object->hrefcount > 0, NULL);

	object->hrefcount--;
	object->_updateTotalHRefCount(-1);

	return NULL;
}

void SPObject::_updateTotalHRefCount(int increment) {
	SPObject *topmost_collectable=NULL;
	for ( SPObject *iter=this ; iter ; iter = SP_OBJECT_PARENT(iter) ) {
		iter->_total_hrefcount += increment;
		if ( iter->_total_hrefcount < iter->hrefcount ) {
			g_critical("HRefs overcounted");
		}
		if ( iter->_total_hrefcount == 0 &&
		     iter->_collection_policy != COLLECT_WITH_PARENT )
		{
			topmost_collectable = iter;
		}
	}
	if (topmost_collectable) {
		topmost_collectable->requestOrphanCollection();
	}
}

void SPObject::requestOrphanCollection() {
	g_return_if_fail(document != NULL);
	document->queueForOrphanCollection(this);
}

void SPObject::setId(gchar const *id) {
	sp_object_set(this, SP_ATTR_ID, id);
}

void SPObject::_sendDeleteSignalRecursive() {
	for (SPObject *child = sp_object_first_child(this); child; child = SP_OBJECT_NEXT (child)) {
		child->_delete_signal.emit(child);
		child->_sendDeleteSignalRecursive();
	}
}

void SPObject::deleteObject(bool propagate, bool propagate_descendants)
{
	if (propagate) {
		_delete_signal.emit(this);
	}
	if (propagate_descendants) {
		this->_sendDeleteSignalRecursive();
	}

	SPRepr *repr=SP_OBJECT_REPR(this);
	if (repr && sp_repr_parent(repr)) {
		sp_repr_unparent(repr);
	}

	if (_successor) {
		_successor->deleteObject(propagate, propagate_descendants);
	}
}

/*
 * Attaching/detaching
 */

void
sp_object_attach_reref (SPObject *parent, SPObject *object, SPObject *next)
{
	g_return_if_fail (parent != NULL);
	g_return_if_fail (SP_IS_OBJECT (parent));
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_OBJECT (object));
	g_return_if_fail (!next || SP_IS_OBJECT (next));
	g_return_if_fail (!object->parent);
	g_return_if_fail (!object->next);

	SPObject **ref;
	for ( ref = &parent->children ; *ref ; ref = &(*ref)->next ) {
		if ( *ref == next ) {
			break;
		}
	}
	if ( *ref != next ) {
		g_critical("sp_object_attach_reref: next is not a child of parent");
	}

	sp_object_ref (object, parent);
	g_object_unref (G_OBJECT (object));
	object->parent = parent;
	parent->_updateTotalHRefCount(object->_total_hrefcount);

	object->next = next;
	*ref = object;
}

void sp_object_reorder(SPObject *object, SPObject *next)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(SP_IS_OBJECT(object));
	g_return_if_fail(object->parent != NULL);
	g_return_if_fail(object != next);
	g_return_if_fail(!next || SP_IS_OBJECT(next));
	g_return_if_fail(!next || next->parent == object->parent);

	SPObject *parent = object->parent;
	SPObject **ref;
	SPObject **old_ref = NULL;
	SPObject **new_ref = NULL;
	for ( ref = &parent->children ; *ref ; ref = &(*ref)->next ) {
		if ( *ref == object ) {
			old_ref = ref;
		}
		if ( *ref == next ) {
			new_ref = ref;
		}
	}
	if ( !new_ref && !next ) {
		new_ref = ref;
	}
	g_assert(old_ref != NULL);
	g_assert(new_ref != NULL);

	*old_ref = object->next;
	object->next = *new_ref;
	*new_ref = object;
}

void sp_object_detach (SPObject *parent, SPObject *object) {
	g_return_if_fail (parent != NULL);
	g_return_if_fail (SP_IS_OBJECT (parent));
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_OBJECT (object));
	g_return_if_fail (object->parent == parent);

	SPObject **ref;
	for ( ref = &parent->children ; *ref ; ref = &(*ref)->next ) {
		if ( *ref == object ) {
			break;
		}
	}
	g_assert(*ref == object);

	*ref = object->next;
	object->next = NULL;
	object->parent = NULL;

	sp_object_invoke_release (object);
	parent->_updateTotalHRefCount(-object->_total_hrefcount);
}

void sp_object_detach_unref (SPObject *parent, SPObject *object)
{
	g_return_if_fail (parent != NULL);
	g_return_if_fail (SP_IS_OBJECT (parent));
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_OBJECT (object));
	g_return_if_fail (object->parent == parent);

	sp_object_detach(parent, object);
	sp_object_unref(object, parent);
}

SPObject *sp_object_first_child(SPObject *parent)
{
	return parent->children;
}

SPObject *sp_object_get_child_by_repr(SPObject *object, SPRepr *repr)
{
	g_return_val_if_fail(object != NULL, NULL);
	g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
	g_return_val_if_fail(repr != NULL, NULL);

	for ( SPObject *child = object->children ; child ; child = child->next ) {
		if ( SP_OBJECT_REPR(child) == repr ) {
			return child;
		}
	}

	return NULL;
}

static void sp_object_child_added (SPObject * object, SPRepr * child, SPRepr * ref)
{
	GType type = sp_repr_type_lookup(child);
	if (!type) {
		return;
	}
	SPObject *ochild = SP_OBJECT(g_object_new(type, 0));
	SPObject *prev = ref ? sp_object_get_child_by_repr(object, ref) : NULL;
	if (prev) {
		sp_object_attach_reref(object, ochild, SP_OBJECT_NEXT(prev));
	} else {
		if (ref) {
			g_critical("Unable to find previous child; adding new child out of order");
		}
		sp_object_attach_reref(object, ochild, sp_object_first_child(object));
	}

	sp_object_invoke_build(ochild, object->document, child, SP_OBJECT_IS_CLONED(object));
}

static void sp_object_remove_child (SPObject * object, SPRepr * child)
{
	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	g_return_if_fail(ochild != NULL);
	sp_object_detach_unref(object, ochild);
}

static void sp_object_order_changed (SPObject * object, SPRepr * child, SPRepr * old_ref,
				     SPRepr * new_ref)
{
	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	g_return_if_fail(ochild != NULL);
	SPObject *prev = new_ref ? sp_object_get_child_by_repr(object, new_ref) : NULL;
	if (prev) {
		sp_object_reorder(ochild, SP_OBJECT_NEXT(prev));
	} else {
		if (new_ref) {
			g_critical("Unable to find new previous child; reordering child to start of list");
		}
		sp_object_reorder(ochild, sp_object_first_child(object));
	}
}

static void sp_object_release(SPObject *object)
{
	while (object->children) {
		sp_object_detach_unref(object, object->children);
	}
}

/*
 * SPObject specific build method
 */

static void
sp_object_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
	/* Nothing specific here */
#ifdef SP_OBJECT_DEBUG
	g_print("sp_object_build: id=%x, typename=%s\n", object, g_type_name_from_instance((GTypeInstance*)object));
#endif

	sp_object_read_attr (object, "xml:space");
	sp_object_read_attr (object, "inkscape:collect");

        for (SPRepr *rchild = repr->children; rchild != NULL; rchild = rchild->next) {
		GType type = sp_repr_type_lookup (rchild);
		if (!type) {
			continue;
		}
		SPObject *child = SP_OBJECT(g_object_new (type, 0));
		sp_object_attach_reref (object, child, NULL);
		sp_object_invoke_build (child, document, rchild, SP_OBJECT_IS_CLONED (object));
	}
}

void
sp_object_invoke_build (SPObject * object, SPDocument * document, SPRepr * repr, unsigned int cloned)
{
#ifdef SP_OBJECT_DEBUG
	g_print("sp_object_invoke_build: id=%x, typename=%s\n", object, g_type_name_from_instance((GTypeInstance*)object));
#endif
	g_assert (object != NULL);
	g_assert (SP_IS_OBJECT (object));
	g_assert (document != NULL);
	g_assert (SP_IS_DOCUMENT (document));
	g_assert (repr != NULL);

	g_assert (object->document == NULL);
	g_assert (object->repr == NULL);
	g_assert (object->id == NULL);

	/* Bookkeeping */

	object->document = document;
	object->repr = repr;
	sp_repr_ref (repr);
	object->cloned = cloned;

	/* If we are not cloned, force unique id */
	if (!SP_OBJECT_IS_CLONED (object)) {
		const gchar *id = sp_repr_attr (object->repr, "id");
		gchar *realid = sp_object_get_unique_id (object, id);
		g_assert (realid != NULL);

		sp_document_def_id (object->document, realid, object);
		object->id = realid;

		/* Redefine ID, if required */
		if ((id == NULL) || (strcmp (id, realid) != 0)) {
			int ret = sp_repr_set_attr (object->repr, "id", realid);
			if (!ret) {
				g_error ("Cannot change id %s -> %s - probably there is stale ref", id, realid);
			}
		}
	} else {
		g_assert (object->id == NULL);
	}

	/* Invoke derived methods, if any */

	if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->build) {
		(*((SPObjectClass *) G_OBJECT_GET_CLASS(object))->build) (object, document, repr);
	}

	/* Signalling (should be connected AFTER processing derived methods */
	sp_repr_add_listener (repr, &object_event_vector, object);
}

void
sp_object_invoke_release (SPObject *object)
{
	g_assert (object != NULL);
	g_assert (SP_IS_OBJECT (object));

	/* Parent refcount us, so there shouldn't be any */
	g_assert (!object->parent);
	g_assert (!object->next);
	g_assert (object->document);
	g_assert (object->repr);

	sp_repr_remove_listener_by_data (object->repr, object);

	g_signal_emit (G_OBJECT (object), object_signals[RELEASE], 0);

	/* all hrefs should be released by the "release" handler */
	g_assert (object->hrefcount == 0);

	if (!SP_OBJECT_IS_CLONED (object)) {
		g_assert (object->id);
		sp_document_def_id (object->document, object->id, NULL);
		g_free (object->id);
		object->id = NULL;
	} else {
		g_assert (!object->id);
	}

	if (object->style) {
		object->style = sp_style_unref (object->style);
	}

	sp_repr_unref (object->repr);

	object->document = NULL;
	object->repr = NULL;
}

static void
sp_object_repr_child_added (SPRepr *repr, SPRepr *child, SPRepr *ref, gpointer data)
{
	SPObject *object = SP_OBJECT (data);

	if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->child_added)
		(*((SPObjectClass *)G_OBJECT_GET_CLASS(object))->child_added) (object, child, ref);
}

static void
sp_object_repr_child_removed (SPRepr *repr, SPRepr *child, SPRepr *ref, gpointer data)
{
	SPObject *object = SP_OBJECT (data);

	if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->remove_child) {
		(* ((SPObjectClass *)G_OBJECT_GET_CLASS(object))->remove_child) (object, child);
	}
}

/* fixme: */

static void
sp_object_repr_order_changed (SPRepr * repr, SPRepr * child, SPRepr * old, SPRepr * newer, gpointer data)
{
	SPObject * object = SP_OBJECT (data);

	if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->order_changed) {
		(* ((SPObjectClass *)G_OBJECT_GET_CLASS(object))->order_changed) (object, child, old, newer);
	}
}

static void
sp_object_private_set (SPObject *object, unsigned int key, const gchar *value)
{
	g_assert (SP_IS_DOCUMENT (object->document));
	/* fixme: rething that cloning issue */
	g_assert (SP_OBJECT_IS_CLONED (object) || object->id != NULL);
	g_assert (key != SP_ATTR_INVALID);

	switch (key) {
	case SP_ATTR_ID:
		if (!SP_OBJECT_IS_CLONED (object)) {
			g_assert (value != NULL);
			g_assert (strcmp ((const char*)value, object->id));
			g_assert (!object->document->getObjectById((const char*)value));
			sp_document_def_id (object->document, object->id, NULL);
			g_free (object->id);
			object->id = g_strdup ((const char*)value);
			sp_document_def_id (object->document, object->id, object);
		} else {
			// This warning fires when the id is changed on the original of an SPUse, because the SPUse is updated from the same repr.
			// The child of an SPUse has a cloned flag set - I have little idea of what it is used for.
			// Anyway, the warning is useless, because the child has no repr of its own, and therefore no id conflict may ensue in XML.
			//g_warning ("ID of cloned object changed, so document is out of sync");
		}
		break;
	case SP_ATTR_INKSCAPE_COLLECT:
		if ( value && !strcmp(value, "always") ) {
			object->setCollectionPolicy(SPObject::ALWAYS_COLLECT);
		} else {
			object->setCollectionPolicy(SPObject::COLLECT_WITH_PARENT);
		}
		break;
	case SP_ATTR_XML_SPACE:
		if (value && !strcmp (value, "preserve")) {
			object->xml_space.value = SP_XML_SPACE_PRESERVE;
			object->xml_space.set = TRUE;
		} else if (value && !strcmp (value, "default")) {
			object->xml_space.value = SP_XML_SPACE_DEFAULT;
			object->xml_space.set = TRUE;
		} else if (object->parent) {
			SPObject *parent;
			parent = object->parent;
			object->xml_space.value = parent->xml_space.value;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
		break;
	default:
		break;
	}
}

void
sp_object_set (SPObject *object, unsigned int key, const gchar *value)
{
	g_assert (object != NULL);
	g_assert (SP_IS_OBJECT (object));

	if (((SPObjectClass *) G_OBJECT_GET_CLASS (object))->set) {
		((SPObjectClass *) G_OBJECT_GET_CLASS(object))->set (object, key, value);
	}
}

void
sp_object_read_attr (SPObject *object, const gchar *key)
{
	g_assert (object != NULL);
	g_assert (SP_IS_OBJECT (object));
	g_assert (key != NULL);

	g_assert (SP_IS_DOCUMENT (object->document));
	g_assert (object->repr != NULL);
	/* fixme: rething that cloning issue */
	g_assert (SP_OBJECT_IS_CLONED (object) || object->id != NULL);

	unsigned int keyid = sp_attribute_lookup (key);
	if (keyid != SP_ATTR_INVALID) {
		/* Retrieve the 'key' attribute from the object's XML representation */
		const gchar *value = sp_repr_attr (object->repr, key);

		sp_object_set (object, keyid, value);
	}
}

static unsigned int
sp_object_repr_change_attr (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, gpointer data)
{
	SPObject *object = SP_OBJECT (data);

	if (strcmp ((const char*)key, "id") == 0) {
		if (!newval) {
			return FALSE;
		}
		gpointer defid = object->document->getObjectById(newval);
		if (defid == object) {
			return TRUE;
		}
		if (defid) {
			return FALSE;
		}
	}

	return TRUE;
}

static void
sp_object_repr_attr_changed (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, gpointer data)
{
	SPObject *object = SP_OBJECT (data);

	sp_object_read_attr (object, key);

	// manual changes to extension attributes require the normal
	// attributes, which depend on them, to be updated immediately
	if (is_interactive) {
		object->updateRepr(repr, 0);
	}
}

static void
sp_object_repr_content_changed (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, gpointer data)
{
	SPObject * object = SP_OBJECT (data);

	if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->read_content)
		(*((SPObjectClass *) G_OBJECT_GET_CLASS(object))->read_content) (object);
}

static const gchar*
sp_xml_get_space_string(unsigned int space)
{
	switch(space)
	{
	case SP_XML_SPACE_DEFAULT:
		return "default";
	case SP_XML_SPACE_PRESERVE:
		return "preserve";
	default:
		return NULL;
	}
}

static SPRepr *
sp_object_private_write (SPObject *object, SPRepr *repr, guint flags)
{
	if (!repr && (flags & SP_OBJECT_WRITE_BUILD)) {
		repr = sp_repr_duplicate (SP_OBJECT_REPR (object));
		if (!( flags & SP_OBJECT_WRITE_EXT )) {
			sp_repr_set_attr(repr, "inkscape:collect", NULL);
		}
	} else {
		sp_repr_set_attr (repr, "id", object->id);

		if (object->xml_space.set) {
			const char *xml_space;
			xml_space = sp_xml_get_space_string(object->xml_space.value);
			sp_repr_set_attr (repr, "xml:space", xml_space);
		}

		if ( object->collectionPolicy() == SPObject::ALWAYS_COLLECT ) {
			sp_repr_set_attr(repr, "inkscape:collect", "always");
		} else {
			sp_repr_set_attr(repr, "inkscape:collect", NULL);
		}
	}
	
	return repr;
}

SPRepr *SPObject::updateRepr(unsigned int flags) {
	if (!SP_OBJECT_IS_CLONED(this)) {
		SPRepr *repr=SP_OBJECT_REPR(this);
		if (repr) {
			return updateRepr(repr, flags);
		} else {
			g_critical("Attempt to update non-existent repr");
			return NULL;
		}
	} else {
		/* cloned objects have no repr */
		return NULL;
	}
}

SPRepr *SPObject::updateRepr(SPRepr *repr, unsigned int flags) {
	if (SP_OBJECT_IS_CLONED(this)) {
		/* cloned objects have no repr */
		return NULL;
	}
	if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->write) {
		if (!(flags & SP_OBJECT_WRITE_BUILD) && !repr) {
			repr = SP_OBJECT_REPR (this);
		}
		return ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->write (this, repr, flags);
	} else {
		g_warning ("Class %s does not implement ::write", G_OBJECT_TYPE_NAME (this));
		if (!repr) {
			if (flags & SP_OBJECT_WRITE_BUILD) {
				repr = sp_repr_duplicate (SP_OBJECT_REPR (this));
			}
			/* fixme: else probably error (Lauris) */
		} else {
			sp_repr_merge (repr, SP_OBJECT_REPR (this), "id");
		}
		return repr;
	}
}

/* Modification */

void
SPObject::requestDisplayUpdate(unsigned int flags)
{
	g_return_if_fail (!(flags & SP_OBJECT_PARENT_MODIFIED_FLAG));
	g_return_if_fail ((flags & SP_OBJECT_MODIFIED_FLAG) || (flags & SP_OBJECT_CHILD_MODIFIED_FLAG));
	g_return_if_fail (!((flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_CHILD_MODIFIED_FLAG)));

	/* Check for propagate before we set any flags */
	/* Propagate means, that this is not passed through by modification request cascade yet */
	unsigned int propagate = (!(this->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG)));

	/* Just set this flags safe even if some have been set before */
	this->uflags |= flags;

	if (propagate) {
		if (this->parent) {
			this->parent->requestDisplayUpdate(SP_OBJECT_CHILD_MODIFIED_FLAG);
		} else {
			sp_document_request_modified (this->document);
		}
	}
}

void
SPObject::updateDisplay(SPCtx *ctx, unsigned int flags)
{
	g_return_if_fail (!(flags & ~SP_OBJECT_MODIFIED_CASCADE));

#ifdef SP_OBJECT_DEBUG_CASCADE
	g_print("Update %s:%s %x %x %x\n", g_type_name_from_instance ((GTypeInstance *) this), SP_OBJECT_ID (this), flags, this->uflags, this->mflags);
#endif

	/* Get this flags */
	flags |= this->uflags;
	/* Copy flags to modified cascade for later processing */
	this->mflags |= this->uflags;
	/* We have to clear flags here to allow rescheduling update */
	this->uflags = 0;

	/* Merge style if we have good reasons to think that parent style is changed */
	/* I am not sure, whether we should check only propagated flag */
	/* We are currently assuming, that style parsing is done immediately */
	/* I think this is correct (Lauris) */
	if ((flags & SP_OBJECT_STYLE_MODIFIED_FLAG) && (flags & SP_OBJECT_PARENT_MODIFIED_FLAG)) {
		if (this->style && this->parent) {
			sp_style_merge_from_parent (this->style, this->parent->style);
		}
		/* attribute */
		/* TODO: this should be handled elsewhere */
		sp_object_read_attr (this, "xml:space");
	}

	if (((SPObjectClass *) G_OBJECT_GET_CLASS (this))->update)
		((SPObjectClass *) G_OBJECT_GET_CLASS (this))->update (this, ctx, flags);
}

void
SPObject::requestModified(unsigned int flags)
{
	/* PARENT_MODIFIED is computed later on and is not intended to be
	 * "manually" queued */
	g_return_if_fail (!(flags & SP_OBJECT_PARENT_MODIFIED_FLAG));

	/* we should be setting either MODIFIED or CHILD_MODIFIED... */
	g_return_if_fail ((flags & SP_OBJECT_MODIFIED_FLAG) || (flags & SP_OBJECT_CHILD_MODIFIED_FLAG));

	/* ...but not both */
	g_return_if_fail (!((flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_CHILD_MODIFIED_FLAG)));

	unsigned int old_mflags=this->mflags;
	this->mflags |= flags;

	/* If we already had MODIFIED or CHILD_MODIFIED queued, we will
	 * have already queued CHILD_MODIFIED with our ancestors and
	 * need not disturb them again.
	 */
	if ( old_mflags & ( SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG ) ) {
		SPObject *parent=SP_OBJECT_PARENT(this);
		if (parent) {
			parent->requestModified(SP_OBJECT_CHILD_MODIFIED_FLAG);
		} else {
			sp_document_request_modified(SP_OBJECT_DOCUMENT(this));
		}
	}
}

void
SPObject::emitModified(unsigned int flags)
{
	/* only the MODIFIED_CASCADE flag is legal here */
	g_return_if_fail (!(flags & ~SP_OBJECT_MODIFIED_CASCADE));

#ifdef SP_OBJECT_DEBUG_CASCADE
	g_print("Modified %s:%s %x %x %x\n", g_type_name_from_instance ((GTypeInstance *) this), SP_OBJECT_ID (this), flags, this->uflags, this->mflags);
#endif

	flags |= this->mflags;
	/* We have to clear mflags beforehand, as signal handlers may
	 * make changes and therefore queue new modification notifications
	 * themselves. */
	this->mflags = 0;

	g_object_ref(G_OBJECT (this));
	g_signal_emit(G_OBJECT (this), object_signals[MODIFIED], 0, flags);
	g_object_unref(G_OBJECT (this));
}

/*
 * Get and set descriptive parameters
 *
 * These are inefficent, so they are not intended to be used interactively
 */

const gchar *
sp_object_title_get (SPObject *object)
{
	return NULL;
}

const gchar *
sp_object_description_get (SPObject *object)
{
	return NULL;
}

unsigned int
sp_object_title_set (SPObject *object, const gchar *title)
{
	return FALSE;
}

unsigned int
sp_object_description_set (SPObject *object, const gchar *desc)
{
	return FALSE;
}

const gchar *
sp_object_tagName_get (const SPObject *object, SPException *ex)
{
	/* If exception is not clear, return */
	if (!SP_EXCEPTION_IS_OK (ex)) {
		return NULL;
	}

	/* fixme: Exception if object is NULL? */
	return sp_repr_name (object->repr);
}

const gchar *
sp_object_getAttribute (const SPObject *object, const gchar *key, SPException *ex)
{
	/* If exception is not clear, return */
	if (!SP_EXCEPTION_IS_OK (ex)) {
		return NULL;
	}

	/* fixme: Exception if object is NULL? */
	return (const gchar *) sp_repr_attr (object->repr, key);
}

void
sp_object_setAttribute (SPObject *object, const gchar *key, const gchar *value, SPException *ex)
{
	/* If exception is not clear, return */
	g_return_if_fail (SP_EXCEPTION_IS_OK (ex));

	/* fixme: Exception if object is NULL? */
	if (!sp_repr_set_attr (object->repr, key, value)) {
		ex->code = SP_NO_MODIFICATION_ALLOWED_ERR;
	}
}

void
sp_object_removeAttribute (SPObject *object, const gchar *key, SPException *ex)
{
	/* If exception is not clear, return */
	g_return_if_fail (SP_EXCEPTION_IS_OK (ex));

	/* fixme: Exception if object is NULL? */
	if (!sp_repr_set_attr (object->repr, key, NULL)) {
		ex->code = SP_NO_MODIFICATION_ALLOWED_ERR;
	}
}

/* Helper */

static gchar *
sp_object_get_unique_id (SPObject * object, const gchar * id)
{
	static gint count = 0;

	g_assert (SP_IS_OBJECT (object));
	g_assert (SP_IS_DOCUMENT (object->document));

	count++;

	const gchar *name = sp_repr_name (object->repr);
	g_assert (name != NULL);

	const gchar *local = strchr (name, ':');
	if (local) {
		name = local + 1;
	}

	gint len = strlen (name) + 17;
	gchar *b = (gchar*) alloca (len);
	g_assert (b != NULL);
	gchar *realid = NULL;

	if (id != NULL) {
		if (object->document->getObjectById(id) == NULL) {
			realid = g_strdup (id);
			g_assert (realid != NULL);
		}
	}

	while (realid == NULL) {
		g_snprintf (b, len, "%s%d", name, count);
		if (object->document->getObjectById(b) == NULL) {
			realid = g_strdup (b);
			g_assert (realid != NULL);
		} else {
			count++;
		}
	}

	return realid;
}

/* Style */

const gchar *
sp_object_get_style_property (SPObject *object, const gchar *key, const gchar *def)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (SP_IS_OBJECT (object), NULL);
	g_return_val_if_fail (key != NULL, NULL);

	const gchar *style = sp_repr_attr (object->repr, "style");
	if (style) {
		gint len = strlen (key);
		for (gchar *p = strstr (style, key); p; p = strstr (style, key)) {
			p += len;
			while ((*p <= ' ') && *p) p++;
			if (*p++ != ':') break;
			while ((*p <= ' ') && *p) p++;
			if (*p) return p;
		}
	}
	const gchar *val = sp_repr_attr (object->repr, key);
	if (val) {
		return val;
	}
	if (object->parent) {
		return sp_object_get_style_property (object->parent, key, def);
	}

	return def;
}

SPVersion
sp_object_get_sodipodi_version (SPObject *object)
{
	static const SPVersion zero_version = { 0, 0 };

	while (object) {
		if (SP_IS_ROOT (object)) {
			return SP_ROOT (object)->version.sodipodi;
		}
		object = SP_OBJECT_PARENT (object);
	}
	
	return zero_version;
}
