#define __SP_SELTRANS_C__

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

#include <math.h>

#include <libnr/nr-matrix.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include "macros.h"
#include "svg/svg.h"
#include "sodipodi-private.h"
#include "sodipodi.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "selection.h"
#include "select-context.h"
#include "sp-item.h"
#include "seltrans-handles.h"
#include "seltrans.h"
#include "sp-metrics.h"
#include "helper/sp-ctrlline.h"

static void sp_sel_trans_update_handles (SPSelTrans * seltrans);
static void sp_sel_trans_update_volatile_state (SPSelTrans * seltrans);

static void sp_remove_handles (SPKnot * knot[], gint num);
static void sp_show_handles (SPSelTrans * seltrans, SPKnot * knot[], const SPSelTransHandle handle[], gint num);

static void sp_sel_trans_handle_grab (SPKnot * knot, guint state, gpointer data);
static void sp_sel_trans_handle_ungrab (SPKnot * knot, guint state, gpointer data);
static void sp_sel_trans_handle_new_event (SPKnot * knot, NRPointF *position, guint32 state, gpointer data);
static gboolean sp_sel_trans_handle_request (SPKnot * knot, NRPointF *p, guint state, gboolean * data);

static void sp_sel_trans_sel_changed (SPSelection * selection, gpointer data);
static void sp_sel_trans_sel_modified (SPSelection * selection, guint flags, gpointer data);

extern GdkPixbuf *handles[];

