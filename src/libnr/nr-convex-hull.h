#ifndef SEEN_NR_CONVEX_HULL_H
#define SEEN_NR_CONVEX_HULL_H

/* ex:set et ts=4 sw=4: */

/*
 * A class representing the convex hull of a set of points.
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <stdexcept>
#include <libnr/nr-rect.h>
#include <libnr/nr-maybe.h>

namespace NR {

class ConvexHull {
public:
	ConvexHull(const Point &p) : _bounds(p, p) {}
	ConvexHull(const ConvexHull &h) : _bounds(h._bounds) {}
													
    Point const &midpoint() const {
        return _bounds.midpoint();
    }

	void add(const Point &p) {
		_bounds.expandTo(p);
	}
	void add(const ConvexHull &h) {
		_bounds.expandTo(h._bounds);
	}
		
	Rect const &getBounds() const {
		return _bounds;
	}
	
private:
	Rect _bounds;
};

} /* namespace NR */

#endif
