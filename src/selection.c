#define __SP_SELECTION_C__

/*
 * Per-desktop selection container
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkmain.h>
#include "macros.h"
#include "helper/sp-marshal.h"
#include "sodipodi-private.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "sp-item.h"
#include "selection.h"

#define SP_SELECTION_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE + 1)

enum {
	CHANGED,
	MODIFIED,
	LAST_SIGNAL
};

static void sp_selection_class_init (SPSelectionClass *klass);
static void sp_selection_init (SPSelection *selection);
static void sp_selection_dispose (GObject *object);

static void sp_selection_private_changed (SPSelection *selection);

static void sp_selection_frozen_empty (SPSelection *selection);

static gint sp_selection_idle_handler (gpointer data);

static GObjectClass *parent_class;
static guint selection_signals[LAST_SIGNAL] = { 0 };

GType
sp_selection_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPSelectionClass),
			NULL, NULL,
			(GClassInitFunc) sp_selection_class_init,
			NULL, NULL,
			sizeof (SPSelection),
			4,
			(GInstanceInitFunc) sp_selection_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPSelection", &info, 0);
	}
	return type;
}

static void
sp_selection_class_init (SPSelectionClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	selection_signals [CHANGED] =  g_signal_new ("changed",
						     G_TYPE_FROM_CLASS(klass),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET (SPSelectionClass, changed),
						     NULL, NULL,
						     sp_marshal_NONE__NONE,
						     G_TYPE_NONE, 0);
	selection_signals [MODIFIED] = g_signal_new ("modified",
						     G_TYPE_FROM_CLASS(klass),
						     G_SIGNAL_RUN_FIRST,
						     G_STRUCT_OFFSET (SPSelectionClass, modified),
						     NULL, NULL,
						     sp_marshal_NONE__UINT,
						     G_TYPE_NONE, 1,
						     G_TYPE_UINT);

	object_class->dispose = sp_selection_dispose;

	klass->changed = sp_selection_private_changed;
}

static void
sp_selection_init (SPSelection *selection)
{
	selection->reprs = NULL;
	selection->items = NULL;
	selection->idle = 0;
	selection->flags = 0;
}

static void
sp_selection_dispose (GObject *object)
{
	SPSelection *selection;

	selection = SP_SELECTION (object);

	sp_selection_frozen_empty (selection);

	if (selection->idle) {
		gtk_idle_remove (selection->idle);
		selection->idle = 0;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
sp_selection_private_changed (SPSelection * selection)
{
	sodipodi_selection_changed (selection);
}

static void
sp_selection_selected_item_release (SPItem * item, SPSelection * selection)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (sp_selection_item_selected (selection, item));

	g_slist_free (selection->reprs);
	selection->reprs = NULL;

	selection->items = g_slist_remove (selection->items, item);

	sp_selection_changed (selection);
}

/* Handler for selected objects "modified" signal */

static void
sp_selection_selected_item_modified (SPItem *item, guint flags, SPSelection *selection)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (sp_selection_item_selected (selection, item));

	if (!selection->idle) {
		/* Request handling to be run in idle loop */
		selection->idle = gtk_idle_add_priority (SP_SELECTION_UPDATE_PRIORITY, sp_selection_idle_handler, selection);
	}

	/* Collect all flags */
	selection->flags |= flags;
}

/* Our idle loop handler */

static gint
sp_selection_idle_handler (gpointer data)
{
	SPSelection *selection;
	guint flags;

	selection = SP_SELECTION (data);

	/* Clear our id, so next request will be rescheduled */
	selection->idle = 0;
	flags = selection->flags;
	selection->flags = 0;
	/* Emit our own "modified" signal */
	g_signal_emit (G_OBJECT (selection), selection_signals [MODIFIED], 0, flags);

	/* Request "selection_modified" signal on Sodipodi */
	sodipodi_selection_modified (selection, flags);

	return FALSE;
}

void
sp_selection_frozen_empty (SPSelection * selection)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	g_slist_free (selection->reprs);
	selection->reprs = NULL;

	while (selection->items) {
		SPItem *item;
		item = SP_ITEM (selection->items->data);
		sp_signal_disconnect_by_data (item, selection);
		selection->items = g_slist_remove (selection->items, item);
	}
}

void
sp_selection_update_statusbar (SPSelection * selection)
{
	gchar * message;
	gint len;

	len = g_slist_length (selection->items);

	if (len == 1) {
		message = sp_item_description (SP_ITEM (selection->items->data));
	} else {
		message = g_strdup_printf ("%i items selected",
			 g_slist_length (selection->items));
	}

	sp_view_set_status (SP_VIEW (SP_ACTIVE_DESKTOP), message, TRUE);
	
	g_free (message);
}

void
sp_selection_changed (SPSelection * selection)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	g_signal_emit (G_OBJECT (selection), selection_signals [CHANGED], 0);

	sp_selection_update_statusbar (selection);
}