static gboolean
sp_seltrans_handle_event (SPKnot *knot, GdkEvent *event, gpointer data)
{
	SPDesktop * desktop;
	SPSelTrans * seltrans;
	
	switch (event->type) {
	case GDK_MOTION_NOTIFY:
		break;
	case GDK_KEY_PRESS:
		if (event->key.keyval == GDK_space) {
			/* stamping mode: both mode(show content and outline) operation with knot */
			if (!SP_KNOT_IS_GRABBED (knot)) return FALSE;
			desktop = knot->desktop;
			seltrans = &SP_SELECT_CONTEXT (desktop->event_context)->seltrans;
			sp_sel_trans_stamp (seltrans);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

void
sp_sel_trans_init (SPSelTrans * seltrans, SPDesktop * desktop)
{
	gint i;

	g_return_if_fail (seltrans != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	seltrans->desktop = desktop;

	seltrans->state = SP_SELTRANS_STATE_SCALE;
	seltrans->show = SP_SELTRANS_SHOW_CONTENT;
	seltrans->transform = SP_SELTRANS_TRANSFORM_OPTIMIZE;

	seltrans->items = NULL;
	seltrans->transforms = NULL;
	seltrans->nitems = 0;

	seltrans->spp = nr_new (NRPointF, SP_SELTRANS_SPP_SIZE);

	seltrans->grabbed = FALSE;
	seltrans->show_handles = TRUE;
	for (i = 0; i < 8; i++) seltrans->shandle[i] = NULL;
	for (i = 0; i < 8; i++) seltrans->rhandle[i] = NULL;
	seltrans->chandle = NULL;

	sp_sel_trans_update_volatile_state (seltrans);
	
	seltrans->center.x = (seltrans->box.x0 + seltrans->box.x1) / 2;
	seltrans->center.y = (seltrans->box.y0 + seltrans->box.y1) / 2;

	sp_sel_trans_update_handles (seltrans);

	seltrans->selection = SP_DT_SELECTION (desktop);
	g_signal_connect (G_OBJECT (seltrans->selection), "changed", G_CALLBACK (sp_sel_trans_sel_changed), seltrans);
	g_signal_connect (G_OBJECT (seltrans->selection), "modified", G_CALLBACK (sp_sel_trans_sel_modified), seltrans);

	seltrans->norm = sp_canvas_item_new (SP_DT_CONTROLS (desktop),
		SP_TYPE_CTRL,
		"anchor", GTK_ANCHOR_CENTER,
		"mode", SP_CTRL_MODE_COLOR,
		"shape", SP_CTRL_SHAPE_BITMAP,
		"size", 13.0,
		"filled", TRUE,
		"fill_color", 0x00000000,
		"stroked", TRUE,
		"stroke_color", 0x000000a0,
		"pixbuf", handles[12],
		NULL);
	seltrans->grip = sp_canvas_item_new (SP_DT_CONTROLS (desktop),
		SP_TYPE_CTRL,
		"anchor", GTK_ANCHOR_CENTER,
		"mode", SP_CTRL_MODE_XOR,
		"shape", SP_CTRL_SHAPE_CROSS,
		"size", 7.0,
		"filled", TRUE,
		"fill_color", 0xffffff7f,
		"stroked", TRUE,
		"stroke_color", 0xffffffff,
		"pixbuf", handles[12],
		NULL);
	sp_canvas_item_hide (seltrans->grip);
	sp_canvas_item_hide (seltrans->norm);

	seltrans->l1 = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
	sp_canvas_item_hide (seltrans->l1);
	seltrans->l2 = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
	sp_canvas_item_hide (seltrans->l2);
	seltrans->l3 = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
	sp_canvas_item_hide (seltrans->l3);
	seltrans->l4 = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
	sp_canvas_item_hide (seltrans->l4);

	seltrans->stamp_cache = NULL;
}

void
sp_sel_trans_shutdown (SPSelTrans *seltrans)
{
	gint i;

	for (i = 0; i < 8; i++) {
		if (seltrans->shandle[i]) {
			g_object_unref (G_OBJECT (seltrans->shandle[i]));
			seltrans->shandle[i] = NULL;
		}
		if (seltrans->rhandle[i]) {
			g_object_unref (G_OBJECT (seltrans->rhandle[i]));
			seltrans->rhandle[i] = NULL;
		}
	}
	if (seltrans->chandle) {
		g_object_unref (G_OBJECT (seltrans->chandle));
		seltrans->chandle = NULL;
	}

	if (seltrans->norm) {
		gtk_object_destroy (GTK_OBJECT (seltrans->norm));
		seltrans->norm = NULL;
	}
	if (seltrans->grip) {
		gtk_object_destroy (GTK_OBJECT (seltrans->grip));
		seltrans->grip = NULL;
	}
	if (seltrans->l1) {
		gtk_object_destroy (GTK_OBJECT (seltrans->l1));
		seltrans->l1 = NULL;
	}
	if (seltrans->l2) {
		gtk_object_destroy (GTK_OBJECT (seltrans->l2));
		seltrans->l2 = NULL;
	}
	if (seltrans->l3) {
		gtk_object_destroy (GTK_OBJECT (seltrans->l3));
		seltrans->l3 = NULL;
	}
	if (seltrans->l4) {
		gtk_object_destroy (GTK_OBJECT (seltrans->l4));
		seltrans->l4 = NULL;
	}

	if (seltrans->selection) {
		sp_signal_disconnect_by_data (seltrans->selection, seltrans);
	}

	nr_free (seltrans->spp);

	if (seltrans->items) {
		int i;
		for (i = 0; i < seltrans->nitems; i++) sp_object_unref (SP_OBJECT (seltrans->items[i]), NULL);
		nr_free (seltrans->items);
	}
	if (seltrans->transforms) nr_free (seltrans->transforms);
}

void
sp_sel_trans_reset_state (SPSelTrans * seltrans)
{
	seltrans->state = SP_SELTRANS_STATE_SCALE;
}

void
sp_sel_trans_increase_state (SPSelTrans * seltrans)
{
	if (seltrans->state == SP_SELTRANS_STATE_SCALE) {
		seltrans->state = SP_SELTRANS_STATE_ROTATE;
	} else {
		seltrans->state = SP_SELTRANS_STATE_SCALE;
	}

	sp_sel_trans_update_handles (seltrans);
}

void
sp_sel_trans_set_center (SPSelTrans * seltrans, gdouble x, gdouble y)
{
	seltrans->center.x = x;
	seltrans->center.y = y;

	sp_sel_trans_update_handles (seltrans);
}

void
sp_sel_trans_grab (SPSelTrans * seltrans, NRPointF *p, gdouble x, gdouble y, gboolean show_handles)
{
	SPSelection *selection;
	const GSList *l;
	int n;

	selection = SP_DT_SELECTION (seltrans->desktop);

	g_return_if_fail (!seltrans->grabbed);

	seltrans->grabbed = TRUE;
	seltrans->show_handles = show_handles;
	sp_sel_trans_update_volatile_state (seltrans);

	seltrans->changed = FALSE;

	if (seltrans->empty) return;

	l = sp_selection_item_list (selection);
	seltrans->nitems = g_slist_length ((GSList *) l);
	seltrans->items = nr_new (SPItem *, seltrans->nitems);
	seltrans->transforms = nr_new (NRMatrixF, seltrans->nitems);
	n = 0;
	while (l) {
		seltrans->items[n] = (SPItem *) sp_object_ref (SP_OBJECT (l->data), NULL);
		sp_item_i2d_affine (seltrans->items[n], &seltrans->transforms[n]);
		l = l->next;
		n += 1;
	}

	nr_matrix_d_set_identity (&seltrans->current);

	seltrans->point.x = p->x;
	seltrans->point.y = p->y;

	seltrans->spp_length = sp_selection_snappoints (selection, seltrans->spp, SP_SELTRANS_SPP_SIZE);

	seltrans->opposit.x = seltrans->box.x0 + (1 - x) * fabs (seltrans->box.x1 - seltrans->box.x0);
	seltrans->opposit.y = seltrans->box.y0 + (1 - y) * fabs (seltrans->box.y1 - seltrans->box.y0);

	if ((x != -1) && (y != -1)) {
		sp_canvas_item_show (seltrans->norm);
		sp_canvas_item_show (seltrans->grip);
	}

	if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
		sp_canvas_item_show (seltrans->l1);
		sp_canvas_item_show (seltrans->l2);
		sp_canvas_item_show (seltrans->l3);
		sp_canvas_item_show (seltrans->l4);
	}


	sp_sel_trans_update_handles (seltrans);
	g_return_if_fail(seltrans->stamp_cache == NULL);
}

void
sp_sel_trans_transform (SPSelTrans * seltrans, NRMatrixD *affine, NRPointF *norm)
{
	NRMatrixD n2p, p2n;

	g_return_if_fail (seltrans->grabbed);
	g_return_if_fail (!seltrans->empty);

	nr_matrix_d_set_translate (&n2p, norm->x, norm->y);
	nr_matrix_d_set_translate (&p2n, -norm->x, -norm->y);
	nr_matrix_multiply_ddd (affine, &p2n, affine);
	nr_matrix_multiply_ddd (affine, affine, &n2p);

	if (seltrans->show == SP_SELTRANS_SHOW_CONTENT) {
	        // update the content
		int i;
		for (i = 0; i < seltrans->nitems; i++) {
			NRMatrixF i2dnew;
			nr_matrix_multiply_ffd (&i2dnew, &seltrans->transforms[i], affine);
			sp_item_set_i2d_affine (seltrans->items[i], &i2dnew);
		}
	} else {
		NRPointF p[4];
        	/* update the outline */
		p[0].x = NR_MATRIX_DF_TRANSFORM_X (affine, seltrans->box.x0, seltrans->box.y0);
		p[0].y = NR_MATRIX_DF_TRANSFORM_Y (affine, seltrans->box.x0, seltrans->box.y0);
		p[1].x = NR_MATRIX_DF_TRANSFORM_X (affine, seltrans->box.x1, seltrans->box.y0);
		p[1].y = NR_MATRIX_DF_TRANSFORM_Y (affine, seltrans->box.x1, seltrans->box.y0);
		p[2].x = NR_MATRIX_DF_TRANSFORM_X (affine, seltrans->box.x1, seltrans->box.y1);
		p[2].y = NR_MATRIX_DF_TRANSFORM_Y (affine, seltrans->box.x1, seltrans->box.y1);
		p[3].x = NR_MATRIX_DF_TRANSFORM_X (affine, seltrans->box.x0, seltrans->box.y1);
		p[3].y = NR_MATRIX_DF_TRANSFORM_Y (affine, seltrans->box.x0, seltrans->box.y1);
		sp_ctrlline_set_coords (SP_CTRLLINE (seltrans->l1), p[0].x, p[0].y, p[1].x, p[1].y);
		sp_ctrlline_set_coords (SP_CTRLLINE (seltrans->l2), p[1].x, p[1].y, p[2].x, p[2].y);
		sp_ctrlline_set_coords (SP_CTRLLINE (seltrans->l3), p[2].x, p[2].y, p[3].x, p[3].y);
		sp_ctrlline_set_coords (SP_CTRLLINE (seltrans->l4), p[3].x, p[3].y, p[0].x, p[0].y);
	}

	
	seltrans->current = *affine;

	seltrans->changed = TRUE;

	sp_sel_trans_update_handles (seltrans);
}

void
sp_sel_trans_ungrab (SPSelTrans * seltrans)
{
	SPItem * item;
	const GSList * l;
	gchar tstr[80];
	NRPointD p;
	unsigned int updh;

	g_return_if_fail (seltrans->grabbed);

	updh = TRUE;
	if (!seltrans->empty && seltrans->changed) {
		l = sp_selection_item_list (SP_DT_SELECTION (seltrans->desktop));

		tstr[79] = '\0';

		while (l != NULL) {
			item = SP_ITEM (l->data);
			/* fixme: We do not have to set it here (Lauris) */
			if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
				NRMatrixF i2d, i2dnew;
				sp_item_i2d_affine (item, &i2d);
				nr_matrix_multiply_ffd (&i2dnew, &i2d, &seltrans->current);
				sp_item_set_i2d_affine (item, &i2dnew);
			}
			if (seltrans->transform == SP_SELTRANS_TRANSFORM_OPTIMIZE) {
				sp_item_write_transform (item, SP_OBJECT_REPR (item), &item->transform);
				/* because item/repr affines may be out of sync, invoke reread */
				/* fixme: We should test equality to avoid unnecessary rereads */
				/* fixme: This probably is not needed (Lauris) */
				sp_object_read_attr (SP_OBJECT (item), "transform");
			} else {
				if (sp_svg_transform_write (tstr, 79, &item->transform)) {
					sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", tstr);
				} else {
					sp_repr_set_attr (SP_OBJECT (item)->repr, "transform", NULL);
				}
			}
			l = l->next;
		}
		p = seltrans->center;
		seltrans->center.x = NR_MATRIX_DF_TRANSFORM_X (&seltrans->current, p.x, p.y);
		seltrans->center.y = NR_MATRIX_DF_TRANSFORM_Y (&seltrans->current, p.x, p.y);
		
		sp_document_done (SP_DT_DOCUMENT (seltrans->desktop));
		sp_selection_changed (SP_DT_SELECTION (seltrans->desktop));

		updh = FALSE;
	}

	if (seltrans->items) {
		int i;
		for (i = 0; i < seltrans->nitems; i++) sp_object_unref (SP_OBJECT (seltrans->items[i]), NULL);
		nr_free (seltrans->items);
		seltrans->items = NULL;
	}
	if (seltrans->transforms) {
		nr_free (seltrans->transforms);
		seltrans->transforms = NULL;
	}
	seltrans->nitems = 0;

	seltrans->grabbed = FALSE;
	seltrans->show_handles = TRUE;
	
	sp_canvas_item_hide (seltrans->norm);
	sp_canvas_item_hide (seltrans->grip);

        if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
		sp_canvas_item_hide (seltrans->l1);
		sp_canvas_item_hide (seltrans->l2);
		sp_canvas_item_hide (seltrans->l3);
		sp_canvas_item_hide (seltrans->l4);
	}

	sp_sel_trans_update_volatile_state (seltrans);
	if (updh) sp_sel_trans_update_handles (seltrans);
	if (seltrans->stamp_cache) {
		g_slist_free(seltrans->stamp_cache);
		seltrans->stamp_cache = NULL;
	}

}

/* fixme: This is really bad, as we compare positoons for each stamp (Lauris) */
/* fixme: IMHO the best way to keep sort cache would be to implement timestamping at last */

static int
sp_object_compare_position (SPObject *a, SPObject *b)
{
	return sp_repr_compare_position (a->repr, b->repr);
}

void
sp_sel_trans_stamp (SPSelTrans * seltrans)
{
	/* stamping mode */
	SPItem * original_item, * copy_item;
	SPRepr * original_repr, * copy_repr;
	GSList * l;

	gchar tstr[80];
	NRMatrixF i2d, i2dnew;
	NRMatrixF *new_affine;

	tstr[79] = '\0';
	
	if (!seltrans->empty) {

                if (seltrans->stamp_cache) {
                        l = seltrans->stamp_cache;
                } else {
                        /* Build cache */
                        l  = (GSList *) sp_selection_item_list (SP_DT_SELECTION (seltrans->desktop));
                        l  = g_slist_copy (l);
                        l  = g_slist_sort (l, (GCompareFunc) sp_object_compare_position);
                        seltrans->stamp_cache = l;
                }

		while (l) {
			original_item = SP_ITEM(l->data);
			original_repr = (SPRepr *)(SP_OBJECT (original_item)->repr);
			copy_repr = sp_repr_duplicate (original_repr);
			copy_item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (seltrans->desktop), 
								     copy_repr);
			
			if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
				sp_item_i2d_affine (original_item, &i2d);
				nr_matrix_multiply_ffd (&i2dnew, &i2d, &seltrans->current);
				sp_item_set_i2d_affine (copy_item, &i2dnew);
				new_affine = &copy_item->transform;
			} else {
				new_affine = &original_item->transform;
			}

			if (sp_svg_transform_write (tstr, 80, new_affine)) {
				sp_repr_set_attr (copy_repr, "transform", tstr);
			} else {
				sp_repr_set_attr (copy_repr, "transform", NULL);
			}
			sp_repr_unref (copy_repr);
			l = l->next;
		}
		sp_document_done (SP_DT_DOCUMENT (seltrans->desktop));
	}
}

