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
#include <sigc++/sigc++.h>
#include <exception>
#include "forward.h"
#include "document.h"

namespace Inkscape {

class UnsupportedURIException {
public:
	const char *what() const { return "Unsupported or malformed URI"; }
};

class URIReference : public SigC::Object {
public:
	URIReference(SPDocument *rel_document, const gchar *uri);
	~URIReference();

	SigC::Signal1<void, SPObject *> changedSignal();

	SPObject *getObject();

private:
	SigC::Connection _connection;
	SPObject *_obj;

	SigC::Signal1<void, SPObject *> _changed_signal;

	void _onIDChanged(SPObject *object);
};

inline SigC::Signal1<void, SPObject *> URIReference::changedSignal() {
	return _changed_signal;
}

inline SPObject *URIReference::getObject() {
	return _obj;
}

}

SPObject *sp_uri_reference_resolve (SPDocument *document, const gchar *uri);

#endif
