#ifndef __SP_DOCUMENT_PRIVATE_H__
#define __SP_DOCUMENT_PRIVATE_H__

/*
 * Seldom needed document data
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <sigc++/sigc++.h>
#include "xml/sp-repr-action-fns.h"
#include "sp-defs.h"
#include "sp-root.h"
#include "document.h"

#define SP_DOCUMENT_DEFS(d) ((SPObject *) SP_ROOT (SP_DOCUMENT_ROOT (d))->defs)

class SPReprAction;

struct SPDocumentPrivate {
	typedef std::map<GQuark, SPDocument::IDChangedSignal> IDChangedSignalMap;
	typedef std::map<GQuark, SPDocument::ResourcesChangedSignal> ResourcesChangedSignalMap;

	GHashTable *iddef;	/**< Dictionary of id -> SPObject mappings */
	/** Dictionary of signals for id changes */
	IDChangedSignalMap id_changed_signals;

	/* Resources */
	/* It is GHashTable of GSLists */
	GHashTable *resources;
	ResourcesChangedSignalMap resources_changed_signals;

	/* Undo/Redo state */
	guint sensitive: 1; /* If we save actions to undo stack */
	SPReprAction * partial; /* partial undo log when interrupted */
	int history_size;
	GSList * undo; /* Undo stack of reprs */
	GSList * redo; /* Redo stack of reprs */
};

#endif
