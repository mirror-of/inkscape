#ifndef __SP_SELTRANS_H__
#define __SP_SELTRANS_H__

/*
 * Helper object for transforming selected items
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-types.h>
#include "knot.h"
#include "desktop-handles.h"

typedef struct _SPSelTrans SPSelTrans;

enum {
	SP_SELTRANS_SHOW_CONTENT,
	SP_SELTRANS_SHOW_OUTLINE
};

enum {
	SP_SELTRANS_TRANSFORM_OPTIMIZE,
	SP_SELTRANS_TRANSFORM_KEEP
};

enum {
	SP_SELTRANS_STATE_SCALE,
	SP_SELTRANS_STATE_ROTATE
};

#define SP_SELTRANS_SPP_SIZE 1024

struct _SPSelTrans {
	SPDesktop *desktop;
	SPSelection *selection;

	guint state : 1;
	guint show : 1;
	guint transform : 1;

	unsigned int grabbed : 1;
	unsigned int show_handles : 1;
	unsigned int empty : 1;
	unsigned int changed : 1;

	SPItem **items;
	NRMatrixF *transforms;
	int nitems;

	NRPointF *spp;
	int spp_length;

	NRRectD box;
        NRMatrixD current;
        NRPointD opposit;
        NRPointD origin;
	NRPointD point;
	NRPointD center;
	SPKnot *shandle[8];
	SPKnot *rhandle[8];
	SPKnot *chandle;
        SPCanvasItem *norm;
        SPCanvasItem *grip;
        SPCanvasItem *l1, *l2, *l3, *l4;
	guint sel_changed_id;
	guint sel_modified_id;
	GSList *stamp_cache;
};

/*
 * Logic
 *
 * grab - removes handles, makes unsensitive
 * ungrab - if changed, flushes, otherwise increases state, shows handles,
 *          makes sensitive
 * if changed or sel changed during grabbing, sets state to scale
 *
 */ 

void sp_sel_trans_init (SPSelTrans * seltrans, SPDesktop * desktop);
void sp_sel_trans_shutdown (SPSelTrans * seltrans);

void sp_sel_trans_reset_state (SPSelTrans * seltrans);
void sp_sel_trans_increase_state (SPSelTrans * seltrans);
void sp_sel_trans_set_center (SPSelTrans * seltrans, gdouble x, gdouble y);

void sp_sel_trans_grab (SPSelTrans * seltrans, NRPointF *p, gdouble x, gdouble y, gboolean show_handles);
void sp_sel_trans_transform (SPSelTrans * seltrans, NRMatrixD *affine, NRPointF *norm);
void sp_sel_trans_ungrab (SPSelTrans * seltrans);
void sp_sel_trans_stamp (SPSelTrans * seltrans);

NRPointF *sp_sel_trans_point_desktop (SPSelTrans *seltrans, NRPointF *p);
NRPointF *sp_sel_trans_origin_desktop (SPSelTrans * seltrans, NRPointF *p);


#endif