NRPointF *
sp_sel_trans_point_desktop (SPSelTrans *seltrans, NRPointF *p)
{
	g_return_val_if_fail (p != NULL, NULL);

	p->x = seltrans->point.x;
	p->y = seltrans->point.y;

	return p;
}

NRPointF *
sp_sel_trans_origin_desktop (SPSelTrans *seltrans, NRPointF *p)
{
	g_return_val_if_fail (p != NULL, NULL);

	p->x = seltrans->origin.x;
	p->y = seltrans->origin.y;

	return p;
}

static void
sp_sel_trans_update_handles (SPSelTrans * seltrans)
{
	NRPointF p;

	g_return_if_fail (seltrans != NULL);

      	if ((!seltrans->show_handles) || (seltrans->empty)) {
		sp_remove_handles (seltrans->shandle, 8);
		sp_remove_handles (seltrans->rhandle, 8);
		sp_remove_handles (&seltrans->chandle, 1);
		return;
	}
	
	if (seltrans->state == SP_SELTRANS_STATE_SCALE) {
		sp_remove_handles (seltrans->rhandle, 8);
		sp_remove_handles (&seltrans->chandle, 1);
		sp_show_handles (seltrans, seltrans->shandle, handles_scale, 8);
	} else {
		sp_remove_handles (seltrans->shandle, 8);
		sp_remove_handles (&seltrans->chandle, 1);
		sp_show_handles (seltrans, seltrans->rhandle, handles_rotate, 8);
	}

	// center handle
	if (seltrans->chandle == NULL) {
	  seltrans->chandle = sp_knot_new (seltrans->desktop);
	  g_object_set (G_OBJECT (seltrans->chandle),
			"anchor", handle_center.anchor, 
			"shape", SP_CTRL_SHAPE_BITMAP,
			"size", 13,
			"mode", SP_CTRL_MODE_COLOR,
			"fill", 0x00000000,
			"fill_mouseover", 0x0000007f,
			"stroke", 0x000000ff,
			"stroke_mouseover", 0x000000ff,
			  //"fill", 0xff40ffa0,
			  //"fill_mouseover", 0x40ffffa0,
			  //"stroke", 0xFFb0b0ff,
			  //"stroke_mouseover", 0xffffffFF,
			"pixbuf", handles[handle_center.control],
#if 0
			"cursor_mouseover", CursorSelectMouseover,
			"cursor_dragging", CursorSelectDragging,
#endif
			NULL);
	  g_signal_connect (G_OBJECT (seltrans->chandle), "request",
			    G_CALLBACK (sp_sel_trans_handle_request), (gpointer) &handle_center);
	  g_signal_connect (G_OBJECT (seltrans->chandle), "moved",
			    G_CALLBACK (sp_sel_trans_handle_new_event), (gpointer) &handle_center);
	  g_signal_connect (G_OBJECT (seltrans->chandle), "grabbed",
			    G_CALLBACK (sp_sel_trans_handle_grab), (gpointer) &handle_center);
	  g_signal_connect (G_OBJECT (seltrans->chandle), "ungrabbed",
			    G_CALLBACK (sp_sel_trans_handle_ungrab), (gpointer) &handle_center);
	}
	sp_knot_show (seltrans->chandle);
	p.x = seltrans->center.x;
	p.y = seltrans->center.y;
	sp_knot_set_position (seltrans->chandle, &p, 0);
}

