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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
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
#include "snap.h"
#include "selection.h"
#include "select-context.h"
#include "uri-references.h"
#include "sp-desktop-widget.h"
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
#include "xml/repr.h"

#include "isnan.h" //temp fix.  make sure included last

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
			SPSelTrans *seltrans = SP_SELECT_CONTEXT(desktop->event_context)->_seltrans;
			sp_sel_trans_stamp(seltrans);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

SPSelTrans::SPSelTrans(SPDesktop *desktop)
: box(NR::Point(0,0), NR::Point(0,0)),
  selcue(desktop), _message_context(desktop->messageStack())
{
	gint i;

	g_return_if_fail (this != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	this->desktop = desktop;

	this->state = SP_SELTRANS_STATE_SCALE;
	this->show = SP_SELTRANS_SHOW_CONTENT;

	this->grabbed = FALSE;
	this->show_handles = TRUE;
	for (i = 0; i < 8; i++) this->shandle[i] = NULL;
	for (i = 0; i < 8; i++) this->rhandle[i] = NULL;
	this->chandle = NULL;

	sp_sel_trans_update_volatile_state(*this);
	
	this->center = this->box.midpoint();

	sp_sel_trans_update_handles(*this);

	this->selection = SP_DT_SELECTION(desktop);

	this->norm = sp_canvas_item_new (SP_DT_CONTROLS (desktop),
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
	this->grip = sp_canvas_item_new (SP_DT_CONTROLS (desktop),
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
	sp_canvas_item_hide (this->grip);
	sp_canvas_item_hide (this->norm);
	
	for(int i = 0;  i < 4; i++) {
		this->l[i] = sp_canvas_item_new (SP_DT_CONTROLS (desktop), SP_TYPE_CTRLLINE, NULL);
		sp_canvas_item_hide (this->l[i]);
	}

	this->stamp_cache = NULL;

        _sel_changed_connection = this->selection->connectChanged (

            sigc::bind (

                sigc::ptr_fun(&sp_sel_trans_sel_changed),

                (gpointer)this )

            



	);
	_sel_modified_connection = this->selection->connectModified (
            sigc::bind (
                sigc::ptr_fun(&sp_sel_trans_sel_modified),
                (gpointer)this
	)            
	);
}

SPSelTrans::~SPSelTrans() {
	this->_sel_changed_connection.disconnect();
	this->_sel_modified_connection.disconnect();

	for (unsigned i = 0; i < 8; i++) {
		if (this->shandle[i]) {
			g_object_unref (G_OBJECT (this->shandle[i]));
			this->shandle[i] = NULL;
		}
		if (this->rhandle[i]) {
			g_object_unref (G_OBJECT (this->rhandle[i]));
			this->rhandle[i] = NULL;
		}
	}
	if (this->chandle) {
		g_object_unref (G_OBJECT (this->chandle));
		this->chandle = NULL;
	}

	if (this->norm) {
		gtk_object_destroy (GTK_OBJECT (this->norm));
		this->norm = NULL;
	}
	if (this->grip) {
		gtk_object_destroy (GTK_OBJECT (this->grip));
		this->grip = NULL;
	}
	for(int i = 0; i < 4; i++) {
		if (this->l[i]) {
			gtk_object_destroy (GTK_OBJECT (this->l[i]));
			this->l[i] = NULL;
		}
	}

	for (unsigned i = 0; i < this->items.size(); i++) {
            sp_object_unref (SP_OBJECT (this->items[i].first), NULL);
        }
	
	this->items.clear();
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
	seltrans->bbox_points = selection->getBBoxPoints ();

        gchar const *scale_origin = prefs_get_string_attribute ("tools.select", "scale_origin");
        bool const origin_on_bbox = (scale_origin == NULL || !strcmp(scale_origin, "bbox"));
        NR::Rect op_box = seltrans->box;
        if (origin_on_bbox == false && seltrans->snap_points.empty() == false) {
            std::vector<NR::Point>::iterator i = seltrans->snap_points.begin();
            op_box = NR::Rect(*i, *i);
            i++;
            while (i != seltrans->snap_points.end()) {
                op_box.expandTo(*i);
                i++;
            }
        }

        seltrans->opposite = ( op_box.min() + ( op_box.dimensions() * NR::scale(1-x, 1-y) ) );
                         
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

				sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform);
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

        seltrans->_message_context.clear();
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

                        NR::Matrix const *new_affine;
			if (seltrans->show == SP_SELTRANS_SHOW_OUTLINE) {
				NR::Matrix const i2d(sp_item_i2d_affine(original_item));
				NR::Matrix const i2dnew( i2d * seltrans->current );
				sp_item_set_i2d_affine(copy_item, i2dnew);
				new_affine = &copy_item->transform;
			} else {
				new_affine = &original_item->transform;
			}

			sp_item_write_transform(copy_item, copy_repr, *new_affine);

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

        sp_remove_handles(&seltrans.chandle, 1);
	if ( seltrans.state == SP_SELTRANS_STATE_SCALE ) {
		sp_remove_handles(seltrans.rhandle, 8);
		sp_show_handles(seltrans, seltrans.shandle, handles_scale, 8);
	} else {
		sp_remove_handles(seltrans.shandle, 8);
		sp_show_handles(seltrans, seltrans.rhandle, handles_rotate, 8);
	}
        if ( seltrans.state == SP_SELTRANS_STATE_SCALE ) {
		sp_knot_hide(seltrans.chandle);
        } else {
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
	SPSelTrans *seltrans = SP_SELECT_CONTEXT(desktop->event_context)->_seltrans;
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
	SPSelTrans *seltrans = SP_SELECT_CONTEXT(desktop->event_context)->_seltrans;

	sp_sel_trans_ungrab(seltrans);
}

static void sp_sel_trans_handle_new_event(SPKnot *knot, NR::Point *position, guint state, gpointer data)
{
	if (!SP_KNOT_IS_GRABBED(knot)) {
		return;
	}

	SPDesktop *desktop = knot->desktop;
	SPSelTrans *seltrans = SP_SELECT_CONTEXT(desktop->event_context)->_seltrans;
	SPSelTransHandle const &handle = *(SPSelTransHandle const *) data;
	handle.action(seltrans, handle, *position, state);
}

/* fixme: Highly experimental test :) */

static gboolean sp_sel_trans_handle_request(SPKnot *knot, NR::Point *position, guint state, gboolean *data)
{
	using NR::X;
	using NR::Y;

	if (!SP_KNOT_IS_GRABBED (knot)) return TRUE;

	SPDesktop *desktop = knot->desktop;
	SPSelTrans *seltrans = SP_SELECT_CONTEXT(desktop->event_context)->_seltrans;
	SPSelTransHandle const &handle = *(SPSelTransHandle const *) data;

	sp_desktop_set_coordinate_status(desktop, *position, 0);
	sp_view_set_position(SP_VIEW(desktop), *position);

	if (state & GDK_MOD1_MASK) {
		NR::Point const &point = seltrans->point;
		*position = point + ( *position - point ) / 10;
	}

	if (!(state & GDK_SHIFT_MASK) == !(seltrans->state == SP_SELTRANS_STATE_ROTATE)) {
		seltrans->origin = seltrans->opposite;
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
		if (!seltrans->changed) { // do not reset center if the change was by seltrans itself; otherwise reset
                seltrans->center = seltrans->box.midpoint();
		} else {
                seltrans->changed = FALSE;
             }
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

    /* Original position of the scale knot */
    NR::Point point = seltrans->point;
    /* Origin for scaling */
    NR::Point const norm(seltrans->origin);

    NR::Point d = point - norm;
    NR::scale s(0, 0);

    /* Work out the new scale factors `s' */
    for ( unsigned int i = 0 ; i < 2 ; i++ ) {
        if ( fabs(d[i]) > 0.001 ) {
            s[i] = ( pt[i] - norm[i] ) / d[i];
            if ( fabs(s[i]) < 1e-9 ) {
                s[i] = 1e-9;
            }
        }
    }

    GtkToggleButton *lock = (GtkToggleButton *) sp_search_by_data_recursive(desktop->owner->aux_toolbox,
                                                                            (gpointer) "lock");

    if ((state & GDK_CONTROL_MASK) || gtk_toggle_button_get_active (lock)) {
        /* Scale is locked to a 1:1 aspect ratio, so that s[X] must be made to equal s[Y] */

        NR::Dim2 locked_dim;

        /* Lock aspect ratio, using the smaller of the x and y factors */
        if (fabs(s[NR::X]) > fabs(s[NR::Y])) {
            s[NR::X] = fabs(s[NR::Y]) * sign(s[NR::X]);
            locked_dim = NR::X;
        } else {
            s[NR::Y] = fabs(s[NR::X]) * sign(s[NR::Y]);
            locked_dim = NR::Y;
        }

        /* Snap the scale factor */
        std::pair<double, bool> bb = namedview_vector_snap_list(desktop->namedview,
                                                                Snapper::BBOX_POINT, seltrans->bbox_points,
                                                                norm, s);
        std::pair<double, bool> sn = namedview_vector_snap_list(desktop->namedview,
                                                                Snapper::SNAP_POINT, seltrans->snap_points,
                                                                norm, s);

        double bd = bb.second ? fabs(bb.first - s[locked_dim]) : NR_HUGE;
        double sd = sn.second ? fabs(sn.first - s[locked_dim]) : NR_HUGE;
        double r = (bd < sd) ? bb.first : sn.first;
        
        for ( unsigned int i = 0 ; i < 2 ; i++ ) {
            s[i] = r * sign(s[i]);
        }
        
    } else {
        /* Scale aspect ratio is unlocked */
        for ( unsigned int i = 0 ; i < 2 ; i++ ) {
            std::pair<double, bool> bb = namedview_dim_snap_list_scale(desktop->namedview,
                                                                       Snapper::BBOX_POINT, seltrans->bbox_points,
                                                                       norm, s[i], NR::Dim2(i));
            std::pair<double, bool> sn = namedview_dim_snap_list_scale(desktop->namedview,
                                                                       Snapper::SNAP_POINT, seltrans->snap_points,
                                                                       norm, s[i], NR::Dim2(i));

            /* Pick the snap that puts us closest to the original scale */
            NR::Coord bd = bb.second ? fabs(bb.first - s[i]) : NR_HUGE;
            NR::Coord sd = sn.second ? fabs(sn.first - s[i]) : NR_HUGE;
            s[i] = (bd < sd) ? bb.first : sn.first;
        }
    }

    /* Update the knot position */
    pt = ( point - norm ) * s + norm;
    
    /* Status text */
    seltrans->_message_context.setF(Inkscape::NORMAL_MESSAGE,
                                    _("Scale %0.2f%%, %0.2f%%"),
                                    100 * s[NR::X], 100 * s[NR::Y]);
    
    return TRUE;
}

gboolean sp_sel_trans_stretch_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;

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
                
                std::pair<double, bool> sn = namedview_vector_snap_list(desktop->namedview,
                                                                        Snapper::BBOX_POINT,
                                                                        seltrans->bbox_points, norm, s);
		std::pair<double, bool> bb = namedview_vector_snap_list(desktop->namedview,
                                                                        Snapper::SNAP_POINT,
                                                                        seltrans->snap_points, norm, s);

                double bd = bb.second ? fabs(bb.first - s[axis]) : NR_HUGE;
                double sd = sn.second ? fabs(sn.first - s[axis]) : NR_HUGE;
                double ratio = (bd < sd) ? bb.first : sn.first;
                
                s[axis] = fabs(ratio) * sign(s[axis]);
		s[perp] = fabs(s[axis]);
	} else {
            std::pair<NR::Coord, bool> bb = namedview_dim_snap_list_scale(desktop->namedview, Snapper::BBOX_POINT,
                                                                          seltrans->bbox_points, norm, s[axis], axis);
            std::pair<NR::Coord, bool> sn = namedview_dim_snap_list_scale(desktop->namedview, Snapper::SNAP_POINT,
                                                                          seltrans->snap_points, norm, s[axis], axis);

            /* Pick the snap that puts us closest to the original scale */
            NR::Coord bd = bb.second ? fabs(bb.first - s[axis]) : NR_HUGE;
            NR::Coord sd = sn.second ? fabs(sn.first - s[axis]) : NR_HUGE;
            s[axis] = (bd < sd) ? bb.first : sn.first;
	}

	pt = ( point - norm ) * NR::scale(s) + norm;
	if (isNaN(pt[X] + pt[Y])) {
		g_warning("point=(%g, %g), norm=(%g, %g), s=(%g, %g)\n",
			  point[X], point[Y], norm[X], norm[Y], s[X], s[Y]);
	}

	// status text
        seltrans->_message_context.setF(Inkscape::NORMAL_MESSAGE,
                                        _("Scale %0.2f%%, %0.2f%%"),
                                        100 * s[NR::X], 100 * s[NR::Y]);

	return TRUE;
}

gboolean sp_sel_trans_skew_request(SPSelTrans *seltrans, SPSelTransHandle const &handle, NR::Point &pt, guint state)
{
    using NR::X;
    using NR::Y;

    if (handle.cursor != GDK_SB_V_DOUBLE_ARROW && handle.cursor != GDK_SB_H_DOUBLE_ARROW) {
        return FALSE;
    }
    
    SPDesktop *desktop = seltrans->desktop;
    
    NR::Point const point(seltrans->point);
    NR::Point const norm(seltrans->origin);

    NR::Dim2 dim_a;
    NR::Dim2 dim_b;
    if (handle.cursor == GDK_SB_V_DOUBLE_ARROW) {
        dim_a = X;
        dim_b = Y;
    } else {
        dim_a = Y;
        dim_b = X;
    }
    
    double skew[2];
    double s[2] = { 1.0, 1.0 };

    if (fabs (point[dim_a] - norm[dim_a]) < NR_EPSILON) {
        return FALSE;
    }
    
    skew[dim_a] = ( pt[dim_b] - point[dim_b] ) / ( point[dim_a] - norm[dim_a] );
    skew[dim_a] = namedview_dim_snap_list_skew(desktop->namedview,
                                               Snapper::SNAP_POINT, seltrans->snap_points,
                                               norm, skew[dim_a], dim_b);
    
    pt[dim_b] = ( point[dim_a] - norm[dim_a] ) * skew[dim_a] + point[dim_b];
    s[dim_a] = ( pt[dim_a] - norm[dim_a] ) / ( point[dim_a] - norm[dim_a] );
    if (state & GDK_CONTROL_MASK) {
        std::pair<NR::Coord, bool> sn = namedview_dim_snap_list_scale(desktop->namedview,
                                                                      Snapper::SNAP_POINT, seltrans->snap_points,
                                                                      norm, s[dim_a], dim_a);
        s[dim_a] = sn.first;
    } else {
        if ( fabs(s[dim_a]) < NR_EPSILON ) {
            s[dim_a] = NR_EPSILON;
        }
        if ( fabs(s[dim_a]) < 1 ) {
            s[dim_a] = sign(s[dim_a]);
        }
        s[dim_a] = floor( s[dim_a] + 0.5 );
    }
    
    if (fabs (s[dim_a]) < NR_EPSILON) {
        s[dim_a] = NR_EPSILON;
    }
    
    pt[dim_a] = ( point[dim_a] - norm[dim_a] ) * s[dim_a] + norm[dim_a];
    
    skew[dim_b] = 0;

    // status text
    seltrans->_message_context.setF(Inkscape::NORMAL_MESSAGE, 
                                    _("Skew %0.2f%c %0.2f%c"),
                                    100 * fabs(skew[1]), '%',
                                    100 * fabs(skew[0]), '%');
    
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

        seltrans->desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, 
            _("Rotate by %0.2f deg"), angle);

	return TRUE;
}

gboolean sp_sel_trans_center_request(SPSelTrans *seltrans, SPSelTransHandle const &, NR::Point &pt, guint state)
{
	using NR::X;
	using NR::Y;
	SPDesktop *desktop = seltrans->desktop;
	namedview_free_snap(desktop->namedview, Snapper::SNAP_POINT, pt);

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
        desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, _("Center at (%s,%s)"), xs->str, ys->str);
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
	if (isNaN(s[dim])) {
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
