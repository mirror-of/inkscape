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
#include "repr-private.h"
#include "sp-repr-action.h"
#include "sp-repr-action-fns.h"
#include "util/list.h"
#include "util/reverse-list.h"

using Inkscape::Util::List;
using Inkscape::Util::reverse_list;

int SPReprAction::_next_serial=0;

void
sp_repr_begin_transaction (SPReprDoc *doc)
{
	if (doc->is_logging) {
		/* FIXME !!! rethink discarding the existing log? */
		sp_repr_free_log (doc->log);
		doc->log = NULL;
	} else {
		doc->is_logging = true;
	}
}

void
sp_repr_rollback (SPReprDoc *doc)
{
	if (doc->is_logging) {
		doc->is_logging = false;
		sp_repr_undo_log (doc->log);
		sp_repr_free_log (doc->log);
		doc->log = NULL;
	}
}

void
sp_repr_commit (SPReprDoc *doc)
{
	if (doc->is_logging) {
		doc->is_logging = false;
		sp_repr_free_log (doc->log);
		doc->log = NULL;
	}
}

SPReprAction *
sp_repr_commit_undoable (SPReprDoc *doc)
{
	SPReprAction *log=NULL;

	if (doc->is_logging) {
		log = doc->log;
		doc->log = NULL;
		doc->is_logging = false;
	}

	return log;
}

void
sp_repr_undo_log (SPReprAction *log)
{
	SPReprAction *action;

	if (log) {
		g_assert(!log->repr->doc->is_logging);
	}

	for ( action = log ; action ; action = action->next ) {
		action->undoOne();
	}
}

void SPReprActionAdd::_undoOne() const {
	sp_repr_remove_child(this->repr, this->child);
}

void SPReprActionDel::_undoOne() const {
	sp_repr_add_child(this->repr, this->child, this->ref);
}

void SPReprActionChgAttr::_undoOne() const {
	sp_repr_set_attr(this->repr, g_quark_to_string(this->key), this->oldval);
}

void SPReprActionChgContent::_undoOne() const {
	sp_repr_set_content(this->repr, this->oldval);
}

void SPReprActionChgOrder::_undoOne() const {
	sp_repr_change_order(this->repr, this->child, this->oldref);
}

void
sp_repr_replay_log (SPReprAction *log)
{
	if (log) {
		g_assert(!log->repr->doc->is_logging);
	}

	List<SPReprAction &> reversed(
		reverse_list<SPReprAction::Iterator>(log, NULL)
	);
	for ( ; reversed ; ++reversed ) {
		reversed->replayOne();
	}
}

void SPReprActionAdd::_replayOne() const {
	sp_repr_add_child(this->repr, this->child, this->ref);
}

void SPReprActionDel::_replayOne() const {
	sp_repr_remove_child(this->repr, this->child);
}

void SPReprActionChgAttr::_replayOne() const {
	sp_repr_set_attr(this->repr, g_quark_to_string(this->key), this->newval);
}

void SPReprActionChgContent::_replayOne() const {
	sp_repr_set_content(this->repr, this->newval);
}

void SPReprActionChgOrder::_replayOne() const {
	sp_repr_change_order(this->repr, this->child, this->newref);
}

SPReprAction *
sp_repr_coalesce_log (SPReprAction *a, SPReprAction *b)
{
	SPReprAction *action;
	SPReprAction **prev_ptr;

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
sp_repr_free_log (SPReprAction *log)
{
	while (log) {
		SPReprAction *action;
		action = log;
		log = action->next;
		delete action;
	}
}

namespace {

template <typename T> struct ActionRelations;

template <>
struct ActionRelations<SPReprActionAdd> {
	typedef SPReprActionDel Opposite;
};

template <>
struct ActionRelations<SPReprActionDel> {
	typedef SPReprActionAdd Opposite;
};

template <typename A>
SPReprAction *cancel_add_or_remove(A *action) {
	typedef typename ActionRelations<A>::Opposite Opposite;
	Opposite *opposite=dynamic_cast<Opposite *>(action->next);

	if ( opposite && opposite->repr == action->repr &&
	     opposite->child == action->child &&
	     opposite->ref == action->ref )
	{
		SPReprAction *remaining=opposite->next;

		delete opposite;
		delete action;

		return remaining;
	} else {
		return action;
	}
}

}

SPReprAction *SPReprActionAdd::_optimizeOne() {
	return cancel_add_or_remove(this);
}

SPReprAction *SPReprActionDel::_optimizeOne() {
	return cancel_add_or_remove(this);
}

SPReprAction *SPReprActionChgAttr::_optimizeOne() {
	SPReprActionChgAttr *chg_attr=dynamic_cast<SPReprActionChgAttr *>(this->next);

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

SPReprAction *SPReprActionChgContent::_optimizeOne() {
	SPReprActionChgContent *chg_content=dynamic_cast<SPReprActionChgContent *>(this->next);

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

SPReprAction *SPReprActionChgOrder::_optimizeOne() {
	SPReprActionChgOrder *chg_order=dynamic_cast<SPReprActionChgOrder *>(this->next);

	/* consecutive chgorders for the same child may be combined or
	 * canceled out */
	if ( chg_order && chg_order->repr == this->repr &&
	     chg_order->child == this->child )
	{
		if ( chg_order->oldref == this->newref ) {
			/* cancel them out */
			SPReprAction *after=chg_order->next;

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

