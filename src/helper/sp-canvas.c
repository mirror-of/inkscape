#define __SP_CANVAS_C__

/*
 * Port of GnomeCanvas for Sodipodi needs
 *
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <libnr/nr-values.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-pixblock.h>

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_uta.h>
#include <libart_lgpl/art_rect_uta.h>
#include <libart_lgpl/art_uta_rect.h>


#include "sp-marshal.h"

#include "sp-canvas.h"

#define SP_CANVAS_UPDATE_PRIORITY G_PRIORITY_HIGH_IDLE

#define SP_CANVAS_WINDOW(c) (((GtkWidget *) (c))->window)
#define DISPLAY_X1(canvas) (((SPCanvas *) (canvas))->x0)
#define DISPLAY_Y1(canvas) (((SPCanvas *) (canvas))->y0)
#define SP_CANVAS_PX_EPSILON 0.0625

enum {
	SP_CANVAS_ITEM_VISIBLE = 1 << 7,
	SP_CANVAS_ITEM_NEED_UPDATE = 1 << 8,
	SP_CANVAS_ITEM_NEED_AFFINE = 1 << 9,
};

struct _SPCanvasGroup {
	SPCanvasItem item;

	GList *items, *last;
};

struct _SPCanvasGroupClass {
	SPCanvasItemClass parent_class;
};

struct _SPCanvasClass {
	GtkWidgetClass parent_class;
};

static void group_add (SPCanvasGroup *group, SPCanvasItem *item);
static void group_remove (SPCanvasGroup *group, SPCanvasItem *item);

/* SPCanvasItem */

enum {ITEM_EVENT, ITEM_LAST_SIGNAL};


static void sp_canvas_request_update (SPCanvas *canvas);

static void sp_canvas_item_class_init (SPCanvasItemClass *class);
static void sp_canvas_item_init (SPCanvasItem *item);
static void sp_canvas_item_dispose (GObject *object);

static int emit_event (SPCanvas *canvas, GdkEvent *event);

static guint item_signals[ITEM_LAST_SIGNAL] = { 0 };

static GtkObjectClass *item_parent_class;

GType
sp_canvas_item_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPCanvasItemClass),
			NULL, NULL,
			(GClassInitFunc) sp_canvas_item_class_init,
			NULL, NULL,
			sizeof (SPCanvasItem),
			0,
			(GInstanceInitFunc) sp_canvas_item_init,
			NULL
		};
		type = g_type_register_static (GTK_TYPE_OBJECT, "SPCanvasItem", &info, 0);
	}

	return type;
}

/* Class initialization function for SPCanvasItemClass */
static void
sp_canvas_item_class_init (SPCanvasItemClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;

	/* fixme: Derive from GObject */
	item_parent_class = gtk_type_class (GTK_TYPE_OBJECT);

	item_signals[ITEM_EVENT] = g_signal_new ("event",
						 G_TYPE_FROM_CLASS (klass),
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET (SPCanvasItemClass, event),
						 NULL, NULL,
						 sp_marshal_BOOLEAN__POINTER,
						 G_TYPE_BOOLEAN, 1,
						 GDK_TYPE_EVENT);

	object_class->dispose = sp_canvas_item_dispose;
}

static void
sp_canvas_item_init (SPCanvasItem *item)
{
	item->object.flags |= SP_CANVAS_ITEM_VISIBLE;
}

SPCanvasItem *
sp_canvas_item_new (SPCanvasGroup *parent, GtkType type, const gchar *first_arg_name, ...)
{
	SPCanvasItem *item;
	va_list args;

	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);
	g_return_val_if_fail (gtk_type_is_a (type, sp_canvas_item_get_type ()), NULL);

	item = SP_CANVAS_ITEM (gtk_type_new (type));

	va_start (args, first_arg_name);
	sp_canvas_item_construct (item, parent, first_arg_name, args);
	va_end (args);

	return item;
}

static void
item_post_create_setup (SPCanvasItem *item)
{
	GtkObject *obj;

	obj = GTK_OBJECT (item);

	group_add (SP_CANVAS_GROUP (item->parent), item);

	sp_canvas_item_request_update (item);
	sp_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	item->canvas->need_repick = TRUE;
}

void
sp_canvas_item_construct (SPCanvasItem *item, SPCanvasGroup *parent, const gchar *first_arg_name, va_list args)
{
	g_return_if_fail (SP_IS_CANVAS_GROUP (parent));
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));

	item->parent = SP_CANVAS_ITEM (parent);
	item->canvas = item->parent->canvas;

	g_object_set_valist (G_OBJECT (item), first_arg_name, args);

	item_post_create_setup (item);
}

static void
redraw_if_visible (SPCanvasItem *item)
{
	if (item->object.flags & SP_CANVAS_ITEM_VISIBLE) {
		sp_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	}
}

static void
sp_canvas_item_dispose (GObject *object)
{
	SPCanvasItem *item;

	item = SP_CANVAS_ITEM (object);

	redraw_if_visible (item);
	item->object.flags &= ~SP_CANVAS_ITEM_VISIBLE;

	if (item == item->canvas->current_item) {
		item->canvas->current_item = NULL;
		item->canvas->need_repick = TRUE;
	}

	if (item == item->canvas->new_current_item) {
		item->canvas->new_current_item = NULL;
		item->canvas->need_repick = TRUE;
	}

	if (item == item->canvas->grabbed_item) {
		item->canvas->grabbed_item = NULL;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	if (item == item->canvas->focused_item)
		item->canvas->focused_item = NULL;

	if (item->parent) {
		group_remove (SP_CANVAS_GROUP (item->parent), item);
	}

	if (item->xform) {
		g_free (item->xform);
		item->xform = NULL;
	}

	G_OBJECT_CLASS (item_parent_class)->dispose (object);
}

/* NB! affine is parent2canvas */

static void
sp_canvas_item_invoke_update (SPCanvasItem *item, double *affine, unsigned int flags)
{
	int child_flags;
	double *child_affine;
	double new_affine[6];

	child_flags = flags;

	/* Apply the child item's transform */
	if (item->xform == NULL) {
		child_affine = affine;
	} else {
		art_affine_multiply (new_affine, item->xform, affine);
		child_affine = new_affine;
	}

	/* apply object flags to child flags */
	child_flags &= ~SP_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & SP_CANVAS_ITEM_NEED_UPDATE)
		child_flags |= SP_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & SP_CANVAS_ITEM_NEED_AFFINE)
		child_flags |= SP_CANVAS_UPDATE_AFFINE;

	if (child_flags & (SP_CANVAS_UPDATE_REQUESTED | SP_CANVAS_UPDATE_AFFINE)) {
		if (SP_CANVAS_ITEM_GET_CLASS (item)->update)
			SP_CANVAS_ITEM_GET_CLASS (item)->update (item, child_affine, child_flags);
	}

	GTK_OBJECT_UNSET_FLAGS (item, SP_CANVAS_ITEM_NEED_UPDATE);
	GTK_OBJECT_UNSET_FLAGS (item, SP_CANVAS_ITEM_NEED_AFFINE);
}

/* This routine invokes the point method of the item.  The argument x, y should
 * be in the parent's item-relative coordinate system.  This routine applies the
 * inverse of the item's transform, maintaining the affine invariant.
 */
static double
sp_canvas_item_invoke_point (SPCanvasItem *item, double x, double y, SPCanvasItem **actual_item)
{
	if (SP_CANVAS_ITEM_GET_CLASS (item)->point)
		return SP_CANVAS_ITEM_GET_CLASS (item)->point (item, x, y, actual_item);

	return NR_HUGE_D;
}

/**
 * sp_canvas_item_affine_absolute:
 * @item: A canvas item.
 * @affine: An affine transformation matrix.
 *
 * Makes the item's affine transformation matrix be equal to the specified
 * matrix.
 **/
