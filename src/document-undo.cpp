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

#include "xml/repr.h"
#include "xml/event-fns.h"
#include "sp-object.h"
#include "sp-item.h"
#include "document-private.h"
#include "document.h"
#include "selection.h"
#include "inkscape.h"
#include "debug/event-tracker.h"
#include "debug/simple-event.h"

/*
 * Undo & redo
 *
 *   Since undo sensitivity needs to be nested, setting undo sensitivity
 *   should be done like this:
 *      gboolean saved = sp_document_get_undo_sensitive(document);
 *      sp_document_set_undo_sensitive(document, FALSE);
 *      ... do stuff ...
 *      sp_document_set_undo_sensitive(document, saved);
 */

void
sp_document_set_undo_sensitive (SPDocument *doc, gboolean sensitive)
{
	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);

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

	doc->priv->sensitive = !!sensitive;
}

gboolean sp_document_get_undo_sensitive(SPDocument const *document) {
	g_assert(document != NULL);
	g_assert(document->priv != NULL);

	return document->priv->sensitive;
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
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->collectOrphans();

	sp_document_ensure_up_to_date (doc);

	sp_document_clear_redo (doc);

	Inkscape::XML::Event *log = sp_repr_coalesce_log (doc->priv->partial, sp_repr_commit_undoable (doc->rdoc));
	doc->priv->partial = NULL;

	if (!log) {
		sp_repr_begin_transaction (doc->rdoc);
		return;
	}

	if (key && doc->actionkey && !strcmp (key, doc->actionkey) && doc->priv->undo) {
		doc->priv->undo->data = sp_repr_coalesce_log ((Inkscape::XML::Event *)doc->priv->undo->data, log);
	} else {
		doc->priv->undo = g_slist_prepend (doc->priv->undo, log);
		doc->priv->history_size++;
	}

	doc->actionkey = key;

	doc->virgin = FALSE;
	if (!doc->rroot->attribute("sodipodi:modified")) {
		sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
	}

	sp_repr_begin_transaction (doc->rdoc);
}

void
sp_document_cancel (SPDocument *doc)
{
	g_assert (doc != NULL);
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

namespace {

void finish_incomplete_transaction(SPDocument &doc) {
	SPDocumentPrivate &priv=*doc.priv;
	Inkscape::XML::Event *log=sp_repr_commit_undoable(doc.rdoc);
	if (log || priv.partial) {
		g_warning ("Incomplete undo transaction:");
		priv.partial = sp_repr_coalesce_log(priv.partial, log);
		sp_repr_debug_print_log(priv.partial);
		priv.undo = g_slist_prepend(priv.undo, priv.partial);
		priv.partial = NULL;
	}
}

}

gboolean
sp_document_undo (SPDocument *doc)
{
	using Inkscape::Debug::EventTracker;
	using Inkscape::Debug::SimpleEvent;

	gboolean ret;

	EventTracker<SimpleEvent<Inkscape::Debug::Event::DOCUMENT> > tracker("undo");

	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->priv->sensitive = FALSE;

	doc->actionkey = NULL;

	finish_incomplete_transaction(*doc);

	if (doc->priv->undo) {
		Inkscape::XML::Event *log=(Inkscape::XML::Event *)doc->priv->undo->data;
		doc->priv->undo = g_slist_remove (doc->priv->undo, log);
		sp_repr_undo_log (log);
		doc->priv->redo = g_slist_prepend (doc->priv->redo, log);

		sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");

		ret = TRUE;
	} else {
		ret = FALSE;
	}

	sp_repr_begin_transaction (doc->rdoc);

	doc->priv->sensitive = TRUE;

	if (ret) 
		inkscape_external_change();

	return ret;
}

gboolean
sp_document_redo (SPDocument *doc)
{
	using Inkscape::Debug::EventTracker;
	using Inkscape::Debug::SimpleEvent;

	gboolean ret;

	EventTracker<SimpleEvent<Inkscape::Debug::Event::DOCUMENT> > tracker("redo");

	g_assert (doc != NULL);
	g_assert (doc->priv != NULL);
	g_assert (doc->priv->sensitive);

	doc->priv->sensitive = FALSE;

	doc->actionkey = NULL;

	finish_incomplete_transaction(*doc);

	if (doc->priv->redo) {
		Inkscape::XML::Event *log=(Inkscape::XML::Event *)doc->priv->redo->data;
		doc->priv->redo = g_slist_remove (doc->priv->redo, log);
		sp_repr_replay_log (log);
		doc->priv->undo = g_slist_prepend (doc->priv->undo, log);

		sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");

		ret = TRUE;
	} else {
		ret = FALSE;
	}

	sp_repr_begin_transaction (doc->rdoc);

	doc->priv->sensitive = TRUE;

	if (ret) 
		inkscape_external_change();

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

		sp_repr_free_log ((Inkscape::XML::Event *)current->data);
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

		sp_repr_free_log ((Inkscape::XML::Event *)current->data);
		g_slist_free_1 (current);
	}
}
