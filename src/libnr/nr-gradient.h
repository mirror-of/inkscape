#ifndef __NR_GRADIENT_H__
#define __NR_GRADIENT_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

typedef struct _NRRGradientRenderer NRRGradientRenderer;

#include <libnr/nr-types.h>
#include <libnr/nr-render.h>

#define NR_GRADIENT_VECTOR_LENGTH 1024

enum {
	NR_GRADIENT_SPREAD_PAD,
	NR_GRADIENT_SPREAD_REFLECT,
	NR_GRADIENT_SPREAD_REPEAT
};

/* Radial */

struct _NRRGradientRenderer {
	NRRenderer renderer;
	const unsigned char *vector;
	unsigned int spread;
	NRMatrixF px2gs;
	float cx, cy;
	float fx, fy;
	float r;
	float C;
};

NRRenderer *nr_rgradient_renderer_setup (NRRGradientRenderer *rgr,
					 const unsigned char *cv,
					 unsigned int spread,
					 const NRMatrixF *gs2px,
					 float cx, float cy,
					 float fx, float fy,
					 float r);

#endif
