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
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point-fns.h>
#include <libnr/nr-rotate-ops.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-translate-ops.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktogglebutton.h>
#include "macros.h"
#include "svg/svg.h"
#include "inkscape-private.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "selection.h"
#include "select-context.h"
#include "uri-references.h"
#include "sp-use.h"
#include "sp-item.h"
#include <sp-item-update-cns.h>
#include "seltrans-handles.h"
#include "selcue.h"
#include "seltrans.h"
#include "sp-metrics.h"
#include "helper/sp-intl.h"
#include "widgets/spw-utilities.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrlrect.h"
#include "prefs-utils.h"

static void sp_sel_trans_update_handles(SPSelTrans &seltrans);
static void sp_sel_trans_update_volatile_state(SPSelTrans &seltrans);

static void sp_remove_handles(SPKnot *knot[], gint num);
static void sp_show_handles(SPSelTrans &seltrans, SPKnot *knot[], SPSelTransHandle const handle[], gint num);

static void sp_sel_trans_handle_grab(SPKnot *knot, guint state, gpointer data);
static void sp_sel_trans_handle_ungrab(SPKnot *knot, guint state, gpointer data);
static void sp_sel_trans_handle_new_event(SPKnot *knot, NR::Point *position, guint32 state, gpointer data);
static gboolean sp_sel_trans_handle_request(SPKnot *knot, NR::Point *p, guint state, gboolean *data);

static void sp_sel_trans_sel_changed(SPSelection *selection, gpointer data);
static void sp_sel_trans_sel_modified(SPSelection *selection, guint flags, gpointer data);

extern GdkPixbuf *handles[];

