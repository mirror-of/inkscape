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

#include <sigc++/sigc++.h>
#include <libnr/nr-point.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include "desktop-handles.h"
#include "forward.h"
#include "knot.h"
#include "selcue.h"
#include <vector>

class SPSelTrans;

enum {
	SP_SELTRANS_SHOW_CONTENT,
	SP_SELTRANS_SHOW_OUTLINE
};

enum {
	SP_SELTRANS_STATE_SCALE,
	SP_SELTRANS_STATE_ROTATE
};

struct SPSelTrans {
	SPSelTrans(SPDesktop *desktop);
	~SPSelTrans();

	SPDesktop *desktop;

	SPSelection *selection;
	guint state : 1;
	guint show : 1;

	unsigned int grabbed : 1;
	unsigned int show_handles : 1;
	unsigned int empty : 1;
	unsigned int changed : 1;
	
	std::vector<std::pair<SPItem *,NR::Matrix> > items;
	
	std::vector<NR::Point> snap_points;
	std::vector<NR::Point> bbox_points;

	NR::Rect box;
	NR::Matrix current;
	NR::Point opposite; ///< opposite point to where a scale is taking place
	NR::Point origin; ///< position of origin for transforms
	NR::Point point; ///< original position of the knot being used for the current transform
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

	SPSelCue selcue;

	SigC::Connection _sel_changed_connection;
	SigC::Connection _sel_modified_connection;
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

void sp_sel_trans_reset_state(SPSelTrans *seltrans);
void sp_sel_trans_increase_state(SPSelTrans *seltrans);
void sp_sel_trans_set_center(SPSelTrans *seltrans, gdouble x, gdouble y);
void sp_sel_trans_update_item_bboxes (SPSelTrans *seltrans);

void sp_sel_trans_grab(SPSelTrans *seltrans, NR::Point const &p, gdouble x, gdouble y, gboolean show_handles);
void sp_sel_trans_transform(SPSelTrans *seltrans, NR::Matrix const &rel_affine, NR::Point const &norm);
void sp_sel_trans_ungrab(SPSelTrans *seltrans);
void sp_sel_trans_stamp(SPSelTrans *seltrans);

inline NR::Point sp_sel_trans_point_desktop(SPSelTrans const *seltrans) {
	return seltrans->point;
}

inline NR::Point sp_sel_trans_origin_desktop(SPSelTrans const *seltrans) {
	return seltrans->origin;
}

#endif