void
sp_canvas_item_affine_absolute (SPCanvasItem *item, const double affine[6])
{
	int i;

	if (item->xform == NULL) {
		item->xform = g_new (double, 6);
	}
	for (i = 0; i < 6; i++) {
		item->xform[i] = affine[i];
	}

	if (!(item->object.flags & SP_CANVAS_ITEM_NEED_AFFINE)) {
		item->object.flags |= SP_CANVAS_ITEM_NEED_AFFINE;
		if (item->parent != NULL)
			sp_canvas_item_request_update (item->parent);
		else
			sp_canvas_request_update (item->canvas);
	}

	item->canvas->need_repick = TRUE;
}

/* Convenience function to reorder items in a group's child list.  This puts the
 * specified link after the "before" link.
 */
static void
put_item_after (GList *link, GList *before)
{
	SPCanvasGroup *parent;

	if (link == before)
		return;

	parent = SP_CANVAS_GROUP (SP_CANVAS_ITEM (link->data)->parent);

	if (before == NULL) {
		if (link == parent->items) return;

		link->prev->next = link->next;

		if (link->next) {
			link->next->prev = link->prev;
		} else {
			parent->last = link->prev;
		}

		link->prev = before;
		link->next = parent->items;
		link->next->prev = link;
		parent->items = link;
	} else {
		if ((link == parent->last) && (before == parent->last->prev))
			return;

		if (link->next)
			link->next->prev = link->prev;

		if (link->prev)
			link->prev->next = link->next;
		else {
			parent->items = link->next;
			parent->items->prev = NULL;
		}

		link->prev = before;
		link->next = before->next;

		link->prev->next = link;

		if (link->next)
			link->next->prev = link;
		else
			parent->last = link;
	}
}


/**
 * sp_canvas_item_raise:
 * @item: A canvas item.
 * @positions: Number of steps to raise the item.
 *
 * Raises the item in its parent's stack by the specified number of positions.
 * If the number of positions is greater than the distance to the top of the
 * stack, then the item is put at the top.
 **/
void
sp_canvas_item_raise (SPCanvasItem *item, int positions)
{
	GList *link, *before;
	SPCanvasGroup *parent;

	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));
	g_return_if_fail (positions >= 0);

	if (!item->parent || positions == 0)
		return;

	parent = SP_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->items, item);
	g_assert (link != NULL);

	for (before = link; positions && before; positions--)
		before = before->next;

	if (!before)
		before = parent->last;

	put_item_after (link, before);

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;
}


/**
 * sp_canvas_item_lower:
 * @item: A canvas item.
 * @positions: Number of steps to lower the item.
 *
 * Lowers the item in its parent's stack by the specified number of positions.
 * If the number of positions is greater than the distance to the bottom of the
 * stack, then the item is put at the bottom.
 **/
void
sp_canvas_item_lower (SPCanvasItem *item, int positions)
{
	GList *link, *before;
	SPCanvasGroup *parent;

	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));
	g_return_if_fail (positions >= 1);

	if (!item->parent || positions == 0)
		return;

	parent = SP_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->items, item);
	g_assert (link != NULL);

	if (link->prev)
		for (before = link->prev; positions && before; positions--)
			before = before->prev;
	else
		before = NULL;

	put_item_after (link, before);

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;
}

void
sp_canvas_item_show (SPCanvasItem *item)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));

	if (item->object.flags & SP_CANVAS_ITEM_VISIBLE)
		return;

	item->object.flags |= SP_CANVAS_ITEM_VISIBLE;

	sp_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	item->canvas->need_repick = TRUE;
}

void
sp_canvas_item_hide (SPCanvasItem *item)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));

	if (!(item->object.flags & SP_CANVAS_ITEM_VISIBLE)) return;

	item->object.flags &= ~SP_CANVAS_ITEM_VISIBLE;

	sp_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	item->canvas->need_repick = TRUE;
}

int
sp_canvas_item_grab (SPCanvasItem *item, guint event_mask, GdkCursor *cursor, guint32 etime)
{
	g_return_val_if_fail (item != NULL, -1);
	g_return_val_if_fail (SP_IS_CANVAS_ITEM (item), -1);
	g_return_val_if_fail (GTK_WIDGET_MAPPED (item->canvas), -1);

	if (item->canvas->grabbed_item) return -1;

	if (!(item->object.flags & SP_CANVAS_ITEM_VISIBLE)) return -1;

	/* fixme: Top hack (Lauris) */
	/* fixme: If we add key masks to event mask, Gdk will abort (Lauris) */
	/* fixme: But Canvas actualle does get key events, so all we need is routing these here */
	gdk_pointer_grab (SP_CANVAS_WINDOW (item->canvas), FALSE,
			  event_mask & (~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK)),
			  NULL, cursor, etime);

	item->canvas->grabbed_item = item;
	item->canvas->grabbed_event_mask = event_mask;
	item->canvas->current_item = item; /* So that events go to the grabbed item */

	return 0;
}

/**
 * sp_canvas_item_ungrab:
 * @item: A canvas item that holds a grab.
 * @etime: The timestamp for ungrabbing the mouse.
 *
 * Ungrabs the item, which must have been grabbed in the canvas, and ungrabs the
 * mouse.
 **/
void
sp_canvas_item_ungrab (SPCanvasItem *item, guint32 etime)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));

	if (item->canvas->grabbed_item != item)
		return;

	item->canvas->grabbed_item = NULL;

	gdk_pointer_ungrab (etime);
}


void
sp_canvas_item_i2w_affine (SPCanvasItem *item, double affine[6])
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));
	g_return_if_fail (affine != NULL);

	art_affine_identity (affine);

	while (item) {
		if (item->xform != NULL) {
			art_affine_multiply (affine, affine, item->xform);
		}
		item = item->parent;
	}
}

void
sp_canvas_item_w2i (SPCanvasItem *item, double *x, double *y)
{
	double i2w[6], w2i[6];
	double px, py;

	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));
	g_return_if_fail (x != NULL);
	g_return_if_fail (y != NULL);

	sp_canvas_item_i2w_affine (item, i2w);
	art_affine_invert (w2i, i2w);

	px = *x;
	py = *y;

	*x = w2i[0] * px + w2i[2] * py + w2i[4];
	*y = w2i[1] * px + w2i[3] * py + w2i[5];
}

void
sp_canvas_item_i2w (SPCanvasItem *item, double *x, double *y)
{
	double i2w[6];
	double px, py;

	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));
	g_return_if_fail (x != NULL);
	g_return_if_fail (y != NULL);

	sp_canvas_item_i2w_affine (item, i2w);

	px = *x;
	py = *y;

	*x = i2w[0] * px + i2w[2] * py + i2w[4];
	*y = i2w[1] * px + i2w[3] * py + i2w[5];
}

static int
is_descendant (SPCanvasItem *item, SPCanvasItem *parent)
{
	while (item) {
		if (item == parent) return TRUE;
		item = item->parent;
	}

	return FALSE;
}

void
sp_canvas_item_grab_focus (SPCanvasItem *item)
{
	SPCanvasItem *focused_item;
	GdkEvent ev;

	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_CANVAS_ITEM (item));
	g_return_if_fail (GTK_WIDGET_CAN_FOCUS (GTK_WIDGET (item->canvas)));

	focused_item = item->canvas->focused_item;

	if (focused_item) {
		ev.focus_change.type = GDK_FOCUS_CHANGE;
		ev.focus_change.window = SP_CANVAS_WINDOW (item->canvas);
		ev.focus_change.send_event = FALSE;
		ev.focus_change.in = FALSE;

		emit_event (item->canvas, &ev);
	}

	item->canvas->focused_item = item;
	gtk_widget_grab_focus (GTK_WIDGET (item->canvas));

	if (focused_item) {
		ev.focus_change.type = GDK_FOCUS_CHANGE;
		ev.focus_change.window = SP_CANVAS_WINDOW (item->canvas);
		ev.focus_change.send_event = FALSE;
		ev.focus_change.in = TRUE;
		
		emit_event (item->canvas, &ev);
	}
}

