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

#include <glib.h>
#include <string.h>

#include "repr.h"
#include "repr-private.h"
#include "repr-action.h"

static SPReprAction *reverse_log (SPReprAction *log);
static SPReprAction *new_action (SPReprAction *log,
                                 SPReprActionType type,
                                 SPRepr *repr);
static void free_action (SPReprAction *action);

static SPReprAction *coalesce_action(SPReprAction *action);
static SPReprAction *coalesce_add(SPReprAction *action);
static SPReprAction *coalesce_remove(SPReprAction *action);
static SPReprAction *coalesce_chgattr(SPReprAction *action);
static SPReprAction *coalesce_chgcontent(SPReprAction *action);
static SPReprAction *coalesce_chgorder(SPReprAction *action);

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
		switch (action->type) {
		case SP_REPR_ACTION_ADD:
			sp_repr_remove_child (action->repr,
			                      action->add.child);
			break;
		case SP_REPR_ACTION_DEL:
			sp_repr_add_child (action->repr,
			                   action->del.child,
			                   action->del.ref);
			break;
		case SP_REPR_ACTION_CHGATTR:
			sp_repr_set_attr (action->repr,
			                  g_quark_to_string (action->chgattr.key),
			                  action->chgattr.oldval);
			break;
		case SP_REPR_ACTION_CHGCONTENT:
			sp_repr_set_content (action->repr,
			                     action->chgcontent.oldval);
			break;
		case SP_REPR_ACTION_CHGORDER:
			sp_repr_change_order (action->repr,
			                      action->chgorder.child,
			                      action->chgorder.oldref);
			break;
		default:
			g_assert_not_reached ();
			break;
		}
	}
}

void
sp_repr_replay_log (SPReprAction *log)
{
	SPReprAction *action;

	if (log) {
		g_assert(!log->repr->doc->is_logging);
	}

	log = reverse_log (log);

	for ( action = log ; action ; action = action->next ) {
		switch (action->type) {
		case SP_REPR_ACTION_ADD:
			sp_repr_add_child (action->repr,
			                   action->add.child,
			                   action->add.ref);
			break;
		case SP_REPR_ACTION_DEL:
			sp_repr_remove_child (action->repr,
			                      action->del.child);
			break;
		case SP_REPR_ACTION_CHGATTR:
			sp_repr_set_attr (action->repr,
			                  g_quark_to_string (action->chgattr.key),
			                  action->chgattr.newval);
			break;
		case SP_REPR_ACTION_CHGCONTENT:
			sp_repr_set_content (action->repr,
			                     action->chgcontent.newval);
			break;
		case SP_REPR_ACTION_CHGORDER:
			sp_repr_change_order (action->repr,
			                      action->chgorder.child,
			                      action->chgorder.newref);
			break;
		default:
			g_assert_not_reached ();
			break;
		}
	}

	reverse_log (log);
}

/** \brief Reverse the action linked list LOG in-place
    \return The head of the resulting reversed list.

    This only changes the raw ordering of events in the list; it leaves their types and saved values alone.
*/
static SPReprAction *
reverse_log (SPReprAction *log)
{
	SPReprAction *prev;

	prev = NULL;
	while ( log != NULL ) {
		SPReprAction *curr;

		curr = log;
		log = log->next;
		curr->next = prev;
		prev = curr;
	}

	return prev;
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
	*prev_ptr = coalesce_action(action);

	return b;
}

void
sp_repr_free_log (SPReprAction *log)
{
	while (log) {
		SPReprAction *action;
		action = log;
		log = action->next;
		free_action (action);
	}
}

SPReprAction *
sp_repr_log_add (SPReprAction *log, SPRepr *repr,
                 SPRepr *child, SPRepr *ref)
{
	SPReprAction *action;

	g_assert (child != NULL);

	action = new_action (log, SP_REPR_ACTION_ADD, repr);
	action->add.child = child;
	action->add.ref = ref;

	sp_repr_ref (child);
	if (ref) sp_repr_ref (ref);

	return coalesce_action(action);
}