static void
sp_sel_trans_update_volatile_state (SPSelTrans * seltrans)
{
	SPSelection *selection;
	NRRectF b;

	g_return_if_fail (seltrans != NULL);

	selection = SP_DT_SELECTION (seltrans->desktop);

	seltrans->empty = sp_selection_is_empty (selection);

	if (seltrans->empty) return;

	sp_selection_bbox (SP_DT_SELECTION (seltrans->desktop), &b);
	seltrans->box.x0 = b.x0;
	seltrans->box.y0 = b.y0;
	seltrans->box.x1 = b.x1;
	seltrans->box.y1 = b.y1;

	if ((seltrans->box.x0 > seltrans->box.x1) || (seltrans->box.y0 > seltrans->box.y1)) {
		seltrans->empty = TRUE;
		return;
	}

	nr_matrix_d_set_identity (&seltrans->current);
}

static void
sp_remove_handles (SPKnot * knot[], gint num)
{
	gint i;

	for (i = 0; i < num; i++) {
		if (knot[i] != NULL) {
			sp_knot_hide (knot[i]);
		}
	}
}

static void
sp_show_handles (SPSelTrans * seltrans, SPKnot * knot[], const SPSelTransHandle handle[], gint num)
{
	NRPointF p;
	gint i;

	for (i = 0; i < num; i++) {
		if (knot[i] == NULL) {
			knot[i] = sp_knot_new (seltrans->desktop);
			g_object_set (G_OBJECT (knot[i]),
					"anchor", handle[i].anchor, 
					"shape", SP_CTRL_SHAPE_BITMAP,
					"size", 13,
					"mode", SP_KNOT_MODE_COLOR,
					"fill", 0x4040ffa0,
					"fill_mouseover", 0xff4040f0,
					"stroke", 0x000000a0,
					"stroke_mouseover", 0x000000FF,
					//"fill", 0xffff0080,
					//"fill_mouseover", 0x00ffff80,
					//"stroke", 0xFFFFFFff,
					//"stroke_mouseover", 0xb0b0b0FF,
					"pixbuf", handles[handle[i].control],
#if 0
					"cursor_mouseover", CursorSelectMouseover,
					"cursor_dragging", CursorSelectDragging,
#endif
					NULL);

			g_signal_connect (G_OBJECT (knot[i]), "request",
					  G_CALLBACK (sp_sel_trans_handle_request), (gpointer) &handle[i]);
 			g_signal_connect (G_OBJECT (knot[i]), "moved",
					  G_CALLBACK (sp_sel_trans_handle_new_event), (gpointer) &handle[i]);
			g_signal_connect (G_OBJECT (knot[i]), "grabbed",
					  G_CALLBACK (sp_sel_trans_handle_grab), (gpointer) &handle[i]);
			g_signal_connect (G_OBJECT (knot[i]), "ungrabbed",
					  G_CALLBACK (sp_sel_trans_handle_ungrab), (gpointer) &handle[i]);
			g_signal_connect (G_OBJECT (knot[i]), "event", G_CALLBACK (sp_seltrans_handle_event), (gpointer) &handle[i]);
		}
		sp_knot_show (knot[i]);

		p.x = seltrans->box.x0 + handle[i].x * fabs (seltrans->box.x1 - seltrans->box.x0);
		p.y = seltrans->box.y0 + handle[i].y * fabs (seltrans->box.y1 - seltrans->box.y0);

		//gtk_signal_handler_block_by_func (GTK_OBJECT (knot[i]),
		//				  GTK_SIGNAL_FUNC (sp_sel_trans_handle_new_event), &handle[i]);
		sp_knot_set_position (knot[i], &p, 0);	    
		//gtk_signal_handler_unblock_by_func (GTK_OBJECT (knot[i]),
		//				    GTK_SIGNAL_FUNC (sp_sel_trans_handle_new_event), &handle[i]);

	}
}

