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

gdouble sp_desktop_horizontal_snap(SPDesktop const *dt, NRPoint* req);

gdouble sp_desktop_vertical_snap(SPDesktop const *dt, NRPoint* req);

/* List of points methods */
double sp_desktop_dim_snap_list_skew(SPDesktop const *desktop, NRPoint const p[], int const length, NRPoint const *norm, double const sy, unsigned const dim);

double sp_desktop_vector_snap_list(SPDesktop const *desktop, NR::Point const p[], const int length, const NR::Point& norm, double sx, const double sy);

// These little functions are only here to provide an edge between NRPoint-land and NR::Point-land.

double sp_desktop_dim_snap_list(SPDesktop const *desktop, NRPoint const p[], int const length,
				double const dx, unsigned const dim);

double sp_desktop_horizontal_snap_list_scale(SPDesktop const *desktop, NRPoint const p[], int const length, NRPoint const *norm, double const sx);
double sp_desktop_vertical_snap_list_scale(SPDesktop const *desktop, NRPoint const p[], int const length, NRPoint const *norm, double const sx);

double sp_desktop_vector_snap_list (SPDesktop const *desktop, NRPoint const p[], int const length, NRPoint const *norm, double const sx, double const sy);

double sp_desktop_horizontal_snap_list_skew(SPDesktop const *desktop, NRPoint const p[], int const length, NRPoint const *norm, double const sx);
double sp_desktop_vertical_snap_list_skew(SPDesktop const *desktop, NRPoint const p[], int const length, NRPoint const *norm, double const sx);

/* Single point methods */
double sp_desktop_free_snap (SPDesktop const *desktop, NRPoint *req);
double sp_desktop_vector_snap (SPDesktop const *desktop, NRPoint *req, double dx, double dy);

#endif
