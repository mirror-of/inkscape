#ifndef __SP_BEZIER_UTILS_H__
#define __SP_BEZIER_UTILS_H__

/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 *
 * Authors:
 *   Philip J. Schneider
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1990 Philip J. Schneider
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */

#include <libnr/nr-types.h>
#include <glib.h>

/* Bezier approximation utils */

gint sp_bezier_fit_cubic (NRPoint *bezier, const NRPoint *data, gint len, gdouble error);

gint sp_bezier_fit_cubic_r (NRPoint *bezier, const NRPoint *data, gint len, gdouble error, gint max_depth);

gint sp_bezier_fit_cubic_full (NRPoint *bezier, NRPoint const *data, gint len,
			       NRPoint const *tHat1, NRPoint const *tHat2, gdouble error, gint max_depth);


/* Data array */

void sp_darray_left_tangent (const NRPoint *d, int first, int length, NRPoint *tHat1);
void sp_darray_right_tangent (const NRPoint *d, int last, int length, NRPoint *tHat2);
void sp_darray_center_tangent (const NRPoint *d, gint center, NRPoint *tHatCenter);

#endif /* __SP_BEZIER_UTILS_H__ */
