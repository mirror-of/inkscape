#ifndef __SP_URI_REFERENCES_H__
#define __SP_URI_REFERENCES_H__

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

#include <glib.h>
#include "forward.h"

typedef struct _SPURICallback SPURICallback;

typedef void (*SPURICallbackFunc)(SPObject *object, gpointer data);

SPURICallback *sp_uri_add_callback(SPDocument *document, const gchar *uri, SPURICallbackFunc func, gpointer data);
void sp_uri_remove_callback(SPURICallback *callback);
SPObject *sp_uri_reference_resolve (SPDocument *document, const gchar *uri);

#endif