static gboolean
sp_seltrans_handle_event(SPKnot *knot, GdkEvent *event, gpointer)
{
	switch (event->type) {
	case GDK_MOTION_NOTIFY:
		break;
	case GDK_KEY_PRESS:
		if (event->key.keyval == GDK_space) {
			/* stamping mode: both mode(show content and outline) operation with knot */
			if (!SP_KNOT_IS_GRABBED (knot)) return FALSE;
			SPDesktop *desktop = knot->desktop;
			SPSelTrans *seltrans = &SP_SELECT_CONTEXT(desktop->event_context)->seltrans;
			sp_sel_trans_stamp(seltrans);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

void sp_sel_trans_init(SPSelTrans *seltrans, SPDesktop *desktop)
{
	gint i;

	g_return_if_fail (seltrans != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	seltrans->desktop = desktop;

	seltrans->state = SP_SELTRANS_STATE_SCALE;
	seltrans->show = SP_SELTRANS_SHOW_CONTENT;

	seltrans->grabbed = FALSE;
	seltrans->show_handles = TRUE;
	for (i = 0; i < 8; i++) seltrans->shandle[i] = NULL;
	for (i = 0; i < 8; i++) seltrans->rhandle[i] = NULL;
	seltrans->chandle = NULL;

	sp_sel_trans_update_volatile_state(*seltrans);
	
	seltrans->center = seltrans->box.midpoint();

	sp_sel_trans_update_handles(*seltrans);

	seltrans->selection = SP_DT_SELECTION(desktop);

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
	
	for(int i = 0;  i < 4; i++) {
		seltrans->l[i] = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
		sp_canvas_item_hide (seltrans->l[i]);
	}

	seltrans->stamp_cache = NULL;

	/* we must call the constructors ourselves */
	new (&seltrans->sel_changed_connection) SigC::Connection(
		seltrans->selection->connectChanged(
			SigC::bind(
				SigC::slot(&sp_sel_trans_sel_changed),
				(gpointer)seltrans
			)
		)
	);
	new (&seltrans->sel_modified_connection) SigC::Connection(
		seltrans->selection->connectModified(
			SigC::bind(
				SigC::slot(&sp_sel_trans_sel_modified),
				(gpointer)seltrans
			)
		)
	);

	sp_sel_cue_init (&seltrans->selcue, desktop);
}

void
sp_sel_trans_shutdown (SPSelTrans *seltrans)
{
	seltrans->sel_changed_connection.disconnect();
	seltrans->sel_modified_connection.disconnect();

	/* destructors are not called automatically */
	seltrans->sel_changed_connection.~Connection();
	seltrans->sel_modified_connection.~Connection();
	seltrans->snap_points.~vector();

	for (unsigned i = 0; i < 8; i++) {
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
	for(int i = 0; i < 4; i++) {
		if (seltrans->l[i]) {
			gtk_object_destroy (GTK_OBJECT (seltrans->l[i]));
			seltrans->l[i] = NULL;
		}
	}

	for (unsigned i = 0; i < seltrans->items.size(); i++)
			sp_object_unref (SP_OBJECT (seltrans->items[i].first), NULL);
	
	seltrans->items.clear();

	sp_sel_cue_shutdown(&seltrans->selcue);
}

void sp_sel_trans_reset_state(SPSelTrans *seltrans)
{
	seltrans->state = SP_SELTRANS_STATE_SCALE;
}

void sp_sel_trans_increase_state(SPSelTrans *seltrans)
{
	if (seltrans->state == SP_SELTRANS_STATE_SCALE) {
		seltrans->state = SP_SELTRANS_STATE_ROTATE;
	} else {
		seltrans->state = SP_SELTRANS_STATE_SCALE;
	}

	sp_sel_trans_update_handles(*seltrans);
}

void sp_sel_trans_set_center(SPSelTrans *seltrans, NR::Point p)
{
	seltrans->center = p;
	
	sp_sel_trans_update_handles(*seltrans);
}

void sp_sel_trans_grab(SPSelTrans *seltrans, NR::Point const &p, gdouble x, gdouble y, gboolean show_handles)
{
	SPSelection *selection = SP_DT_SELECTION (seltrans->desktop);

	g_return_if_fail (!seltrans->grabbed);

	seltrans->grabbed = TRUE;
	seltrans->show_handles = show_handles;
	sp_sel_trans_update_volatile_state(*seltrans);

	seltrans->changed = FALSE;

	if (seltrans->empty) return;

	for(const GSList *l = selection->itemList(); l; l = l->next) {
		SPItem* it = (SPItem*)sp_object_ref (SP_OBJECT (l->data), NULL);
		seltrans->items.push_back(std::pair<SPItem *,NR::Matrix>(it, sp_item_i2d_affine (it)));
	}
	
	seltrans->current.set_identity();

	seltrans->point = p;

	seltrans->snap_points = selection->getSnapPoints ();

	seltrans->opposit = ( seltrans->box.min()
			      + ( seltrans->box.dimensions()
				  * NR::scale(1-x, 1-y) ) );

	if ((x != -1) && (y != -1)) {
		sp_canvas_item_show (seltrans->norm);
		sp_canvas_item_show (seltrans->grip);
	}

	if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
		for(int i = 0; i < 4; i++)
			sp_canvas_item_show (seltrans->l[i]);
	}


	sp_sel_trans_update_handles(*seltrans);
	g_return_if_fail(seltrans->stamp_cache == NULL);
}

void sp_sel_trans_transform(SPSelTrans *seltrans, NR::Matrix const &rel_affine, NR::Point const &norm)
{
	g_return_if_fail (seltrans->grabbed);
	g_return_if_fail (!seltrans->empty);

	NR::Matrix const affine( NR::translate(-norm) * rel_affine * NR::translate(norm) );

	if (seltrans->show == SP_SELTRANS_SHOW_CONTENT) {
	        // update the content
		for (unsigned i = 0; i < seltrans->items.size(); i++) {
			SPItem &item = *seltrans->items[i].first;
			NR::Matrix const &prev_transform = seltrans->items[i].second;
			sp_item_set_i2d_affine(&item, prev_transform * affine);
		}
	} else {
		NR::Point p[4];
        	/* update the outline */
		for(unsigned i = 0 ; i < 4 ; i++) {
			p[i] = seltrans->box.corner(i) * affine;
		}
		for(unsigned i = 0 ; i < 4 ; i++) {
			sp_ctrlline_set_coords (SP_CTRLLINE (seltrans->l[i]), 
						p[i], p[(i+1)%4]);
		}
	}

	seltrans->current = affine;

	seltrans->changed = TRUE;

	sp_sel_trans_update_handles(*seltrans);
}

void sp_sel_trans_ungrab(SPSelTrans *seltrans)
{
	g_return_if_fail(seltrans->grabbed);

	SPSelection *selection = SP_DT_SELECTION(seltrans->desktop);

	bool updh = true;
	if (!seltrans->empty && seltrans->changed) {

		for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
			SPItem *item = SP_ITEM(l->data);

#if 0 /* Re-enable this once persistent guides have a graphical indication.
	 At the time of writing, this is the only place to re-enable. */
			sp_item_update_cns(*item, *seltrans->desktop);
#endif

			// If this is a clone and it's selected along with its original, do not move it;
			// it will feel the transform of its original and respond to it itself. 
			// WIthout this, a clone is doubly transformed, very unintuitive.
			if (seltrans->current.is_translation() && SP_IS_USE(item) && selection->includesItem(sp_use_get_original (SP_USE(item)))) {
				// just restore the transform field from the repr
				sp_object_read_attr (SP_OBJECT (item), "transform");
			} else {

				/* fixme: We do not have to set it here (Lauris) */
				if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
					NR::Matrix const i2dnew( sp_item_i2d_affine(item) * seltrans->current );
					sp_item_set_i2d_affine(item, i2dnew);
				}

				sp_item_write_transform (item, SP_OBJECT_REPR (item), &item->transform);
			} 
		}
		seltrans->center *= seltrans->current;

		sp_document_done (SP_DT_DOCUMENT (seltrans->desktop));

		updh = false;
	}

	for (unsigned i = 0; i < seltrans->items.size(); i++)
		sp_object_unref (SP_OBJECT (seltrans->items[i].first), NULL);
	seltrans->items.clear();

	seltrans->grabbed = FALSE;
	seltrans->show_handles = TRUE;
	
	sp_canvas_item_hide (seltrans->norm);
	sp_canvas_item_hide (seltrans->grip);

        if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
		for(int i = 0; i < 4; i++)
			sp_canvas_item_hide (seltrans->l[i]);
	}

	sp_sel_trans_update_volatile_state(*seltrans);
	if (updh) {
		sp_sel_trans_update_handles(*seltrans);
	}
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

void sp_sel_trans_stamp (SPSelTrans *seltrans)
{
	SPSelection *selection = SP_DT_SELECTION (seltrans->desktop);

	/* stamping mode */
	if (!seltrans->empty) {
		GSList *l;
		if (seltrans->stamp_cache) {
			l = seltrans->stamp_cache;
		} else {
			/* Build cache */
			l  = g_slist_copy ((GSList *) selection->itemList ());
			l  = g_slist_sort (l, (GCompareFunc) sp_object_compare_position);
			seltrans->stamp_cache = l;
		}

		while (l) {
			SPItem *original_item = SP_ITEM(l->data);
			SPRepr *original_repr = SP_OBJECT_REPR (original_item);

			// remember the position of the item
			gint pos = sp_repr_position (original_repr);
			// remember parent
			SPRepr *parent = sp_repr_parent (original_repr);

			SPRepr *copy_repr = sp_repr_duplicate (original_repr);

			// add the new repr to the parent
			sp_repr_append_child (parent, copy_repr);
			// move to the saved position 
			sp_repr_set_position_absolute (copy_repr, pos > 0 ? pos : 0);

			SPItem *copy_item = (SPItem *) SP_DT_DOCUMENT(seltrans->desktop)->getObjectByRepr(copy_repr);

			NRMatrix *new_affine;
			if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
				NR::Matrix const i2d(sp_item_i2d_affine(original_item));
				NR::Matrix const i2dnew( i2d * seltrans->current );
				sp_item_set_i2d_affine(copy_item, i2dnew);
				new_affine = &copy_item->transform;
			} else {
				new_affine = &original_item->transform;
			}

			sp_item_write_transform (copy_item, copy_repr, new_affine);

			sp_repr_unref (copy_repr);
			l = l->next;
		}
		sp_document_done (SP_DT_DOCUMENT (seltrans->desktop));
	}
}

static void sp_sel_trans_update_handles(SPSelTrans &seltrans)
{
      	if ( !seltrans.show_handles
	     || seltrans.empty )
	{
		sp_remove_handles(seltrans.shandle, 8);
		sp_remove_handles(seltrans.rhandle, 8);
		sp_remove_handles(&seltrans.chandle, 1);
		return;
	}

	// center handle
        if ( seltrans.chandle == NULL ) {
            seltrans.chandle = sp_knot_new(seltrans.desktop);
            g_object_set(G_OBJECT(seltrans.chandle),
                         "anchor", handle_center.anchor, 
                         "shape", SP_CTRL_SHAPE_BITMAP,
                         "size", 13,
                         "mode", SP_CTRL_MODE_XOR,
                         "fill", 0x00000000,
                         "fill_mouseover", 0x00000000,
                         "stroke", 0x000000ff,
                         "stroke_mouseover", 0xff0000b0,
                         "pixbuf", handles[handle_center.control],
                         NULL);
            g_signal_connect(G_OBJECT(seltrans.chandle), "request",
                             G_CALLBACK (sp_sel_trans_handle_request), (gpointer) &handle_center);
            g_signal_connect(G_OBJECT(seltrans.chandle), "moved",
                             G_CALLBACK (sp_sel_trans_handle_new_event), (gpointer) &handle_center);
            g_signal_connect(G_OBJECT(seltrans.chandle), "grabbed",
                             G_CALLBACK (sp_sel_trans_handle_grab), (gpointer) &handle_center);
            g_signal_connect(G_OBJECT(seltrans.chandle), "ungrabbed",
                             G_CALLBACK (sp_sel_trans_handle_ungrab), (gpointer) &handle_center);
        }

	if ( seltrans.state == SP_SELTRANS_STATE_SCALE ) {
		sp_remove_handles(seltrans.rhandle, 8);
		sp_remove_handles(&seltrans.chandle, 1);
		sp_show_handles(seltrans, seltrans.shandle, handles_scale, 8);
		sp_knot_hide(seltrans.chandle);
	} else {
		sp_remove_handles(seltrans.shandle, 8);
		sp_remove_handles(&seltrans.chandle, 1);
		sp_show_handles(seltrans, seltrans.rhandle, handles_rotate, 8);
		sp_knot_show(seltrans.chandle);
		sp_knot_set_position(seltrans.chandle, &seltrans.center, 0);
	}
}

static void sp_sel_trans_update_volatile_state(SPSelTrans &seltrans)
{
	SPSelection *selection = SP_DT_SELECTION(seltrans.desktop);
	seltrans.empty = selection->isEmpty();

	if (seltrans.empty) {
		return;
	}

	seltrans.box = selection->bounds();
	if (seltrans.box.isEmpty()) {
		seltrans.empty = TRUE;
		return;
	}

	seltrans.current.set_identity();
}

static void sp_remove_handles(SPKnot *knot[], gint num)
{
	gint i;

	for (i = 0; i < num; i++) {
		if (knot[i] != NULL) {
			sp_knot_hide (knot[i]);
		}
	}
}

static void sp_show_handles(SPSelTrans &seltrans, SPKnot *knot[], SPSelTransHandle const handle[], gint num)
{
	g_return_if_fail( !seltrans.empty );

	for (gint i = 0; i < num; i++) {
		if (knot[i] == NULL) {
			knot[i] = sp_knot_new(seltrans.desktop);
			g_object_set (G_OBJECT (knot[i]),
					"anchor", handle[i].anchor, 
					"shape", SP_CTRL_SHAPE_BITMAP,
					"size", 13,
					"mode", SP_KNOT_MODE_XOR,
					"fill", 0x000000ff, // inversion
 					"fill_mouseover", 0x00ff6600, // green
					"stroke", 0x000000ff, // inversion
					"stroke_mouseover", 0x000000ff, // inversion
					"pixbuf", handles[handle[i].control],
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

		NR::Point const handle_pt(handle[i].x, handle[i].y);
		NR::Point p( seltrans.box.min()
			     + ( seltrans.box.dimensions()
				 * NR::scale(handle_pt) ) );

		sp_knot_set_position(knot[i], &p, 0);
	}
}

static void sp_sel_trans_handle_grab(SPKnot *knot, guint state, gpointer data)
{
	SPDesktop *desktop = knot->desktop;
	SPSelTrans *seltrans = &SP_SELECT_CONTEXT(desktop->event_context)->seltrans;
	SPSelTransHandle const &handle = *(SPSelTransHandle const *) data;

	switch(handle.anchor) {
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

	sp_sel_trans_grab(seltrans, sp_knot_position(knot), handle.x, handle.y, FALSE);
}

static void sp_sel_trans_handle_ungrab(SPKnot *knot, guint state, gpointer data)
{
	SPDesktop *desktop = knot->desktop;
	SPSelTrans *seltrans = &SP_SELECT_CONTEXT(desktop->event_context)->seltrans;

	sp_sel_trans_ungrab(seltrans);
}

static void sp_sel_trans_handle_new_event(SPKnot *knot, NR::Point *position, guint state, gpointer data)
{
	if (!SP_KNOT_IS_GRABBED(knot)) {
		return;
	}

	SPDesktop *desktop = knot->desktop;
	SPSelTrans *seltrans = &SP_SELECT_CONTEXT(desktop->event_context)->seltrans;
	SPSelTransHandle const &handle = *(SPSelTransHandle const *) data;
	handle.action(seltrans, handle, *position, state);

	//sp_desktop_coordinate_status (desktop, position, 4);
}

/* fixme: Highly experimental test :) */

static gboolean sp_sel_trans_handle_request(SPKnot *knot, NR::Point *position, guint state, gboolean *data)
{
	using NR::X;
	using NR::Y;

	if (!SP_KNOT_IS_GRABBED (knot)) return TRUE;

	SPDesktop *desktop = knot->desktop;
	SPSelTrans *seltrans = &SP_SELECT_CONTEXT(desktop->event_context)->seltrans;
	SPSelTransHandle const &handle = *(SPSelTransHandle const *) data;

	sp_desktop_set_coordinate_status(desktop, *position, 0);
	sp_view_set_position(SP_VIEW(desktop), *position);

	if (state & GDK_MOD1_MASK) {
		NR::Point const &point = seltrans->point;
		*position = point + ( *position - point ) / 10;
	}

	if (!(state & GDK_SHIFT_MASK) == !(seltrans->state == SP_SELTRANS_STATE_ROTATE)) {
		seltrans->origin = seltrans->opposit;
	} else {
		seltrans->origin = seltrans->center;
	}
	if (handle.request(seltrans, handle, *position, state)) {
		sp_knot_set_position(knot, position, state);
		SP_CTRL(seltrans->grip)->moveto(*position);
		SP_CTRL(seltrans->norm)->moveto(seltrans->origin);
	}

	return TRUE;
}

static void sp_sel_trans_sel_changed(SPSelection *selection, gpointer data)
{
	SPSelTrans *seltrans = (SPSelTrans *) data;

	if (!seltrans->grabbed) {
		sp_sel_trans_update_volatile_state(*seltrans);
		seltrans->center = seltrans->box.midpoint();
		sp_sel_trans_update_handles(*seltrans);
	}

}

static void
sp_sel_trans_sel_modified (SPSelection *selection, guint flags, gpointer data)
{
	SPSelTrans *seltrans;

	seltrans = (SPSelTrans *) data;

	if (!seltrans->grabbed) {
		sp_sel_trans_update_volatile_state(*seltrans);
		sp_sel_trans_update_handles(*seltrans);
	}

}

/*
 * handlers for handle move-request
 */

/** Returns -1 or 1 according to the sign of x.  Returns 1 for 0 and NaN. */
static double sign(double const x)
{
	return ( x < 0
		 ? -1
		 : 1 );
}

gboolean sp_sel_trans_scale_request(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;

	SPDesktop *desktop = seltrans->desktop;

	NR::Point point = seltrans->point;
	NR::Point const norm(seltrans->origin);

	NR::Point d = point - norm;
	NR::scale s(d);
	for ( unsigned i = 0 ; i < 2 ; i++ ) {
		if ( fabs(s[i]) > 0.001 ) {
			s[i] = ( pt[i] - norm[i] ) / ( point[i] - norm[i] );
			if ( fabs(s[i]) < 1e-9 ) {
				s[i] = 1e-9;
			}
		} else {
			s[i] = 0.0;
		}
	}

	GtkToggleButton *lock = (GtkToggleButton *) sp_search_by_data_recursive (seltrans->desktop->owner->aux_toolbox, (gpointer) "lock");

	if ((state & GDK_CONTROL_MASK) || gtk_toggle_button_get_active (lock)) {
	        double r;

		if ( !d[NR::X] || !d[NR::Y] ) {
			return FALSE;
		}

		for ( unsigned i = 0 ; i < 2 ; i++ ) {
			unsigned oi = 1 - i;
			if ( fabs(s[i]) > fabs(s[oi]) ) {
				s[i] = fabs(s[oi]) * sign(s[i]); 
				break;
			}
		}
		r = fabs(sp_desktop_vector_snap_list(desktop, seltrans->snap_points, norm, s));
		for ( unsigned i = 0 ; i < 2 ; i++ ) {
			s[i] = r * sign(s[i]);
		}
	} else {
		for ( unsigned i = 0 ; i < 2 ; i++ ) {
			if (d[i]) {
				s[i] = sp_desktop_dim_snap_list_scale(desktop, seltrans->snap_points, norm, s[i], s[i] ? NR::X : NR::Y);
			}
		}
	}

	pt = ( point - norm ) * s + norm;

	// status text
	gchar *status;
	status = g_strdup_printf(_("Scale %0.2f%%, %0.2f%%"), 100 * s[NR::X], 100 * s[NR::Y]);
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);
	g_free(status);

	return TRUE;
}

gboolean sp_sel_trans_stretch_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;
	double ratio;

	SPDesktop *desktop = seltrans->desktop;
 
	NR::Point point = seltrans->point;
	NR::Point const norm(seltrans->origin);

	NR::Dim2 axis, perp;

	switch (handle.cursor) {
	case GDK_TOP_SIDE:
	case GDK_BOTTOM_SIDE:
		axis = NR::Y;
		perp = NR::X;
		break;
	case GDK_LEFT_SIDE:
	case GDK_RIGHT_SIDE:
		axis = NR::X;
		perp = NR::Y;
		break;
	default:
		g_assert_not_reached();
		return TRUE;
	};

	if ( fabs( point[axis] - norm[axis] ) < 1e-15 ) {
		return FALSE;
	}

	NR::scale s(1, 1);
	s[axis] = ( ( pt[axis] - norm[axis] )
		    / ( point[axis] - norm[axis] ) );
	if ( fabs(s[axis]) < 1e-15 ) {
		s[axis] = 1e-15;
	}
	if ( state & GDK_CONTROL_MASK ) {
		s[perp] = fabs(s[axis]);
		ratio = sp_desktop_vector_snap_list(desktop, seltrans->snap_points, norm, s);
		s[axis] = fabs(ratio) * sign(s[axis]);
		s[perp] = fabs(s[axis]);
	} else {
		s[axis] = sp_desktop_dim_snap_list_scale(desktop, seltrans->snap_points, norm, s[axis], axis);
	}

	pt = ( point - norm ) * NR::scale(s) + norm;
	if (isnan(pt[X] + pt[Y])) {
		g_warning("point=(%g, %g), norm=(%g, %g), s=(%g, %g)\n",
			  point[X], point[Y], norm[X], norm[Y], s[X], s[Y]);
	}

	// status text
	gchar *status;
	status = g_strdup_printf(_("Scale %0.2f%%, %0.2f%%"), 100 * s[NR::X], 100 * s[NR::Y]);
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);
	g_free(status);

	return TRUE;
}

gboolean sp_sel_trans_skew_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;

	SPDesktop *desktop = seltrans->desktop;

	NR::Point const point(seltrans->point);
	NR::Point const norm(seltrans->origin);

        double skew[6], sx = 1.0, sy = 1.0;
	skew[4]=0;
	skew[5]=0;
	skew[0]=1;
	skew[3]=1;

	switch(handle.cursor) {
	case GDK_SB_V_DOUBLE_ARROW:
	  if (fabs (point[X] - norm[X]) < 1e-15) return FALSE;
	  skew[1] = ( pt[Y] - point[Y] ) / ( point[X] - norm[X] );
	  skew[1] = sp_desktop_dim_snap_list_skew(desktop, seltrans->snap_points, norm, skew[1], Y);
	  pt[Y] = ( point[X] - norm[X] ) * skew[1] + point[Y];
	  sx = ( pt[X] - norm[X] ) / ( point[X] - norm[X] );
	  if (state & GDK_CONTROL_MASK) {
	    sx = sp_desktop_dim_snap_list_scale(desktop, seltrans->snap_points, norm, sx, X);
	  } else {
	    if ( fabs(sx) < 1e-15 ) sx = 1e-15;
	    if ( fabs(sx) < 1 ) sx = sign(sx);
	    sx = floor( sx + 0.5 );
	  }
	  if (fabs (sx) < 1e-15) sx = 1e-15;
	  pt[X] = ( point[X] - norm[X] ) * sx + norm[X];
	  
	  skew[2] = 0;
	  break;
	case GDK_SB_H_DOUBLE_ARROW:
	  if (fabs (point[Y] - norm[Y]) < 1e-15) return FALSE;
	  skew[2] = ( pt[X] - point[X] ) / ( point[Y] - norm[Y] );
	  skew[2] = sp_desktop_dim_snap_list_skew(desktop, seltrans->snap_points, norm, skew[2], X);
	  pt[X] = ( point[Y] - norm[Y] ) * skew[2] + point[X];
	  sy = ( pt[Y] - norm[Y] ) / ( point[Y] - norm[Y] );
	  
	  if (state & GDK_CONTROL_MASK) {
	    sy = sp_desktop_dim_snap_list_scale(desktop, seltrans->snap_points, norm, sy, Y);
	  } else {
	    if ( fabs(sy) < 1e-15 ) sy = 1e-15;
	    if ( fabs(sy) < 1 ) sy = sign(sy);
	    sy = floor( sy + 0.5 );
	  }
	  if (fabs (sy) < 1e-15) sy = 1e-15;
	  pt[Y] = ( point[Y] - norm[Y] ) * sy + norm[Y];

	  skew[1] = 0;
	  break;
	default:
		break;
	}

	// status text
        gchar status[80];
	sprintf (status, _("Skew %0.2f%c %0.2f%c"), 100 * fabs(skew[2]), '%', 100 * fabs(skew[1]), '%');
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);

	return TRUE;
}

