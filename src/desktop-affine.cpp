#define __SP_DESKTOP_AFFINE_C__

/*
 * Editable view and widget implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-matrix.h>
#include "desktop.h"
#include "document.h"
#include "sp-root.h"
#include "desktop-affine.h"

NRMatrix *
sp_desktop_w2dt_affine (SPDesktop const *desktop, NRMatrix *w2dt)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (w2dt != NULL, NULL);

	*w2dt = desktop->w2d;

	return w2dt;
}

NRMatrix *
sp_desktop_dt2w_affine (SPDesktop const *desktop, NRMatrix *dt2w)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (dt2w != NULL, NULL);

	*dt2w = desktop->d2w;

	return dt2w;
}

NRMatrix *
sp_desktop_dt2doc_affine (SPDesktop const *desktop, NRMatrix *dt2doc)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (dt2doc != NULL, NULL);

	nr_matrix_invert (dt2doc, (NRMatrix *) desktop->doc2dt);

	return dt2doc;
}

NRMatrix *
sp_desktop_doc2dt_affine (SPDesktop const *desktop, NRMatrix *doc2dt)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (doc2dt != NULL, NULL);

	*doc2dt = desktop->doc2dt;

	return doc2dt;
}

NRMatrix *
sp_desktop_w2doc_affine (SPDesktop const *desktop, NRMatrix *w2doc)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (w2doc != NULL, NULL);

	*w2doc = desktop->w2d * desktop->doc2dt.inverse();

	return w2doc;
}

NRMatrix *
sp_desktop_doc2w_affine (SPDesktop const * desktop, NRMatrix *doc2w)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (doc2w != NULL, NULL);

	*doc2w = desktop->doc2dt * desktop->d2w;

	return doc2w;
}

NRMatrix *
sp_desktop_root2dt_affine (SPDesktop const *dt, NRMatrix *root2dt)
{
	SPRoot *root = SP_ROOT (SP_DOCUMENT_ROOT (SP_VIEW_DOCUMENT (dt)));

	*root2dt = root->c2p * dt->doc2dt;

	return root2dt;
}

NRMatrix *
sp_desktop_dt2root_affine (SPDesktop const *dt, NRMatrix *dt2root)
{
	*dt2root = sp_desktop_root2dt_affine (dt).inverse();

	return dt2root;
}

NRPoint *
sp_desktop_w2d_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->w2d, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->w2d, x, y);

	return p;
}

NRPoint *
sp_desktop_d2w_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->d2w, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->d2w, x, y);

	return p;
}

NRPoint *
sp_desktop_d2doc_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	NRMatrix dt2doc;

	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	nr_matrix_invert (&dt2doc, (NRMatrix *) dt->doc2dt);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2doc, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2doc, x, y);

	return p;
}

NRPoint *
sp_desktop_doc2d_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->doc2dt, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->doc2dt, x, y);

	return p;
}

NRPoint *
sp_desktop_w2doc_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	NRMatrix dt2doc;
	double dtx, dty;

	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	nr_matrix_invert (&dt2doc, (NRMatrix *) dt->doc2dt);

	dtx = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->w2d, x, y);
	dty = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->w2d, x, y);
	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2doc, dtx, dty);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2doc, dtx, dty);

	return p;
}

NRPoint *
sp_desktop_doc2w_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	double dtx, dty;

	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	dtx = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->doc2dt, x, y);
	dty = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->doc2dt, x, y);
	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->d2w, dtx, dty);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->d2w, dtx, dty);

	return p;
}

NRPoint *
sp_desktop_root2dt_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	NRMatrix root2dt;

	sp_desktop_root2dt_affine (dt, &root2dt);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&root2dt, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&root2dt, x, y);

	return p;
}

NRPoint *
sp_desktop_dt2root_xy_point (SPDesktop const *dt, NRPoint *p, double x, double y)
{
	NRMatrix dt2root;

	sp_desktop_dt2root_affine (dt, &dt2root);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2root, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2root, x, y);

	return p;
}



NR::Matrix const sp_desktop_w2dt_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->w2d;
}

NR::Matrix const sp_desktop_dt2w_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->d2w;
}

NR::Matrix const sp_desktop_dt2doc_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->doc2dt.inverse();
}

NR::Matrix const sp_desktop_doc2dt_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->doc2dt;
}

NR::Matrix const sp_desktop_w2doc_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->w2d * desktop->doc2dt.inverse();
}

NR::Matrix const sp_desktop_doc2w_affine (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->doc2dt * desktop->d2w;
}

NR::Matrix const sp_desktop_root2dt_affine (SPDesktop const *dt)
{
	const SPRoot *root = SP_ROOT (SP_DOCUMENT_ROOT (SP_VIEW_DOCUMENT (dt)));
	
	return root->c2p * dt->doc2dt;
}

NR::Matrix const sp_desktop_dt2root_affine (SPDesktop const *dt)
{
	return sp_desktop_root2dt_affine (dt).inverse();
}



NR::Point sp_desktop_w2d_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);
	assert (SP_IS_DESKTOP (dt));

	return dt->w2d * p;
}

NR::Point sp_desktop_d2w_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);
	assert (SP_IS_DESKTOP (dt));

	return dt->d2w * p;
}

NR::Point sp_desktop_d2doc_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);
	assert (SP_IS_DESKTOP (dt));

	return dt->doc2dt.inverse() * p;
}

NR::Point sp_desktop_doc2d_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);
	assert (SP_IS_DESKTOP (dt));

	return dt->doc2dt * p;
}

NR::Point sp_desktop_w2doc_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);
	assert (SP_IS_DESKTOP (dt));

	return dt->doc2dt.inverse() * (dt->w2d * p);
}

NR::Point sp_desktop_doc2w_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);
	assert (SP_IS_DESKTOP (dt));

	return dt->d2w * (dt->doc2dt * p);
}

NR::Point sp_desktop_root2dt_xy_point (SPDesktop const *dt, const NR::Point p)
{
	return sp_desktop_root2dt_affine(dt) * p;
}

NR::Point sp_desktop_dt2root_xy_point (SPDesktop const *dt, const NR::Point p)
{
	return sp_desktop_dt2root_affine (dt) * p;
}

