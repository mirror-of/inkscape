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

#include "config.h"

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "xml/repr-private.h"
#include "xml/repr-action.h"
#include "sp-object.h"
#include "sp-item.h"
#include "document-private.h"
#include "document.h"
#include "selection.h"

/*
 * Undo & redo
 */

void
sp_document_set_undo_sensitive (SPDocument *doc, gboolean sensitive)
{
	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_return_if_fail ( !(sensitive) != !(doc->priv->sensitive) );

	if ( !(sensitive) == !(doc->priv->sensitive) )
		return;

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
sp_document_reset_key (Inkscape::Application *inkscape, SPDesktop *desktop, GtkObject *base)
{
	SPDocument *doc = (SPDocument *) base;
	doc->actionkey = NULL;
}

void
sp_document_maybe_done (SPDocument *doc, const gchar *key)
{
	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	sp_document_ensure_up_to_date (doc);

	sp_document_clear_redo (doc);

	SPReprAction *log = sp_repr_coalesce_log (doc->priv->partial, sp_repr_commit_undoable (doc->rdoc));
	doc->priv->partial = NULL;

	if (!log) {
		sp_repr_begin_transaction (doc->rdoc);
		return;
	}

	if (key && doc->actionkey && !strcmp (key, doc->actionkey) && doc->priv->undo) {
		doc->priv->undo->data = sp_repr_coalesce_log ((SPReprAction *)doc->priv->undo->data, log);
	} else {
		doc->priv->undo = g_slist_prepend (doc->priv->undo, log);
		doc->priv->history_size++;
	}

	doc->actionkey = key;

	doc->virgin = FALSE;
	if (!sp_repr_attr (doc->rroot, "sodipodi:modified")) {
		sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
	}

	sp_repr_begin_transaction (doc->rdoc);
}

void
sp_document_cancel (SPDocument *doc)
{
	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	sp_repr_rollback (doc->rdoc);

	if (doc->priv->partial) {
		sp_repr_undo_log (doc->priv->partial);
		sp_repr_free_log (doc->priv->partial);
		doc->priv->partial = NULL;
	}

	sp_repr_begin_transaction (doc->rdoc);
}

gboolean
sp_document_undo (SPDocument *doc)
{
	SPReprAction *log;
	gboolean ret;

	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->actionkey = NULL;
	log = sp_repr_commit_undoable (doc->rdoc);

	if (log || doc->priv->partial) {
		g_warning ("Undo aborted: last operation did not complete transaction");
		doc->priv->partial = sp_repr_coalesce_log (doc->priv->partial, log);
		ret = FALSE;
	} else if (doc->priv->undo) {
		log = (SPReprAction *) doc->priv->undo->data;
		doc->priv->undo = g_slist_remove (doc->priv->undo, log);
		sp_repr_undo_log (log);
		doc->priv->redo = g_slist_prepend (doc->priv->redo, log);
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	sp_repr_begin_transaction (doc->rdoc);

	return ret;
}

gboolean
sp_document_redo (SPDocument *doc)
{
	SPReprAction *log;
	gboolean ret;

	g_assert (doc != NULL);
	g_assert (SP_IS_DOCUMENT (doc));
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->actionkey = NULL;
	log = sp_repr_commit_undoable (doc->rdoc);

	if (log || doc->priv->partial) {
		g_warning ("Redo aborted: last operation did not complete transaction");
		doc->priv->partial = sp_repr_coalesce_log (doc->priv->partial, log);
		ret = FALSE;
	} else if (doc->priv->redo) {
		log = (SPReprAction *) doc->priv->redo->data;
		doc->priv->redo = g_slist_remove (doc->priv->redo, log);
		sp_repr_replay_log (log);
		doc->priv->undo = g_slist_prepend (doc->priv->undo, log);
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	sp_repr_begin_transaction (doc->rdoc);

	return ret;
}

void
sp_document_clear_undo (SPDocument *doc)
{
	while (doc->priv->undo) {
		GSList *current;

		current = doc->priv->undo;
		doc->priv->undo = current->next;
		doc->priv->history_size--;

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
		doc->priv->history_size--;

		sp_repr_free_log ((SPReprAction *)current->data);
		g_slist_free_1 (current);
	}
}