gboolean sp_sel_trans_rotate_request(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	NR::Point const point = seltrans->point;
	NR::Point const norm = seltrans->origin;

	// rotate affine in rotate
	NR::Point const d1 = point - norm;
	NR::Point const d2 = pt    - norm;

	NR::Coord h1 = NR::L2 (d1);
	if (h1 < 1e-15) return FALSE;
	NR::Point q1 = d1 / h1;
	NR::Coord h2 = NR::L2 (d2);
	if (fabs (h2) < 1e-15) return FALSE;
	NR::Point q2 = d2 / h2;

	if (state & GDK_CONTROL_MASK) {
		/* Have to restrict movement. */
		double cos_t = NR::dot(q1, q2);
		double sin_t = NR::dot(NR::rot90(q1), q2);
		double theta = atan2(sin_t, cos_t);
		if (snaps) {
			theta = ( M_PI / snaps ) * floor( theta * snaps / M_PI + .5 );
		}
		q1 = NR::Point(1, 0);
		q2 = NR::Point(cos(theta), sin(theta));
	}

	NR::rotate const r1(q1);
	NR::rotate const r2(q2);
	pt = point * NR::translate(-norm) * ( r2 / r1 ) * NR::translate(norm);

	/* status text */
	double angle = 180 / M_PI * atan2 (NR::dot(NR::rot90(d1), d2), 
					   NR::dot(d1, d2));

	if (angle > 180) angle -= 360;
	if (angle < -180) angle += 360;

	sp_view_set_statusf (SP_VIEW (seltrans->desktop), _("Rotate by %0.2f deg"), angle);

	return TRUE;
}

