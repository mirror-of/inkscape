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
#include <libnr/nr-matrix-div.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-matrix-ops.h>
#include "desktop.h"
#include "document.h"
#include "sp-root.h"
#include "desktop-affine.h"

NR::Matrix const sp_desktop_w2dt_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->w2d;
}

NR::Matrix const sp_desktop_dt2w_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->d2w;
}

NR::Matrix const sp_desktop_dt2doc_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->doc2dt.inverse();
}

NR::Matrix const sp_desktop_doc2dt_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->doc2dt;
}

NR::Matrix const sp_desktop_w2doc_affine (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->w2d / desktop->doc2dt;
}

NR::Matrix const sp_desktop_doc2w_affine(SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->doc2dt * desktop->d2w;
}

NR::Matrix const sp_desktop_root2dt_affine (SPDesktop const *dt)
{
	SPRoot const *root = SP_ROOT(SP_DOCUMENT_ROOT(dt->doc()));
	return root->c2p * dt->doc2dt;
}

NR::Matrix const sp_desktop_dt2root_affine (SPDesktop const *dt)
{
	return sp_desktop_root2dt_affine(dt).inverse();
}



NR::Point sp_desktop_w2d_xy_point(SPDesktop const *dt, NR::Point const p)
{
	assert (dt != NULL);

	return p * dt->w2d;
}

NR::Point sp_desktop_d2w_xy_point(SPDesktop const *dt, NR::Point const p)
{
	assert (dt != NULL);

	return p * dt->d2w;
}

NR::Point sp_desktop_d2doc_xy_point(SPDesktop const *dt, NR::Point const p)
{
	assert (dt != NULL);

	return p / dt->doc2dt;
}

NR::Point sp_desktop_doc2d_xy_point(SPDesktop const *dt, NR::Point const p)
{
	assert (dt != NULL);

	return p * dt->doc2dt;
}

NR::Point sp_desktop_w2doc_xy_point (SPDesktop const *dt, const NR::Point p)
{
	assert (dt != NULL);

	return p * dt->w2d / dt->doc2dt;
}

NR::Point sp_desktop_doc2w_xy_point(SPDesktop const *dt, NR::Point const p)
{
	assert (dt != NULL);

	return p * dt->doc2dt * dt->d2w;
}

NR::Point sp_desktop_root2dt_xy_point(SPDesktop const *dt, NR::Point const p)
{
	return p * sp_desktop_root2dt_affine(dt);
}

NR::Point sp_desktop_dt2root_xy_point(SPDesktop const *dt, NR::Point const p)
{
	return p * sp_desktop_dt2root_affine(dt);
}

