#ifndef __SP_ENUMS_H__
#define __SP_ENUMS_H__

/*
 * Main program enumerated types
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* preserveAspectRatio */

enum {
	SP_ASPECT_NONE,
	SP_ASPECT_XMIN_YMIN,
	SP_ASPECT_XMID_YMIN,
	SP_ASPECT_XMAX_YMIN,
	SP_ASPECT_XMIN_YMID,
	SP_ASPECT_XMID_YMID,
	SP_ASPECT_XMAX_YMID,
	SP_ASPECT_XMIN_YMAX,
	SP_ASPECT_XMID_YMAX,
	SP_ASPECT_XMAX_YMAX
};

enum {
	SP_ASPECT_MEET,
	SP_ASPECT_SLICE
};

/* maskUnits */
/* maskContentUnits */

enum {
	SP_CONTENT_UNITS_USERSPACEONUSE,
	SP_CONTENT_UNITS_OBJECTBOUNDINGBOX
};

/* markerUnits */

enum {
	SP_MARKER_UNITS_STROKEWIDTH,
	SP_MARKER_UNITS_USERSPACEONUSE
};

/* stroke-linejoin */

enum {
	SP_STROKE_LINEJOIN_MITER,
	SP_STROKE_LINEJOIN_ROUND,
	SP_STROKE_LINEJOIN_BEVEL
};

/* stroke-linecap */

enum {
	SP_STROKE_LINECAP_BUTT,
	SP_STROKE_LINECAP_ROUND,
	SP_STROKE_LINECAP_SQUARE
};

/* fill-rule */
/* clip-rule */

enum {
	SP_WIND_RULE_NONZERO,
	SP_WIND_RULE_INVALID_1,
	SP_WIND_RULE_EVENODD
};

#endif

