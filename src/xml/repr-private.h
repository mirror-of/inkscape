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
#include "refcounted.h"
#include "xml/xml-forward.h"

struct SPReprClass;
struct SPReprAttr;
struct SPReprListener;
struct SPReprEventVector;

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

enum SPReprType {
	SP_XML_DOCUMENT_NODE,
	SP_XML_ELEMENT_NODE,
	SP_XML_TEXT_NODE,
	SP_XML_COMMENT_NODE
};

struct SPRepr : public Inkscape::Refcounted {
	SPReprType type;

	~SPRepr();

	int name;

	SPReprDoc *doc;
	SPRepr *parent;
	SPRepr *next;
	SPRepr *children;

	SPReprAttr *attributes;
	SPReprListener *listeners;
	SPReprListener *last_listener;
	gchar *content;

	SPRepr *duplicate() const { return _duplicate(); }

protected:
	SPRepr(SPReprType t, int code);
	SPRepr(SPRepr const &repr);

	virtual SPRepr *_duplicate() const=0;

private:
	void operator=(SPRepr const &); // no assign
};

struct SPReprElement : public SPRepr {
	explicit SPReprElement(int code) : SPRepr(SP_XML_ELEMENT_NODE, code) {}

protected:
	SPRepr *_duplicate() const { return new SPReprElement(*this); }
};

struct SPReprText : public SPRepr {
	explicit SPReprText(int code) : SPRepr(SP_XML_TEXT_NODE, code) {}

protected:
	SPRepr *_duplicate() const { return new SPReprText(*this); }
};

struct SPReprComment : public SPRepr {
	explicit SPReprComment(int code) : SPRepr(SP_XML_COMMENT_NODE, code) {}

protected:
	SPRepr *_duplicate() const { return new SPReprComment(*this); }
};

struct SPReprDoc : public SPRepr {
	explicit SPReprDoc(int code);
	~SPReprDoc();

	bool is_logging;
	SPReprAction *log;

protected:
	SPReprDoc(SPReprDoc const &doc);

	SPRepr *_duplicate() const { return new SPReprDoc(*this); }
};

struct SPXMLNs {
	SPXMLNs *next;
	unsigned int uri, prefix;
};

#define SP_REPR_NAME(r) g_quark_to_string ((r)->name)
#define SP_REPR_TYPE(r) ((r)->type)
#define SP_REPR_CONTENT(r) ((r)->content)

unsigned int sp_repr_change_order (SPRepr *repr, SPRepr *child, SPRepr *ref);

void sp_repr_synthesize_events (SPRepr *repr, const SPReprEventVector *vector, void * data);

void sp_repr_add_listener (SPRepr *repr, const SPReprEventVector *vector, void * data);
void sp_repr_remove_listener_by_data (SPRepr *repr, void * data);

SPReprDoc *sp_repr_document_new_list (GSList *reprs);
SPRepr *sp_repr_document_first_child(SPReprDoc const *doc);

#endif
