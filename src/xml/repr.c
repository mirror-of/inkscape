#define __SP_REPR_C__

/*
 * Fuzzy DOM-like tree implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noREPR_VERBOSE

#include <string.h>
#include <stddef.h>

#include <glib.h>

#include "repr-private.h"

typedef struct _SPReprListener SPListener;

static void repr_init (SPRepr *repr);
static void repr_doc_init (SPRepr *repr);

static void repr_copy (SPRepr *to, const SPRepr *from);
static void repr_doc_copy (SPRepr *to, const SPRepr *from);

static void repr_finalize (SPRepr *repr);
static void repr_doc_finalize (SPRepr *repr);

static void bind_document (SPReprDoc *doc, SPRepr *repr);

SPReprClass _sp_repr_xml_document_class = {
	sizeof (SPReprDoc),
	NULL,
	repr_doc_init,
	repr_doc_copy,
	repr_doc_finalize
};

SPReprClass _sp_repr_xml_element_class = {
	sizeof (SPRepr),
	NULL,
	repr_init,
	repr_copy,
	repr_finalize
};

SPReprClass _sp_repr_xml_text_class = {
	sizeof (SPRepr),
	NULL,
	repr_init,
	repr_copy,
	repr_finalize
};

static SPRepr *sp_repr_new_from_code (SPReprClass *type, int code);
static void sp_repr_remove_attribute (SPRepr *repr, SPReprAttr *attr);
static void sp_repr_remove_listener (SPRepr *repr, SPListener *listener);

static SPReprAttr *sp_attribute_duplicate (const SPReprAttr *attr);
static SPReprAttr *sp_attribute_new_from_code (int key, const unsigned char *value);

static SPRepr * sp_repr_alloc (SPReprClass *type);
static void sp_repr_free (SPRepr *repr);
static SPReprAttr * sp_attribute_alloc (void);
static void sp_attribute_free (SPReprAttr *attribute);
static SPListener *sp_listener_alloc (void);
static void sp_listener_free (SPListener *listener);

static SPRepr *
sp_repr_new_from_code (SPReprClass *type, int code)
{
	SPRepr * repr;

	repr = sp_repr_alloc (type);
	repr->name = code;
	repr->type->init (repr);

	return repr;
}

SPRepr *
sp_repr_new (const unsigned char *name)
{
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);

	return sp_repr_new_from_code (SP_XML_ELEMENT_NODE, g_quark_from_string (name));
}

SPRepr *
sp_repr_new_text (const unsigned char *content)
{
	SPRepr * repr;
	g_return_val_if_fail (content != NULL, NULL);
	repr = sp_repr_new_from_code (SP_XML_TEXT_NODE, g_quark_from_static_string ("text"));
	repr->content = g_strdup (content);
	repr->type = SP_XML_TEXT_NODE;
	return repr;
}

static void
repr_init (SPRepr *repr)
{
	repr->refcount = 1;
	repr->doc = NULL;
	repr->parent = repr->next = repr->children = NULL;
	repr->attributes = NULL;
	repr->listeners = NULL;
	repr->content = NULL;
}

static void
repr_doc_init (SPRepr *repr)
{
	SPReprDoc *doc=(SPReprDoc *)repr;

	repr_init (repr);

	repr->doc = doc;
	doc->log = NULL;
	doc->is_logging = 0;
}

SPRepr *
sp_repr_ref (SPRepr *repr)
{
	g_return_val_if_fail (repr != NULL, NULL);
	g_return_val_if_fail (repr->refcount > 0, NULL);

	repr->refcount += 1;

	return repr;
}

SPRepr *
sp_repr_unref (SPRepr *repr)
{
	g_return_val_if_fail (repr != NULL, NULL);
	g_return_val_if_fail (repr->refcount > 0, NULL);

	repr->refcount -= 1;

	if (repr->refcount < 1) {
		repr->type->finalize (repr);
		sp_repr_free (repr);
	}

	return NULL;
}

static void
repr_finalize (SPRepr *repr)
{
	SPReprListener *rl;
	/* Parents reference children */
	g_assert (repr->parent == NULL);
	g_assert (repr->next == NULL);
	repr->doc = NULL;

	for (rl = repr->listeners; rl; rl = rl->next) {
		if (rl->vector->destroy) (* rl->vector->destroy) (repr, rl->data);
	}
	while (repr->children) sp_repr_remove_child (repr, repr->children);
	while (repr->attributes) sp_repr_remove_attribute (repr, repr->attributes);
	if (repr->content) g_free (repr->content);
	while (repr->listeners) sp_repr_remove_listener (repr, repr->listeners);
}

