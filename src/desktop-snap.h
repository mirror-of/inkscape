#ifndef __SP_DESKTOP_SNAP_H__
#define __SP_DESKTOP_SNAP_H__

/*
 * Snap distance calculation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "desktop.h"

/* Single point methods */
double sp_desktop_free_snap (SPDesktop const *desktop, NR::Point &req);
double sp_desktop_vector_snap (SPDesktop const *desktop, NR::Point &req, NR::Point const &d);

gdouble sp_desktop_dim_snap(SPDesktop const *dt, NR::Point& req, NR::Dim2 const dim);

/* List of points methods */

NR::Coord sp_desktop_vector_snap_list(SPDesktop const *desktop, NR::Point const p[], int const length,
				      NR::Point const &norm, NR::scale const &s);

NR::Coord sp_desktop_dim_snap_list(SPDesktop const *desktop, NR::Point const p[], int const length,
				   double const dx, unsigned const dim);

NR::Coord sp_desktop_dim_snap_list_scale(SPDesktop const *desktop, NR::Point const p[], int const length,
					 NR::Point const &norm, double const sx, NR::Dim2 const dim);

NR::Coord sp_desktop_dim_snap_list_skew(SPDesktop const *desktop, NR::Point const p[], int const length,
					NR::Point const &norm, double const sx, NR::Dim2 const dim);

// These little functions are only here to provide an edge between NR::Point-land and NR::Point-land.

NR::Coord sp_desktop_dim_snap_list(SPDesktop const *desktop, NR::Point const p[], int const length,
				   double const dx, NR::Dim2 const dim);


/* Single point methods */
double sp_desktop_free_snap (SPDesktop const *desktop, NR::Point *req);
double sp_desktop_vector_snap (SPDesktop const *desktop, NR::Point *req, NR::Coord dx, NR::Coord dy);

#endif
