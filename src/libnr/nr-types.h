#ifndef __NR_TYPES_H__
#define __NR_TYPES_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr_config.h>

typedef struct _NRMatrix {
	double c[6];
} NRMatrix;

typedef struct _NRPoint {
	double x, y;
} NRPoint;

typedef struct _NRPointL {
	NRLong x, y;
} NRPointL;

typedef struct _NRPointS {
	NRShort x, y;
} NRPointS;

typedef struct _NRRect {
	double x0, y0, x1, y1;
} NRRect;

typedef struct _NRRectL {
	NRLong x0, y0, x1, y1;
} NRRectL;

typedef struct _NRRectS {
	NRShort x0, y0, x1, y1;
} NRRectS;

#endif
