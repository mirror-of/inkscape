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
sp_desktop_w2dt_affine (SPDesktop *desktop, NRMatrix *w2dt)
{
	int i;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (w2dt != NULL, NULL);

	for (i = 0; i < 6; i++) w2dt->c[i] = desktop->w2d[i];

	return w2dt;
}

NRMatrix *
sp_desktop_dt2w_affine (SPDesktop *desktop, NRMatrix *dt2w)
{
	int i;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (dt2w != NULL, NULL);

	for (i = 0; i < 6; i++) dt2w->c[i] = desktop->d2w[i];

	return dt2w;
}

NRMatrix *
sp_desktop_dt2doc_affine (SPDesktop *desktop, NRMatrix *dt2doc)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (dt2doc != NULL, NULL);

	nr_matrix_invert (dt2doc, (NRMatrix *) desktop->doc2dt);

	return dt2doc;
}

NRMatrix *
sp_desktop_doc2dt_affine (SPDesktop *desktop, NRMatrix *doc2dt)
{
	int i;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (doc2dt != NULL, NULL);

	for (i = 0; i < 6; i++) doc2dt->c[i] = desktop->doc2dt[i];

	return doc2dt;
}

NRMatrix *
sp_desktop_w2doc_affine (SPDesktop *desktop, NRMatrix *w2doc)
{
	NRMatrix dt2doc;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (w2doc != NULL, NULL);

	nr_matrix_invert (&dt2doc, (NRMatrix *) desktop->doc2dt);
	nr_matrix_multiply (w2doc, (NRMatrix *) desktop->w2d, &dt2doc);

	return w2doc;
}

NRMatrix *
sp_desktop_doc2w_affine (SPDesktop * desktop, NRMatrix *doc2w)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);
	g_return_val_if_fail (doc2w != NULL, NULL);

	nr_matrix_multiply (doc2w, (NRMatrix *) desktop->doc2dt, (NRMatrix *) desktop->d2w);

	return doc2w;
}

NRMatrix *
sp_desktop_root2dt_affine (SPDesktop *dt, NRMatrix *root2dt)
{
	SPRoot *root;

	root = SP_ROOT (SP_DOCUMENT_ROOT (SP_VIEW_DOCUMENT (dt)));

	nr_matrix_multiply (root2dt, &root->c2p, (NRMatrix *) dt->doc2dt);

	return root2dt;
}

NRMatrix *
sp_desktop_dt2root_affine (SPDesktop *dt, NRMatrix *dt2root)
{
	NRMatrix root2dt;

	sp_desktop_root2dt_affine (dt, &root2dt);

	nr_matrix_invert (dt2root, &root2dt);

	return dt2root;
}

NRPoint *
sp_desktop_w2d_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->w2d, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->w2d, x, y);

	return p;
}

NRPoint *
sp_desktop_d2w_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->d2w, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->d2w, x, y);

	return p;
}

NRPoint *
sp_desktop_d2doc_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
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
sp_desktop_doc2d_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
{
	g_return_val_if_fail (dt != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (dt), NULL);
	g_return_val_if_fail (p != NULL, NULL);

	p->x = NR_MATRIX_DF_TRANSFORM_X ((NRMatrix *) dt->doc2dt, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y ((NRMatrix *) dt->doc2dt, x, y);

	return p;
}

NRPoint *
sp_desktop_w2doc_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
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
sp_desktop_doc2w_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
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
sp_desktop_root2dt_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
{
	NRMatrix root2dt;

	sp_desktop_root2dt_affine (dt, &root2dt);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&root2dt, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&root2dt, x, y);

	return p;
}

NRPoint *
sp_desktop_dt2root_xy_point (SPDesktop *dt, NRPoint *p, float x, float y)
{
	NRMatrix dt2root;

	sp_desktop_dt2root_affine (dt, &dt2root);

	p->x = NR_MATRIX_DF_TRANSFORM_X (&dt2root, x, y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&dt2root, x, y);

	return p;
}