/**
 * sp_canvas_item_request_update
 * @item: A canvas item.
 *
 * To be used only by item implementations.  Requests that the canvas queue an
 * update for the specified item.
 **/
void
sp_canvas_item_request_update (SPCanvasItem *item)
{
	if (item->object.flags & SP_CANVAS_ITEM_NEED_UPDATE)
		return;

	item->object.flags |= SP_CANVAS_ITEM_NEED_UPDATE;

	if (item->parent != NULL) {
		/* Recurse up the tree */
		sp_canvas_item_request_update (item->parent);
	} else {
		/* Have reached the top of the tree, make sure the update call gets scheduled. */
		sp_canvas_request_update (item->canvas);
	}
}

gint sp_canvas_item_order (SPCanvasItem * item)
{
	return g_list_index (SP_CANVAS_GROUP (item->parent)->items, item);
}

/* SPCanvasGroup */

static void sp_canvas_group_class_init (SPCanvasGroupClass *class);
static void sp_canvas_group_init (SPCanvasGroup *group);
static void sp_canvas_group_destroy (GtkObject *object);

static void sp_canvas_group_update (SPCanvasItem *item, double *affine, unsigned int flags);
static double sp_canvas_group_point (SPCanvasItem *item, double x, double y, SPCanvasItem **actual_item);
static void sp_canvas_group_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *group_parent_class;

GtkType
sp_canvas_group_get_type (void)
{
	static GtkType group_type = 0;

	if (!group_type) {
		static const GtkTypeInfo group_info = {
			"SPCanvasGroup",
			sizeof (SPCanvasGroup),
			sizeof (SPCanvasGroupClass),
			(GtkClassInitFunc) sp_canvas_group_class_init,
			(GtkObjectInitFunc) sp_canvas_group_init,
			NULL, NULL
		};

		group_type = gtk_type_unique (sp_canvas_item_get_type (), &group_info);
	}

	return group_type;
}

/* Class initialization function for SPCanvasGroupClass */
static void
sp_canvas_group_class_init (SPCanvasGroupClass *class)
{
	GtkObjectClass *object_class;
	SPCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (SPCanvasItemClass *) class;

	group_parent_class = gtk_type_class (sp_canvas_item_get_type ());

	object_class->destroy = sp_canvas_group_destroy;

	item_class->update = sp_canvas_group_update;
	item_class->render = sp_canvas_group_render;
	item_class->point = sp_canvas_group_point;
}

static void
sp_canvas_group_init (SPCanvasGroup *group)
{
	/* Nothing here */
}

static void
sp_canvas_group_destroy (GtkObject *object)
{
	SPCanvasGroup *group;
	SPCanvasItem *child;
	GList *list;

	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_CANVAS_GROUP (object));

	group = SP_CANVAS_GROUP (object);

	list = group->items;
	while (list) {
		child = list->data;
		list = list->next;

		gtk_object_destroy (GTK_OBJECT (child));
	}

	if (GTK_OBJECT_CLASS (group_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (group_parent_class)->destroy) (object);
}

/* Update handler for canvas groups */
static void
sp_canvas_group_update (SPCanvasItem *item, double *affine, unsigned int flags)
{
	SPCanvasGroup *group;
	GList *list;
	SPCanvasItem *i;
	ArtDRect bbox, child_bbox;

	group = SP_CANVAS_GROUP (item);

	bbox.x0 = 0;
	bbox.y0 = 0;
	bbox.x1 = 0;
	bbox.y1 = 0;

	for (list = group->items; list; list = list->next) {
		i = list->data;

		sp_canvas_item_invoke_update (i, affine, flags);

		child_bbox.x0 = i->x1;
		child_bbox.y0 = i->y1;
		child_bbox.x1 = i->x2;
		child_bbox.y1 = i->y2;
		art_drect_union (&bbox, &bbox, &child_bbox);
	}
	item->x1 = bbox.x0;
	item->y1 = bbox.y0;
	item->x2 = bbox.x1;
	item->y2 = bbox.y1;
}

/* Point handler for canvas groups */
static double
sp_canvas_group_point (SPCanvasItem *item, double x, double y, SPCanvasItem **actual_item)
{
	SPCanvasGroup *group;
	GList *list;
	SPCanvasItem *child, *point_item;
	int x1, y1, x2, y2;
	double dist, best;
	int has_point;

	group = SP_CANVAS_GROUP (item);

	x1 = x - item->canvas->close_enough;
	y1 = y - item->canvas->close_enough;
	x2 = x + item->canvas->close_enough;
	y2 = y + item->canvas->close_enough;

	best = 0.0;
	*actual_item = NULL;

	dist = 0.0; /* keep gcc happy */

	for (list = group->items; list; list = list->next) {
		child = list->data;

		if ((child->x1 <= x2) && (child->y1 <= y2) && (child->x2 >= x1) && (child->y2 >= y1)) {
			point_item = NULL; /* cater for incomplete item implementations */

			if ((child->object.flags & SP_CANVAS_ITEM_VISIBLE) && SP_CANVAS_ITEM_GET_CLASS (child)->point) {
				dist = sp_canvas_item_invoke_point (child, x, y, &point_item);
				has_point = TRUE;
			} else
				has_point = FALSE;

			if (has_point && point_item && ((int) (dist + 0.5) <= item->canvas->close_enough)) {
				best = dist;
				*actual_item = point_item;
			}
		}
	}

	return best;
}

static void
sp_canvas_group_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
	SPCanvasGroup *group;
	SPCanvasItem *child;
	GList *list;

	group = SP_CANVAS_GROUP (item);

	for (list = group->items; list; list = list->next) {
		child = list->data;
		if (child->object.flags & SP_CANVAS_ITEM_VISIBLE) {
			if ((child->x1 < buf->rect.x1) &&
			    (child->y1 < buf->rect.y1) &&
			    (child->x2 > buf->rect.x0) &&
			    (child->y2 > buf->rect.y0)) {
				if (SP_CANVAS_ITEM_GET_CLASS (child)->render)
					SP_CANVAS_ITEM_GET_CLASS (child)->render (child, buf);
			}
		}
	}
}

/* Adds an item to a group */
static void
group_add (SPCanvasGroup *group, SPCanvasItem *item)
{
	gtk_object_ref (GTK_OBJECT (item));
	gtk_object_sink (GTK_OBJECT (item));

	if (!group->items) {
		group->items = g_list_append (group->items, item);
		group->last = group->items;
	} else {
		group->last = g_list_append (group->last, item)->next;
	}

	sp_canvas_item_request_update (item);
}

/* Removes an item from a group */
static void
group_remove (SPCanvasGroup *group, SPCanvasItem *item)
{
	GList *children;

	g_return_if_fail (group != NULL);
	g_return_if_fail (SP_IS_CANVAS_GROUP (group));
	g_return_if_fail (item != NULL);

	for (children = group->items; children; children = children->next) {
		if (children->data == item) {

			/* Unparent the child */

			item->parent = NULL;
			gtk_object_unref (GTK_OBJECT (item));

			/* Remove it from the list */

			if (children == group->last) group->last = children->prev;

			group->items = g_list_remove_link (group->items, children);
			g_list_free (children);
			break;
		}
	}
}

/* SPCanvas */

static void sp_canvas_class_init (SPCanvasClass *class);
static void sp_canvas_init (SPCanvas *canvas);
static void sp_canvas_destroy (GtkObject *object);

