#ifndef __SP_REPR_PRIVATE_H__
#define __SP_REPR_PRIVATE_H__

/** \file
 * Fuzzy DOM-like tree implementation
 */

/*
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Frank Felfe
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gslist.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "repr.h"
#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include "xml/xml-forward.h"
#include "xml/node-observer.h"
#include "xml/session.h"
#include "xml/transaction-logger.h"
#include "util/shared-c-string-ptr.h"

struct SPReprClass;
struct SPReprAttr;
struct SPReprListener;
struct SPReprEventVector;

class SPReprAction;

enum SPReprType {
	SP_XML_DOCUMENT_NODE,
	SP_XML_ELEMENT_NODE,
	SP_XML_TEXT_NODE,
	SP_XML_COMMENT_NODE
};

class SPRepr : public Inkscape::GC::Managed<>, public Inkscape::GC::Anchored {
public:
	SPReprType type() const { return _type; }

	Inkscape::XML::Session *session() {
		return ( _logger ? &_logger->session() : NULL );
	}

	gchar const *name() const;
	int code() const { return _name; }
	void setCodeUnsafe(int code) {
		g_assert(!_logger && !_listeners);
		_name = code;
	}

	SPReprDoc *document() { return _document; }
	SPReprDoc const *document() const {
		return const_cast<SPRepr *>(this)->document();
	}

	SPRepr *duplicate() const { return _duplicate(); }

	SPRepr *root();
	SPRepr const *root() const {
		return const_cast<SPRepr *>(this)->root();
	}

	SPRepr *parent() { return _parent; }
	SPRepr const *parent() const { return _parent; }

	SPRepr *next() { return _next; }
	SPRepr const *next() const { return _next; }

	SPRepr *firstChild() { return _children; }
	SPRepr const *firstChild() const { return _children; }
	SPRepr *lastChild();
	SPRepr const *lastChild() const {
		return const_cast<SPRepr *>(this)->lastChild();
	}

	unsigned childCount() const { return _child_count; }
	SPRepr *nthChild(unsigned index);
	SPRepr const *nthChild(unsigned index) const {
		return const_cast<SPRepr *>(this)->nthChild(index);
	}

	void addChild(SPRepr *child, SPRepr *ref);
	void appendChild(SPRepr *child) {
		addChild(child, lastChild());
	}
	void removeChild(SPRepr *child);
	void changeOrder(SPRepr *child, SPRepr *ref);

	unsigned position() const;
	void setPosition(int pos);

	gchar const *attribute(gchar const *key) const;
	void setAttribute(gchar const *key, gchar const *value, bool is_interactive=false);
	bool matchAttributeName(gchar const *partial_name) const;

	gchar const *content() const;
	void setContent(gchar const *value);

	void mergeFrom(SPRepr const *src, gchar const *key);

	SPReprAttr const *attributeList() const { return _attributes; }

	void synthesizeEvents(SPReprEventVector const *vector, void *data);
	void addListener(SPReprEventVector const *vector, void *data);
	void removeListenerByData(void *data);

protected:
	SPRepr(SPReprType t, int code);
	SPRepr(SPRepr const &repr);

	virtual SPRepr *_duplicate() const=0;

	void _setParent(SPRepr *parent) { _parent = parent; }
	void _setNext(SPRepr *next) { _next = next; }
	void _bindDocument(SPReprDoc &document);
	void _bindLogger(Inkscape::XML::TransactionLogger &logger);

	unsigned _childPosition(SPRepr const &child) const;
	unsigned _cachedPosition() const { return _cached_position; }
	void _setCachedPosition(unsigned position) const {
		_cached_position = position;
	}

	void _changeAttribute(gchar const *key, gchar const *value, bool is_interactive);
	void _deleteAttribute(gchar const *key, bool is_interactive);

private:
	void operator=(SPRepr const &); // no assign

	int _name;

	SPReprDoc *_document;

	Inkscape::XML::TransactionLogger *_logger;

	SPReprType _type;
	SPReprAttr *_attributes;
	Inkscape::Util::SharedCStringPtr _content;
	SPRepr *_parent;
	SPRepr *_children;
	SPRepr *_next;

	unsigned _child_count;
	mutable bool _cached_positions_valid;
	mutable unsigned _cached_position;

	SPReprListener *_listeners;
	SPReprListener *_last_listener;
};

struct SPReprElement : public SPRepr {
	explicit SPReprElement(int code) : SPRepr(SP_XML_ELEMENT_NODE, code) {}

protected:
	SPRepr *_duplicate() const { return new SPReprElement(*this); }
};

struct SPReprText : public SPRepr {
	SPReprText(Inkscape::Util::SharedCStringPtr content)
	: SPRepr(SP_XML_TEXT_NODE, g_quark_from_static_string("string"))
	{
		setContent(content);
	}

protected:
	SPRepr *_duplicate() const { return new SPReprText(*this); }
};

struct SPReprComment : public SPRepr {
	explicit SPReprComment(Inkscape::Util::SharedCStringPtr content)
	: SPRepr(SP_XML_COMMENT_NODE, g_quark_from_static_string("comment"))
	{
		setContent(content);
	}

protected:
	SPRepr *_duplicate() const { return new SPReprComment(*this); }
};

struct SPReprDoc : public SPRepr {
	explicit SPReprDoc(int code);

protected:
	SPReprDoc(SPReprDoc const &doc);

	SPRepr *_duplicate() const { return new SPReprDoc(*this); }
};

struct SPXMLNs {
	SPXMLNs *next;
	unsigned int uri, prefix;
};

unsigned int sp_repr_change_order (SPRepr *repr, SPRepr *child, SPRepr *ref);

SPReprDoc *sp_repr_document_new_list (GSList *reprs);
SPRepr *sp_repr_document_first_child(SPReprDoc const *doc);

#endif
