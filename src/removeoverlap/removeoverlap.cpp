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
void removeoverlap(GSList const *items, double xGap, double yGap) {
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
		NR::Rect item_box = sp_item_bbox_desktop (*it);
		NR::Point min = item_box.min();
		NR::Point max = item_box.max();
		rs[i++]=new Rectangle(min[NR::X],max[NR::X],min[NR::Y],max[NR::Y]);
	}
	removeRectangleOverlap(rs,n,xGap,yGap);
	i=0;
	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		NR::Rect item_box = sp_item_bbox_desktop (*it);
		Rectangle *r=rs[i++];

		NR::Point min = item_box.min();
		double x = r->getMinX() - min[NR::X];
		double y = r->getMinY() - min[NR::Y];
		NR::Point t = NR::Point (x, y);
		sp_item_move_rel(*it, NR::translate(t));
		delete r;
	}
	delete [] rs;
}
