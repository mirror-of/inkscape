#define __NR_VALUES_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-values.h"
#include <math.h>
#include "nr-macros.h"

int vla;

double NR::Point::L1() {
	double d = 0;
	for(int i = 0; i < 2; i++)
		d += fabs(pt[i]);
	return d;
}

double NR::Point::L2() {
	return hypot(pt[0], pt[1]);
}

double NR::Point::Linfty() {
	double d = 0;
	for(int i = 0; i < 2; i++)
		d = MAX(d, fabs(pt[i]));
	return d;
}
