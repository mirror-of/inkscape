#ifndef __SP_DESKTOP_AFFINE_H__
#define __SP_DESKTOP_AFFINE_H__

/*
 * Desktop transformations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-types.h>
#include "forward.h"

NRMatrix *sp_desktop_w2dt_affine (SPDesktop *desktop, NRMatrix *w2dt);
NRMatrix *sp_desktop_dt2w_affine (SPDesktop *desktop, NRMatrix *dt2w);

NRMatrix *sp_desktop_dt2doc_affine (SPDesktop *desktop, NRMatrix *d2doc);
NRMatrix *sp_desktop_doc2dt_affine (SPDesktop *desktop, NRMatrix *doc2d);

NRMatrix *sp_desktop_w2doc_affine (SPDesktop *desktop, NRMatrix *w2doc);
NRMatrix *sp_desktop_doc2w_affine (SPDesktop *desktop, NRMatrix *doc2w);

NRMatrix *sp_desktop_root2dt_affine (SPDesktop *dt, NRMatrix *root2dt);
NRMatrix *sp_desktop_dt2root_affine (SPDesktop *dt, NRMatrix *dt2root);

NRPoint *sp_desktop_w2d_xy_point (SPDesktop *desktop, NRPoint *p, double x, double y);
NRPoint *sp_desktop_d2w_xy_point (SPDesktop *desktop, NRPoint *p, double x, double y);
NRPoint *sp_desktop_w2doc_xy_point (SPDesktop *desktop, NRPoint *p, double x, double y);
NRPoint *sp_desktop_doc2w_xy_point (SPDesktop *desktop, NRPoint *p, double x, double y);
NRPoint *sp_desktop_d2doc_xy_point (SPDesktop *desktop, NRPoint *p, double x, double y);
NRPoint *sp_desktop_doc2d_xy_point (SPDesktop *desktop, NRPoint *p, double x, double y);

NRPoint *sp_desktop_root2dt_xy_point (SPDesktop *dt, NRPoint *p, double x, double y);
NRPoint *sp_desktop_dt2root_xy_point (SPDesktop *dt, NRPoint *p, double x, double y);

#endif