SPReprAction *
sp_repr_log_remove (SPReprAction *log, SPRepr *repr,
                    SPRepr *child, SPRepr *ref)
{
	SPReprAction *action;

	g_assert (repr != NULL);
	g_assert (child != NULL);

	action = new_action (log, SP_REPR_ACTION_DEL, repr);
	action->del.child = child;
	action->del.ref = ref;

	sp_repr_ref (child);
	if (ref) sp_repr_ref (ref);

	return coalesce_action(action);
}

SPReprAction *
sp_repr_log_chgattr (SPReprAction *log, SPRepr *repr,
                     int key, gchar *oldval,
                     const gchar *newval)
{
	SPReprAction *action;

	g_assert (repr != NULL);

	action = new_action (log, SP_REPR_ACTION_CHGATTR, repr);
	action->chgattr.key = key;
	action->chgattr.oldval = oldval;
	action->chgattr.newval = ( newval ? g_strdup (newval) : NULL );

	return coalesce_action(action);
}

SPReprAction *
sp_repr_log_chgcontent (SPReprAction *log, SPRepr *repr,
                        gchar *oldval, const gchar *newval)
{
	SPReprAction *action;

	g_assert (repr != NULL);

	action = new_action (log, SP_REPR_ACTION_CHGCONTENT, repr);
	action->chgcontent.oldval = oldval;
	action->chgcontent.newval = ( newval ? g_strdup (newval) : NULL );

	return coalesce_action(action);
}

SPReprAction *
sp_repr_log_chgorder (SPReprAction *log, SPRepr *repr,
                      SPRepr *child, SPRepr *oldref, SPRepr *newref)
{
	SPReprAction *action;

	g_assert (repr != NULL);
	g_assert (child != NULL);

	action = new_action (log, SP_REPR_ACTION_CHGORDER, repr);
	action->chgorder.child = child;
	action->chgorder.oldref = oldref;
	action->chgorder.newref = newref;

	sp_repr_ref (child);
	if (oldref) sp_repr_ref (oldref);
	if (newref) sp_repr_ref (newref);

	return coalesce_action(action);
}

#define SP_REPR_ACTION_ALLOC_SIZE 256
static SPReprAction *action_pool = NULL;

static SPReprAction *
new_action (SPReprAction *log, SPReprActionType type, SPRepr *repr)
{
	SPReprAction *action;
	static int next_serial=0;

	g_assert (repr != NULL);

	if (action_pool) {
		action = action_pool;
		action_pool = action_pool->next;
	} else {
		gint i;

		action = g_new (SPReprAction, SP_REPR_ACTION_ALLOC_SIZE);

		for (i = 1; i < SP_REPR_ACTION_ALLOC_SIZE - 1; i++)
		  action[i].next = action + i + 1;

		action[SP_REPR_ACTION_ALLOC_SIZE - 1].next = NULL;
		action_pool = action + 1;
	}

	action->next = log;
	action->type = type;
	action->repr = repr;
	action->serial = next_serial++;

	sp_repr_ref (repr);

	return action;
}

