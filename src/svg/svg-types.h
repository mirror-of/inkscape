#ifndef __SP_SVG_TYPES_H__
#define __SP_SVG_TYPES_H__

/*
 * SVG datatype containers
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

typedef struct _SPSVGLength SPSVGLength;

#define SP_SVG_PX_FROM_PT(v) ((v) / 1.25)
#define SP_SVG_PX_FROM_MM(v) ((v) / 3.543307)
#define SP_SVG_PT_FROM_PX(v) ((v) * 1.25)
#define SP_SVG_MM_FROM_PX(v) ((v) * 3.543307)

enum {
	SP_SVG_UNIT_NONE,
	SP_SVG_UNIT_PX,
	SP_SVG_UNIT_PT,
	SP_SVG_UNIT_PC,
	SP_SVG_UNIT_MM,
	SP_SVG_UNIT_CM,
	SP_SVG_UNIT_IN,
	SP_SVG_UNIT_EM,
	SP_SVG_UNIT_EX,
	SP_SVG_UNIT_PERCENT
};

struct _SPSVGLength {
	unsigned int set : 1;
	unsigned int unit : 4;
	float value;
	float computed;
};

#endif
