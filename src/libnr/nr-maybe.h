#ifndef __NR_MAYBE_H__
#define __NR_MAYBE_H__

/*
 * Functionalesque "Maybe" class
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in the public domain.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <stdexcept>
#include <typeinfo>

namespace NR {

template <typename T>
struct MaybeTraits {
	typedef T storage;
	typedef T& reference;
	typedef const T &const_reference;
	static storage to_storage(const_reference t) { return t; }
	static reference from_storage(storage t) { return t; }
};

template <typename T>
struct MaybeTraits<T&> {
	typedef T *storage;
	typedef T &reference;
	typedef T &const_reference;
	static storage to_storage(const_reference t) { return &t; }
	static reference from_storage(storage t) { return *t; }
};

template <typename T>
class Maybe {
public:
	typedef MaybeTraits<T> traits;
	typedef typename traits::storage storage;
	typedef typename traits::reference reference;
	typedef typename traits::const_reference const_reference;

	class IsNothing : public std::domain_error {
		IsNothing()
		: domain_error(string("Is not ") + typeid(T).name()) {}
	};

	Maybe() : _is_nothing(true), _t() {}
	Maybe(const Maybe<T> &m) : _is_nothing(m._is_nothing), _t(m._t) {}

	Maybe(const_reference t)
	: _is_nothing(false), _t(traits::to_storage(t)) {}

	bool is_nothing() const { return _is_nothing; }

	operator reference() const throw(IsNothing) {
		if (_is_nothing) {
			throw IsNothing();
		} else {
			return traits::from_storage(_t);
		}
	}

private:
	bool _is_nothing;
	storage _t;
};

} /* namespace NR */

