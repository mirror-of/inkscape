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

#include "gc-managed.h"

namespace Inkscape {
namespace XML {

struct NodeEventVector;

struct NodeListener : public Inkscape::GC::Managed<> {
	NodeListener(NodeEventVector const *v, void *d,
	               NodeListener *n=NULL)
	: next(n), vector(v), data(d) {}

	NodeListener(NodeListener const &listener, NodeListener *n=NULL)
	: next(n), vector(listener.vector), data(listener.data) {}

	NodeListener *next;
	const NodeEventVector *vector;
	void * data;
};

}
}

#endif
