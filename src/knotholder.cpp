#define __KNOT_HOLDER_C__

/*
 * Container for SPKnot visual handles
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *
 * Copyright (C) 2001-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noKNOT_HOLDER_DEBUG

#include <glib.h>
#include <gtk/gtksignal.h>
#include "document.h"
#include "desktop.h"
#include "sp-item.h"
#include "sp-shape.h"
#include "knotholder.h"
#include <libnr/nr-matrix-ops.h>

static void knot_moved_handler (SPKnot *knot, NR::Point *p, guint state, gpointer data);
static void knot_ungrabbed_handler (SPKnot *knot, unsigned int state, SPKnotHolder *kh);

#ifdef KNOT_HOLDER_DEBUG
#include <gtk/gtk.h>

static void
sp_knot_holder_debug (GtkObject *object, gpointer data)
{
	g_print ("sp-knot-holder-debug: [type=%s] [data=%s]\n", gtk_type_name (GTK_OBJECT_TYPE(object)), (const gchar *)data);
}
#endif

SPKnotHolder *
sp_knot_holder_new (SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler)
{
	SPRepr *repr = SP_OBJECT (item)->repr;

	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP(desktop), NULL);
	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (SP_IS_ITEM(item), NULL);

	SPKnotHolder *knot_holder = g_new (SPKnotHolder, 1);
	knot_holder->desktop = desktop;
	knot_holder->item = item;
	g_object_ref (G_OBJECT (item));
	knot_holder->entity = NULL;

	knot_holder->released = relhandler;

	knot_holder->repr = repr;
	knot_holder->local_change = FALSE;

#ifdef KNOT_HOLDER_DEBUG
	g_signal_connect (G_OBJECT (desktop), "destroy", sp_knot_holder_debug, (gpointer)"SPKnotHolder::item");
#endif

	return knot_holder;
}

void
sp_knot_holder_destroy	(SPKnotHolder *kh)
{
	if (kh) {
		g_object_unref (G_OBJECT (kh->item));
		while (kh->entity) {
			SPKnotHolderEntity *e;
			e = (SPKnotHolderEntity *) kh->entity->data;
			/* unref should call destroy */
			g_object_unref (G_OBJECT (e->knot));
			g_free (e);
			kh->entity = g_slist_remove (kh->entity, e);
		}

		g_free (kh);
	}
}

void
sp_knot_holder_add (SPKnotHolder *knot_holder, SPKnotHolderSetFunc knot_set, SPKnotHolderGetFunc knot_get)
{
	sp_knot_holder_add_full (knot_holder, knot_set, knot_get, SP_KNOT_SHAPE_DIAMOND, SP_KNOT_MODE_XOR);
}

void
sp_knot_holder_add_full	(SPKnotHolder       *knot_holder,
			 SPKnotHolderSetFunc knot_set,
			 SPKnotHolderGetFunc knot_get,
			 SPKnotShapeType     shape,
			 SPKnotModeType      mode)
{
	g_return_if_fail (knot_holder != NULL);
	g_return_if_fail (knot_set != NULL);
	g_return_if_fail (knot_get != NULL);
	
	SPItem *item = SP_ITEM (knot_holder->item);

	/* create new SPKnotHolderEntry */
	SPKnotHolderEntity *e = g_new (SPKnotHolderEntity, 1);
	e->knot = sp_knot_new (knot_holder->desktop);
	e->knot_set = knot_set;
	e->knot_get = knot_get;

	g_object_set (G_OBJECT (e->knot->item), "shape", shape, NULL);
	g_object_set (G_OBJECT (e->knot->item), "mode", mode, NULL);
	knot_holder->entity = g_slist_append (knot_holder->entity, e);

	/* Move to current point. */
	NR::Point dp = e->knot_get(item) * sp_item_i2d_affine(item);
	sp_knot_set_position (e->knot, &dp, SP_KNOT_STATE_NORMAL);

	e->handler_id = g_signal_connect (G_OBJECT (e->knot), "moved", G_CALLBACK (knot_moved_handler), knot_holder);

	g_signal_connect (G_OBJECT (e->knot), "ungrabbed", G_CALLBACK (knot_ungrabbed_handler), knot_holder);

#ifdef KNOT_HOLDER_DEBUG
	g_signal_connect (ob, "destroy", sp_knot_holder_debug, "SPKnotHolder::knot");
#endif
	sp_knot_show (e->knot);
}

static void
knot_moved_handler (SPKnot *knot, NR::Point *p, guint state, gpointer data)
{
	SPKnotHolder *knot_holder = (SPKnotHolder *) data;
	SPItem *item  = SP_ITEM (knot_holder->item);
	// this was a local change and the knotholder does not need to be recreated:
	knot_holder->local_change = TRUE; 

	for (GSList *el = knot_holder->entity; el; el = el->next) {
		SPKnotHolderEntity *e = (SPKnotHolderEntity *)el->data;
		if (e->knot == knot) {
			NR::Point const q = *p * sp_item_i2d_affine(item).inverse();
			e->knot_set (item, q, state);
			break;
		}
	}
	
	sp_shape_set_shape (SP_SHAPE (item));

	NR::Matrix const i2d(sp_item_i2d_affine(item));

	for (GSList *el = knot_holder->entity; el; el = el->next) {
		SPKnotHolderEntity *e = (SPKnotHolderEntity *)el->data;
		GObject *kob = G_OBJECT (e->knot);

		NR::Point dp( e->knot_get(item) * i2d );
		g_signal_handler_block (kob, e->handler_id);
		sp_knot_set_position (e->knot, &dp, SP_KNOT_STATE_NORMAL);
		g_signal_handler_unblock (kob, e->handler_id);
	}
}

static void
knot_ungrabbed_handler (SPKnot *knot, unsigned int state, SPKnotHolder *kh)
{
	if (kh->released) {
		kh->released (kh->item);
	} else {
		SPObject *object = (SPObject *) kh->item;
		object->updateRepr(object->repr, SP_OBJECT_WRITE_EXT);
		sp_document_done (SP_OBJECT_DOCUMENT (kh->item));
	}
}

