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

gint sp_bezier_fit_cubic (NR::Point *bezier, const NR::Point *data, gint len, gdouble error);

gint sp_bezier_fit_cubic_r (NR::Point *bezier, const NR::Point *data, gint len, gdouble error, gint max_depth);

gint sp_bezier_fit_cubic_full (NR::Point *bezier, NR::Point const data[], gint len,
			       NR::Point const &tHat1, NR::Point const &tHat2, gdouble error, gint max_depth);

#endif /* __SP_BEZIER_UTILS_H__ */