static void
free_action (SPReprAction *action)
{
	switch (action->type) {
	case SP_REPR_ACTION_ADD:
		sp_repr_unref (action->add.child);
		if (action->add.ref)
		  sp_repr_unref (action->add.ref);
		break;
	case SP_REPR_ACTION_DEL:
		sp_repr_unref (action->del.child);
		if (action->del.ref)
		  sp_repr_unref (action->del.ref);
		break;
	case SP_REPR_ACTION_CHGATTR:
		g_free (action->chgattr.oldval);
		g_free (action->chgattr.newval);
		break;
	case SP_REPR_ACTION_CHGCONTENT:
		g_free (action->chgcontent.oldval);
		g_free (action->chgcontent.newval);
		break;
	case SP_REPR_ACTION_CHGORDER:
		sp_repr_unref (action->chgorder.child);
		if (action->chgorder.oldref)
		  sp_repr_unref (action->chgorder.oldref);
		if (action->chgorder.newref)
		  sp_repr_unref (action->chgorder.newref);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	sp_repr_unref (action->repr);
	action->type = SP_REPR_ACTION_INVALID;

	action->next = action_pool;
	action_pool = action;
}

static SPReprAction *
coalesce_action(SPReprAction *action)
{
	if ( !action || !action->next ) {
		return action;
	}

	switch (action->type) {
		case SP_REPR_ACTION_ADD: {
			return coalesce_add(action);
		}
		case SP_REPR_ACTION_DEL: {
			return coalesce_remove(action);
		}
		case SP_REPR_ACTION_CHGATTR: {
			return coalesce_chgattr(action);
		}
		case SP_REPR_ACTION_CHGCONTENT: {
			return coalesce_chgcontent(action);
		}
		case SP_REPR_ACTION_CHGORDER: {
			return coalesce_chgorder(action);
		}
		default: {
			g_assert_not_reached();
			return action;
		}
	};
}

static SPReprAction *
coalesce_add(SPReprAction *action)
{
	SPReprAction *next=action->next;

	/* an add and a matching remove cancel each other out */
	if ( next->type == SP_REPR_ACTION_DEL &&
	     next->repr == action->repr &&
	     next->del.child == action->add.child &&
	     next->del.ref == action->add.ref )
	{
		SPReprAction *after=next->next;

		free_action(action);
		free_action(next);

		return action = after;
	}

	return action;
}

static SPReprAction *
coalesce_remove(SPReprAction *action)
{
	SPReprAction *next=action->next;

	/* an add and a matching remove cancel each other out */
	if ( next->type == SP_REPR_ACTION_ADD &&
	     next->repr == action->repr &&
	     next->add.child == action->del.child &&
	     next->add.ref == action->del.ref )
	{
		SPReprAction *after=next->next;

		free_action(action);
		free_action(next);

		action = after;
	}

	return action;
}

static SPReprAction *
coalesce_chgattr(SPReprAction *action)
{
	SPReprAction *next=action->next;

	/* consecutive chgattrs on the same key can be combined */
	if ( next->type == SP_REPR_ACTION_CHGATTR &&
	     next->repr == action->repr &&
	     next->chgattr.key == action->chgattr.key )
	{
		/* replace our oldval with next's */
		g_free(action->chgattr.oldval);
		action->chgattr.oldval = next->chgattr.oldval;
		next->chgattr.oldval = NULL;

		/* get rid of next */
		action->next = next->next;
		free_action(next);
	}

	return action;
}

static SPReprAction *
coalesce_chgcontent(SPReprAction *action)
{
	SPReprAction *next=action->next;

	/* consecutive content changes can be combined */
	if ( next->type == SP_REPR_ACTION_CHGCONTENT &&
	     next->repr == action->repr )
	{
		/* replace our oldval with next's */
		g_free(action->chgcontent.oldval);
		action->chgcontent.oldval = next->chgcontent.oldval;
		next->chgcontent.oldval = NULL;

		/* get rid of next */
		action->next = next->next;
		free_action(next);
	}

	return action;
}

static SPReprAction *
coalesce_chgorder(SPReprAction *action)
{
	SPReprAction *next=action->next;

	/* consecutive chgorders for the same child may be combined or
	 * canceled out */
	if ( next->type == SP_REPR_ACTION_CHGORDER &&
	     next->repr == action->repr &&
	     next->chgorder.child == action->chgorder.child )
	{
		if ( next->chgorder.oldref == action->chgorder.newref ) {
			/* cancel them out */
			SPReprAction *after=next->next;

			free_action(action);
			free_action(next);

			action = after;
		} else {
			/* combine them */
			if (action->chgorder.oldref) {
				sp_repr_unref(action->chgorder.oldref);
			}
			action->chgorder.oldref = next->chgorder.oldref;
			next->chgorder.oldref = NULL;

			/* get rid of next */
			action->next = next->next;
			free_action(next);
		}
	}

	return action;
}

