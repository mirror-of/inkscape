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
#include "uri.h"
#include "forward.h"
#include "document.h"

namespace Inkscape {

/**
 * A class encapsulating a reference to a particular URI; observers can
 * be notified when the URI comes to reference a different SPObject.
 *
 * The URIReference increments and decrements the SPObject's hrefcount
 * automatically.
 *
 * @see SPObject
 * @see sp_object_href
 * @see sp_object_hunref
 */
class URIReference : public SigC::Object {
public:
	/**
	 * Constructor.
	 *
	 */
	URIReference();

	/**
	 * Destructor.  Calls shutdown() if the reference has not been
	 * shut down yet.
	 */
	virtual ~URIReference();

	/**
	 * Attaches to a URI, relative to the specified document.
	 *
	 * Throws a BadURIException if the URI is unsupported,
	 * or the fragment identifier is xpointer and malformed.
	 *
	 * @param rel_document document for relative URIs
	 * @param uri the URI to watch
	 */
	void attach(SPDocument *document, const URI &uri)
	  throw(BadURIException);

	/**
	 * Detaches from the currently attached URI target, if any;
	 * the current referrent is signaled as NULL.
	 */
	void detach();

	/**
	 * Returns a pointer to the current referrent of the attached
	 * URI, or NULL.
	 *
	 * @return a pointer to the referenced SPObject or NULL
	 */
	SPObject *getObject();

	/**
	 * Accessor for the referrent change notification signal;
	 * this signal is emitted whenever the URIReference's
	 * referrent changes.
	 *
	 * Signal handlers take two parameters: the old and new
	 * referrents.
	 *
	 * @returns a signal
	 */
	SigC::Signal2<void, SPObject *, SPObject *> changedSignal();

private:
	SigC::Connection _connection;
	SPObject *_obj;

	SigC::Signal2<void, SPObject *, SPObject *> _changed_signal;

	void _setObject(SPObject *object);
	static void _release(SPObject *object, URIReference *reference);
	void operator=(const URIReference &ref) {}
};

inline SigC::Signal2<void, SPObject *, SPObject *> URIReference::changedSignal(){
	return _changed_signal;
}

inline SPObject *URIReference::getObject() {
	return _obj;
}

}

SPObject *sp_uri_reference_resolve (SPDocument *document, const gchar *uri);

#endif
