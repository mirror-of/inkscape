#ifndef __SP_REPR_PRIVATE_H__
#define __SP_REPR_PRIVATE_H__

/** \file
 * Fuzzy DOM-like tree implementation
 */

/*
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
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "repr.h"
#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include "xml/xml-forward.h"
#include "util/shared-c-string.h"

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

struct SPRepr : public Inkscape::GC::Managed<>, public Inkscape::GC::Anchored {
	SPReprType type;

	int name;

	SPReprDoc *doc;
	SPRepr *parent;
	SPRepr *next;
	SPRepr *children;

	SPReprAttr *attributes;
	SPReprListener *listeners;
	SPReprListener *last_listener;
	Inkscape::Util::SharedCString content;

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

struct SPReprDoc : public SPRepr, public Inkscape::GC::Finalized {
	explicit SPReprDoc(int code);
	~SPReprDoc();

	typedef sigc::signal<void, SPRepr *, SPRepr *, SPRepr *, SPRepr *, SPRepr *> NodeMovedSignal;
	typedef sigc::signal<void, SPRepr *, gchar const *, gchar const *, gchar const *> AttrChangedSignal;
	typedef sigc::signal<void, SPRepr *, gchar const *, gchar const *> ContentChangedSignal;

	sigc::connection connectNodeMoved(NodeMovedSignal::slot_type slot) {
		return _node_moved_signal.connect(slot);
	}
	sigc::connection connectAttrChanged(AttrChangedSignal::slot_type slot) {
		return _attr_changed_signal.connect(slot);
	}
	sigc::connection connectContentChanged(ContentChangedSignal::slot_type slot) {
		return _content_changed_signal.connect(slot);
	}

	void _emitNodeMoved(SPRepr *node, SPRepr *old_parent, SPRepr *old_ref, SPRepr *new_parent, SPRepr *new_ref) {
		_node_moved_signal.emit(node, old_parent, old_ref, new_parent, new_ref);
	}
	void _emitAttrChanged(SPRepr *node, gchar const *name, gchar const *old_value, gchar const *new_value) {
		_attr_changed_signal.emit(node, name, old_value, new_value);
	}
	void _emitContentChanged(SPRepr *node, gchar const *old_content, gchar const *new_content) {
		_content_changed_signal.emit(node, old_content, new_content);
	}

	bool is_logging;
	SPReprAction *log;

protected:
	NodeMovedSignal _node_moved_signal;
	AttrChangedSignal _attr_changed_signal;
	ContentChangedSignal _content_changed_signal;

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

SPReprDoc *sp_repr_document_new_list (GSList *reprs);
SPRepr *sp_repr_document_first_child(SPReprDoc const *doc);

#endif
