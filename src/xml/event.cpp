/*
 * Repr transaction logging
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY  <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * g++ port Copyright (C) 2003 Nathan Hurst
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "repr.h"
#include "event.h"
#include "event-fns.h"
#include "util/list.h"
#include "util/reverse-list.h"
#include "xml/node.h"
#include "xml/document.h"
#include "xml/session.h"
#include <cstring>

using Inkscape::Util::List;
using Inkscape::Util::reverse_list;

int Inkscape::XML::Event::_next_serial=0;

using Inkscape::XML::Session;

void
sp_repr_begin_transaction (Inkscape::XML::Document *doc)
{
	g_assert(doc != NULL);
	Session *session=doc->session();
	g_assert(session != NULL);
	session->beginTransaction();
}

void
sp_repr_rollback (Inkscape::XML::Document *doc)
{
	g_assert(doc != NULL);
	Session *session=doc->session();
	g_assert(session != NULL);
	session->rollback();
}

void
sp_repr_commit (Inkscape::XML::Document *doc)
{
	g_assert(doc != NULL);
	Session *session=doc->session();
	g_assert(session != NULL);
	session->commit();
}

Inkscape::XML::Event *
sp_repr_commit_undoable (Inkscape::XML::Document *doc)
{
	g_assert(doc != NULL);
	Session *session=doc->session();
	g_assert(session != NULL);
	return session->commitUndoable();
}

void
sp_repr_undo_log (Inkscape::XML::Event *log)
{
	Inkscape::XML::Event *action;

	if (log) {
		g_assert(!log->repr->session()->inTransaction());
	}

	for ( action = log ; action ; action = action->next ) {
		action->undoOne();
	}
}

void
sp_repr_debug_print_log(Inkscape::XML::Event const *log) {
	List<Inkscape::XML::Event const &> reversed(
		reverse_list<Inkscape::XML::Event::ConstIterator>(log, NULL)
	);
	for ( ; reversed ; ++reversed ) {
		g_warning("Event %d: %s", reversed->serial, reversed->describe().c_str());
	}
}

void Inkscape::XML::EventAdd::_undoOne() const {
	sp_repr_remove_child(this->repr, this->child);
}

void Inkscape::XML::EventDel::_undoOne() const {
	sp_repr_add_child(this->repr, this->child, this->ref);
}

void Inkscape::XML::EventChgAttr::_undoOne() const {
	sp_repr_set_attr(this->repr, g_quark_to_string(this->key), this->oldval);
}

void Inkscape::XML::EventChgContent::_undoOne() const {
	this->repr->setContent(this->oldval);
}

void Inkscape::XML::EventChgOrder::_undoOne() const {
	sp_repr_change_order(this->repr, this->child, this->oldref);
}

void
sp_repr_replay_log (Inkscape::XML::Event *log)
{
	if (log) {
		g_assert(!log->repr->session()->inTransaction());
	}

	List<Inkscape::XML::Event &> reversed(
		reverse_list<Inkscape::XML::Event::Iterator>(log, NULL)
	);
	for ( ; reversed ; ++reversed ) {
		reversed->replayOne();
	}
}

void Inkscape::XML::EventAdd::_replayOne() const {
	sp_repr_add_child(this->repr, this->child, this->ref);
}

void Inkscape::XML::EventDel::_replayOne() const {
	sp_repr_remove_child(this->repr, this->child);
}

void Inkscape::XML::EventChgAttr::_replayOne() const {
	sp_repr_set_attr(this->repr, g_quark_to_string(this->key), this->newval);
}

void Inkscape::XML::EventChgContent::_replayOne() const {
	this->repr->setContent(this->newval);
}

void Inkscape::XML::EventChgOrder::_replayOne() const {
	sp_repr_change_order(this->repr, this->child, this->newref);
}

Inkscape::XML::Event *
sp_repr_coalesce_log (Inkscape::XML::Event *a, Inkscape::XML::Event *b)
{
	Inkscape::XML::Event *action;
	Inkscape::XML::Event **prev_ptr;

	if (!b) return a;
	if (!a) return b;

	/* find the earliest action in the second log */
	/* (also noting the pointer that references it, so we can
	 *  replace it later) */
	prev_ptr = &b;
	for ( action = b ; action->next ; action = action->next ) {
		prev_ptr = &action->next;
	}

	/* add the first log after it */
	action->next = a;

	/* optimize the result */
	*prev_ptr = action->optimizeOne();

	return b;
}

void
sp_repr_free_log (Inkscape::XML::Event *log)
{
	while (log) {
		Inkscape::XML::Event *action;
		action = log;
		log = action->next;
		delete action;
	}
}

