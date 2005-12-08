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

#ifndef SEEN_REMOVEOVERLAP_GENERATE_CONSTRAINTS_H
#define SEEN_REMOVEOVERLAP_GENERATE_CONSTRAINTS_H

class Rectangle {
private:
	double minX,maxX,minY,maxY;
public:
	static double xBorder,yBorder;
	Rectangle(double x, double X, double y, double Y);
	double getMaxX() { return maxX+xBorder; }
	double getMaxY() { return maxY+yBorder; }
	double getMinX() { return minX; }
	double getMinY() { return minY; }
	double getCentreX() { return minX+width()/2.0; }
	double getCentreY() { return minY+height()/2.0; }
	double width() { return getMaxX()-minX; }
	double height() { return getMaxY()-minY; }
	static void setXBorder(double x) {xBorder=x;}
	static void setYBorder(double y) {yBorder=y;}
	void moveCentreX(double x) {
		moveMinX(x-width()/2.0);
	}
	void moveCentreY(double y) {
		moveMinY(y-height()/2.0);
	}
	void moveMinX(double x) {
		maxX=x+width()-xBorder;
		minX=x;
	}
	void moveMinY(double y) {
		maxY=y+height()-yBorder;
		minY=y;
	}
	inline double overlapX(Rectangle *r) {
		if (getCentreX() <= r->getCentreX() && r->minX < getMaxX())
			return getMaxX() - r->minX;
		if (r->getCentreX() <= getCentreX() && minX < r->getMaxX())
			return r->getMaxX() - minX;
		return 0;
	}
	inline double overlapY(Rectangle *r) {
		if (getCentreY() <= r->getCentreY() && r->minY < getMaxY())
			return getMaxY() - r->minY;
		if (r->getCentreY() <= getCentreY() && minY < r->getMaxY())
			return r->getMaxY() - minY;
		return 0;
	}
};

class Variable;
class Constraint;

// returns number of constraints generated
int generateXConstraints(Rectangle *rs[], double weights[], const int n, Variable **&vs, Constraint **&cs);

int generateYConstraints(Rectangle *rs[], double weights[], const int n, Variable **&vs, Constraint **&cs);

#endif // SEEN_REMOVEOVERLAP_GENERATE_CONSTRAINTS_H
