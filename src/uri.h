#ifndef INKSCAPE_URI_H
#define INKSCAPE_URI_H

#include <glib.h>
#include <exception>

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
	const gchar *getFragment() const;
private:
	class Impl {
	public:
		static Impl *create(const gchar *fragment);
		void reference();
		void unreference();
		const gchar *getFragment() const;
	private:
		Impl(gchar *fragment);
		int _refcount;
		gchar *_fragment;
	};
	Impl *_impl;
};

inline const gchar *URI::getFragment() const {
	return _impl->getFragment();
}

};

#endif