static void sp_canvas_realize (GtkWidget *widget);
static void sp_canvas_unrealize (GtkWidget *widget);

static void sp_canvas_size_request (GtkWidget *widget, GtkRequisition *req);
static void sp_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gint sp_canvas_button (GtkWidget *widget, GdkEventButton *event);
static gint sp_canvas_motion (GtkWidget *widget, GdkEventMotion *event);
static gint sp_canvas_expose (GtkWidget *widget, GdkEventExpose *event);
static gint sp_canvas_key (GtkWidget *widget, GdkEventKey *event);
static gint sp_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event);
static gint sp_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint sp_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event);

static GtkWidgetClass *canvas_parent_class;

/**
 * sp_canvas_get_type:
 *
 * Registers the &SPCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &SPCanvas class.
 **/
GtkType
sp_canvas_get_type (void)
{
	static GtkType canvas_type = 0;

	if (!canvas_type) {
		static const GtkTypeInfo canvas_info = {
			"SPCanvas",
			sizeof (SPCanvas),
			sizeof (SPCanvasClass),
			(GtkClassInitFunc) sp_canvas_class_init,
			(GtkObjectInitFunc) sp_canvas_init,
			NULL, NULL
		};

		canvas_type = gtk_type_unique (GTK_TYPE_WIDGET, &canvas_info);
	}

	return canvas_type;
}

/* Class initialization function for SPCanvasClass */
static void
sp_canvas_class_init (SPCanvasClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	canvas_parent_class = gtk_type_class (GTK_TYPE_WIDGET);

	object_class->destroy = sp_canvas_destroy;

	widget_class->realize = sp_canvas_realize;
	widget_class->unrealize = sp_canvas_unrealize;
	widget_class->size_request = sp_canvas_size_request;
	widget_class->size_allocate = sp_canvas_size_allocate;
	widget_class->button_press_event = sp_canvas_button;
	widget_class->button_release_event = sp_canvas_button;
	widget_class->motion_notify_event = sp_canvas_motion;
	widget_class->expose_event = sp_canvas_expose;
	widget_class->key_press_event = sp_canvas_key;
	widget_class->key_release_event = sp_canvas_key;
	widget_class->enter_notify_event = sp_canvas_crossing;
	widget_class->leave_notify_event = sp_canvas_crossing;
	widget_class->focus_in_event = sp_canvas_focus_in;
	widget_class->focus_out_event = sp_canvas_focus_out;
}

/* Object initialization function for SPCanvas */
static void
sp_canvas_init (SPCanvas *canvas)
{
	GTK_WIDGET_UNSET_FLAGS (canvas, GTK_NO_WINDOW);
	GTK_WIDGET_UNSET_FLAGS (canvas, GTK_DOUBLE_BUFFERED);
	GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);

	canvas->pick_event.type = GDK_LEAVE_NOTIFY;
	canvas->pick_event.crossing.x = 0;
	canvas->pick_event.crossing.y = 0;

	/* Create the root item as a special case */
	canvas->root = SP_CANVAS_ITEM (gtk_type_new (sp_canvas_group_get_type ()));
	canvas->root->canvas = canvas;

	gtk_object_ref (GTK_OBJECT (canvas->root));
	gtk_object_sink (GTK_OBJECT (canvas->root));

	canvas->need_repick = TRUE;
}

/* Convenience function to remove the idle handler of a canvas */
static void
remove_idle (SPCanvas *canvas)
{
	if (canvas->idle_id) {
		gtk_idle_remove (canvas->idle_id);
		canvas->idle_id = 0;
	}
}

