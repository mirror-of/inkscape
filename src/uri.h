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

#ifndef INKSCAPE_URI_H
#define INKSCAPE_URI_H

#include <glib.h>
#include <exception>
#include <libxml/uri.h>

namespace Inkscape {

class BadURIException : public std::exception {};

class UnsupportedURIException : public BadURIException {
public:
	const char *what() const throw() { return "Unsupported URI"; }
};

class MalformedURIException : public BadURIException {
public:
	const char *what() const throw() { return "Malformed URI"; }
};

class URI {
public:
	URI(const URI &uri);
	explicit URI(const gchar *uri_string) throw(BadURIException);
	~URI();

	bool isRelative() const { return _impl->isRelative(); }
	const gchar *getScheme() const { return _impl->getScheme(); }
	const gchar *getQuery() const { return _impl->getQuery(); }
	const gchar *getFragment() const { return _impl->getFragment(); }

	gchar *toString() const { return _impl->toString(); }
private:
	class Impl {
	public:
		static Impl *create(xmlURIPtr uri);
		void reference();
		void unreference();

		bool isRelative() const;
		const gchar *getScheme() const;
		const gchar *getQuery() const;
		const gchar *getFragment() const;
		gchar *toString() const;
	private:
		Impl(xmlURIPtr uri);
		~Impl();
		int _refcount;
		xmlURIPtr _uri;
	};
	Impl *_impl;
};

}; /* namespace Inkscape */

#endif
