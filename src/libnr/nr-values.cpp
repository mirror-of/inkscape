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

int vla;

double NR::Point::L2() {
	return hypot(pt[0], pt[1]);
}
