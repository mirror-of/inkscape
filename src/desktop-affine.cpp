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

NRMatrixD *
sp_desktop_w2dt_affine (SPDesktop *desktop, NRMatrixD *w2dt)
{
	int i;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (w2dt != NULL, NULL);

	for (i = 0; i < 6; i++) w2dt->c[i] = desktop->w2d[i];

	return w2dt;
}

NRMatrixD *
sp_desktop_dt2w_affine (SPDesktop *desktop, NRMatrixD *dt2w)
{
	int i;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (dt2w != NULL, NULL);

	for (i = 0; i < 6; i++) dt2w->c[i] = desktop->d2w[i];

	return dt2w;
}

NRMatrixD *
sp_desktop_dt2doc_affine (SPDesktop *desktop, NRMatrixD *dt2doc)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (dt2doc != NULL, NULL);

	nr_matrix_d_invert (dt2doc, (NRMatrixD *) desktop->doc2dt);

	return dt2doc;
}

NRMatrixD *
sp_desktop_doc2dt_affine (SPDesktop *desktop, NRMatrixD *doc2dt)
{
	int i;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (doc2dt != NULL, NULL);

	for (i = 0; i < 6; i++) doc2dt->c[i] = desktop->doc2dt[i];

	return doc2dt;
}

NRMatrixD *
sp_desktop_w2doc_affine (SPDesktop *desktop, NRMatrixD *w2doc)
{
	NRMatrixD dt2doc;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (w2doc != NULL, NULL);

	nr_matrix_d_invert (&dt2doc, (NRMatrixD *) desktop->doc2dt);
	nr_matrix_multiply_ddd (w2doc, (NRMatrixD *) desktop->w2d, &dt2doc);

	return w2doc;
}

NRMatrixD *
sp_desktop_doc2w_affine (SPDesktop * desktop, NRMatrixD *doc2w)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (doc2w != NULL, NULL);

	nr_matrix_multiply_ddd (doc2w, (NRMatrixD *) desktop->doc2dt, (NRMatrixD *) desktop->d2w);

	return doc2w;
}

NRMatrixD *
sp_desktop_root2dt_affine (SPDesktop *dt, NRMatrixD *root2dt)
{
	SPRoot *root;

	root = SP_ROOT (SP_DOCUMENT_ROOT (SP_VIEW_DOCUMENT (dt)));

	nr_matrix_multiply_ddd (root2dt, &root->c2p, (NRMatrixD *) dt->doc2dt);

	return root2dt;
}

NRMatrixD *
sp_desktop_dt2root_affine (SPDesktop *dt, NRMatrixD *dt2root)
{
	NRMatrixD root2dt;

	sp_desktop_root2dt_affine (dt, &root2dt);

	nr_matrix_d_invert (dt2root, &root2dt);

	return dt2root;
}

NRPointF *
sp_desktop_w2d_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrixD *) dt->w2d, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrixD *) dt->w2d, x, y);

	return p;
}

NRPointF *
sp_desktop_d2w_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrixD *) dt->d2w, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrixD *) dt->d2w, x, y);

	return p;
}

NRPointF *
sp_desktop_d2doc_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	NRMatrixD dt2doc;

	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	nr_matrix_d_invert (&dt2doc, (NRMatrixD *) dt->doc2dt);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2doc, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2doc, x, y);

	return p;
}

NRPointF *
sp_desktop_doc2d_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrixD *) dt->doc2dt, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrixD *) dt->doc2dt, x, y);

	return p;
}

NRPointF *
sp_desktop_w2doc_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	NRMatrixD dt2doc;
	double dtx, dty;

	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	nr_matrix_d_invert (&dt2doc, (NRMatrixD *) dt->doc2dt);

	dtx = NR_MATRIX_DF_TRANSFORM_X ((NRMatrixD *) dt->w2d, x, y);
	dty = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrixD *) dt->w2d, x, y);
	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2doc, dtx, dty);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2doc, dtx, dty);

	return p;
}

NRPointF *
sp_desktop_doc2w_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	double dtx, dty;

	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	dtx = NR_MATRIX_DF_TRANSFORM_X ((NRMatrixD *) dt->doc2dt, x, y);
	dty = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrixD *) dt->doc2dt, x, y);
	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrixD *) dt->d2w, dtx, dty);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrixD *) dt->d2w, dtx, dty);

	return p;
}

NRPointF *
sp_desktop_root2dt_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	NRMatrixD root2dt;

	sp_desktop_root2dt_affine (dt, &root2dt);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&root2dt, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&root2dt, x, y);

	return p;
}

NRPointF *
sp_desktop_dt2root_xy_point (SPDesktop *dt, NRPointF *p, float x, float y)
{
	NRMatrixD dt2root;

	sp_desktop_dt2root_affine (dt, &dt2root);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2root, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2root, x, y);

	return p;
}