static void
sp_sel_trans_handle_grab (SPKnot * knot, guint state, gpointer data)
{
	SPDesktop * desktop;
	SPSelTrans * seltrans;
	SPSelTransHandle * handle;
	NRPointF p;
	NRPointF pf;

	desktop = knot->desktop;
	seltrans = &SP_SELECT_CONTEXT (desktop->event_context)->seltrans;
	handle = (SPSelTransHandle *) data;

	switch (handle->anchor) {
	case GTK_ANCHOR_CENTER:
	  g_object_set (G_OBJECT (seltrans->grip),
			  "shape", SP_CTRL_SHAPE_BITMAP,
			  "size", 13.0,
			  NULL);
	  sp_canvas_item_show (seltrans->grip);
	  break;
	default:
	  g_object_set (G_OBJECT (seltrans->grip),
			  "shape", SP_CTRL_SHAPE_CROSS,
			  "size", 7.0,
			  NULL);
	  sp_canvas_item_show (seltrans->norm);
	  sp_canvas_item_show (seltrans->grip);

	  break;
	}

	sp_knot_position (knot, &p);

	pf.x = p.x;
	pf.y = p.y;
	sp_sel_trans_grab (seltrans, &pf, handle->x, handle->y, FALSE);
}

static void
sp_sel_trans_handle_ungrab (SPKnot * knot, guint state, gpointer data)
{
	SPDesktop * desktop;
	SPSelTrans * seltrans;
	
	desktop = knot->desktop;
	seltrans = &SP_SELECT_CONTEXT (desktop->event_context)->seltrans;

	sp_sel_trans_ungrab (seltrans);
	//sp_selection_changed (SP_DT_SELECTION (seltrans->desktop));
	
}

static void
sp_sel_trans_handle_new_event (SPKnot * knot, NRPointF *position, guint state, gpointer data)
{
	SPDesktop * desktop;
	SPSelTrans * seltrans;
	SPSelTransHandle * handle;

	if (!SP_KNOT_IS_GRABBED (knot)) return;

	desktop = knot->desktop;
	seltrans = &SP_SELECT_CONTEXT (desktop->event_context)->seltrans;
	handle = (SPSelTransHandle *) data;
	handle->action (seltrans, handle, position, state);

	//sp_desktop_coordinate_status (desktop, position->x, position->y, 4);
}

/* fixme: Highly experimental test :) */

static gboolean
sp_sel_trans_handle_request (SPKnot * knot, NRPointF *position, guint state, gboolean * data)
{
	SPDesktop * desktop;
	SPSelTrans * seltrans;
	SPSelTransHandle * handle;
	NRPointF point;

	if (!SP_KNOT_IS_GRABBED (knot)) return TRUE;

	desktop = knot->desktop;
	seltrans = &SP_SELECT_CONTEXT (desktop->event_context)->seltrans;
	handle = (SPSelTransHandle *) data;

	sp_desktop_set_coordinate_status (desktop, position->x, position->y, 0);
	sp_view_set_position (SP_VIEW (desktop), position->x, position->y);

	if (state & GDK_MOD1_MASK) {
		sp_sel_trans_point_desktop (seltrans, &point);
		position->x = point.x + (position->x - point.x) / 10;
		position->y = point.y + (position->y - point.y) / 10;
	}

	if (!(state & GDK_SHIFT_MASK) == !(seltrans->state == SP_SELTRANS_STATE_ROTATE)) {
		seltrans->origin = seltrans->opposit;
	} else {
		seltrans->origin = seltrans->center;
	}

	if (handle->request (seltrans, handle, position, state)) {
		sp_knot_set_position (knot, position, state);
		sp_ctrl_moveto (SP_CTRL (seltrans->grip), position->x, position->y);
		sp_ctrl_moveto (SP_CTRL (seltrans->norm), seltrans->origin.x, seltrans->origin.y);
	}
	
	return TRUE;
}

