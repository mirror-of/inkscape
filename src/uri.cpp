#include <glib.h>
#include "uri.h"

namespace Inkscape {

URI::URI(const URI &uri) {
	uri._impl->reference();
	_impl = uri._impl;
}

URI::URI(const gchar *uri_string) throw(BadURIException) {
	if ( uri_string[0] != '#' || !uri_string[1] ) {
		throw UnsupportedURIException();
	}
	_impl = Impl::create(uri_string+1);
}

URI::~URI() {
	_impl->unreference();
}

URI::Impl *URI::Impl::create(const gchar *fragment) {
	if (fragment) {
		return new Impl(g_strdup(fragment));
	} else {
		return new Impl(NULL);
	}
}

URI::Impl::Impl(gchar *fragment)
: _refcount(1),  _fragment(fragment) {}

void URI::Impl::reference() {
	_refcount++;
}

void URI::Impl::unreference() {
	if (!--_refcount) {
		delete this;
	}
}

const gchar *URI::Impl::getFragment() const {
	return _fragment;
}

};