gboolean sp_sel_trans_center_request(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;
	SPDesktop *desktop = seltrans->desktop;
	sp_desktop_free_snap(desktop, pt);

	if (state & GDK_CONTROL_MASK) {
		NR::Point point = seltrans->point;
	        if ( fabs( point[X] - pt[X] )  >
		     fabs( point[Y] - pt[Y] ) ) {
			pt[Y] = point[X];
		} else {
			pt[X] = point[X];
		}
	}

	if (state & GDK_SHIFT_MASK) {
		pt = seltrans->box.midpoint();
	}

	// status text
	GString *xs = SP_PT_TO_METRIC_STRING(pt[X], SP_DEFAULT_METRIC);
	GString *ys = SP_PT_TO_METRIC_STRING(pt[Y], SP_DEFAULT_METRIC);
	gchar *status = g_strdup_printf(_("Center at (%s,%s)"), xs->str, ys->str);
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);
	g_free(status);
	g_string_free (xs, FALSE);
	g_string_free (ys, FALSE);

	return TRUE;
}

/*
 * handlers for handle movement
 *
 */

void sp_sel_trans_stretch(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;

	NR::Dim2 dim;
	switch(handle.cursor) {
	case GDK_LEFT_SIDE:
	case GDK_RIGHT_SIDE:
		dim = X;
		break;
	case GDK_TOP_SIDE:
	case GDK_BOTTOM_SIDE:
		dim = Y;
		break;
	default:
		g_assert_not_reached();
		abort();		
		break;
	}

	NR::Point const scale_origin(seltrans->origin);
	double const offset = seltrans->point[dim] - scale_origin[dim];
	if (!( fabs(offset) >= 1e-15 )) {
		return;
	}
	NR::scale s(1, 1);
	s[dim] = ( pt[dim] - scale_origin[dim] ) / offset;
	if (isnan(s[dim])) {
		g_warning("s[dim]=%g, pt[dim]=%g, scale_origin[dim]=%g, point[dim]=%g\n",
			  s[dim], pt[dim], scale_origin[dim], seltrans->point[dim]);
	}
	if (!( fabs(s[dim]) >= 1e-15 )) {
		s[dim] = 1e-15;
	}
	if (state & GDK_CONTROL_MASK) {
		/* Preserve aspect ratio, but never flip in the dimension not being edited. */
		s[!dim] = fabs(s[dim]);
	}
	NR::Matrix const stretch(s);
	sp_sel_trans_transform(seltrans, stretch, scale_origin);
}