/* Removes the transient state of the canvas (idle handler, grabs). */
static void
shutdown_transients (SPCanvas *canvas)
{
	/* We turn off the need_redraw flag, since if the canvas is mapped again
	 * it will request a redraw anyways.  We do not turn off the need_update
	 * flag, though, because updates are not queued when the canvas remaps
	 * itself.
	 */
	if (canvas->need_redraw) {
		canvas->need_redraw = FALSE;
		art_uta_free (canvas->redraw_area);
		canvas->redraw_area = NULL;
	}

	if (canvas->grabbed_item) {
		canvas->grabbed_item = NULL;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	remove_idle (canvas);
}

/* Destroy handler for SPCanvas */
static void
sp_canvas_destroy (GtkObject *object)
{
	SPCanvas *canvas;

	canvas = SP_CANVAS (object);

	if (canvas->root) {
		gtk_object_unref (GTK_OBJECT (canvas->root));
		canvas->root = NULL;
	}

	shutdown_transients (canvas);

	if (GTK_OBJECT_CLASS (canvas_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (canvas_parent_class)->destroy) (object);
}

GtkWidget *
sp_canvas_new_aa (void)
{
	SPCanvas *canvas;

	canvas = gtk_type_new (sp_canvas_get_type ());

	return (GtkWidget *) canvas;
}

static void
sp_canvas_realize (GtkWidget *widget)
{
	SPCanvas *canvas;
	GdkWindowAttr attributes;
	gint attributes_mask;

	canvas = SP_CANVAS (widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gdk_rgb_get_visual ();
	attributes.colormap = gdk_rgb_get_cmap ();
	attributes.event_mask = (gtk_widget_get_events (widget) |
				 GDK_EXPOSURE_MASK |
				 GDK_BUTTON_PRESS_MASK |
				 GDK_BUTTON_RELEASE_MASK |
				 GDK_POINTER_MOTION_MASK |
				 GDK_KEY_PRESS_MASK |
				 GDK_KEY_RELEASE_MASK |
				 GDK_ENTER_NOTIFY_MASK |
				 GDK_LEAVE_NOTIFY_MASK |
				 GDK_FOCUS_CHANGE_MASK);
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, widget);

	canvas->pixmap_gc = gdk_gc_new (SP_CANVAS_WINDOW (canvas));
}

static void
sp_canvas_unrealize (GtkWidget *widget)
{
	SPCanvas *canvas;

	canvas = SP_CANVAS (widget);

	shutdown_transients (canvas);

	gdk_gc_destroy (canvas->pixmap_gc);
	canvas->pixmap_gc = NULL;

	if (GTK_WIDGET_CLASS (canvas_parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->unrealize) (widget);
}

static void
sp_canvas_size_request (GtkWidget *widget, GtkRequisition *req)
{
	SPCanvas *canvas;

	canvas = SP_CANVAS (widget);

	req->width = 256;
	req->height = 256;
}

static void
sp_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	SPCanvas *canvas;

	canvas = SP_CANVAS (widget);

	/* Schedule redraw of new region */
	if (allocation->width > widget->allocation.width) {
		sp_canvas_request_redraw (canvas,
					  canvas->x0 + widget->allocation.width,
					  0,
					  canvas->x0 + allocation->width,
					  canvas->y0 + allocation->height);
	}
	if (allocation->height > widget->allocation.height) {
		sp_canvas_request_redraw (canvas,
					  0,
					  canvas->y0 + widget->allocation.height,
					  canvas->x0 + allocation->width,
					  canvas->y0 + allocation->height);
	}

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED (widget)) {
		gdk_window_move_resize (widget->window,
					widget->allocation.x, widget->allocation.y,
					widget->allocation.width, widget->allocation.height);
	}
}

static void
scroll_to (SPCanvas *canvas, float x, float y, unsigned int clear)
{
	int ix, iy, dx, dy;

	ix = (int) (x + 0.5);
	iy = (int) (y + 0.5);
	dx = ix - canvas->x0;
	dy = iy - canvas->y0;

	canvas->dx0 = x;
	canvas->dy0 = y;
	canvas->x0 = ix;
	canvas->y0 = iy;

	if (!clear) {
		if ((dx != 0) || (dy != 0)) {
			int width, height;
			width = canvas->widget.allocation.width;
			height = canvas->widget.allocation.height;
			if (GTK_WIDGET_REALIZED (canvas)) {
				gdk_window_scroll (SP_CANVAS_WINDOW (canvas), -dx, -dy);
				gdk_window_process_updates (SP_CANVAS_WINDOW (canvas), TRUE);
			}
			if (dx < 0) {
				sp_canvas_request_redraw (canvas, ix + 0, iy + 0, ix - dx, iy + height);
			} else if (dx > 0) {
				sp_canvas_request_redraw (canvas, ix + width - dx, iy + 0, ix + width, iy + height);
			}
			if (dy < 0) {
				sp_canvas_request_redraw (canvas, ix + 0, iy + 0, ix + width, iy - dy);
			} else if (dy > 0) {
				sp_canvas_request_redraw (canvas, ix + 0, iy + height - dy, ix + width, iy + height);
			}
		}
	} else {
		gtk_widget_queue_draw (GTK_WIDGET (canvas));
	}
}

/* Emits an event for an item in the canvas, be it the current item, grabbed
 * item, or focused item, as appropriate.
 */
static int
emit_event (SPCanvas *canvas, GdkEvent *event)
{
	GdkEvent ev;
	gint finished;
	SPCanvasItem *item;
	SPCanvasItem *parent;
	guint mask;

	/* Perform checks for grabbed items */
	if (canvas->grabbed_item && !is_descendant (canvas->current_item, canvas->grabbed_item)) return FALSE;

	if (canvas->grabbed_item) {
		switch (event->type) {
		case GDK_ENTER_NOTIFY:
			mask = GDK_ENTER_NOTIFY_MASK;
			break;
		case GDK_LEAVE_NOTIFY:
			mask = GDK_LEAVE_NOTIFY_MASK;
			break;
		case GDK_MOTION_NOTIFY:
			mask = GDK_POINTER_MOTION_MASK;
			break;
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
			mask = GDK_BUTTON_PRESS_MASK;
			break;
		case GDK_BUTTON_RELEASE:
			mask = GDK_BUTTON_RELEASE_MASK;
			break;
		case GDK_KEY_PRESS:
			mask = GDK_KEY_PRESS_MASK;
			break;
		case GDK_KEY_RELEASE:
			mask = GDK_KEY_RELEASE_MASK;
			break;
		default:
			mask = 0;
			break;
		}

		if (!(mask & canvas->grabbed_event_mask)) return FALSE;
	}

	/* Convert to world coordinates -- we have two cases because of diferent
	 * offsets of the fields in the event structures.
	 */

	ev = *event;

	switch (ev.type) {
	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		ev.crossing.x += canvas->x0;
		ev.crossing.y += canvas->y0;
		break;
	case GDK_MOTION_NOTIFY:
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		ev.motion.x += canvas->x0;
		ev.motion.y += canvas->y0;
		break;
	default:
		break;
	}

	/* Choose where we send the event */

	item = canvas->current_item;

	if (canvas->focused_item &&
	    ((event->type == GDK_KEY_PRESS) || (event->type == GDK_KEY_RELEASE) || (event->type == GDK_FOCUS_CHANGE))) {
		item = canvas->focused_item;
	}

	/* The event is propagated up the hierarchy (for if someone connected to
	 * a group instead of a leaf event), and emission is stopped if a
	 * handler returns TRUE, just like for GtkWidget events.
	 */

	finished = FALSE;

	while (item && !finished) {
		gtk_object_ref (GTK_OBJECT (item));
		gtk_signal_emit (GTK_OBJECT (item), item_signals[ITEM_EVENT], &ev, &finished);
#if 0
		/* Why the heck is this good for? (Lauris) */
		if (GTK_OBJECT_DESTROYED (item)) finished = TRUE;
#endif
		parent = item->parent;
		gtk_object_unref (GTK_OBJECT (item));
		item = parent;
	}

	return finished;
}

/* Re-picks the current item in the canvas, based on the event's coordinates.
 * Also emits enter/leave events for items as appropriate.
 */
static int
pick_current_item (SPCanvas *canvas, GdkEvent *event)
{
	int button_down;
	double x, y;
	int retval;

	retval = FALSE;

	/* If a button is down, we'll perform enter and leave events on the
	 * current item, but not enter on any other item.  This is more or less
	 * like X pointer grabbing for canvas items.
	 */
	button_down = canvas->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK | GDK_BUTTON4_MASK | GDK_BUTTON5_MASK);

	if (!button_down) canvas->left_grabbed_item = FALSE;

	/* Save the event in the canvas.  This is used to synthesize enter and
	 * leave events in case the current item changes.  It is also used to
	 * re-pick the current item if the current one gets deleted.  Also,
	 * synthesize an enter event.
	 */
	if (event != &canvas->pick_event) {
		if ((event->type == GDK_MOTION_NOTIFY) || (event->type == GDK_BUTTON_RELEASE)) {
			/* these fields have the same offsets in both types of events */

			canvas->pick_event.crossing.type       = GDK_ENTER_NOTIFY;
			canvas->pick_event.crossing.window     = event->motion.window;
			canvas->pick_event.crossing.send_event = event->motion.send_event;
			canvas->pick_event.crossing.subwindow  = NULL;
			canvas->pick_event.crossing.x          = event->motion.x;
			canvas->pick_event.crossing.y          = event->motion.y;
			canvas->pick_event.crossing.mode       = GDK_CROSSING_NORMAL;
			canvas->pick_event.crossing.detail     = GDK_NOTIFY_NONLINEAR;
			canvas->pick_event.crossing.focus      = FALSE;
			canvas->pick_event.crossing.state      = event->motion.state;

			/* these fields don't have the same offsets in both types of events */

			if (event->type == GDK_MOTION_NOTIFY) {
				canvas->pick_event.crossing.x_root = event->motion.x_root;
				canvas->pick_event.crossing.y_root = event->motion.y_root;
			} else {
				canvas->pick_event.crossing.x_root = event->button.x_root;
				canvas->pick_event.crossing.y_root = event->button.y_root;
			}
		} else {
			canvas->pick_event = *event;
		}
	}

	/* Don't do anything else if this is a recursive call */
	if (canvas->in_repick) return retval;

	/* LeaveNotify means that there is no current item, so we don't look for one */
	if (canvas->pick_event.type != GDK_LEAVE_NOTIFY) {
		/* these fields don't have the same offsets in both types of events */

		if (canvas->pick_event.type == GDK_ENTER_NOTIFY) {
			x = canvas->pick_event.crossing.x;
			y = canvas->pick_event.crossing.y;
		} else {
			x = canvas->pick_event.motion.x;
			y = canvas->pick_event.motion.y;
		}

		/* world coords */
		x += canvas->x0;
		y += canvas->y0;

		/* find the closest item */
		if (canvas->root->object.flags & SP_CANVAS_ITEM_VISIBLE) {
			sp_canvas_item_invoke_point (canvas->root, x, y, &canvas->new_current_item);
		} else {
			canvas->new_current_item = NULL;
		}
	} else {
		canvas->new_current_item = NULL;
	}

	if ((canvas->new_current_item == canvas->current_item) && !canvas->left_grabbed_item) {
		return retval; /* current item did not change */
	}

	/* Synthesize events for old and new current items */

	if ((canvas->new_current_item != canvas->current_item)
	    && (canvas->current_item != NULL)
	    && !canvas->left_grabbed_item) {
		GdkEvent new_event;
		SPCanvasItem *item;

		item = canvas->current_item;

		new_event = canvas->pick_event;
		new_event.type = GDK_LEAVE_NOTIFY;

		new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
		new_event.crossing.subwindow = NULL;
		canvas->in_repick = TRUE;
		retval = emit_event (canvas, &new_event);
		canvas->in_repick = FALSE;
	}

	/* new_current_item may have been set to NULL during the call to emit_event() above */

	if ((canvas->new_current_item != canvas->current_item) && button_down) {
		canvas->left_grabbed_item = TRUE;
		return retval;
	}

	/* Handle the rest of cases */

	canvas->left_grabbed_item = FALSE;
	canvas->current_item = canvas->new_current_item;

	if (canvas->current_item != NULL) {
		GdkEvent new_event;

		new_event = canvas->pick_event;
		new_event.type = GDK_ENTER_NOTIFY;
		new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
		new_event.crossing.subwindow = NULL;
		retval = emit_event (canvas, &new_event);
	}

	return retval;
}

