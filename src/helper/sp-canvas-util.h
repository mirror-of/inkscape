#ifndef __SP_CANVAS_UTILS_H__
#define __SP_CANVAS_UTILS_H__

/*
 * Helper stuff for SPCanvas
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libart_lgpl/art_svp.h>
#include "sp-canvas.h"

/* Miscellaneous utility & convenience functions for general canvas objects */

void sp_canvas_update_bbox (SPCanvasItem *item, int x1, int y1, int x2, int y2);
void sp_canvas_item_reset_bounds (SPCanvasItem *item);
void sp_canvas_buf_ensure_buf (SPCanvasBuf *buf);

/* fill buffer with background color */

void
sp_canvas_clear_buffer (SPCanvasBuf * buf);

/* render svp, translated to x, y */

void
sp_canvas_render_svp_translated (SPCanvasBuf *buf, ArtSVP *svp, guint32 rgba, gint x, gint y);

/* get i2p (item to parent) affine transformation as general 6-element array */

void sp_canvas_item_i2p_affine (SPCanvasItem * item, double affine[]);

/* get i2i (item to item) affine transformation as general 6-element array */

void sp_canvas_item_i2i_affine (SPCanvasItem * from, SPCanvasItem * to, double affine[]);

/* set item affine matrix to achieve given i2w matrix */

void sp_canvas_item_set_i2w_affine (SPCanvasItem * item, double i2w[]);

void sp_canvas_item_move_to_z (SPCanvasItem * item, gint z);

gint sp_canvas_item_compare_z (SPCanvasItem * a, SPCanvasItem * b);

#endif
