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

NRMatrixD *sp_desktop_w2dt_affine (SPDesktop *desktop, NRMatrixD *w2dt);
NRMatrixD *sp_desktop_dt2w_affine (SPDesktop *desktop, NRMatrixD *dt2w);

NRMatrixD *sp_desktop_dt2doc_affine (SPDesktop *desktop, NRMatrixD *d2doc);
NRMatrixD *sp_desktop_doc2dt_affine (SPDesktop *desktop, NRMatrixD *doc2d);

NRMatrixD *sp_desktop_w2doc_affine (SPDesktop *desktop, NRMatrixD *w2doc);
NRMatrixD *sp_desktop_doc2w_affine (SPDesktop *desktop, NRMatrixD *doc2w);

NRMatrixD *sp_desktop_root2dt_affine (SPDesktop *dt, NRMatrixD *root2dt);
NRMatrixD *sp_desktop_dt2root_affine (SPDesktop *dt, NRMatrixD *dt2root);

NRPointF *sp_desktop_w2d_xy_point (SPDesktop *desktop, NRPointF *p, float x, float y);
NRPointF *sp_desktop_d2w_xy_point (SPDesktop *desktop, NRPointF *p, float x, float y);
NRPointF *sp_desktop_w2doc_xy_point (SPDesktop *desktop, NRPointF *p, float x, float y);
NRPointF *sp_desktop_doc2w_xy_point (SPDesktop *desktop, NRPointF *p, float x, float y);
NRPointF *sp_desktop_d2doc_xy_point (SPDesktop *desktop, NRPointF *p, float x, float y);
NRPointF *sp_desktop_doc2d_xy_point (SPDesktop *desktop, NRPointF *p, float x, float y);

NRPointF *sp_desktop_root2dt_xy_point (SPDesktop *dt, NRPointF *p, float x, float y);
NRPointF *sp_desktop_dt2root_xy_point (SPDesktop *dt, NRPointF *p, float x, float y);

#endif
