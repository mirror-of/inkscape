/*
 * Classes for representing and manipulating URIs.
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <libxml/xmlmemory.h>
#include "uri.h"

namespace Inkscape {

URI::URI(const URI &uri) {
	uri._impl->reference();
	_impl = uri._impl;
}

URI::URI(const gchar *uri_string) throw(BadURIException) {
	xmlURIPtr uri;
	if (!uri_string) {
		throw MalformedURIException();
	}
	uri = xmlParseURI(uri_string);
	if (!uri) {
		throw MalformedURIException();
	}
	_impl = Impl::create(uri);
}

URI::~URI() {
	_impl->unreference();
}

URI::Impl *URI::Impl::create(xmlURIPtr uri) {
	return new Impl(uri);
}

URI::Impl::Impl(xmlURIPtr uri)
: _refcount(1), _uri(uri) {}

URI::Impl::~Impl() {
	if (_uri) {
		xmlFreeURI(_uri);
		_uri = NULL;
	}
}

void URI::Impl::reference() {
	_refcount++;
}

void URI::Impl::unreference() {
	if (!--_refcount) {
		delete this;
	}
}

bool URI::Impl::isRelative() const {
	return !_uri->scheme;
}

const gchar *URI::Impl::getScheme() const {
	return (gchar *)_uri->scheme;
}

const gchar *URI::Impl::getQuery() const {
	return (gchar *)_uri->query;
}

const gchar *URI::Impl::getFragment() const {
	return (gchar *)_uri->fragment;
}

gchar *URI::Impl::toString() const {
	xmlChar *string = xmlSaveUri(_uri);
	if (string) {
		/* hand the string off to glib memory management */
		gchar *glib_string = g_strdup((gchar *)string);
		xmlFree(string);
		return glib_string;
	} else {
		return NULL;
	}
}

};