SPSelection *
sp_selection_new (SPDesktop * desktop)
{
	SPSelection * selection;

	selection = g_object_new (SP_TYPE_SELECTION, NULL);

	selection->desktop = desktop;

	return selection;
}

gboolean
sp_selection_item_selected (SPSelection * selection, SPItem * item)
{
	g_return_val_if_fail (selection != NULL, FALSE);
	g_return_val_if_fail (SP_IS_SELECTION (selection), FALSE);
	g_return_val_if_fail (item != NULL, FALSE);
	g_return_val_if_fail (SP_IS_ITEM (item), FALSE);

	return (g_slist_find (selection->items, item) != NULL);
}

gboolean
sp_selection_repr_selected (SPSelection * selection, SPRepr * repr)
{
	GSList * l;

	g_return_val_if_fail (selection != NULL, FALSE);
	g_return_val_if_fail (SP_IS_SELECTION (selection), FALSE);
	g_return_val_if_fail (repr != NULL, FALSE);

	for (l = selection->items; l != NULL; l = l->next) {
		if (((SPObject *)l->data)->repr == repr) return TRUE;
	}

	return FALSE;
}

void
sp_selection_add_item (SPSelection * selection, SPItem * item)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_ITEM (item));
	g_return_if_fail (!sp_selection_item_selected (selection, item));

	g_slist_free (selection->reprs);
	selection->reprs = NULL;

	selection->items = g_slist_prepend (selection->items, item);
	g_signal_connect (G_OBJECT (item), "release",
			  G_CALLBACK (sp_selection_selected_item_release), selection);
	g_signal_connect (G_OBJECT (item), "modified",
			  G_CALLBACK (sp_selection_selected_item_modified), selection);

	sp_selection_changed (selection);
}

void
sp_selection_add_repr (SPSelection * selection, SPRepr * repr)
{
	const gchar * id;
	SPObject * object;

	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (repr != NULL);

	id = sp_repr_attr (repr, "id");
	g_return_if_fail (id != NULL);

	object = sp_document_lookup_id (SP_DT_DOCUMENT (selection->desktop), id);
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_ITEM (object));

	sp_selection_add_item (selection, SP_ITEM (object));
}

void
sp_selection_set_item (SPSelection * selection, SPItem * item)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_ITEM (item));

	sp_selection_frozen_empty (selection);

	sp_selection_add_item (selection, item);
}

void
sp_selection_set_repr (SPSelection * selection, SPRepr * repr)
{
	const gchar * id;
	SPObject * object;

	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (repr != NULL);

	id = sp_repr_attr (repr, "id");
	g_return_if_fail (id != NULL);

	object = sp_document_lookup_id (SP_DT_DOCUMENT (selection->desktop), id);
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_ITEM (object));

	sp_selection_set_item (selection, SP_ITEM (object));
}

void
sp_selection_remove_item (SPSelection * selection, SPItem * item)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_ITEM (item));
	g_return_if_fail (sp_selection_item_selected (selection, item));

	g_slist_free (selection->reprs);
	selection->reprs = NULL;

 	sp_signal_disconnect_by_data (item, selection);
	selection->items = g_slist_remove (selection->items, item);

	sp_selection_changed (selection);
}

void
sp_selection_remove_repr (SPSelection * selection, SPRepr * repr)
{
	const gchar * id;
	SPObject * object;

	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));
	g_return_if_fail (repr != NULL);

	id = sp_repr_attr (repr, "id");
	g_return_if_fail (id != NULL);

	object = sp_document_lookup_id (SP_DT_DOCUMENT (selection->desktop), id);
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_ITEM (object));

	sp_selection_remove_item (selection, SP_ITEM (object));
}

void
sp_selection_set_item_list (SPSelection * selection, const GSList * list)
{
	SPItem * i;
	const GSList * l;

	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	sp_selection_frozen_empty (selection);

	if (list != NULL) {
		for (l = list; l != NULL; l = l->next) {
			i = (SPItem *) l->data;
			if (!SP_IS_ITEM (i)) break;
			selection->items = g_slist_prepend (selection->items, i);
			g_signal_connect (G_OBJECT (i), "release",
					  G_CALLBACK (sp_selection_selected_item_release), selection);
			g_signal_connect (G_OBJECT (i), "modified",
					  G_CALLBACK (sp_selection_selected_item_modified), selection);
		}
	}

	sp_selection_changed (selection);
}

void
sp_selection_set_repr_list (SPSelection * selection, const GSList * list)
{
	GSList * itemlist;
	const GSList * l;
	SPRepr * repr;
	const gchar * id;
	SPObject * object;

	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	itemlist = NULL;

	for (l = list; l != NULL; l = l->next) {
		repr = (SPRepr *) l->data;
		g_return_if_fail (repr != NULL);
		id = sp_repr_attr (repr, "id");
		g_return_if_fail (id != NULL);

		object = sp_document_lookup_id (SP_DT_DOCUMENT (selection->desktop), id);
		g_return_if_fail (object != NULL);
		g_return_if_fail (SP_IS_ITEM (object));

		itemlist = g_slist_prepend (itemlist, object);
	}

	sp_selection_set_item_list (selection, itemlist);

	g_slist_free (itemlist);
}