static void
sp_sel_trans_sel_changed (SPSelection * selection, gpointer data)
{
	SPSelTrans * seltrans;

	seltrans = (SPSelTrans *) data;

	if (!seltrans->grabbed) {
		sp_sel_trans_update_volatile_state (seltrans);
		seltrans->center.x = (seltrans->box.x0 + seltrans->box.x1) / 2;
		seltrans->center.y = (seltrans->box.y0 + seltrans->box.y1) / 2;
		sp_sel_trans_update_handles (seltrans);
	}

}

static void
sp_sel_trans_sel_modified (SPSelection *selection, guint flags, gpointer data)
{
	SPSelTrans *seltrans;

	seltrans = (SPSelTrans *) data;

	if (!seltrans->grabbed) {
		sp_sel_trans_update_volatile_state (seltrans);
		seltrans->center.x = (seltrans->box.x0 + seltrans->box.x1) / 2;
		seltrans->center.y = (seltrans->box.y0 + seltrans->box.y1) / 2;
		sp_sel_trans_update_handles (seltrans);
	}

}

/*
 * handlers for handle move-request
 */

gboolean
sp_sel_trans_scale_request (SPSelTrans *seltrans, SPSelTransHandle *handle, NRPointF *p, guint state)
{
	NRPointF norm, point;
	gdouble sx, sy;
	gchar status[80];
	SPDesktop *desktop;
	int xd, yd;

	desktop = seltrans->desktop;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);

	if (fabs (point.x - norm.x) > 0.0625) {
		sx = (p->x - norm.x) / (point.x - norm.x);
		if (fabs (sx) < 1e-9) sx = 1e-9;
		xd = TRUE;
	} else {
		sx = 0.0;
		xd = FALSE;
	}
	if (fabs (point.y - norm.y) > 0.0625) {
		sy = (p->y - norm.y) / (point.y - norm.y);
		if (fabs (sy) < 1e-9) sy = 1e-9;
		yd = TRUE;
	} else {
		sy = 0.0;
		yd = FALSE;
	}

	if (state & GDK_CONTROL_MASK) {
		double r;
		if (!xd || !yd) return FALSE;
	        if (fabs (sy) > fabs (sx)) sy = fabs (sx) * sy / fabs (sy);
		if (fabs (sx) > fabs (sy)) sx = fabs (sy) * sx / fabs (sx);
		r = sp_desktop_vector_snap_list (desktop, seltrans->spp, seltrans->spp_length, &norm, sx, sy);
		sx = fabs (r) * sx / fabs (sx);
		sy = fabs (r) * sy / fabs (sy);
	} else {
		if (xd) sx = sp_desktop_horizontal_snap_list_scale (desktop, seltrans->spp, seltrans->spp_length, &norm, sx);
		if (yd) sy = sp_desktop_vertical_snap_list_scale (desktop, seltrans->spp, seltrans->spp_length, &norm, sy);
	}

	p->x = (point.x - norm.x) * sx + norm.x;
	p->y = (point.y - norm.y) * sy + norm.y;

	// status text
	sprintf (status, "Scale  %0.2f%c, %0.2f%c", 100 * sx, '%', 100 * sy, '%');
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);

	return TRUE;
}

gboolean
sp_sel_trans_stretch_request (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
	double sx = 1.0, sy = 1.0, ratio;
	gchar status[80];	
	NRPointF norm, point;
	SPDesktop * desktop;

	desktop = seltrans->desktop;
 
	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);

	switch (handle->cursor) {
	case GDK_TOP_SIDE:
	case GDK_BOTTOM_SIDE:
		if (fabs (point.y - norm.y) < 1e-15) return FALSE;
		sy = (p->y - norm.y) / (point.y - norm.y);	    
		if (fabs (sy) < 1e-15) sy = 1e-15;
		if (state & GDK_CONTROL_MASK) {
			if (fabs (sy) > 1) sy = sy / fabs (sy);
			sx = fabs (sy);
			ratio = sp_desktop_vector_snap_list (desktop, seltrans->spp, seltrans->spp_length, &norm, sx, sy);
			sy = (fabs (ratio) < 1) ? fabs (ratio) * sy / fabs (sy) : sy / fabs (sy);
			sx = fabs (sy);
		} else {
			sy = sp_desktop_vertical_snap_list_scale (desktop, seltrans->spp, seltrans->spp_length, &norm, sy); 
		}
		break;
	case GDK_LEFT_SIDE:
	case GDK_RIGHT_SIDE:
		if (fabs (point.x - norm.x) < 1e-5) return FALSE;
		sx = (p->x - norm.x) / (point.x - norm.x);
		if (fabs (sx) < 1e-5) sx = 1e-15;
		if (state & GDK_CONTROL_MASK) {
			if (fabs (sx) > 1) sx = sx / fabs (sx);
			sy = fabs (sx);
			ratio = sp_desktop_vector_snap_list (desktop, seltrans->spp, seltrans->spp_length, &norm, sx, sy);
			sx = (fabs (sx) < 1) ? fabs (ratio) * sx / fabs (sx) : sx / fabs (sx);
			sy = fabs (ratio);
		} else {
			sx = sp_desktop_horizontal_snap_list_scale (desktop, seltrans->spp, seltrans->spp_length, &norm, sx);
		}
		break;
	default:
		break;
	}

	p->x = (point.x - norm.x)* sx + norm.x;
	p->y = (point.y - norm.y)* sy + norm.y;

	// status text
	sprintf (status, "Scale  %0.2f%c, %0.2f%c", 100 * sx, '%', 100 * sy, '%');
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);

	return TRUE;
}