static void
repr_doc_finalize (SPRepr *repr)
{
	SPReprDoc *doc = (SPReprDoc *)repr;
	if (doc->log) {
		sp_repr_free_log (doc->log);
		doc->log = NULL;
	}
	repr_finalize (repr);
}

static SPRepr *
sp_repr_attach (SPRepr *parent, SPRepr *child)
{
	g_assert (parent != NULL);
	g_assert (child != NULL);
	g_assert (child->parent == NULL);
	g_assert (child->doc == NULL && parent->doc == NULL);

	child->parent = parent;

	return child;
}

SPRepr *
sp_repr_duplicate (const SPRepr *repr)
{
	SPRepr *new;

	new = sp_repr_new_from_code (repr->type, repr->name);

	repr->type->copy (new, repr);

	return new;
}

void
repr_copy (SPRepr *to, const SPRepr *from)
{
	SPRepr *child, *lastchild;
	SPReprAttr *attr, *lastattr;

	g_return_if_fail (from != NULL);

	if (from->content != NULL) to->content = g_strdup (from->content);

	lastchild = NULL;
	for (child = from->children; child != NULL; child = child->next) {
		if (lastchild) {
			lastchild = lastchild->next = sp_repr_attach (to, sp_repr_duplicate (child));
		} else {
			lastchild = to->children = sp_repr_attach (to, sp_repr_duplicate (child));
		}
	}

	lastattr = NULL;
	for (attr = from->attributes; attr != NULL; attr = attr->next) {
		if (lastattr) {
			lastattr = lastattr->next = sp_attribute_duplicate (attr);
		} else {
			lastattr = to->attributes = sp_attribute_duplicate (attr);
		}
	}
}

void
repr_doc_copy (SPRepr *to, const SPRepr *from)
{
	SPReprDoc *to_doc=(SPReprDoc *)to;
	repr_copy (to, from);
	to_doc->log = NULL;
	to_doc->is_logging = 0;
}

const unsigned char *
sp_repr_name (const SPRepr *repr)
{
	g_return_val_if_fail (repr != NULL, NULL);

	return SP_REPR_NAME (repr);
}

const unsigned char *
sp_repr_content (const SPRepr *repr)
{
	g_assert (repr != NULL);

	return SP_REPR_CONTENT (repr);
}

