#define __SP_DOCUMENT_UNDO_C__

/*
 * Undo/Redo stack implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <stdlib.h>
#include "xml/repr-private.h"
#include "xml/repr-action.h"
#include "sp-object.h"
#include "sp-item.h"
#include "document-private.h"

/* fixme: Implement in preferences */

#define MAX_UNDO 128

/*
 * Undo & redo
 */

void
sp_document_set_undo_sensitive (SPDocument *doc, gboolean sensitive)
{
	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_return_if_fail (sensitive != doc->priv->sensitive);

	if (sensitive == doc->priv->sensitive) return;

	if (sensitive) {
		sp_repr_begin_transaction (doc->rdoc);
	} else {
		doc->priv->partial = sp_repr_coalesce_log (
			doc->priv->partial,
			sp_repr_commit_undoable (doc->rdoc)
		);
	}

	doc->priv->sensitive = sensitive;
}

void
sp_document_done (SPDocument *doc)
{
	sp_document_maybe_done (doc, NULL);
}

void
sp_document_maybe_done (SPDocument *doc, const gchar *key)
{
	SPReprAction *log;

	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	sp_document_clear_redo (doc);

	log = sp_repr_coalesce_log (doc->priv->partial, sp_repr_commit_undoable (doc->rdoc));
	doc->priv->partial = NULL;

	if (!log) return;

	if (key && doc->actionkey && !strcmp (key, doc->actionkey) && doc->priv->undo) {
		doc->priv->undo->data = sp_repr_coalesce_log ((SPReprAction *)doc->priv->undo->data, log);
	} else {
		doc->priv->undo = g_slist_prepend (doc->priv->undo, log);
		doc->priv->undo_size++;

		if (doc->priv->undo_size > MAX_UNDO) {
			GSList *bottom;
	
			g_message ("DEBUG: trimming undo list");

			g_assert (doc->priv->undo != NULL);

			bottom = g_slist_nth (doc->priv->undo, MAX_UNDO - 1);
			g_slist_foreach (bottom->next,
			                 (GFunc)sp_repr_free_log, NULL);
			g_slist_free (bottom->next);
			bottom->next = NULL;

			doc->priv->undo_size = MAX_UNDO;
		}
	}

	doc->actionkey = NULL;

	if (!sp_repr_attr (doc->rroot, "sodipodi:modified")) {
		sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
	}

	sp_repr_begin_transaction (doc->rdoc);
}

void
sp_document_cancel (SPDocument *doc)
{
	SPReprAction *log;

	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	sp_repr_rollback (doc->rdoc);

	if (doc->priv->partial) {
		sp_repr_undo_log (doc->rdoc, doc->priv->partial);
		sp_repr_free_log (doc->priv->partial);
		doc->priv->partial = NULL;
	}

	sp_repr_begin_transaction (doc->rdoc);
}

void
sp_document_undo (SPDocument *doc)
{
	SPReprAction *log;

	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->actionkey = NULL;
	log = sp_repr_commit_undoable (doc->rdoc);

	if (log || doc->priv->partial) {
		g_warning ("Undo aborted: last operation did not complete transaction");
		doc->priv->partial = sp_repr_coalesce_log (doc->priv->partial, log);
		sp_repr_begin_transaction (doc->rdoc);
		return;
	}

	if (doc->priv->undo == NULL) return;

	log = (SPReprAction *) doc->priv->undo->data;
	doc->priv->undo = g_slist_remove (doc->priv->undo, log);
	sp_repr_undo_log (doc->rdoc, log);
	doc->priv->redo = g_slist_prepend (doc->priv->redo, log);

	sp_repr_begin_transaction (doc->rdoc);
}

void
sp_document_redo (SPDocument *doc)
{
	SPReprAction *log;

	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->actionkey = NULL;
	log = sp_repr_commit_undoable (doc->rdoc);

	if (log || doc->priv->partial) {
		g_warning ("Redo aborted: last operation did not complete transaction");
		doc->priv->partial = sp_repr_coalesce_log (doc->priv->partial, log);
		sp_repr_begin_transaction (doc->rdoc);
		return;
	}

	if (doc->priv->redo == NULL) return;

	log = (SPReprAction *) doc->priv->redo->data;
	doc->priv->redo = g_slist_remove (doc->priv->redo, log);
	sp_repr_replay_log (doc->rdoc, log);
	doc->priv->undo = g_slist_prepend (doc->priv->undo, log);

	sp_repr_begin_transaction (doc->rdoc);
}

void
sp_document_clear_undo (SPDocument *doc)
{
	while (doc->priv->undo) {
		GSList *current;

		current = doc->priv->undo;
		doc->priv->undo = current->next;
		doc->priv->undo_size--;

		sp_repr_free_log ((SPReprAction *)current->data);
		g_slist_free_1 (current);
	}
}

void
sp_document_clear_redo (SPDocument *doc)
{
	while (doc->priv->redo) {
		GSList *current;

		current = doc->priv->redo;
		doc->priv->redo = current->next;
		doc->priv->undo_size--;

		sp_repr_free_log ((SPReprAction *)current->data);
		g_slist_free_1 (current);
	}
}