/* Button event handler for the canvas */
static gint
sp_canvas_button (GtkWidget *widget, GdkEventButton *event)
{
	SPCanvas *canvas;
	int mask;
	int retval;

	canvas = SP_CANVAS (widget);

	retval = FALSE;

	/* dispatch normally regardless of the event's window if an item has
	   has a pointer grab in effect */
	if (!canvas->grabbed_item && event->window != SP_CANVAS_WINDOW (canvas)) return retval;

	switch (event->button) {
	case 1:
		mask = GDK_BUTTON1_MASK;
		break;
	case 2:
		mask = GDK_BUTTON2_MASK;
		break;
	case 3:
		mask = GDK_BUTTON3_MASK;
		break;
	case 4:
		mask = GDK_BUTTON4_MASK;
		break;
	case 5:
		mask = GDK_BUTTON5_MASK;
		break;
	default:
		mask = 0;
	}

	switch (event->type) {
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
		/* Pick the current item as if the button were not pressed, and
		 * then process the event.
		 */
		canvas->state = event->state;
		pick_current_item (canvas, (GdkEvent *) event);
		canvas->state ^= mask;
		retval = emit_event (canvas, (GdkEvent *) event);
		break;

	case GDK_BUTTON_RELEASE:
		/* Process the event as if the button were pressed, then repick
		 * after the button has been released
		 */
		canvas->state = event->state;
		retval = emit_event (canvas, (GdkEvent *) event);
		event->state ^= mask;
		canvas->state = event->state;
		pick_current_item (canvas, (GdkEvent *) event);
		event->state ^= mask;
		break;

	default:
		g_assert_not_reached ();
	}

	return retval;
}

/* Motion event handler for the canvas */
static int
sp_canvas_motion (GtkWidget *widget, GdkEventMotion *event)
{
	SPCanvas *canvas;
	int ret;

	canvas = SP_CANVAS (widget);

	if (event->window != SP_CANVAS_WINDOW (canvas)) return FALSE;

	if (canvas->grabbed_event_mask & GDK_POINTER_MOTION_HINT_MASK) {
		gint x, y;
		gdk_window_get_pointer (widget->window, &x, &y, NULL);
		event->x = x;
		event->y = y;
	}

	canvas->state = event->state;
	pick_current_item (canvas, (GdkEvent *) event);
	ret = emit_event (canvas, (GdkEvent *) event);

	return ret;
}

/* We have to fit into pixelstore 64K */
#define IMAGE_WIDTH_AA 341
#define IMAGE_HEIGHT_AA 64

static void
sp_canvas_paint_rect (SPCanvas *canvas, int x0, int y0, int x1, int y1)
{
	GtkWidget *widget;
	int draw_x1, draw_y1, draw_x2, draw_y2;
	int bw, bh, sw, sh;

	g_return_if_fail (!canvas->need_update);

	widget = GTK_WIDGET (canvas);

	draw_x1 = MAX (x0, canvas->x0);
	draw_y1 = MAX (y0, canvas->y0);
	draw_x2 = MIN (x1, draw_x1 + GTK_WIDGET (canvas)->allocation.width);
	draw_y2 = MIN (y1, draw_y1 + GTK_WIDGET (canvas)->allocation.height);

	bw = draw_x2 - draw_x1;
	bh = draw_y2 - draw_y1;
	if ((bw < 1) || (bh < 1)) return;

	/* 65536 is max cached buffer and we need 3 channels */
	if (bw * bh < 21845) {
		/* We can go with single buffer */
		sw = bw;
		sh = bh;
	} else if (bw <= (16 * IMAGE_WIDTH_AA)) {
		/* Go with row buffer */
		sw = bw;
		sh = 21845 / bw;
	} else if (bh <= (16 * IMAGE_HEIGHT_AA)) {
		/* Go with column buffer */
		sw = 21845 / bh;
		sh = bh;
	} else {
		sw = IMAGE_WIDTH_AA;
		sh = IMAGE_HEIGHT_AA;
	}

	/* As we can come from expose, we have to tile here */
	for (y0 = draw_y1; y0 < draw_y2; y0 += sh) {
		y1 = MIN (y0 + sh, draw_y2);
		for (x0 = draw_x1; x0 < draw_x2; x0 += sw) {
			SPCanvasBuf buf;
			GdkColor *color;

			x1 = MIN (x0 + sw, draw_x2);

			buf.buf = nr_pixelstore_64K_new (0, 0);
			buf.buf_rowstride = sw * 3;
			buf.rect.x0 = x0;
			buf.rect.y0 = y0;
			buf.rect.x1 = x1;
			buf.rect.y1 = y1;
			color = &widget->style->bg[GTK_STATE_NORMAL];
			buf.bg_color = (((color->red & 0xff00) << 8)
					| (color->green & 0xff00)
					| (color->blue >> 8));
			buf.is_bg = 1;
			buf.is_buf = 0;

			if (canvas->root->object.flags & SP_CANVAS_ITEM_VISIBLE) {
				SP_CANVAS_ITEM_GET_CLASS (canvas->root)->render (canvas->root, &buf);
			}

			if (buf.is_bg) {
				gdk_rgb_gc_set_foreground (canvas->pixmap_gc, buf.bg_color);
				gdk_draw_rectangle (SP_CANVAS_WINDOW (canvas),
						    canvas->pixmap_gc,
						    TRUE,
						    x0 - canvas->x0, y0 - canvas->y0,
						    x1 - x0, y1 - y0);
			} else {
				gdk_draw_rgb_image_dithalign (SP_CANVAS_WINDOW (canvas),
							      canvas->pixmap_gc,
							      x0 - canvas->x0, y0 - canvas->y0,
							      x1 - x0, y1 - y0,
							      GDK_RGB_DITHER_MAX,
							      buf.buf,
							      sw * 3,
							      x0 - canvas->x0, y0 - canvas->y0);
			}
			nr_pixelstore_64K_free (buf.buf);
	  	}
	}
}

static gint
sp_canvas_expose (GtkWidget *widget, GdkEventExpose *event)
{
	SPCanvas *canvas;
	GdkRectangle *rects;
	int n_rects, i;

	canvas = SP_CANVAS (widget);

	if (!GTK_WIDGET_DRAWABLE (widget) || (event->window != SP_CANVAS_WINDOW (canvas))) return FALSE;

	gdk_region_get_rectangles (event->region, &rects, &n_rects);

	for (i = 0; i < n_rects; i++) {
		ArtIRect rect;

		rect.x0 = rects[i].x + canvas->x0;
		rect.y0 = rects[i].y + canvas->y0;
		rect.x1 = rect.x0 + rects[i].width;
		rect.y1 = rect.y0 + rects[i].height;

		if (canvas->need_update || canvas->need_redraw) {
			ArtUta *uta;
			/* Update or drawing is scheduled, so just mark exposed area as dirty */
			uta = art_uta_from_irect (&rect);
			sp_canvas_request_redraw_uta (canvas, uta);
		} else {
			/* No pending updates, draw exposed area immediately */
			sp_canvas_paint_rect (canvas, rect.x0, rect.y0, rect.x1, rect.y1);
		}
	}

	if (n_rects > 0) g_free (rects);

	return FALSE;
}

