/*
 *  geom.h
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include "libnr/nr-types.h"

extern NR::Coord L1(const NR::Point p);
/** Compute the L1 norm, or manhattan distance, of this vector */
extern NR::Coord L2(const NR::Point p);
/** Compute the L2 or euclidean norm of this vector */
extern NR::Coord Linfty(const NR::Point p);
/** Compute the L infinity or maximum norm of this vector */

typedef enum sp_intersector_kind{
	intersects = 0,
	parallel,
	coincident,
	no_intersection
} sp_intersector_kind;

/* Define here various primatives, such as line, line segment, circle, bezier path etc. */



/* intersectors */

/* Do p0, p1, p2 move in an anticlockwise direction, or clockwise? */
static int sp_intersector_ccw(const NR::Point p0, const NR::Point p1, const NR::Point p2);

sp_intersector_kind sp_intersector_line_intersection(const NR::Point n0, const double d0, const NR::Point n1, const double d1, NR::Point& result);

int
sp_intersector_segment_intersectp(const NR::Point p00, const NR::Point p01, const NR::Point p10, const NR::Point p11);

sp_intersector_kind
sp_intersector_segment_intersect(const NR::Point p00, const NR::Point p01, const NR::Point p10, const NR::Point p11, NR::Point& result);
