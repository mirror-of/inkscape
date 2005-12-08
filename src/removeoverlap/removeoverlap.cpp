/**
* \brief Remove overlaps function
*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
*
* Copyright (C) 2005 Authors
*
* Released under GNU GPL.  Read the file 'COPYING' for more information.
*/
#include <glib/gslist.h>
#include <stdio.h>
#include "../inkscape.h"
#include "../util/glib-list-iterators.h"
#include "../libnr/nr-rect.h"
#include "../libnr/nr-point.h"
#include <list>
#include "../sp-item.h"
#include "../sp-item-transform.h"
#include "generate-constraints.h"
#include "remove_rectangle_overlap.h"

/**
* Takes a list of inkscape items and moves them as little as possible
* such that rectangular bounding boxes are separated by at least xGap
* horizontally and yGap vertically
*/
void removeoverlap(GSList const *const items, double const xGap, double const yGap) {
	if(!items) {
		return;
	}

	using Inkscape::Util::GSListConstIterator;
	std::list<SPItem *> selected;
	selected.insert<GSListConstIterator<SPItem *> >(selected.end(), items, NULL);
	if (selected.empty()) return;
	int n=selected.size();

	//Check 2 or more selected objects
	if (n < 2) return;

	Rectangle **rs = new Rectangle*[n];
	int i=0;

	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		using NR::X; using NR::Y;
		NR::Rect const item_box(sp_item_bbox_desktop(*it));
		NR::Point const min(item_box.min());
		NR::Point const max(item_box.max());
		/* The current algorithm requires widths & heights to be strictly positive. */
		rs[i++] = new Rectangle(min[X], std::max(min[X] + 1, max[X] + xGap),
					min[Y], std::max(min[Y] + 1, max[Y] + yGap));		
	}
	removeRectangleOverlap(rs, n, 0.0, 0.0);
	i=0;
	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		NR::Rect const item_box(sp_item_bbox_desktop(*it));
		Rectangle *r = rs[i++];
		NR::Point const curr(item_box.min());
		NR::Point const dest(r->getMinX(),
				     r->getMinY());
		sp_item_move_rel(*it, NR::translate(dest - curr));
		delete r;
	}
	delete [] rs;
}