const unsigned char *
sp_repr_attr (const SPRepr *repr, const unsigned char *key)
{
	SPReprAttr *ra;
	unsigned int q;

	g_return_val_if_fail (repr != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	q = g_quark_from_string (key);

	for (ra = repr->attributes; ra != NULL; ra = ra->next) if (ra->key == q) return ra->value;

	return NULL;
}

unsigned int
sp_repr_set_content (SPRepr *repr, const unsigned char *newcontent)
{
	SPReprListener *rl;
	unsigned char *oldcontent;
	unsigned int allowed;

	g_return_val_if_fail (repr != NULL, FALSE);

	oldcontent = SP_REPR_CONTENT (repr);

	allowed = TRUE;
	for (rl = repr->listeners; rl && allowed; rl = rl->next) {
		if (rl->vector->change_content) allowed = (* rl->vector->change_content) (repr, oldcontent, newcontent, rl->data);
	}

	if (allowed) {
		if (newcontent) {
			newcontent = repr->content = g_strdup (newcontent);
		} else {
			repr->content = NULL;
		}
		if ( repr->doc && repr->doc->is_logging ) {
			repr->doc->log = sp_repr_log_chgcontent (repr->doc->log, repr, oldcontent, newcontent);
		}
		for (rl = repr->listeners; rl != NULL; rl = rl->next) {
			if (rl->vector->content_changed) (* rl->vector->content_changed) (repr, oldcontent, newcontent, rl->data);
		}
		if ( oldcontent && ( !repr->doc || !repr->doc->is_logging ) ) {
			g_free (oldcontent);
		}
	}

	return allowed;
}

static unsigned int
sp_repr_del_attr (SPRepr *repr, const unsigned char *key)
{
	SPReprAttr *prev, *attr;
	SPReprListener *rl;
	unsigned int allowed;
	unsigned int q;

	g_return_val_if_fail (repr != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	allowed = TRUE;

	q = g_quark_from_string (key);
	prev = NULL;
	for (attr = repr->attributes; attr && (attr->key != q); attr = attr->next) prev = attr;

	if (attr) {
		unsigned char *oldval;

		oldval = attr->value;

		for (rl = repr->listeners; rl && allowed; rl = rl->next) {
			if (rl->vector->change_attr) allowed = (* rl->vector->change_attr) (repr, key, oldval, NULL, rl->data);
		}

		if (allowed) {
			if (prev) {
				prev->next = attr->next;
			} else {
				repr->attributes = attr->next;
			}

			if ( repr->doc && repr->doc->is_logging ) {
				repr->doc->log = sp_repr_log_chgattr (repr->doc->log, repr, q, oldval, NULL);
			}
			for (rl = repr->listeners; rl != NULL; rl = rl->next) {
				if (rl->vector->attr_changed) (* rl->vector->attr_changed) (repr, key, oldval, NULL, rl->data);
			}
			if ( repr->doc && repr->doc->is_logging ) {
				attr->value = NULL;
			}

			sp_attribute_free (attr);
		}
	}

	return allowed;
}

static unsigned int
sp_repr_chg_attr (SPRepr *repr, const unsigned char *key, const unsigned char *value)
{
	SPReprAttr *prev, *attr;
	SPReprListener *rl;
	unsigned int allowed;
	char *oldval;
	unsigned int q;

	g_return_val_if_fail (repr != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	oldval = NULL;
	q = g_quark_from_string (key);
	prev = NULL;
	for (attr = repr->attributes; attr && (attr->key != q); attr = attr->next) prev = attr;
	if (attr) {
		if (!strcmp (attr->value, value)) return TRUE;
		oldval = attr->value;
	}

	allowed = TRUE;
	for (rl = repr->listeners; rl && allowed; rl = rl->next) {
		if (rl->vector->change_attr) allowed = (* rl->vector->change_attr) (repr, key, oldval, value, rl->data);
	}

	if (allowed) {
		if (attr) {
			attr->value = g_strdup (value);
		} else {
			attr = sp_attribute_new_from_code (q, value);
			if (prev) {
				prev->next = attr;
			} else {
				repr->attributes = attr;
			}
		}

		if ( repr->doc && repr->doc->is_logging ) {
			repr->doc->log = sp_repr_log_chgattr (repr->doc->log, repr, q, oldval, value);
		}

		for (rl = repr->listeners; rl != NULL; rl = rl->next) {
			if (rl->vector->attr_changed) (* rl->vector->attr_changed) (repr, key, oldval, value, rl->data);
		}

		if ( oldval && ( !repr->doc || !repr->doc->is_logging ) ) {
			g_free (oldval);
		}
	}

	return allowed;
}

unsigned int
sp_repr_set_attr (SPRepr *repr, const unsigned char *key, const unsigned char *value)
{
	g_return_val_if_fail (repr != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	if (!value) return sp_repr_del_attr (repr, key);

	return sp_repr_chg_attr (repr, key, value);
}


static void
sp_repr_remove_attribute (SPRepr *repr, SPReprAttr *attr)
{
	g_assert (repr != NULL);
	g_assert (attr != NULL);

	if (attr == repr->attributes) {
		repr->attributes = attr->next;
	} else {
		SPReprAttr * prev;
		prev = repr->attributes;
		while (prev->next != attr) {
			prev = prev->next;
			g_assert (prev != NULL);
		}
		prev->next = attr->next;
	}

	g_free (attr->value);
	sp_attribute_free (attr);
}

SPRepr *
sp_repr_parent (SPRepr * repr)
{
	g_assert (repr != NULL);

	return repr->parent;
}

unsigned int
sp_repr_add_child (SPRepr *repr, SPRepr *child, SPRepr *ref)
{
	SPReprListener * rl;
	unsigned int allowed;

	g_assert (repr != NULL);
	g_assert (child != NULL);
	g_assert (!ref || ref->parent == repr);
	g_assert (child->parent == NULL);
	g_assert (child->doc == NULL || child->doc == repr->doc);

	allowed = TRUE;
	for (rl = repr->listeners; rl != NULL; rl = rl->next) {
		if (rl->vector->add_child) allowed = (* rl->vector->add_child) (repr, child, ref, rl->data);
	}

	if (allowed) {
		if (ref != NULL) {
			child->next = ref->next;
			ref->next = child;
		} else {
			child->next = repr->children;
			repr->children = child;
		}

		child->parent = repr;
		sp_repr_ref (child);

		if (child->doc == NULL) bind_document (repr->doc, child);

		if ( repr->doc && repr->doc->is_logging ) {
			repr->doc->log = sp_repr_log_add (repr->doc->log, repr, child, ref);
		}

		for (rl = repr->listeners; rl != NULL; rl = rl->next) {
			if (rl->vector->child_added) (* rl->vector->child_added) (repr, child, ref, rl->data);
		}
	}

	return allowed;
}

static void
bind_document (SPReprDoc *doc, SPRepr *repr)
{
	SPRepr *child;

	g_assert (repr->doc == NULL);

	repr->doc = doc;

	for ( child = repr->children ; child != NULL ; child = child->next ) {
		bind_document (doc, child);
	}
}

unsigned int
sp_repr_remove_child (SPRepr *repr, SPRepr *child)
{
	SPReprListener *rl;
	SPRepr *ref;
	unsigned int allowed;

	g_assert (repr != NULL);
	g_assert (child != NULL);
	g_assert (child->parent == repr);

	ref = NULL;
	if (child != repr->children) {
		ref = repr->children;
		while (ref->next != child) ref = ref->next;
	}

	allowed = TRUE;
	for (rl = repr->listeners; rl != NULL; rl = rl->next) {
		if (rl->vector->remove_child) allowed = (* rl->vector->remove_child) (repr, child, ref, rl->data);
	}

	if (allowed) {
		if (ref) {
			ref->next = child->next;
		} else {
			repr->children = child->next;
		}
		child->parent = NULL;
		child->next = NULL;

		if ( repr->doc && repr->doc->is_logging ) {
			repr->doc->log = sp_repr_log_remove (repr->doc->log, repr, child, ref);
		}

		for (rl = repr->listeners; rl != NULL; rl = rl->next) {
			if (rl->vector->child_removed) (* rl->vector->child_removed) (repr, child, ref, rl->data);
		}

#ifdef REPR_VERBOSE
		if (child->refcount > 1) {
			g_warning ("Detaching repr with refcount > 1");
		}
#endif
		sp_repr_unref (child);
	}

	return allowed;
}

unsigned int
sp_repr_change_order (SPRepr *repr, SPRepr *child, SPRepr *ref)
{
	SPRepr * prev;
	SPReprListener * rl;
	int allowed;

	prev = NULL;
	if (child != repr->children) {
		SPRepr *cur;
		for (cur = repr->children; cur != child; cur = cur->next) prev = cur;
	}

	if (prev == ref) return TRUE;

	allowed = TRUE;
	for (rl = repr->listeners; rl && allowed; rl = rl->next) {
		if (rl->vector->change_order) allowed = (* rl->vector->change_order) (repr, child, prev, ref, rl->data);
	}

	if (allowed) {
		if (prev) {
			prev->next = child->next;
		} else {
			repr->children = child->next;
		}
		if (ref) {
			child->next = ref->next;
			ref->next = child;
		} else {
			child->next = repr->children;
			repr->children = child;
		}

		if ( repr->doc && repr->doc->is_logging ) {
			repr->doc->log = sp_repr_log_chgorder (repr->doc->log, repr, child, prev, ref);
		}

		for (rl = repr->listeners; rl != NULL; rl = rl->next) {
			if (rl->vector->order_changed) (* rl->vector->order_changed) (repr, child, prev, ref, rl->data);
		}
	}

	return allowed;
}

void
sp_repr_set_position_absolute (SPRepr * repr, int pos)
{
	SPRepr * parent;
	SPRepr * ref, * cur;

	parent = repr->parent;

	if (pos < 0) pos = 0x7fffffff;

	ref = NULL;
	cur = parent->children;
	while (pos > 0 && cur) {
		ref = cur;
		cur = cur->next;
		pos -= 1;
	}

	if (ref == repr) {
		ref = repr->next;
		if (!ref) return;
	}

	sp_repr_change_order (parent, repr, ref);
}

void
sp_repr_synthesize_events (SPRepr *repr, const SPReprEventVector *vector, void * data)
{
	
	if (vector->attr_changed) {
		SPReprAttr *attr;
		attr = repr->attributes;
		for ( ; attr ; attr = attr->next ) {
			vector->attr_changed (repr, g_quark_to_string (attr->key), NULL, attr->value, data);
		}
	}
	if (vector->child_added) {
		SPRepr *child, *ref;
		ref = NULL; child = repr->children;
		for ( ; child ; ref = child, child = child->next ) {
			vector->child_added (repr, child, ref, data);
		}
	}
	if (vector->content_changed) {
		vector->content_changed (repr, NULL, repr->content, data);
	}
}

void
sp_repr_add_listener (SPRepr *repr, const SPReprEventVector *vector, void * data)
{
	SPReprListener *rl, *last;

	g_assert (repr != NULL);
	g_assert (vector != NULL);

	last = NULL;
	if (repr->listeners) {
		last = repr->listeners;
		while (last->next) last = last->next;
	}

	rl = sp_listener_alloc ();
	rl->next = NULL;
	rl->vector = vector;
	rl->data = data;

	if (last) {
		last->next = rl;
	} else {
		repr->listeners = rl;
	}
}

static void
sp_repr_remove_listener (SPRepr *repr, SPListener *listener)
{
	if (listener == repr->listeners) {
		repr->listeners = listener->next;
	} else {
		SPListener *prev;
		prev = repr->listeners;
		while (prev->next != listener) prev = prev->next;
		prev->next = listener->next;
	}

	sp_listener_free (listener);
}

void
sp_repr_remove_listener_by_data (SPRepr *repr, void *data)
{
	SPReprListener * last, * rl;

	last = NULL;
	for (rl = repr->listeners; rl != NULL; rl = rl->next) {
		if (rl->data == data) {
			if (last) {
				last->next = rl->next;
			} else {
				repr->listeners = rl->next;
			}
			sp_listener_free (rl);
			return;
		}
		last = rl;
	}
}

SPRepr *
sp_repr_nth_child (const SPRepr * repr, int n)
{
	SPRepr * child;

	child = repr->children;

	while (n > 0 && child) {
		child = child->next;
		n -= 1;
	}

	return child;
}

/* Documents - 1st step in migrating to real XML */
/* fixme: Do this somewhere, somehow The Right Way (TM) */

SPReprDoc *
sp_repr_document_new (const char *rootname)
{
	SPReprDoc * doc;
	SPRepr * root;

	doc = (SPReprDoc *)sp_repr_new_from_code (SP_XML_DOCUMENT_NODE, g_quark_from_static_string ("xml"));
	if (!strcmp (rootname, "svg")) {
		sp_repr_set_attr (&doc->repr, "version", "1.0");
		sp_repr_set_attr (&doc->repr, "standalone", "no");
		sp_repr_set_attr (&doc->repr, "doctype",
				  "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n"
				  "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");
		sp_repr_set_attr (&doc->repr, "comment",
				  "<!-- Created with Sodipodi (\"http://www.sodipodi.com/\") -->\n");
	}

	root = sp_repr_new (rootname);

	sp_repr_add_child (&doc->repr, root, 0);
	sp_repr_unref (root);

	return doc;
}

void
sp_repr_document_ref (SPReprDoc * doc)
{
	sp_repr_ref ((SPRepr *) doc);
}

void
sp_repr_document_unref (SPReprDoc * doc)
{
	sp_repr_unref ((SPRepr *) doc);
}

void
sp_repr_document_set_root (SPReprDoc *doc, SPRepr *repr)
{
	g_assert (doc != NULL);
	g_assert (repr != NULL);
	g_assert (doc->repr.children != NULL);
	g_assert (repr->doc == NULL || repr->doc == doc);

	sp_repr_remove_child (&doc->repr, doc->repr.children);
	sp_repr_add_child (&doc->repr, repr, NULL);
}

SPReprDoc *
sp_repr_document (const SPRepr *repr)
{
	return repr->doc;
}

SPRepr *
sp_repr_document_root (const SPReprDoc *doc)
{
	g_assert (doc != NULL);
	return doc->repr.children;
}

/*
 * Duplicate all attributes and children from src into doc
 * Does NOT erase original attributes and children
 */

unsigned int
sp_repr_document_merge (SPReprDoc *doc, const SPReprDoc *src, const unsigned char *key)
{
	SPRepr *rdoc;
	const SPRepr *rsrc;

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	rdoc = sp_repr_document_root (doc);
	rsrc = sp_repr_document_root (src);
	
	return sp_repr_merge (rdoc, rsrc, key);
}

/*
 * Duplicate all attributes and children from src into doc
 * Does NOT erase original attributes and children
 */

unsigned int
sp_repr_merge (SPRepr *repr, const SPRepr *src, const unsigned char *key)
{
	SPRepr *child;
	SPReprAttr *attr;
	
	g_return_val_if_fail (repr != NULL, FALSE);
	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (src->content) {
		if (repr->content) g_free (repr->content);
		repr->content = g_strdup (src->content);
	} else {
		if (repr->content) g_free (repr->content);
		repr->content = NULL;
	}
	
	for (child = src->children; child != NULL; child = child->next) {
		SPRepr *rch;
		const unsigned char *id;
		id = sp_repr_attr (child, key);
		if (id) {
			rch = sp_repr_lookup_child (repr, key, id);
			if (rch) {
				sp_repr_merge (rch, child, key);
			} else {
				rch = sp_repr_duplicate (child);
				sp_repr_append_child (repr, rch);
			}
		} else {
			rch = sp_repr_duplicate (child);
			sp_repr_append_child (repr, rch);
		}
	}
	
	for (attr = src->attributes; attr != NULL; attr = attr->next) {
		sp_repr_set_attr (repr, SP_REPR_ATTRIBUTE_KEY (attr), SP_REPR_ATTRIBUTE_VALUE (attr));
	}

	return TRUE;
}

static SPReprAttr *
sp_attribute_duplicate (const SPReprAttr *attr)
{
	SPReprAttr *new;

	new = sp_attribute_alloc ();

	new->next = NULL;
	new->key = attr->key;
	new->value = g_strdup (attr->value);

	return new;
}

static SPReprAttr *
sp_attribute_new_from_code (int key, const unsigned char *value)
{
	SPReprAttr *new;

	new = sp_attribute_alloc ();

	new->next = NULL;
	new->key = key;
	new->value = g_strdup (value);

	return new;
}

#define SP_REPR_CHUNK_SIZE 32

static SPRepr *
sp_repr_alloc (SPReprClass *type)
{
	SPRepr *repr;

	if (type->pool) {
		repr = type->pool;
		type->pool = repr->next;
	} else {
		char *chunk;
		size_t max, offset;

		chunk = (char *)g_malloc (type->size * SP_REPR_CHUNK_SIZE);

		max = type->size * ( SP_REPR_CHUNK_SIZE - 1 );
		for ( offset = type->size ;
		      offset < max ;
		      offset += type->size )
		{
			repr = (SPRepr *)(chunk+offset);
			repr->next = (SPRepr *)(chunk+offset+type->size);
		}

		((SPRepr *)(chunk+offset))->next = NULL;
		type->pool = (SPRepr *)(chunk+type->size);

		repr = (SPRepr *)chunk;
	}
	repr->type = type;

	return repr;
}

static void
sp_repr_free (SPRepr *repr)
{
	repr->next = repr->type->pool;
	repr->type->pool = repr;
}

#define SP_ATTRIBUTE_ALLOC_SIZE 32
static SPReprAttr *free_attribute = NULL;

static SPReprAttr *
sp_attribute_alloc (void)
{
	SPReprAttr *attribute;
	int i;

	if (free_attribute) {
		attribute = free_attribute;
		free_attribute = free_attribute->next;
	} else {
		attribute = g_new (SPReprAttr, SP_ATTRIBUTE_ALLOC_SIZE);
		for (i = 1; i < SP_ATTRIBUTE_ALLOC_SIZE - 1; i++) attribute[i].next = attribute + i + 1;
		attribute[SP_ATTRIBUTE_ALLOC_SIZE - 1].next = NULL;
		free_attribute = attribute + 1;
	}

	return attribute;
}

static void
sp_attribute_free (SPReprAttr *attribute)
{
	attribute->next = free_attribute;
	free_attribute = attribute;
}

#define SP_LISTENER_ALLOC_SIZE 32
static SPListener *free_listener = NULL;

static SPListener *
sp_listener_alloc (void)
{
	SPListener *listener;
	int i;

	if (free_listener) {
		listener = free_listener;
		free_listener = free_listener->next;
	} else {
		listener = g_new (SPListener, SP_LISTENER_ALLOC_SIZE);
		for (i = 1; i < SP_LISTENER_ALLOC_SIZE - 1; i++) listener[i].next = listener + i + 1;
		listener[SP_LISTENER_ALLOC_SIZE - 1].next = NULL;
		free_listener = listener + 1;
	}

	return listener;
}

static void
sp_listener_free (SPListener *listener)
{
	listener->next = free_listener;
	free_listener = listener;
}

