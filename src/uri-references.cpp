#define __SP_URI_REFERENCES_C__

/*
 * Helper methods for resolving URI References
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "document.h"
#include "uri-references.h"

struct _SPURICallback {
	SPDocument *document;
	SPDocumentIDCallback *callback;
	SPURICallbackFunc func;
	gpointer data;
};

static gchar *
uri_to_id(SPDocument *document, const gchar *uri)
{
	const gchar *e;
	gchar *id;
	gint len;

	g_return_val_if_fail (document != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);

	if (!uri) return NULL;
	/* fixme: xpointer, everything */
	if (strncmp (uri, "url(#", 5)) return NULL;

	e = uri + 5;
	while (*e) {
		if (*e == ')') break;
		if (!isalnum (*e) && (*e != '_') && (*e != '-')) return NULL;
		e += 1;
		if (!*e) return NULL;
	}

	len = e - uri - 5;
	if (len < 1) return NULL;

	id = (gchar*)g_new(gchar, len + 1);
	memcpy (id, uri + 5, len);
	id[len] = '\0';

	return id;
}


static void
id_callback(SPDocument *document, const gchar *id, SPObject *object, gpointer data)
{
	SPURICallback *callback=(SPURICallback *)data;
	callback->func(object, callback->data);
}

SPURICallback *
sp_uri_add_callback(SPDocument *document, const gchar *uri, SPURICallbackFunc func, gpointer data)
{
	gchar *id;
	SPURICallback *callback;

	/* FIXME !!! add signal handler for document 'destroy' signal to disable this callback if the document is destroyed */

	id = uri_to_id(document, uri);
	if (!id) return NULL;

	callback = g_new(SPURICallback, 1);

	callback->document = document;
	callback->func = func;
	callback->data = data;
	callback->callback = sp_document_add_id_callback(document, id, id_callback, (gpointer)callback);

	return callback;
}

void
sp_uri_remove_callback(SPURICallback *callback)
{
	sp_document_remove_id_callback(callback->document, callback->callback);
	g_free(callback);
}

SPObject *
sp_uri_reference_resolve (SPDocument *document, const gchar *uri)
{
	gchar *id;

	id = uri_to_id(document, uri);
	if (!id) return NULL;

	SPObject *ref;
	ref = sp_document_lookup_id (document, id);
	g_free(id);
	return ref;
}

