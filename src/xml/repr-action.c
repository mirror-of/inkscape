/*
 * Repr transaction logging
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY  <mental@rydia.net>
 *
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
static void coalesce_chgattr (SPReprAction *action);
static void coalesce_chgcontent (SPReprAction *action);
static SPReprAction *new_action (SPReprAction *log,
                                 SPReprActionType type,
                                 SPRepr *repr);
static void free_action (SPReprAction *action);

void
sp_repr_begin_transaction (SPReprDoc *doc)
{
	if (doc->is_logging) {
		sp_repr_free_log (doc->log);
		doc->log = NULL;
	} else {
		doc->is_logging = 1;
	}
}

void
sp_repr_rollback (SPReprDoc *doc)
{
	if (doc->is_logging) {
		sp_repr_undo_log (doc->log);
		sp_repr_free_log (doc->log);
		doc->log = NULL;
		doc->is_logging = 0;
	}
}

void
sp_repr_commit (SPReprDoc *doc)
{
	if (doc->is_logging) {
		sp_repr_free_log (doc->log);
		doc->log = NULL;
		doc->is_logging = 0;
	}
}

SPReprAction *
sp_repr_commit_undoable (SPReprDoc *doc)
{
	SPReprAction *log=NULL;

	if (doc->is_logging) {
		log = doc->log;
		doc->log = NULL;
		doc->is_logging = 0;
	}

	return log;
}

void
sp_repr_undo_log (SPReprAction *log)
{
	SPReprAction *action;

	for ( action = log ; action ; action = action->next ) {
		switch (action->type) {
		case SP_REPR_ACTION_ADD:
			sp_repr_remove_child (action->repr,
			                      action->act.add.child);
			break;
		case SP_REPR_ACTION_DEL:
			sp_repr_add_child (action->repr,
			                   action->act.del.child,
			                   action->act.del.ref);
			break;
		case SP_REPR_ACTION_CHGATTR:
			sp_repr_set_attr (action->repr,
			                  g_quark_to_string (action->act.chgattr.key),
			                  action->act.chgattr.oldval);
			break;
		case SP_REPR_ACTION_CHGCONTENT:
			sp_repr_set_content (action->repr,
			                     action->act.chgattr.oldval);
			break;
		case SP_REPR_ACTION_CHGORDER:
			sp_repr_change_order (action->repr,
			                      action->act.chgorder.child,
			                      action->act.chgorder.oldref);
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

	log = reverse_log (log);

	for ( action = log ; action ; action = action->next ) {
		switch (action->type) {
		case SP_REPR_ACTION_ADD:
			sp_repr_add_child (action->repr,
			                   action->act.add.child,
			                   action->act.add.ref);
			break;
		case SP_REPR_ACTION_DEL:
			sp_repr_remove_child (action->repr,
			                      action->act.del.child);
			break;
		case SP_REPR_ACTION_CHGATTR:
			sp_repr_set_attr (action->repr,
			                  g_quark_to_string (action->act.chgattr.key),
			                  action->act.chgattr.newval);
			break;
		case SP_REPR_ACTION_CHGCONTENT:
			sp_repr_set_content (action->repr,
			                     action->act.chgcontent.newval);
			break;
		case SP_REPR_ACTION_CHGORDER:
			sp_repr_change_order (action->repr,
			                      action->act.chgorder.child,
			                      action->act.chgorder.newref);
			break;
		default:
			g_assert_not_reached ();
			break;
		}
	}

	reverse_log (log);
}

/** \brief Reverse the action linked list LOG in-place, i.e. rewrite its `next' pointers.
    \return The head of the resulting reversed list.

    Changes only `next' fields; e.g. does not change the "direction" of each action
    (add <-> delete).
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

	if (!b) return a;
	if (!a) return b;

	for ( action = b ; action->next ; action = action->next );
	action->next = a;

	for ( action = b ; action ; action = action->next ) {
		if (action->next) switch (action->type) {
		case SP_REPR_ACTION_CHGATTR:
			coalesce_chgattr (action);
			break;
		case SP_REPR_ACTION_CHGCONTENT:
			coalesce_chgcontent (action);
			break;
		case SP_REPR_ACTION_ADD:
		case SP_REPR_ACTION_DEL:
		case SP_REPR_ACTION_CHGORDER:
			break;
		default:
			g_assert_not_reached ();
		}
	}

	return b;
}


static void
coalesce_chgattr (SPReprAction *action)
{
	SPReprAction *iter, *prev, *next;
	static int id_key=0;

	if (!id_key) id_key = g_quark_from_static_string ("id");

	if (!action) return;

	prev = action;
	for ( iter = action->next ;
	      iter && iter->type == SP_REPR_ACTION_CHGATTR ;
	      iter = next )
	{
		next = iter->next;
		if ( action->repr == iter->repr &&
		     action->act.chgattr.key == iter->act.chgattr.key )
		{
			/* ensure changes are continuous */
			if (strcmp(action->act.chgattr.oldval, 
			           iter->act.chgattr.newval)) break;

			if (action->act.chgattr.oldval)
			  g_free (action->act.chgattr.oldval);

			action->act.chgattr.oldval = iter->act.chgattr.oldval;
			iter->act.chgattr.oldval = NULL;
			free_action (iter);
			prev->next = next;
		} else if ( action->act.chgattr.key == id_key ) {
			/* id changes tend to be sensitive to reordering */
			break;
		} else {
			prev = iter;
		}
	}
}

