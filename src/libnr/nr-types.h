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

typedef struct _NRMatrixD NRMatrixD;
typedef struct _NRMatrixF NRMatrixF;
typedef struct _NRPointD NRPointD;
typedef struct _NRPointF NRPointF;
typedef struct _NRPointL NRPointL;
typedef struct _NRPointS NRPointS;
typedef struct _NRRectD NRRectD;
typedef struct _NRRectF NRRectF;
typedef struct _NRRectL NRRectL;
typedef struct _NRRectS NRRectS;

struct _NRMatrixD {
	double c[6];
};

struct _NRMatrixF {
	float c[6];
};

struct _NRPointD {
	double x, y;
};

struct _NRPointF {
	float x, y;
};

struct _NRPointL {
	NRLong x, y;
};

struct _NRPointS {
	NRShort x, y;
};

struct _NRRectD {
	double x0, y0, x1, y1;
};

struct _NRRectF {
	float x0, y0, x1, y1;
};

struct _NRRectL {
	NRLong x0, y0, x1, y1;
};

struct _NRRectS {
	NRShort x0, y0, x1, y1;
};

#endif