namespace {

template <typename T> struct ActionRelations;

template <>
struct ActionRelations<Inkscape::XML::EventAdd> {
	typedef Inkscape::XML::EventDel Opposite;
};

template <>
struct ActionRelations<Inkscape::XML::EventDel> {
	typedef Inkscape::XML::EventAdd Opposite;
};

template <typename A>
Inkscape::XML::Event *cancel_add_or_remove(A *action) {
	typedef typename ActionRelations<A>::Opposite Opposite;
	Opposite *opposite=dynamic_cast<Opposite *>(action->next);

	if ( opposite && opposite->repr == action->repr &&
	     opposite->child == action->child &&
	     opposite->ref == action->ref )
	{
		Inkscape::XML::Event *remaining=opposite->next;

		delete opposite;
		delete action;

		return remaining;
	} else {
		return action;
	}
}

}

Inkscape::XML::Event *Inkscape::XML::EventAdd::_optimizeOne() {
	return cancel_add_or_remove(this);
}

Inkscape::XML::Event *Inkscape::XML::EventDel::_optimizeOne() {
	return cancel_add_or_remove(this);
}

Inkscape::XML::Event *Inkscape::XML::EventChgAttr::_optimizeOne() {
	Inkscape::XML::EventChgAttr *chg_attr=dynamic_cast<Inkscape::XML::EventChgAttr *>(this->next);

	/* consecutive chgattrs on the same key can be combined */
	if ( chg_attr && chg_attr->repr == this->repr &&
	     chg_attr->key == this->key )
	{
		/* replace our oldval with the prior action's */
		this->oldval = chg_attr->oldval;

		/* discard the prior action */
		this->next = chg_attr->next;
		delete chg_attr;
	}

	return this;
}

Inkscape::XML::Event *Inkscape::XML::EventChgContent::_optimizeOne() {
	Inkscape::XML::EventChgContent *chg_content=dynamic_cast<Inkscape::XML::EventChgContent *>(this->next);

	/* consecutive content changes can be combined */
	if ( chg_content && chg_content->repr == this->repr ) {
		/* replace our oldval with the prior action's */
		this->oldval = chg_content->oldval;

		/* get rid of the prior action*/
		this->next = chg_content->next;
		delete chg_content;
	}

	return this;
}

Inkscape::XML::Event *Inkscape::XML::EventChgOrder::_optimizeOne() {
	Inkscape::XML::EventChgOrder *chg_order=dynamic_cast<Inkscape::XML::EventChgOrder *>(this->next);

	/* consecutive chgorders for the same child may be combined or
	 * canceled out */
	if ( chg_order && chg_order->repr == this->repr &&
	     chg_order->child == this->child )
	{
		if ( chg_order->oldref == this->newref ) {
			/* cancel them out */
			Inkscape::XML::Event *after=chg_order->next;

			delete chg_order;
			delete this;

			return after;
		} else {
			/* combine them */
			this->oldref = chg_order->oldref;

			/* get rid of the other one */
			this->next = chg_order->next;
			delete chg_order;

			return this;
		}
	} else {
		return this;
	}
}

namespace {
Glib::ustring node_to_string(Inkscape::XML::Node const &node) {
	Glib::ustring result;
	char const *type_name=NULL;
	switch (node.type()) {
	case Inkscape::XML::DOCUMENT_NODE:
		type_name = "Document";
		break;
	case Inkscape::XML::ELEMENT_NODE:
		type_name = "Element";
		break;
	case Inkscape::XML::TEXT_NODE:
		type_name = "Text";
		break;
	case Inkscape::XML::COMMENT_NODE:
		type_name = "Comment";
		break;
	default:
		g_assert_not_reached();
	}
	char buffer[40];
	result.append("#<");
	result.append(type_name);
	result.append(":");
	std::snprintf(buffer, 40, "0x%p", &node);
	result.append(buffer);
	result.append(">");

	return result;
}
}

Glib::ustring Inkscape::XML::EventAdd::_describe() const {
	return Glib::ustring("Added ") + node_to_string(*child) + Glib::ustring(" to ") + node_to_string(*repr) + " after " + ( ref ? node_to_string(*ref) : "NULL" );
}

Glib::ustring Inkscape::XML::EventDel::_describe() const {
	return Glib::ustring("Removed ") + node_to_string(*child) + " from " + node_to_string(*repr);
}

Glib::ustring Inkscape::XML::EventChgAttr::_describe() const {
	if (newval) {
		return Glib::ustring("Set attribute ") + g_quark_to_string(key) + " on " + node_to_string(*repr) + Glib::ustring(" to ") + static_cast<char const *>(newval);
	} else {
		return Glib::ustring("Unset attribute ") + g_quark_to_string(key) + " on " + node_to_string(*repr);
	}
}

Glib::ustring Inkscape::XML::EventChgContent::_describe() const {
	if (newval) {
		return Glib::ustring("Set content of ") + node_to_string(*repr) + Glib::ustring(" to ") + static_cast<char const *>(newval);
	} else {
		return Glib::ustring("Unset content of ") + node_to_string(*repr);
	}
}

Glib::ustring Inkscape::XML::EventChgOrder::_describe() const {
	return Glib::ustring("Moved ") + node_to_string(*child) + " in " + node_to_string(*repr) + " to after " + ( newref ? node_to_string(*newref) : "NULL" );
}