gboolean
sp_sel_trans_skew_request (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
        double skew[6], sx = 1.0, sy = 1.0;
        gchar status[80];
	NRPointF norm, point;
	SPDesktop * desktop;

	desktop = seltrans->desktop;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);

  
	skew[4]=0;
	skew[5]=0;
	skew[0]=1;
	skew[3]=1;

	switch (handle->cursor) {
	case GDK_SB_V_DOUBLE_ARROW:
	  if (fabs (point.x - norm.x) < 1e-15) return FALSE;
	  skew[1] = (p->y - point.y) / (point.x - norm.x);
	  skew[1] = sp_desktop_vertical_snap_list_skew (desktop, seltrans->spp, seltrans->spp_length, &norm, skew[1]);
	  p->y = (point.x - norm.x) * skew[1] + point.y; 
	  sx = (p->x - norm.x) / (point.x - norm.x);
	  if (state & GDK_CONTROL_MASK) {
	    sx = sp_desktop_horizontal_snap_list_scale (desktop, seltrans->spp, seltrans->spp_length, &norm, sx);
	  } else {
	    if (fabs (sx) < 1e-15) sx = 1e-15;
	    if (fabs (sx) < 1) sx = fabs (sx) / sx;
	    sx = (double)((int)(sx + 0.5*(fabs(sx)/sx)));
	  }
	  if (fabs (sx) < 1e-15) sx = 1e-15;
	  p->x = (point.x - norm.x) * sx + norm.x;
	  
	  skew[2] = 0;
	  break;
	case GDK_SB_H_DOUBLE_ARROW:
	  if (fabs (point.y - norm.y) < 1e-15) return FALSE;
	  skew[2] = (p->x - point.x) / (point.y - norm.y);
	  skew[2] = sp_desktop_horizontal_snap_list_skew (desktop, seltrans->spp, seltrans->spp_length, &norm, skew[2]);
	  p->x = (point.y - norm.y) * skew[2] + point.x; 
	  sy = (p->y - norm.y) / (point.y - norm.y);
	  
	  if (state & GDK_CONTROL_MASK) {
	    sy = sp_desktop_vertical_snap_list_scale (desktop, seltrans->spp, seltrans->spp_length, &norm, sy);
	  } else {
	    if (fabs (sy) < 1e-15) sy = 1e-15;
	    if (fabs (sy) < 1) sy = fabs (sy) / sy;
	    sy = (double)((int)(sy + 0.5*(fabs(sy)/sy)));
	  }
	  if (fabs (sy) < 1e-15) sy = 1e-15;
	  p->y = (point.y - norm.y) * sy + norm.y;
	  
	  skew[1] = 0;
	  break;
	default:
		break;
	}
	// status text
	sprintf (status, "Skew  %0.2f%c %0.2f%c", 100 * fabs(skew[2]), '%', 100 * fabs(skew[1]), '%');
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);

	return TRUE;
}

gboolean
sp_sel_trans_rotate_request (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
	NRPointF norm, point, q1, q2;
	double h1, h2;
	NRMatrixD r1, r2, n2p, p2n;
	NRMatrixF rotate;
	double dx1, dx2, dy1, dy2, sg, angle;
	SPDesktop * desktop;
	gchar status[80];

	desktop = seltrans->desktop;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);
	
	// rotate affine in rotate
	dx1 = point.x - norm.x;
	dy1 = point.y - norm.y;
	dx2 = p->x    - norm.x;
	dy2 = p->y    - norm.y;
	h1 = hypot (dx1, dy1);
	if (fabs (h1) < 1e-15) return FALSE;
	q1.x = (dx1) / h1;
	q1.y = (dy1) / h1;
	h2 = hypot (dx2, dy2);
	if (fabs (h2) < 1e-15) return FALSE;
	q2.x = (dx2) / h2;
	q2.y = (dy2) / h2;
	r1.c[0] = q1.x;  r1.c[1] = -q1.y;  r1.c[2] =  q1.y;  r1.c[3] = q1.x;  r1.c[4] = 0;  r1.c[5] = 0;
	r2.c[0] = q2.x;  r2.c[1] =  q2.y;  r2.c[2] = -q2.y;  r2.c[3] = q2.x;  r2.c[4] = 0;  r2.c[5] = 0;
	nr_matrix_d_set_translate (&n2p, norm.x, norm.y);
	nr_matrix_d_invert (&p2n, &n2p);
	nr_matrix_multiply_fdd (&rotate, &p2n, &r1);
	nr_matrix_multiply_ffd (&rotate, &rotate, &r2);
	nr_matrix_multiply_ffd (&rotate, &rotate, &n2p);

#if 0
	/* Snap */
	sp_desktop_circular_snap_list (desktop, seltrans->spp, seltrans->spp_length, &norm, &rotate);
#endif
	p->x = NR_MATRIX_DF_TRANSFORM_X (&rotate, point.x, point.y);
	p->y = NR_MATRIX_DF_TRANSFORM_Y (&rotate, point.x, point.y);

	/* status text */
	dx2 = p->x    - norm.x;
	dy2 = p->y    - norm.y;
	h2 = hypot (dx2, dy2);
	if (fabs (h2) < 1e-15) return FALSE;
	angle = 180 / M_PI * acos ((dx1*dx2 + dy1*dy2) / (h1 * h2));
	sg = (dx1 * dy2 + dy1 * dx2) / (dx1*dx1 + dy1*dy1);
	if (fabs (sg) > 1e-15) angle *= sg / fabs (sg);
	
	sprintf (status, "Rotate by %0.2f deg", angle);
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);

	return TRUE;
}

