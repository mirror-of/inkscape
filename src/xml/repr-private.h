#ifndef __SP_REPR_PRIVATE_H__
#define __SP_REPR_PRIVATE_H__

/*
 * Fuzzy DOM-like tree implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gslist.h>

#include "repr.h"
#include "repr-action.h"
#include "xml/xml-forward.h"

struct SPReprClass;
struct SPReprAttr;
struct SPReprListener;
struct SPReprEventVector;

struct SPReprClass {
	size_t size;
	SPRepr *pool;
	void (*init)(SPRepr *repr);
	void (*copy)(SPRepr *to, const SPRepr *from);
	void (*finalize)(SPRepr *repr);
};

struct SPReprListener {
	SPReprListener *next;
	const SPReprEventVector *vector;
	void * data;
};

struct SPReprEventVector {
	/* Immediate signals */
	void (* destroy) (SPRepr *repr, void * data);
	unsigned int (* add_child) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	void (* child_added) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	unsigned int (* remove_child) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	void (* child_removed) (SPRepr *repr, SPRepr *child, SPRepr *ref, void * data);
	unsigned int (* change_attr) (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, void * data);
	void (* attr_changed) (SPRepr *repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);
	unsigned int (* change_content) (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);
	void (* content_changed) (SPRepr *repr, const gchar *oldcontent, const gchar *newcontent, void * data);
	unsigned int (* change_order) (SPRepr *repr, SPRepr *child, SPRepr *oldref, SPRepr *newref, void * data);
	void (* order_changed) (SPRepr *repr, SPRepr *child, SPRepr *oldref, SPRepr *newref, void * data);
};

struct SPRepr {
	SPReprClass *type;
	int refcount;

	int name;

	SPReprDoc *doc;
	SPRepr *parent;
	SPRepr *next;
	SPRepr *children;

	SPReprAttr *attributes;
	SPReprListener *listeners;
	SPReprListener *last_listener;
	gchar *content;
};

struct SPReprDoc {
	SPRepr repr;
	bool is_logging;
	SPReprAction *log;
};

struct SPXMLNs {
	SPXMLNs *next;
	unsigned int uri, prefix;
};

#define SP_REPR_NAME(r) g_quark_to_string ((r)->name)
#define SP_REPR_TYPE(r) ((r)->type)
#define SP_REPR_CONTENT(r) ((r)->content)

extern SPReprClass _sp_repr_xml_document_class;
extern SPReprClass _sp_repr_xml_element_class;
extern SPReprClass _sp_repr_xml_text_class;
extern SPReprClass _sp_repr_xml_comment_class;

#define SP_XML_DOCUMENT_NODE &_sp_repr_xml_document_class
#define SP_XML_ELEMENT_NODE &_sp_repr_xml_element_class
#define SP_XML_TEXT_NODE &_sp_repr_xml_text_class
#define SP_XML_COMMENT_NODE &_sp_repr_xml_comment_class

unsigned int sp_repr_change_order (SPRepr *repr, SPRepr *child, SPRepr *ref);

void sp_repr_synthesize_events (SPRepr *repr, const SPReprEventVector *vector, void * data);

void sp_repr_add_listener (SPRepr *repr, const SPReprEventVector *vector, void * data);
void sp_repr_remove_listener_by_data (SPRepr *repr, void * data);

SPReprDoc *sp_repr_document_new_list (GSList *reprs);
SPRepr *sp_repr_document_first_child(SPReprDoc const *doc);

#endif