static gint
sp_canvas_key (GtkWidget *widget, GdkEventKey *event)
{
	SPCanvas *canvas;

	canvas = SP_CANVAS (widget);

	return emit_event (canvas, (GdkEvent *) event);
}

/* Crossing event handler for the canvas */
static gint
sp_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event)
{
	SPCanvas *canvas;

	canvas = SP_CANVAS (widget);

	if (event->window != SP_CANVAS_WINDOW (canvas)) return FALSE;

	canvas->state = event->state;
	return pick_current_item (canvas, (GdkEvent *) event);
}

/* Focus in handler for the canvas */
static gint
sp_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
	SPCanvas *canvas;

	GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

	canvas = SP_CANVAS (widget);

	if (canvas->focused_item) {
		return emit_event (canvas, (GdkEvent *) event);
	} else {
		return FALSE;
	}
}

/* Focus out handler for the canvas */
static gint
sp_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
	SPCanvas *canvas;

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

	canvas = SP_CANVAS (widget);

	if (canvas->focused_item)
		return emit_event (canvas, (GdkEvent *) event);
	else
		return FALSE;
}

#ifdef SP_CANVAS_INTERRUPLTIBLE
static void
uta_clear (ArtUta *uta, ArtIRect *rect)
{
	int Ux0, Uy0, Ux1, Uy1;
	int ux0, uy0, ux1, uy1;
	int x, y;

	Ux0 = rect->x0 / ART_UTILE_SIZE;
	ux0 = rect->x0 % ART_UTILE_SIZE;
	Uy0 = rect->y0 / ART_UTILE_SIZE;
	uy0 = rect->y0 % ART_UTILE_SIZE;
	Ux1 = (rect->x1 - 1) / ART_UTILE_SIZE;
	ux1 = rect->x1 % ART_UTILE_SIZE;
	Uy1 = (rect->y1 - 1) / ART_UTILE_SIZE;
	uy1 = rect->y1 % ART_UTILE_SIZE;

	for (y = Uy0; y <= Uy1; y++) {
		for (x = Ux0; x <= Ux1; x++) {
			ArtUtaBbox *u;
			int x0, y0, x1, y1;
			u = uta->utiles + (y - uta->y0) * uta->width + (x - uta->x0);
			x0 = ART_UTA_BBOX_X0 (*u);
			y0 = ART_UTA_BBOX_Y0 (*u);
			x1 = ART_UTA_BBOX_X1 (*u);
			y1 = ART_UTA_BBOX_Y1 (*u);
			if (x == Ux0) {
				x1 = MIN (x1, ux0);
				x0 = MIN (x0, x1);
			} else if (x == Ux1) {
				x0 = MAX (x0, ux1);
				x1 = MAX (x1, x0);
			} else {
				x0 = 0;
				x1 = 0;
			}
			if (y == Uy0) {
				y1 = MIN (y1, uy0);
				y0 = MIN (y0, y1);
			} else if (y == Uy1) {
				y0 = MAX (y0, uy1);
				y1 = MAX (y1, y0);
			} else {
				y0 = 0;
				y1 = 0;
			}
			if ((x1 > x0) && (y1 > y0)) {
				*u = ART_UTA_BBOX_CONS (x0, y0, x1, y1);
			} else {
				*u = ART_UTA_BBOX_CONS (0, 0, 0, 0);
			}
		}
	}
}
#endif

/* Repaints the areas in the canvas that need it */
static int
paint (SPCanvas *canvas)
{
	GtkWidget *widget;
	ArtIRect *rects;
	gint n_rects, i;

	widget = GTK_WIDGET (canvas);

	if (canvas->need_update) {
		double affine[6];
		art_affine_identity (affine);
		sp_canvas_item_invoke_update (canvas->root, affine, 0);
		canvas->need_update = FALSE;
	}

	if (!canvas->need_redraw) return TRUE;

	rects = art_rect_list_from_uta (canvas->redraw_area, IMAGE_WIDTH_AA, IMAGE_HEIGHT_AA, &n_rects);

	art_uta_free (canvas->redraw_area);
	canvas->redraw_area = NULL;
	canvas->need_redraw = FALSE;

	for (i = 0; i < n_rects; i++) {
		int x0, y0, x1, y1;

		x0 = MAX (rects[i].x0, canvas->x0);
		y0 = MAX (rects[i].y0, canvas->y0);
		x1 = MIN (rects[i].x1, canvas->x0 + GTK_WIDGET (canvas)->allocation.width);
		y1 = MIN (rects[i].y1, canvas->y0 + GTK_WIDGET (canvas)->allocation.height);

		if ((x0 < x1) && (y0 < y1)) {
			sp_canvas_paint_rect (canvas, x0, y0, x1, y1);
	  	}
	}

	art_free (rects);

	return TRUE;
}

static int
do_update (SPCanvas *canvas)
{
	/* Cause the update if necessary */
	if (canvas->need_update) {
		double affine[6];
		art_affine_identity (affine);
		sp_canvas_item_invoke_update (canvas->root, affine, 0);
		canvas->need_update = FALSE;
	}

	/* Paint if able to */
	if (GTK_WIDGET_DRAWABLE (canvas)) {
		return paint (canvas);
	}

	/* Pick new current item */
	while (canvas->need_repick) {
		canvas->need_repick = FALSE;
		pick_current_item (canvas, &canvas->pick_event);
	}

	return TRUE;
}

/* Idle handler for the canvas.  It deals with pending updates and redraws. */
static gint
idle_handler (gpointer data)
{
	SPCanvas *canvas;
	int ret;

#if 1
	GDK_THREADS_ENTER ();
#endif

	canvas = SP_CANVAS (data);

	ret = do_update (canvas);

	if (ret) {
		/* Reset idle id */
		canvas->idle_id = 0;
	}

#if 1
	GDK_THREADS_LEAVE ();
#endif

	return !ret;
}

/* Convenience function to add an idle handler to a canvas */
static void
add_idle (SPCanvas *canvas)
{
	if (canvas->idle_id != 0) return;

	canvas->idle_id = gtk_idle_add_priority (SP_CANVAS_UPDATE_PRIORITY, idle_handler, canvas);
}

/**
 * sp_canvas_root:
 * @canvas: A canvas.
 *
 * Queries the root group of a canvas.
 *
 * Return value: The root group of the specified canvas.
 **/
SPCanvasGroup *
sp_canvas_root (SPCanvas *canvas)
{
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (SP_IS_CANVAS (canvas), NULL);

	return SP_CANVAS_GROUP (canvas->root);
}

void
sp_canvas_scroll_to (SPCanvas *canvas, float cx, float cy, unsigned int clear)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (SP_IS_CANVAS (canvas));

	scroll_to (canvas, cx, cy, clear);
}

void
sp_canvas_update_now (SPCanvas *canvas)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (SP_IS_CANVAS (canvas));

	if (!(canvas->need_update || canvas->need_redraw)) return;

	remove_idle (canvas);
	do_update (canvas);
}

static void
sp_canvas_request_update (SPCanvas *canvas)
{
	canvas->need_update = TRUE;
	add_idle (canvas);
}

/* Computes the union of two microtile arrays while clipping the result to the
 * specified rectangle.  Any of the specified utas can be NULL, in which case it
 * is taken to be an empty region.
 */