gboolean
sp_sel_trans_center_request (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
	gchar status[80];
	GString * xs, * ys;
	SPDesktop * desktop;
	NRPointF point;
	
	desktop  = seltrans->desktop;
	sp_desktop_free_snap (desktop, p);

	if (state & GDK_CONTROL_MASK) {
	        sp_sel_trans_point_desktop (seltrans, &point);
	        if (fabs (point.x - p->x) > fabs (point.y - p->y)) 
		  p->y = point.y;
		else 
		  p->x = point.x;
	}

	if (state & GDK_SHIFT_MASK) {
		  p->x = (seltrans->box.x0 + seltrans->box.x1) / 2;
		  p->y = (seltrans->box.y0 + seltrans->box.y1) / 2;
	}

	// status text
	xs = SP_PT_TO_METRIC_STRING (p->x, SP_DEFAULT_METRIC);
	ys = SP_PT_TO_METRIC_STRING (p->y, SP_DEFAULT_METRIC);
	sprintf (status, "Center  %s, %s", xs->str, ys->str);
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);
	g_string_free (xs, FALSE);
	g_string_free (ys, FALSE);

	return TRUE;
}

/*
 * handlers for handle movement
 *
 */

void
sp_sel_trans_stretch (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
	NRMatrixD stretch;
	double sx = 1.0, sy = 1.0;
	NRPointF norm, point;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);

	switch (handle->cursor) {
	case GDK_TOP_SIDE:
	case GDK_BOTTOM_SIDE:
		if (fabs (point.y - norm.y) < 1e-15) return;
		sy = (p->y - norm.y) / (point.y - norm.y);
		if (fabs (sy) < 1e-15) sy = 1e-15;
		if (state & GDK_CONTROL_MASK) {
			if (fabs (sy) > fabs (sx)) sy = sy / fabs (sy);
			if (fabs (sy) < fabs (sx)) sx = fabs (sy) * sx / fabs (sx);
		}
		break;
	case GDK_LEFT_SIDE:
	case GDK_RIGHT_SIDE:
		if (fabs (point.x - norm.x) < 1e-15) return;
		sx = (p->x - norm.x) / (point.x - norm.x);
		if (fabs (sx) < 1e-15) sx = 1e-15;
		if (state & GDK_CONTROL_MASK) {
			if (fabs (sx) > fabs (sy)) sx = sx / fabs (sx);
			if (fabs (sx) < fabs (sy)) sy = fabs (sx) * sy / fabs (sy);
		}
		break;
	default:
		break;
	}

	if (fabs (sx) < 1e-15) sx = 1e-15; 
	if (fabs (sy) < 1e-15) sy = 1e-15;
	nr_matrix_d_set_scale (&stretch, sx, sy);
	sp_sel_trans_transform (seltrans, &stretch, &norm);
}

void
sp_sel_trans_scale (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF * p, guint state) 
{
	NRMatrixD scale;
	double sx, sy;
	NRPointF norm, point;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);

	if (fabs (point.x - norm.x) > 1e-9) {
		sx = (p->x - norm.x) / (point.x - norm.x);
	} else {
		sx = 1.0;
	}

	if (fabs (point.y - norm.y) > 1e-9) {
		sy = (p->y - norm.y) / (point.y - norm.y);
	} else {
		sy = 1.0;
	}

	if (fabs (sx) < 1e-9) sx = 1e-9; 
	if (fabs (sy) < 1e-9) sy = 1e-9;

	nr_matrix_d_set_scale (&scale, sx, sy);
	sp_sel_trans_transform (seltrans, &scale, &norm);
}

void
sp_sel_trans_skew (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
        NRMatrixD skew;
	NRPointF norm, point;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);
  
	skew.c[4]=0;
	skew.c[5]=0;
	
	switch (handle->cursor) {
	case GDK_SB_H_DOUBLE_ARROW:
		if (fabs (point.y - norm.y) < 1e-15) return;
		skew.c[2] = (p->x - point.x) / (point.y - norm.y);
		skew.c[3] = (p->y - norm.y) / (point.y - norm.y);
		skew.c[1] = 0;
		skew.c[0] = 1;
		break;
	case GDK_SB_V_DOUBLE_ARROW:
		if (fabs (point.x - norm.x) < 1e-15) return;
		skew.c[1] = (p->y - point.y) / (point.x - norm.x);
		skew.c[0] = (p->x - norm.x) / (point.x - norm.x);
		skew.c[2] = 0;
		skew.c[3] = 1;
		break;
	default:
		break;
	}

	if (fabs (skew.c[0]) < 1e-15) skew.c[0] = 1e-15;
	if (fabs (skew.c[3]) < 1e-15) skew.c[3] = 1e-15;
	sp_sel_trans_transform (seltrans, &skew, &norm);
}

void
sp_sel_trans_rotate (SPSelTrans *seltrans, SPSelTransHandle *handle, NRPointF *p, guint state)
{
        NRPointF q1, q2, point, norm;
	NRMatrixD rotate, r1, r2;
	double h1, h2;

	sp_sel_trans_point_desktop (seltrans, &point);
	sp_sel_trans_origin_desktop (seltrans, &norm);

	h1 = hypot (point.x - norm.x, point.y - norm.y);
	if (fabs (h1) < 1e-15) return;
	q1.x = (point.x - norm.x) / h1;
	q1.y = (point.y - norm.y) / h1;
	h2 = hypot (p->x - norm.x, p->y - norm.y);
	if (fabs (h2) < 1e-15) return;
	q2.x = (p->x - norm.x) / h2;
	q2.y = (p->y - norm.y) / h2;
	r1.c[0] = q1.x;  r1.c[1] = -q1.y;  r1.c[2] =  q1.y;  r1.c[3] = q1.x;  r1.c[4] = 0;  r1.c[5] = 0;
	r2.c[0] = q2.x;  r2.c[1] =  q2.y;  r2.c[2] = -q2.y;  r2.c[3] = q2.x;  r2.c[4] = 0;  r2.c[5] = 0;

	nr_matrix_multiply_ddd (&rotate, &r1, &r2);
	sp_sel_trans_transform (seltrans, &rotate, &norm);
}

void
sp_sel_trans_center (SPSelTrans * seltrans, SPSelTransHandle * handle, NRPointF *p, guint state)
{
	sp_sel_trans_set_center (seltrans, p->x, p->y);
}