void
sp_selection_empty (SPSelection * selection)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	sp_selection_frozen_empty (selection);

	sp_selection_changed (selection);
}

const GSList *
sp_selection_item_list (SPSelection * selection)
{
	g_return_val_if_fail (selection != NULL, NULL);
	g_return_val_if_fail (SP_IS_SELECTION (selection), NULL);

	return selection->items;
}

const GSList *
sp_selection_repr_list (SPSelection * selection)
{
	SPItem * i;
	GSList * l;

	g_return_val_if_fail (selection != NULL, NULL);
	g_return_val_if_fail (SP_IS_SELECTION (selection), NULL);

	g_slist_free (selection->reprs);
	selection->reprs = NULL;

	for (l = selection->items; l != NULL; l = l->next) {
		i = (SPItem *) l->data;
		selection->reprs = g_slist_prepend (selection->reprs, SP_OBJECT (i)->repr);
	}

	return selection->reprs;
}

SPItem *
sp_selection_item (SPSelection * selection)
{
	g_return_val_if_fail (selection != NULL, NULL);
	g_return_val_if_fail (SP_IS_SELECTION (selection), NULL);

	if (selection->items == NULL) return NULL;
	if (selection->items->next != NULL) return NULL;

	return SP_ITEM (selection->items->data);
}

SPRepr *
sp_selection_repr (SPSelection * selection)
{
	g_return_val_if_fail (selection != NULL, NULL);
	g_return_val_if_fail (SP_IS_SELECTION (selection), NULL);

	if (selection->items == NULL) return NULL;
	if (selection->items->next != NULL) return NULL;

	return SP_OBJECT (selection->items->data)->repr;
}

NRRectF *
sp_selection_bbox (SPSelection *selection, NRRectF *bbox)
{
	SPItem *item;
	NRRectF b;
	GSList *l;

	g_return_val_if_fail (selection != NULL, NULL);
	g_return_val_if_fail (SP_IS_SELECTION (selection), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	if (sp_selection_is_empty (selection)) {
		bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
		return bbox;
	}

	bbox->x0 = bbox->y0 = 1e18;
	bbox->x1 = bbox->y1 = -1e18;

	for (l = selection->items; l != NULL; l = l-> next) {
		item = SP_ITEM (l->data);
		sp_item_bbox_desktop (item, &b);
		if (b.x0 < bbox->x0) bbox->x0 = b.x0;
		if (b.y0 < bbox->y0) bbox->y0 = b.y0;
		if (b.x1 > bbox->x1) bbox->x1 = b.x1;
		if (b.y1 > bbox->y1) bbox->y1 = b.y1;
	}

	return bbox;
}

NRRectF *
sp_selection_bbox_document (SPSelection *selection, NRRectF *bbox)
{
	GSList *l;

	g_return_val_if_fail (selection != NULL, NULL);
	g_return_val_if_fail (SP_IS_SELECTION (selection), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	if (sp_selection_is_empty (selection)) {
		bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
		return bbox;
	}

	bbox->x0 = bbox->y0 = 1e18;
	bbox->x1 = bbox->y1 = -1e18;

	for (l = selection->items; l != NULL; l = l-> next) {
		NRMatrixF i2docf;
		NRMatrixD i2docd;

		sp_item_i2doc_affine (SP_ITEM (l->data), &i2docf);
		nr_matrix_d_from_f (&i2docd, &i2docf);
		sp_item_invoke_bbox (SP_ITEM (l->data), bbox, &i2docd, FALSE);
	}

	return bbox;
}

int
sp_selection_snappoints (SPSelection *selection, NRPointF *points, int size)
/* compute the list of points in the selection 
 * which are to be considered for snapping */
{
        GSList *l;
	NRRectF bbox;

	g_return_val_if_fail (selection != NULL, 0);
	g_return_val_if_fail (SP_IS_SELECTION (selection), 0);

	l = selection->items;

	if (l == NULL) {
		return 0;
	} else if (l->next == NULL) {
		/* selection has only one item -> take snappoints of item */
		return sp_item_snappoints (SP_ITEM (l->data), points, size);
	} else {
		int pos;
		/* selection has more than one item -> take corners of selection */
		sp_selection_bbox (selection, &bbox);

		pos = 0;
		if (pos < size) {
			points[pos].x = bbox.x0;
			points[pos].y = bbox.y0;
			pos += 1;
		}
		if (pos < size) {
			points[pos].x = bbox.x1;
			points[pos].y = bbox.y0;
			pos += 1;
		}
		if (pos < size) {
			points[pos].x = bbox.x1;
			points[pos].y = bbox.y1;
			pos += 1;
		}
		if (pos < size) {
			points[pos].x = bbox.x0;
			points[pos].y = bbox.y1;
			pos += 1;
		}
		return pos;
	}
}
