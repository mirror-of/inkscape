#ifndef __NR_STROKE_H__
#define __NR_STROKE_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-path.h>
#include <libnr/nr-svp.h>

enum {
	NR_STROKE_CAP_BUTT,
	NR_STROKE_CAP_ROUND,
	NR_STROKE_CAP_SQUARE
};

enum {
	NR_STROKE_JOIN_MITER,
	NR_STROKE_JOIN_ROUND,
	NR_STROKE_JOIN_BEVEL
};

NRSVL *nr_bpath_stroke (const NRBPath *path, NRMatrixF *transform,
			float width,
			unsigned int cap, unsigned int join, float miterlimit,
			float flatness);

NRSVL *nr_vpath_stroke (const ArtVpath *path, NRMatrixF *transform,
			float width,
			unsigned int cap, unsigned int join, float miterlimit,
			float flatness);

#endif
