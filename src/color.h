#ifndef __SP_COLOR_H__
#define __SP_COLOR_H__

/*
 * Colors and colorspaces
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-types.h>
#include "forward.h"

/* Useful composition macros */

#define SP_RGBA32_R_U(v) (((v) >> 24) & 0xff)
#define SP_RGBA32_G_U(v) (((v) >> 16) & 0xff)
#define SP_RGBA32_B_U(v) (((v) >> 8) & 0xff)
#define SP_RGBA32_A_U(v) ((v) & 0xff)
#define SP_COLOR_U_TO_F(v) ((v) / 255.0)
#define SP_COLOR_F_TO_U(v) ((unsigned int) ((v) * 255.9999))
#define SP_RGBA32_R_F(v) SP_COLOR_U_TO_F (SP_RGBA32_R_U (v))
#define SP_RGBA32_G_F(v) SP_COLOR_U_TO_F (SP_RGBA32_G_U (v))
#define SP_RGBA32_B_F(v) SP_COLOR_U_TO_F (SP_RGBA32_B_U (v))
#define SP_RGBA32_A_F(v) SP_COLOR_U_TO_F (SP_RGBA32_A_U (v))
#define SP_RGBA32_U_COMPOSE(r,g,b,a) ((((r) & 0xff) << 24) | (((g) & 0xff) << 16) | (((b) & 0xff) << 8) | ((a) & 0xff))
#define SP_RGBA32_F_COMPOSE(r,g,b,a) SP_RGBA32_U_COMPOSE (SP_COLOR_F_TO_U (r), SP_COLOR_F_TO_U (g), SP_COLOR_F_TO_U (b), SP_COLOR_F_TO_U (a))

typedef enum {
	SP_COLORSPACE_CLASS_INVALID,
	SP_COLORSPACE_CLASS_NONE,
	SP_COLORSPACE_CLASS_UNKNOWN,
	SP_COLORSPACE_CLASS_PROCESS,
	SP_COLORSPACE_CLASS_SPOT
} SPColorSpaceClass;

typedef enum {
	SP_COLORSPACE_TYPE_INVALID,
	SP_COLORSPACE_TYPE_NONE,
	SP_COLORSPACE_TYPE_UNKNOWN,
	SP_COLORSPACE_TYPE_RGB,
	SP_COLORSPACE_TYPE_CMYK
} SPColorSpaceType;

struct _SPColor {
	const SPColorSpace *colorspace;
	union {
		float c[4];
	} v;
};

SPColorSpaceClass sp_color_get_colorspace_class (const SPColor *color);
SPColorSpaceType sp_color_get_colorspace_type (const SPColor *color);

void sp_color_copy (SPColor *dst, const SPColor *src);

unsigned int sp_color_is_equal (SPColor *c0, SPColor *c1);

void sp_color_set_rgb_float (SPColor *color, float r, float g, float b);
void sp_color_set_rgb_rgba32 (SPColor *color, NRULong value);

void sp_color_set_cmyk_float (SPColor *color, float c, float m, float y, float k);

NRULong sp_color_get_rgba32_ualpha (const SPColor *color, NRULong alpha);
NRULong sp_color_get_rgba32_falpha (const SPColor *color, float alpha);

void sp_color_get_rgb_floatv (const SPColor *color, float *rgb);
void sp_color_get_cmyk_floatv (const SPColor *color, float *cmyk);

/* Plain mode helpers */

void sp_color_rgb_to_hsv_floatv (float *hsv, float r, float g, float b);
void sp_color_hsv_to_rgb_floatv (float *rgb, float h, float s, float v);
void sp_color_rgb_to_cmyk_floatv (float *cmyk, float r, float g, float b);
void sp_color_cmyk_to_rgb_floatv (float *rgb, float c, float m, float y, float k);

#endif

