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

struct SPSVGLength {
	unsigned int set : 1;
	unsigned int unit : 4;
	float value;
	float computed;
};

#endif
