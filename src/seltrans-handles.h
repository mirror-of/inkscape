#ifndef __SP_SELTRANS_HANDLES_H__
#define __SP_SELTRANS_HANDLES_H__

/*
 * Seltrans knots
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "helper/sodipodi-ctrl.h"
#include "seltrans.h"

class SPSelTransHandle;

// request handlers
gboolean sp_sel_trans_scale_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
gboolean sp_sel_trans_stretch_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
gboolean sp_sel_trans_skew_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
gboolean sp_sel_trans_rotate_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
gboolean sp_sel_trans_center_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);

// action handlers
void sp_sel_trans_scale(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
void sp_sel_trans_stretch(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
void sp_sel_trans_skew(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
void sp_sel_trans_rotate(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
void sp_sel_trans_center(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);

struct SPSelTransHandle {
	GtkAnchorType anchor;
	GdkCursorType cursor;
	guint control;
	void (* action) (SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
	gboolean (* request) (SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &p, guint state);
	gdouble x, y;
};

extern SPSelTransHandle const handles_scale[8];
extern SPSelTransHandle const handles_rotate[8];
extern SPSelTransHandle const handle_center;

#endif

