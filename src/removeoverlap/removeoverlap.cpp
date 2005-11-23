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
#include "generate-constraints.h"
#include "solve_VPSC.h"
#include "variable.h"

double Rectangle::xBorder=0;
double Rectangle::yBorder=0;
/**
 * Takes an array of n rectangles and moves them as little as possible
 * such that rectangles are separated by at least xBorder horizontally
 * and yBorder vertically
 */
void removeOverlaps(Rectangle *rs[], const int n, const double xBorder=0, const double yBorder=0) {
	// The extra gap avoids numerical imprecision problems
	Rectangle::setXBorder(xBorder+0.001);
	Rectangle::setYBorder(yBorder);
	double *ws=new double[n];
	for(int i=0;i<n;i++) {
		ws[i]=1;
	}
	Variable **vs;
	Constraint **cs;
	int m=generateXConstraints(rs,ws,n,vs,cs);
	double cost=solve_VPSC(vs,n,cs,m);
	for(int i=0;i<n;i++) {
		rs[i]->moveMinX(vs[i]->position());
	}
	delete [] vs;
	delete [] cs;
	Rectangle::setXBorder(Rectangle::xBorder-0.001);
	m=generateYConstraints(rs,ws,n,vs,cs);
	cost=solve_VPSC(vs,n,cs,m);
	for(int i=0;i<n;i++) {
		rs[i]->moveMinY(vs[i]->position());
	}
	delete [] vs;
	delete [] cs;
	delete [] ws;
}

//#ifndef REMOVEOVERLAP_STANDALONE_TEST
#include <glib/gslist.h>
#include <stdio.h>
#include "../inkscape.h"
#include "../util/glib-list-iterators.h"
#include "../libnr/nr-rect.h"
#include "../libnr/nr-point.h"
#include <list>
#include "../sp-item.h"
#include "../sp-item-transform.h"

/**
 * Takes a list of inkscape items and moves them as little as possible
 * such that rectangular bounding boxes are separated by at least xGap
 * horizontally and yGap vertically
 */
void removeoverlap(GSList *items, double xGap, double yGap) {
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
	removeOverlaps(rs,n,xGap,yGap);
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
//#endif
