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

NR::Coord NR::Point::L1() const {
	NR::Coord d = 0;
	for(int i = 0; i < 2; i++)
		d += fabs(pt[i]);
	return d;
}

NR::Coord NR::Point::L2() const {
	return hypot(pt[0], pt[1]);
}

NR::Coord NR::Point::Linfty() const {
	NR::Coord d = 0;
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


/*
The following predefined objects are for reference
and comparison.
*/
NRMatrix NR_MATRIX_IDENTITY =
       {{1.0, 0.0, 0.0, 1.0, 0.0, 0.0}};
NRRect   NR_RECT_EMPTY =
       {NR_HUGE, NR_HUGE, -NR_HUGE, -NR_HUGE};
NRRectL  NR_RECT_L_EMPTY =
       {NR_HUGE_L, NR_HUGE_L, -NR_HUGE_L, -NR_HUGE_L};
NRRectL  NR_RECT_S_EMPTY =
       {NR_HUGE_S, NR_HUGE_S, -NR_HUGE_S, -NR_HUGE_S};


