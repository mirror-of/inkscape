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
	const gchar *getQuery() const { return _impl->getQuery(); }
	const gchar *getFragment() const { return _impl->getFragment(); }
private:
	class Impl {
	public:
		static Impl *create(xmlURIPtr uri);
		void reference();
		void unreference();

		bool isRelative() const;
		const gchar *getQuery() const;
		const gchar *getFragment() const;
	private:
		Impl(xmlURIPtr uri);
		~Impl();
		int _refcount;
		xmlURIPtr _uri;
	};
	Impl *_impl;
};

};

#endif
