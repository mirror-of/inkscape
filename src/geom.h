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

typedef enum sp_intersector_kind{
	intersects = 0,
	parallel,
	coincident,
	no_intersection
} sp_intersector_kind;

/* Define here various primatives, such as line, line segment, circle, bezier path etc. */



/* intersectors */

sp_intersector_kind sp_intersector_line_intersection(const NR::Point n0, const double d0, const NR::Point n1, const double d1, NR::Point& result);