static void
coalesce_chgcontent (SPReprAction *action)
{
	SPReprAction *iter, *prev, *next;

	if (!action) return;

	prev = action;
	for ( iter = action->next ;
	      iter && iter->type == SP_REPR_ACTION_CHGCONTENT ;
	      iter = next )
	{
		next = iter->next;
		if ( action->repr == iter->repr ) {
			/* ensure changes are continuous */
			if (strcmp(action->act.chgcontent.oldval, 
			           iter->act.chgcontent.newval)) break;

			if (action->act.chgcontent.oldval)
			  g_free (action->act.chgcontent.oldval);

			action->act.chgcontent.oldval =
			  iter->act.chgcontent.oldval;

			iter->act.chgcontent.oldval = NULL;
			free_action (iter);
			prev->next = next;
		} else {
			prev = iter;
		}
	}
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
	action->act.add.child = child;
	action->act.add.ref = ref;

	sp_repr_ref (child);
	if (ref) sp_repr_ref (ref);

	return action;
}

SPReprAction *
sp_repr_log_remove (SPReprAction *log, SPRepr *repr,
                    SPRepr *child, SPRepr *ref)
{
	SPReprAction *action;

	g_assert (repr != NULL);
	g_assert (child != NULL);

	action = new_action (log, SP_REPR_ACTION_DEL, repr);
	action->act.del.child = child;
	action->act.del.ref = ref;

	sp_repr_ref (child);
	if (ref) sp_repr_ref (ref);

	return action;
}

SPReprAction *
sp_repr_log_chgattr (SPReprAction *log, SPRepr *repr,
                     int key, gchar *oldval,
                     const gchar *newval)
{
	SPReprAction *action;

	g_assert (repr != NULL);

	action = new_action (log, SP_REPR_ACTION_CHGATTR, repr);
	action->act.chgattr.key = key;
	action->act.chgattr.oldval = oldval;
	action->act.chgattr.newval = ( newval ? g_strdup (newval) : NULL );

	return action;
}

SPReprAction *
sp_repr_log_chgcontent (SPReprAction *log, SPRepr *repr,
                        gchar *oldval, const gchar *newval)
{
	SPReprAction *action;

	g_assert (repr != NULL);

	action = new_action (log, SP_REPR_ACTION_CHGCONTENT, repr);
	action->act.chgcontent.oldval = oldval;
	action->act.chgcontent.newval = ( newval ? g_strdup (newval) : NULL );

	return action;
}

SPReprAction *
sp_repr_log_chgorder (SPReprAction *log, SPRepr *repr,
                      SPRepr *child, SPRepr *oldref, SPRepr *newref)
{
	SPReprAction *action;

	g_assert (repr != NULL);
	g_assert (child != NULL);

	action = new_action (log, SP_REPR_ACTION_CHGORDER, repr);
	action->act.chgorder.child = child;
	action->act.chgorder.oldref = oldref;
	action->act.chgorder.newref = newref;

	sp_repr_ref (child);
	if (oldref) sp_repr_ref (oldref);
	if (newref) sp_repr_ref (newref);

	return action;
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
		sp_repr_unref (action->act.add.child);
		if (action->act.add.ref)
		  sp_repr_unref (action->act.add.ref);
		break;
	case SP_REPR_ACTION_DEL:
		sp_repr_unref (action->act.del.child);
		if (action->act.del.ref)
		  sp_repr_unref (action->act.del.ref);
		break;
	case SP_REPR_ACTION_CHGATTR:
		if (action->act.chgattr.oldval)
		  g_free (action->act.chgattr.oldval);
		if (action->act.chgattr.newval)
		  g_free (action->act.chgattr.newval);
		break;
	case SP_REPR_ACTION_CHGCONTENT:
		if (action->act.chgcontent.oldval)
		  g_free (action->act.chgcontent.oldval);
		if (action->act.chgcontent.newval)
		  g_free (action->act.chgcontent.newval);
		break;
	case SP_REPR_ACTION_CHGORDER:
		sp_repr_unref (action->act.chgorder.child);
		if (action->act.chgorder.oldref)
		  sp_repr_unref (action->act.chgorder.oldref);
		if (action->act.chgorder.newref)
		  sp_repr_unref (action->act.chgorder.newref);
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

