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
#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include "knot.h"
#include "desktop-handles.h"
#include <vector>

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
	
	std::vector<std::pair<SPItem *,NR::Matrix> > items;
	
	NR::Point *spp;
	int spp_length;

	NR::Rect box;
	NR::Matrix current;
	NR::Point opposit;
	NR::Point origin;
	NR::Point point;
	NR::Point center;
	SPKnot *shandle[8];
	SPKnot *rhandle[8];
	SPKnot *chandle;
	SPCanvasItem *norm;
	SPCanvasItem *grip;
	SPCanvasItem *l[4];
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

void sp_sel_trans_init(SPSelTrans *seltrans, SPDesktop *desktop);
void sp_sel_trans_shutdown(SPSelTrans *seltrans);

void sp_sel_trans_reset_state(SPSelTrans *seltrans);
void sp_sel_trans_increase_state(SPSelTrans *seltrans);
void sp_sel_trans_set_center(SPSelTrans *seltrans, gdouble x, gdouble y);

void sp_sel_trans_grab(SPSelTrans *seltrans, NR::Point const p, gdouble x, gdouble y, gboolean show_handles);
void sp_sel_trans_transform(SPSelTrans *seltrans, NR::Matrix const &rel_affine, NR::Point const &norm);
void sp_sel_trans_ungrab(SPSelTrans *seltrans);
void sp_sel_trans_stamp(SPSelTrans *seltrans);

NRPoint *sp_sel_trans_point_desktop(SPSelTrans const *seltrans, NRPoint *p);
NRPoint *sp_sel_trans_origin_desktop(SPSelTrans const *seltrans, NRPoint *p);

inline NR::Point sp_sel_trans_point_desktop(SPSelTrans const *seltrans)
{
	return seltrans->point;
}


#endif
