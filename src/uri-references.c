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

SPObject *
sp_uri_reference_resolve (SPDocument *document, const guchar *uri)
{
	SPObject *ref;
	const guchar *e;
	guchar *id;
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

	id = alloca (len + 1);
	memcpy (id, uri + 5, len);
	id[len] = '\0';

	ref = sp_document_lookup_id (document, id);

	return ref;
}