void sp_sel_trans_scale(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	const NR::Point point = seltrans->point;
	NR::Point norm = seltrans->origin;
	const NR::Point offset = point - norm;
	
	NR::Point s(1, 1);
	for(int i = 0; i < 2; i++) {
		if (fabs (offset[i]) > 1e-9)
			s[i] = (pt[i] - norm[i]) / offset[i];
		if (fabs (s[i]) < 1e-9)
			s[i] = 1e-9; 
	}
	NR::Matrix const scale((NR::scale(s)));
	sp_sel_trans_transform (seltrans, scale, norm);
}

void sp_sel_trans_skew(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &pt, guint state)
{
	const NR::Point point = seltrans->point;
	NR::Point norm = seltrans->origin;
	const NR::Point offset = point - norm;

	unsigned dim;
	switch(handle.cursor) {
	case GDK_SB_H_DOUBLE_ARROW:
		dim = NR::Y;
		break;
	case GDK_SB_V_DOUBLE_ARROW:
		dim = NR::X;
		break;
	default:
		g_assert_not_reached();
		abort();
		break;
	}
	if (fabs (offset[dim]) < 1e-15)
		return;
        NR::Matrix skew = NR::identity();
	skew[2*dim + dim] = (pt[dim] - norm[dim]) / offset[dim];
	skew[2*dim + (1-dim)] = (pt[1-dim] - point[1-dim]) / offset[dim];
	skew[2*(1-dim) + (dim)] = 0;
	skew[2*(1-dim) + (1-dim)] = 1;
	
	for(int i = 0; i < 2; i++) {
		if (fabs (skew[3*i]) < 1e-15) {
			skew[3*i] = 1e-15;
		}
	}
	sp_sel_trans_transform (seltrans, skew, norm);
}

void sp_sel_trans_rotate(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	const NR::Point point = seltrans->point;
	NR::Point norm = seltrans->origin;
	const NR::Point offset = point - norm;

	const NR::Coord h1 = NR::L2(offset);
	if (h1 < 1e-15)
		return;
	const NR::Point q1 = offset / h1;
	const NR::Coord h2 = NR::L2 (pt - norm);
	if (h2 < 1e-15)
		return;
	NR::Point const q2 = (pt - norm) / h2;
	NR::rotate const r1(q1);
	NR::rotate const r2(q2);

	NR::Matrix rotate( r2 / r1 );
	sp_sel_trans_transform (seltrans, rotate, norm);
}

void sp_sel_trans_center(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	sp_sel_trans_set_center (seltrans, pt);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
