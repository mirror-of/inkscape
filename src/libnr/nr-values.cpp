#define __NR_VALUES_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This code is in public domain
 * This code is more connected with nr-types.h, but this file has
 * become a catchall for point operators
 */

#include "nr-values.h"
#include <math.h>
#include "nr-macros.h"

double NR::Point::L1() const {
	double d = 0;
	for(int i = 0; i < 2; i++)
		d += fabs(pt[i]);
	return d;
}

double NR::Point::L2() const {
	return hypot(pt[0], pt[1]);
}

double NR::Point::Linfty() const {
	double d = 0;
	for(int i = 0; i < 2; i++)
		d = MAX(d, fabs(pt[i]));
	return d;
}

inline NR::Point abs(NR::Point const &b) {
	NR::Point ret;
	for(int i = 0; i < 2; i++) {
		ret.pt[i] = fabs(b.pt[i]);
	}
	return ret;
}

