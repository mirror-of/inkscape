#ifndef SEEN_INKSCAPE_XML_SP_REPR_LISTENER
#define SEEN_INKSCAPE_XML_SP_REPR_LISTENER

/*
 * Fuzzy DOM-like tree implementation
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski and Frank Felfe
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gc-object.h"

struct SPReprEventVector;

struct SPReprListener : public Inkscape::GC::Object<> {
	SPReprListener(SPReprEventVector const *v, void *d,
	               SPReprListener *n=NULL)
	: next(n), vector(v), data(d) {}

	SPReprListener(SPReprListener const &listener, SPReprListener *n=NULL)
	: next(n), vector(listener.vector), data(listener.data) {}

	SPReprListener *next;
	const SPReprEventVector *vector;
	void * data;
};

#endif
