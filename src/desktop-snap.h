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
double sp_desktop_free_snap (SPDesktop *desktop, NRPoint *req);
#define sp_desktop_horizontal_snap(dt,req) sp_desktop_vector_snap (dt, req, 1.0, 0.0);
#define sp_desktop_vertical_snap(dt,req) sp_desktop_vector_snap (dt, req, 0.0, 1.0);
double sp_desktop_vector_snap (SPDesktop *desktop, NRPoint *req, double dx, double dy);
double sp_desktop_circular_snap (SPDesktop *desktop, NRPoint *req, double cx, double cy);

/* List of points methods */
double sp_desktop_horizontal_snap_list (SPDesktop *desktop, NRPoint *p, int length, double dx);
double sp_desktop_vertical_snap_list (SPDesktop *desktop, NRPoint *p, int length, double dy);
double sp_desktop_horizontal_snap_list_scale (SPDesktop *desktop, NRPoint *p, int length, NRPoint *norm, double sx);
double sp_desktop_vertical_snap_list_scale (SPDesktop *desktop, NRPoint *p, int length, NRPoint *norm, double sy);
double sp_desktop_vector_snap_list (SPDesktop *desktop, NRPoint *p, int length, NRPoint *norm, double sx, double sy);
double sp_desktop_horizontal_snap_list_skew (SPDesktop *desktop, NRPoint *p, int length, NRPoint *norm, double sy);
double sp_desktop_vertical_snap_list_skew (SPDesktop *desktop, NRPoint *p, int length, NRPoint *norm, double sx);

NRMatrix *sp_desktop_circular_snap_list (SPDesktop *desktop, NRPoint *p, int length, NRPoint *norm, NRMatrix *rotate);



#endif
