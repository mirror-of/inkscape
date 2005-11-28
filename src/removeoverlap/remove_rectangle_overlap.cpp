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
void removeRectangleOverlap(Rectangle *rs[], const int n, const double xBorder=0, const double yBorder=0) {
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
	VPSC vpsc_x(vs,n,cs,m);
	double cost=vpsc_x.solve();
	for(int i=0;i<n;i++) {
		rs[i]->moveMinX(vs[i]->position());
	}
	delete [] vs;
	delete [] cs;
	Rectangle::setXBorder(Rectangle::xBorder-0.001);
	VPSC vpsc_y(vs,n,cs,m);
	m=generateYConstraints(rs,ws,n,vs,cs);
	cost=vpsc_y.solve();
	for(int i=0;i<n;i++) {
		rs[i]->moveMinY(vs[i]->position());
	}
	delete [] vs;
	delete [] cs;
	delete [] ws;
}