static ArtUta *
uta_union_clip (ArtUta *uta1, ArtUta *uta2, ArtIRect *clip)
{
	ArtUta *uta;
	ArtUtaBbox *utiles;
	int clip_x1, clip_y1, clip_x2, clip_y2;
	int union_x1, union_y1, union_x2, union_y2;
	int new_x1, new_y1, new_x2, new_y2;
	int x, y;
	int ofs, ofs1, ofs2;

	g_assert (clip != NULL);

	/* Compute the tile indices for the clipping rectangle */

	clip_x1 = clip->x0 >> ART_UTILE_SHIFT;
	clip_y1 = clip->y0 >> ART_UTILE_SHIFT;
	clip_x2 = (clip->x1 >> ART_UTILE_SHIFT) + 1;
	clip_y2 = (clip->y1 >> ART_UTILE_SHIFT) + 1;

	/* Get the union of the bounds of both utas */

	if (!uta1) {
		if (!uta2)
			return art_uta_new (clip_x1, clip_y1, clip_x1 + 1, clip_y1 + 1);

		union_x1 = uta2->x0;
		union_y1 = uta2->y0;
		union_x2 = uta2->x0 + uta2->width;
		union_y2 = uta2->y0 + uta2->height;
	} else {
		if (!uta2) {
			union_x1 = uta1->x0;
			union_y1 = uta1->y0;
			union_x2 = uta1->x0 + uta1->width;
			union_y2 = uta1->y0 + uta1->height;
		} else {
			union_x1 = MIN (uta1->x0, uta2->x0);
			union_y1 = MIN (uta1->y0, uta2->y0);
			union_x2 = MAX (uta1->x0 + uta1->width, uta2->x0 + uta2->width);
			union_y2 = MAX (uta1->y0 + uta1->height, uta2->y0 + uta2->height);
		}
	}

	/* Clip the union of the bounds */

	new_x1 = MAX (clip_x1, union_x1);
	new_y1 = MAX (clip_y1, union_y1);
	new_x2 = MIN (clip_x2, union_x2);
	new_y2 = MIN (clip_y2, union_y2);

	if (new_x1 >= new_x2 || new_y1 >= new_y2)
		return art_uta_new (clip_x1, clip_y1, clip_x1 + 1, clip_y1 + 1);

	/* Make the new clipped union */

	uta = art_new (ArtUta, 1);
	uta->x0 = new_x1;
	uta->y0 = new_y1;
	uta->width = new_x2 - new_x1;
	uta->height = new_y2 - new_y1;
	uta->utiles = utiles = art_new (ArtUtaBbox, uta->width * uta->height);

	ofs = 0;
	ofs1 = ofs2 = 0;

	for (y = new_y1; y < new_y2; y++) {
		if (uta1)
			ofs1 = (y - uta1->y0) * uta1->width + new_x1 - uta1->x0;

		if (uta2)
			ofs2 = (y - uta2->y0) * uta2->width + new_x1 - uta2->x0;

		for (x = new_x1; x < new_x2; x++) {
			ArtUtaBbox bb1, bb2, bb;

			if (!uta1
			    || x < uta1->x0 || y < uta1->y0
			    || x >= uta1->x0 + uta1->width || y >= uta1->y0 + uta1->height)
				bb1 = 0;
			else
				bb1 = uta1->utiles[ofs1];

			if (!uta2
			    || x < uta2->x0 || y < uta2->y0
			    || x >= uta2->x0 + uta2->width || y >= uta2->y0 + uta2->height)
				bb2 = 0;
			else
				bb2 = uta2->utiles[ofs2];

			if (bb1 == 0)
				bb = bb2;
			else if (bb2 == 0)
				bb = bb1;
			else
				bb = ART_UTA_BBOX_CONS (MIN (ART_UTA_BBOX_X0 (bb1),
							     ART_UTA_BBOX_X0 (bb2)),
							MIN (ART_UTA_BBOX_Y0 (bb1),
							     ART_UTA_BBOX_Y0 (bb2)),
							MAX (ART_UTA_BBOX_X1 (bb1),
							     ART_UTA_BBOX_X1 (bb2)),
							MAX (ART_UTA_BBOX_Y1 (bb1),
							     ART_UTA_BBOX_Y1 (bb2)));

			utiles[ofs] = bb;

			ofs++;
			ofs1++;
			ofs2++;
		}
	}

	return uta;
}

/**
 * sp_canvas_request_redraw_uta:
 * @canvas: A canvas.
 * @uta: Microtile array that specifies the area to be redrawn.
 *
 * Informs a canvas that the specified area, given as a microtile array, needs
 * to be repainted.  To be used only by item implementations.
 **/
void
sp_canvas_request_redraw_uta (SPCanvas *canvas, ArtUta *uta)
{
	ArtIRect visible;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (SP_IS_CANVAS (canvas));
	g_return_if_fail (uta != NULL);

	if (!GTK_WIDGET_DRAWABLE (canvas))
		return;

	visible.x0 = DISPLAY_X1 (canvas);
	visible.y0 = DISPLAY_Y1 (canvas);
	visible.x1 = visible.x0 + GTK_WIDGET (canvas)->allocation.width;
	visible.y1 = visible.y0 + GTK_WIDGET (canvas)->allocation.height;

	if (canvas->need_redraw) {
		ArtUta *new_uta;

		g_assert (canvas->redraw_area != NULL);

		new_uta = uta_union_clip (canvas->redraw_area, uta, &visible);
		art_uta_free (canvas->redraw_area);
		art_uta_free (uta);
		canvas->redraw_area = new_uta;
	} else {
		ArtUta *new_uta;

		g_assert (canvas->redraw_area == NULL);

		new_uta = uta_union_clip (uta, NULL, &visible);
		art_uta_free (uta);
		canvas->redraw_area = new_uta;
		canvas->need_redraw = TRUE;
	}

	add_idle (canvas);
}

void
sp_canvas_request_redraw (SPCanvas *canvas, int x0, int y0, int x1, int y1)
{
	ArtUta *uta;
	ArtIRect bbox;
	ArtIRect visible;
	ArtIRect clip;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (SP_IS_CANVAS (canvas));

	if (!GTK_WIDGET_DRAWABLE (canvas)) return;
	if ((x0 >= x1) || (y0 >= y1)) return;

	bbox.x0 = x0;
	bbox.y0 = y0;
	bbox.x1 = x1;
	bbox.y1 = y1;

	visible.x0 = DISPLAY_X1 (canvas);
	visible.y0 = DISPLAY_Y1 (canvas);
	visible.x1 = visible.x0 + GTK_WIDGET (canvas)->allocation.width;
	visible.y1 = visible.y0 + GTK_WIDGET (canvas)->allocation.height;

	art_irect_intersect (&clip, &bbox, &visible);

	if (!art_irect_empty (&clip)) {
		uta = art_uta_from_irect (&clip);
		sp_canvas_request_redraw_uta (canvas, uta);
	}
}

void
sp_canvas_window_to_world (SPCanvas *canvas, double winx, double winy, double *worldx, double *worldy)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (SP_IS_CANVAS (canvas));

	if (worldx) *worldx = canvas->x0 + winx;
	if (worldy) *worldy = canvas->y0 + winy;
}

void
sp_canvas_world_to_window (SPCanvas *canvas, double worldx, double worldy, double *winx, double *winy)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (SP_IS_CANVAS (canvas));

	if (winx) *winx = worldx - canvas->x0;
	if (winy) *winy = worldy - canvas->y0;
}

NRRectF *
sp_canvas_get_viewbox (SPCanvas *canvas, NRRectF *viewbox)
{
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (SP_IS_CANVAS (canvas), NULL);
	g_return_val_if_fail (viewbox != NULL, NULL);

	viewbox->x0 = canvas->dx0;
	viewbox->y0 = canvas->dy0;
	viewbox->x1 = viewbox->x0 + GTK_WIDGET (canvas)->allocation.width;
	viewbox->y1 = viewbox->y0 + GTK_WIDGET (canvas)->allocation.height;

	return viewbox;
}

