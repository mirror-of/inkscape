#define __SP_CANVAS_C__

/*
 * Port of GnomeCanvas for Inkscape needs
 *
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   fred
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

#include <helper/sp-marshal.h>
#include <display/sp-canvas.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-convex-hull.h>

const gint sp_canvas_update_priority = G_PRIORITY_HIGH_IDLE;

#define SP_CANVAS_WINDOW(c) (((GtkWidget *) (c))->window)

enum {
    SP_CANVAS_ITEM_VISIBLE = 1 << 7,
    SP_CANVAS_ITEM_NEED_UPDATE = 1 << 8,
    SP_CANVAS_ITEM_NEED_AFFINE = 1 << 9
};

struct SPCanvasGroup : public SPCanvasItem{
    GList *items, *last;
};

struct SPCanvasGroupClass :public SPCanvasItemClass {};

struct SPCanvasClass : public GtkWidgetClass{};

static void group_add (SPCanvasGroup *group, SPCanvasItem *item);
static void group_remove (SPCanvasGroup *group, SPCanvasItem *item);

/* SPCanvasItem */

enum {ITEM_EVENT, ITEM_LAST_SIGNAL};


static void sp_canvas_request_update (SPCanvas *canvas);

static void sp_canvas_item_class_init (SPCanvasItemClass *klass);
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
        type = g_type_register_static (GTK_TYPE_OBJECT, "SPCanvasItem", &info, (GTypeFlags)0);
    }

    return type;
}

/* Class initialization function for SPCanvasItemClass */
static void
sp_canvas_item_class_init (SPCanvasItemClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;

    /* fixme: Derive from GObject */
    item_parent_class = (GtkObjectClass*)gtk_type_class (GTK_TYPE_OBJECT);

    item_signals[ITEM_EVENT] = g_signal_new ("event",
                                             G_TYPE_FROM_CLASS (klass),
                                             G_SIGNAL_RUN_LAST,
                                             0,
                                             NULL, NULL,
                                             sp_marshal_BOOLEAN__POINTER,
                                             G_TYPE_BOOLEAN, 1,
                                             GDK_TYPE_EVENT);

    object_class->dispose = sp_canvas_item_dispose;
}

static void
sp_canvas_item_init (SPCanvasItem *item)
{
    item->flags |= SP_CANVAS_ITEM_VISIBLE;
    item->xform = NR::Matrix(NR::identity());
}

SPCanvasItem *
sp_canvas_item_new (SPCanvasGroup *parent, GtkType type, const gchar *first_arg_name, ...)
{
    va_list args;

    g_return_val_if_fail (parent != NULL, NULL);
    g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);
    g_return_val_if_fail (gtk_type_is_a (type, sp_canvas_item_get_type ()), NULL);

    SPCanvasItem *item = SP_CANVAS_ITEM (gtk_type_new (type));

    va_start (args, first_arg_name);
    sp_canvas_item_construct (item, parent, first_arg_name, args);
    va_end (args);

    return item;
}

static void
item_post_create_setup (SPCanvasItem *item)
{
    group_add (SP_CANVAS_GROUP (item->parent), item);

    sp_canvas_item_request_update (item);
    sp_canvas_request_redraw (item->canvas, (int)(item->x1), (int)(item->y1), (int)(item->x2 + 1), (int)(item->y2 + 1));
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
    if (item->flags & SP_CANVAS_ITEM_VISIBLE) {
        sp_canvas_request_redraw (item->canvas, (int)(item->x1), (int)(item->y1), (int)(item->x2 + 1), (int)(item->y2 + 1));
    }
}

static void
sp_canvas_item_dispose (GObject *object)
{
    SPCanvasItem *item = SP_CANVAS_ITEM (object);

    redraw_if_visible (item);
    item->flags &= ~SP_CANVAS_ITEM_VISIBLE;

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

    G_OBJECT_CLASS (item_parent_class)->dispose (object);
}

/* NB! affine is parent2canvas */

static void
sp_canvas_item_invoke_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    /* Apply the child item's transform */
    NR::Matrix child_affine = item->xform * affine;

    /* apply object flags to child flags */
    int child_flags = flags & ~SP_CANVAS_UPDATE_REQUESTED;

    if (item->flags & SP_CANVAS_ITEM_NEED_UPDATE)
        child_flags |= SP_CANVAS_UPDATE_REQUESTED;

    if (item->flags & SP_CANVAS_ITEM_NEED_AFFINE)
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
sp_canvas_item_invoke_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item)
{
    if (SP_CANVAS_ITEM_GET_CLASS (item)->point)
        return SP_CANVAS_ITEM_GET_CLASS (item)->point (item, p, actual_item);

    return NR_HUGE;
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
sp_canvas_item_affine_absolute (SPCanvasItem *item, NR::Matrix const& affine)
{
    item->xform = affine;

    if (!(item->flags & SP_CANVAS_ITEM_NEED_AFFINE)) {
        item->flags |= SP_CANVAS_ITEM_NEED_AFFINE;
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
    if (link == before)
        return;

    SPCanvasGroup *parent = SP_CANVAS_GROUP (SP_CANVAS_ITEM (link->data)->parent);

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
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));
    g_return_if_fail (positions >= 0);

    if (!item->parent || positions == 0)
        return;

    SPCanvasGroup *parent = SP_CANVAS_GROUP (item->parent);
    GList *link = g_list_find (parent->items, item);
    g_assert (link != NULL);

    GList *before;
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
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));
    g_return_if_fail (positions >= 1);

    if (!item->parent || positions == 0)
        return;

    SPCanvasGroup *parent = SP_CANVAS_GROUP (item->parent);
    GList *link = g_list_find (parent->items, item);
    g_assert (link != NULL);

    GList *before;
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

    if (item->flags & SP_CANVAS_ITEM_VISIBLE)
        return;

    item->flags |= SP_CANVAS_ITEM_VISIBLE;

    sp_canvas_request_redraw (item->canvas, (int)(item->x1), (int)(item->y1), (int)(item->x2 + 1), (int)(item->y2 + 1));
    item->canvas->need_repick = TRUE;
}

void
sp_canvas_item_hide (SPCanvasItem *item)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));

    if (!(item->flags & SP_CANVAS_ITEM_VISIBLE))
        return;

    item->flags &= ~SP_CANVAS_ITEM_VISIBLE;

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)(item->x2 + 1), (int)(item->y2 + 1));
    item->canvas->need_repick = TRUE;
}

int
sp_canvas_item_grab (SPCanvasItem *item, guint event_mask, GdkCursor *cursor, guint32 etime)
{
    g_return_val_if_fail (item != NULL, -1);
    g_return_val_if_fail (SP_IS_CANVAS_ITEM (item), -1);
    g_return_val_if_fail (GTK_WIDGET_MAPPED (item->canvas), -1);

    if (item->canvas->grabbed_item)
        return -1;

    if (!(item->flags & SP_CANVAS_ITEM_VISIBLE))
        return -1;

    /* fixme: Top hack (Lauris) */
    /* fixme: If we add key masks to event mask, Gdk will abort (Lauris) */
    /* fixme: But Canvas actualle does get key events, so all we need is routing these here */
    gdk_pointer_grab (SP_CANVAS_WINDOW (item->canvas), FALSE,
                      (GdkEventMask)(event_mask & (~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK))),
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


NR::Matrix sp_canvas_item_i2w_affine(SPCanvasItem const *item)
{
    g_assert (SP_IS_CANVAS_ITEM (item)); // should we get this?

    NR::Matrix affine = NR::identity();

    while (item) {
        affine *= item->xform;
        item = item->parent;
    }
    return affine;
}

static bool is_descendant(SPCanvasItem const *item, SPCanvasItem const *parent)
{
    while (item) {
        if (item == parent)
            return true;
        item = item->parent;
    }

    return false;
}

void
sp_canvas_item_grab_focus (SPCanvasItem *item)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));
    g_return_if_fail (GTK_WIDGET_CAN_FOCUS (GTK_WIDGET (item->canvas)));

    SPCanvasItem *focused_item = item->canvas->focused_item;

    if (focused_item) {
        GdkEvent ev;
        ev.focus_change.type = GDK_FOCUS_CHANGE;
        ev.focus_change.window = SP_CANVAS_WINDOW (item->canvas);
        ev.focus_change.send_event = FALSE;
        ev.focus_change.in = FALSE;

        emit_event (item->canvas, &ev);
    }

    item->canvas->focused_item = item;
    gtk_widget_grab_focus (GTK_WIDGET (item->canvas));

    if (focused_item) {
        GdkEvent ev;
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
    if (item->flags & SP_CANVAS_ITEM_NEED_UPDATE)
        return;

    item->flags |= SP_CANVAS_ITEM_NEED_UPDATE;

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

static void sp_canvas_group_class_init (SPCanvasGroupClass *klass);
static void sp_canvas_group_init (SPCanvasGroup *group);
static void sp_canvas_group_destroy (GtkObject *object);

static void sp_canvas_group_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static double sp_canvas_group_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item);
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
            NULL, NULL, NULL
        };

        group_type = gtk_type_unique (sp_canvas_item_get_type (), &group_info);
    }

    return group_type;
}

/* Class initialization function for SPCanvasGroupClass */
static void
sp_canvas_group_class_init (SPCanvasGroupClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    group_parent_class = (SPCanvasItemClass*)gtk_type_class (sp_canvas_item_get_type ());

    object_class->destroy = sp_canvas_group_destroy;

    item_class->update = sp_canvas_group_update;
    item_class->render = sp_canvas_group_render;
    item_class->point = sp_canvas_group_point;
}

static void
sp_canvas_group_init (SPCanvasGroup */*group*/)
{
    /* Nothing here */
}

static void
sp_canvas_group_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CANVAS_GROUP (object));

    const SPCanvasGroup *group = SP_CANVAS_GROUP (object);

    GList *list = group->items;
    while (list) {
        SPCanvasItem *child = (SPCanvasItem *)list->data;
        list = list->next;

        gtk_object_destroy (GTK_OBJECT (child));
    }

    if (GTK_OBJECT_CLASS (group_parent_class)->destroy)
        (* GTK_OBJECT_CLASS (group_parent_class)->destroy) (object);
}

/* Update handler for canvas groups */
static void
sp_canvas_group_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    const SPCanvasGroup *group = SP_CANVAS_GROUP (item);
    NR::ConvexHull corners(NR::Point(0, 0));
    bool empty=true;

    for (GList *list = group->items; list; list = list->next) {
        SPCanvasItem *i = (SPCanvasItem *)list->data;

        sp_canvas_item_invoke_update (i, affine, flags);

        if ( i->x2 > i->x1 && i->y2 > i->y1 ) {
            if (empty) {
                corners = NR::ConvexHull(NR::Point(i->x1, i->y1));
                empty = false;
            } else {
                corners.add(NR::Point(i->x1, i->y1));
            }
            corners.add(NR::Point(i->x2, i->y2));
        }
    }

    NR::Rect const &bounds = corners.bounds();
    item->x1 = bounds.min()[NR::X];
    item->y1 = bounds.min()[NR::Y];
    item->x2 = bounds.max()[NR::X];
    item->y2 = bounds.max()[NR::Y];
}

/* Point handler for canvas groups */
static double
sp_canvas_group_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item)
{
    const SPCanvasGroup *group = SP_CANVAS_GROUP (item);
    const double x = p[NR::X];
    const double y = p[NR::Y];
    int x1 = (int)(x - item->canvas->close_enough);
    int y1 = (int)(y - item->canvas->close_enough);
    int x2 = (int)(x + item->canvas->close_enough);
    int y2 = (int)(y + item->canvas->close_enough);

    double best = 0.0;
    *actual_item = NULL;

    double dist = 0.0;

    for (GList *list = group->items; list; list = list->next) {
        SPCanvasItem *child = (SPCanvasItem *)list->data;

        if ((child->x1 <= x2) && (child->y1 <= y2) && (child->x2 >= x1) && (child->y2 >= y1)) {
            SPCanvasItem *point_item = NULL; /* cater for incomplete item implementations */

            int has_point;
            if ((child->flags & SP_CANVAS_ITEM_VISIBLE) && SP_CANVAS_ITEM_GET_CLASS (child)->point) {
                dist = sp_canvas_item_invoke_point (child, p, &point_item);
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
    const SPCanvasGroup *group = SP_CANVAS_GROUP (item);

    for (GList *list = group->items; list; list = list->next) {
        SPCanvasItem *child = (SPCanvasItem *)list->data;
        if (child->flags & SP_CANVAS_ITEM_VISIBLE) {
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
    g_return_if_fail (group != NULL);
    g_return_if_fail (SP_IS_CANVAS_GROUP (group));
    g_return_if_fail (item != NULL);

    for (GList *children = group->items; children; children = children->next) {
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

static void sp_canvas_class_init (SPCanvasClass *klass);
static void sp_canvas_init (SPCanvas *canvas);
static void sp_canvas_destroy (GtkObject *object);

static void sp_canvas_realize (GtkWidget *widget);
static void sp_canvas_unrealize (GtkWidget *widget);

static void sp_canvas_size_request (GtkWidget *widget, GtkRequisition *req);
static void sp_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gint sp_canvas_button (GtkWidget *widget, GdkEventButton *event);
static gint sp_canvas_scroll (GtkWidget *widget, GdkEventScroll *event);
static gint sp_canvas_motion (GtkWidget *widget, GdkEventMotion *event);
static gint sp_canvas_expose (GtkWidget *widget, GdkEventExpose *event);
static gint sp_canvas_key (GtkWidget *widget, GdkEventKey *event);
static gint sp_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event);
static gint sp_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint sp_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event);

static GtkWidgetClass *canvas_parent_class;

void sp_canvas_resize_tiles(SPCanvas* canvas,int nl,int nt,int nr,int nb);
void sp_canvas_dirty_rect(SPCanvas* canvas,int nl,int nt,int nr,int nb);
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
            NULL, NULL, NULL
        };

        canvas_type = gtk_type_unique (GTK_TYPE_WIDGET, &canvas_info);
    }

    return canvas_type;
}

/* Class initialization function for SPCanvasClass */
static void
sp_canvas_class_init (SPCanvasClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    canvas_parent_class = (GtkWidgetClass *)gtk_type_class (GTK_TYPE_WIDGET);

    object_class->destroy = sp_canvas_destroy;

    widget_class->realize = sp_canvas_realize;
    widget_class->unrealize = sp_canvas_unrealize;
    widget_class->size_request = sp_canvas_size_request;
    widget_class->size_allocate = sp_canvas_size_allocate;
    widget_class->button_press_event = sp_canvas_button;
    widget_class->button_release_event = sp_canvas_button;
    widget_class->motion_notify_event = sp_canvas_motion;
    widget_class->scroll_event = sp_canvas_scroll;
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

    canvas->tiles=NULL;
    canvas->tLeft=canvas->tTop=canvas->tRight=canvas->tBottom=0;
    canvas->tileH=canvas->tileV=0;
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
    }
    if ( canvas->tiles ) free(canvas->tiles);
    canvas->tiles=NULL;
    canvas->tLeft=canvas->tTop=canvas->tRight=canvas->tBottom=0;
    canvas->tileH=canvas->tileV=0;
  
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
    SPCanvas *canvas = SP_CANVAS (object);

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
    SPCanvas *canvas = (SPCanvas *)gtk_type_new (sp_canvas_get_type ());

    return (GtkWidget *) canvas;
}

static void
sp_canvas_realize (GtkWidget *widget)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    GdkWindowAttr attributes;
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
    gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    canvas->pixmap_gc = gdk_gc_new (SP_CANVAS_WINDOW (canvas));
}

static void
sp_canvas_unrealize (GtkWidget *widget)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    shutdown_transients (canvas);

    gdk_gc_destroy (canvas->pixmap_gc);
    canvas->pixmap_gc = NULL;

    if (GTK_WIDGET_CLASS (canvas_parent_class)->unrealize)
        (* GTK_WIDGET_CLASS (canvas_parent_class)->unrealize) (widget);
}

static void
sp_canvas_size_request (GtkWidget *widget, GtkRequisition *req)
{
    static_cast<void>(SP_CANVAS (widget));

    req->width = 256;
    req->height = 256;
}

static void
sp_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    /* Schedule redraw of new region */
    sp_canvas_resize_tiles(canvas,canvas->x0,canvas->y0,canvas->x0+allocation->width,canvas->y0+allocation->height);
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
    int ix = (int) (x + 0.5);
    int iy = (int) (y + 0.5);
    int dx = ix - canvas->x0;
    int dy = iy - canvas->y0;

    canvas->dx0 = x;
    canvas->dy0 = y;
    canvas->x0 = ix;
    canvas->y0 = iy;

    sp_canvas_resize_tiles(canvas,canvas->x0,canvas->y0,canvas->x0+canvas->widget.allocation.width,canvas->y0+canvas->widget.allocation.height);
  
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
    guint mask;

    /* Perform checks for grabbed items */
    if (canvas->grabbed_item && 
        !is_descendant (canvas->current_item, canvas->grabbed_item))
        return FALSE;

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
        case GDK_SCROLL:
            mask = GDK_SCROLL;
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

    GdkEvent ev = *event;

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

    SPCanvasItem *item = canvas->current_item;

    if (canvas->focused_item &&
        ((event->type == GDK_KEY_PRESS) ||
         (event->type == GDK_KEY_RELEASE) ||
         (event->type == GDK_FOCUS_CHANGE))) {
        item = canvas->focused_item;
    }

    /* The event is propagated up the hierarchy (for if someone connected to
     * a group instead of a leaf event), and emission is stopped if a
     * handler returns TRUE, just like for GtkWidget events.
     */

    gint finished = FALSE;

    while (item && !finished) {
        gtk_object_ref (GTK_OBJECT (item));
        gtk_signal_emit (GTK_OBJECT (item), item_signals[ITEM_EVENT], &ev, &finished);
        SPCanvasItem *parent = item->parent;
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

    int retval = FALSE;

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
        if (canvas->root->flags & SP_CANVAS_ITEM_VISIBLE) {
            sp_canvas_item_invoke_point (canvas->root, NR::Point(x, y), &canvas->new_current_item);
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
    SPCanvas *canvas = SP_CANVAS (widget);

    int retval = FALSE;

    /* dispatch normally regardless of the event's window if an item has
       has a pointer grab in effect */
    if (!canvas->grabbed_item && 
        event->window != SP_CANVAS_WINDOW (canvas))
        return retval;

    int mask;
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

/* Scroll event handler for the canvas */
/* FIXME: generate motion events to re-select items */
static gint
sp_canvas_scroll (GtkWidget *widget, GdkEventScroll *event)
{
    return emit_event (SP_CANVAS (widget), (GdkEvent *) event);
}

/* Motion event handler for the canvas */
static int
sp_canvas_motion (GtkWidget *widget, GdkEventMotion *event)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    if (event->window != SP_CANVAS_WINDOW (canvas))
        return FALSE;

    if (canvas->grabbed_event_mask & GDK_POINTER_MOTION_HINT_MASK) {
        gint x, y;
        gdk_window_get_pointer (widget->window, &x, &y, NULL);
        event->x = x;
        event->y = y;
    }

    canvas->state = event->state;
    pick_current_item (canvas, (GdkEvent *) event);

    return emit_event (canvas, (GdkEvent *) event);
}

/* We have to fit into pixelstore 64K */
#define IMAGE_WIDTH_AA 341
#define IMAGE_HEIGHT_AA 64

int sp_canvas_bigtile_floor(int x)
{
    int r=x&(~127);
    return r/128;
}
int sp_canvas_bigtile_ceil(int x)
{
    int r=x&(~127);
    if ( x&127 ) r+=128;
    return r/128;
}

static void
sp_canvas_paint_rect (SPCanvas *canvas, int xx0, int yy0, int xx1, int yy1)
{
    g_return_if_fail (!canvas->need_update);

    GtkWidget *widget = GTK_WIDGET (canvas);

    int draw_x1 = MAX (xx0, canvas->x0);
    int draw_y1 = MAX (yy0, canvas->y0);
    int draw_x2 = MIN (xx1, canvas->x0/*draw_x1*/ + GTK_WIDGET (canvas)->allocation.width);
    int draw_y2 = MIN (yy1, canvas->y0/*draw_y1*/ + GTK_WIDGET (canvas)->allocation.height);

    int bw = draw_x2 - draw_x1;
    int bh = draw_y2 - draw_y1;
    if ((bw < 1) || (bh < 1))
        return;

    int sw, sh;
    /* 65536 is max cached buffer and we need 3 channels */
    if (bw * bh < 21845) {
        // We can go with single buffer 
        sw = bw;
        sh = bh;
    } else if (bw <= (16 * IMAGE_WIDTH_AA)) {
        // Go with row buffer 
        sw = bw;
        sh = 21845 / bw;
    } else if (bh <= (16 * IMAGE_HEIGHT_AA)) {
        // Go with column buffer 
        sw = 21845 / bh;
        sh = bh;
    } else {
        sw = IMAGE_WIDTH_AA;
        sh = IMAGE_HEIGHT_AA;
    }

    // As we can come from expose, we have to tile here 
    for (int y0 = draw_y1; y0 < draw_y2; y0 += sh) {
        int y1 = MIN (y0 + sh, draw_y2);
        for (int x0 = draw_x1; x0 < draw_x2; x0 += sw) {
            int x1 = MIN (x0 + sw, draw_x2);

            SPCanvasBuf buf;
            buf.buf = nr_pixelstore_64K_new (0, 0);
            buf.buf_rowstride = sw * 3;
            buf.rect.x0 = x0;
            buf.rect.y0 = y0;
            buf.rect.x1 = x1;
            buf.rect.y1 = y1;
            GdkColor *color = &widget->style->bg[GTK_STATE_NORMAL];
            buf.bg_color = (((color->red & 0xff00) << 8)
                            | (color->green & 0xff00)
                            | (color->blue >> 8));
            buf.is_bg = 1;
            buf.is_buf = 0;
      
            if (canvas->root->flags & SP_CANVAS_ITEM_VISIBLE) {
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
    SPCanvas *canvas = SP_CANVAS (widget);

    if (!GTK_WIDGET_DRAWABLE (widget) || 
        (event->window != SP_CANVAS_WINDOW (canvas)))
        return FALSE;

    int n_rects;
    GdkRectangle *rects;
    gdk_region_get_rectangles (event->region, &rects, &n_rects);

    for (int i = 0; i < n_rects; i++) {
        NRRectL rect;
		
        rect.x0 = rects[i].x + canvas->x0;
        rect.y0 = rects[i].y + canvas->y0;
        rect.x1 = rect.x0 + rects[i].width;
        rect.y1 = rect.y0 + rects[i].height;

        if (canvas->need_update || canvas->need_redraw) {
            sp_canvas_request_redraw (canvas, rect.x0, rect.y0, rect.x1, rect.y1);
        } else {
            /* No pending updates, draw exposed area immediately */
            sp_canvas_paint_rect (canvas, rect.x0, rect.y0, rect.x1, rect.y1);
        }
    }

    if (n_rects > 0)
        g_free (rects);

    return FALSE;
}

static gint
sp_canvas_key (GtkWidget *widget, GdkEventKey *event)
{
    return emit_event (SP_CANVAS (widget), (GdkEvent *) event);
}

/* Crossing event handler for the canvas */
static gint
sp_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    if (event->window != SP_CANVAS_WINDOW (canvas))
        return FALSE;

    canvas->state = event->state;
    return pick_current_item (canvas, (GdkEvent *) event);
}

/* Focus in handler for the canvas */
static gint
sp_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
    GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

    SPCanvas *canvas = SP_CANVAS (widget);

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
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

    SPCanvas *canvas = SP_CANVAS (widget);

    if (canvas->focused_item)
        return emit_event (canvas, (GdkEvent *) event);
    else
        return FALSE;
}

/* Repaints the areas in the canvas that need it */
static int
paint (SPCanvas *canvas)
{
    if (canvas->need_update) {
        sp_canvas_item_invoke_update (canvas->root, NR::identity(), 0);
        canvas->need_update = FALSE;
    }

    if (!canvas->need_redraw)
        return TRUE;

    GtkWidget const *widget = GTK_WIDGET(canvas);
    int const canvas_x1 = canvas->x0 + widget->allocation.width;
    int const canvas_y1 = canvas->y0 + widget->allocation.height;

    for (int j=canvas->tTop&(~3);j<canvas->tBottom;j+=4) {
        for (int i=canvas->tLeft&(~3);i<canvas->tRight;i+=4) {
            int  mode=0;
      
            int pl=i+1,pr=i,pt=j+4,pb=j;
            for (int l=MAX(j,canvas->tTop);l<MIN(j+4,canvas->tBottom);l++) {
                for (int k=MAX(i,canvas->tLeft);k<MIN(i+4,canvas->tRight);k++) {
                    if ( canvas->tiles[(k-canvas->tLeft)+(l-canvas->tTop)*canvas->tileH] ) {
                        mode|=1<<((k-i)+(l-j)*4);
                        if ( k < pl ) pl=k;
                        if ( k+1 > pr ) pr=k+1;
                        if ( l < pt ) pt=l;
                        if ( l+1 > pb ) pb=l+1;
                    }
                    canvas->tiles[(k-canvas->tLeft)+(l-canvas->tTop)*canvas->tileH]=0;
                }
            }
      
            if ( mode ) {
                int x0 = 0,y0 = 0,x1 = 0,y1 = 0;
                x0 = MAX (pl*32, canvas->x0);
                y0 = MAX (pt*32, canvas->y0);
                x1 = MIN (pr*32, canvas_x1);
                y1 = MIN (pb*32, canvas_y1);
                if ((x0 < x1) && (y0 < y1)) sp_canvas_paint_rect (canvas, x0, y0, x1, y1);
            }
        }
    }
  
    canvas->need_redraw = FALSE;
    return TRUE;
}

static int
do_update (SPCanvas *canvas)
{
    /* Cause the update if necessary */
    if (canvas->need_update) {
        sp_canvas_item_invoke_update (canvas->root, NR::identity(), 0);
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
    GDK_THREADS_ENTER ();

    SPCanvas *canvas = SP_CANVAS (data);

    const int ret = do_update (canvas);

    if (ret) {
        /* Reset idle id */
        canvas->idle_id = 0;
    }

    GDK_THREADS_LEAVE ();

    return !ret;
}

/* Convenience function to add an idle handler to a canvas */
static void
add_idle (SPCanvas *canvas)
{
    if (canvas->idle_id != 0)
        return;

    canvas->idle_id = gtk_idle_add_priority (sp_canvas_update_priority, idle_handler, canvas);
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

    if (!(canvas->need_update || 
          canvas->need_redraw))
        return;

    remove_idle (canvas);
    do_update (canvas);
}

static void
sp_canvas_request_update (SPCanvas *canvas)
{
    canvas->need_update = TRUE;
    add_idle (canvas);
}

void
sp_canvas_request_redraw (SPCanvas *canvas, int x0, int y0, int x1, int y1)
{
    NRRectL bbox;
    NRRectL visible;
    NRRectL clip;

    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (!GTK_WIDGET_DRAWABLE (canvas)) return;
    if ((x0 >= x1) || (y0 >= y1)) return;

    bbox.x0 = x0;
    bbox.y0 = y0;
    bbox.x1 = x1;
    bbox.y1 = y1;

    visible.x0 = canvas->x0;
    visible.y0 = canvas->y0;
    visible.x1 = visible.x0 + GTK_WIDGET (canvas)->allocation.width;
    visible.y1 = visible.y0 + GTK_WIDGET (canvas)->allocation.height;

    nr_rect_l_intersect (&clip, &bbox, &visible);

    sp_canvas_dirty_rect(canvas,x0,y0,x1,y1);
    add_idle (canvas);
}

void sp_canvas_window_to_world(SPCanvas const *canvas, double winx, double winy, double *worldx, double *worldy)
{
    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (worldx) *worldx = canvas->x0 + winx;
    if (worldy) *worldy = canvas->y0 + winy;
}

void sp_canvas_world_to_window(SPCanvas const *canvas, double worldx, double worldy, double *winx, double *winy)
{
    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (winx) *winx = worldx - canvas->x0;
    if (winy) *winy = worldy - canvas->y0;
}

NR::Point sp_canvas_window_to_world(SPCanvas const *canvas, NR::Point const win)
{
    g_assert (canvas != NULL);
    g_assert (SP_IS_CANVAS (canvas));

    return NR::Point(canvas->x0 + win[0], canvas->y0 + win[1]);
}

NR::Point sp_canvas_world_to_window(SPCanvas const *canvas, NR::Point const world)
{
    g_assert (canvas != NULL);
    g_assert (SP_IS_CANVAS (canvas));

    return NR::Point(world[0] - canvas->x0, world[1] - canvas->y0);
}

bool sp_canvas_world_pt_inside_window(SPCanvas const *canvas, NR::Point const &world)
{
    g_assert( canvas != NULL );
    g_assert(SP_IS_CANVAS(canvas));

    using NR::X;
    using NR::Y;
    GtkWidget const &w = *GTK_WIDGET(canvas);
    return ( ( canvas->x0 <= world[X] )  &&
             ( canvas->y0 <= world[Y] )  &&
             ( world[X] < canvas->x0 + w.allocation.width )  &&
             ( world[Y] < canvas->y0 + w.allocation.height ) );
}

NRRect *sp_canvas_get_viewbox(SPCanvas const *canvas, NRRect *viewbox)
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

int sp_canvas_tile_floor(int x)
{
    return (x&(~31))/32;
}

int sp_canvas_tile_ceil(int x)
{
    return ((x+31)&(~31))/32;
}

void sp_canvas_resize_tiles(SPCanvas* canvas,int nl,int nt,int nr,int nb)
{
    if ( nl >= nr || nt >= nb ) {
        if ( canvas->tiles ) free(canvas->tiles);
        canvas->tLeft=canvas->tTop=canvas->tRight=canvas->tBottom=0;
        canvas->tileH=canvas->tileV=0;
        canvas->tiles=NULL;
        return;
    }
    int tl=sp_canvas_tile_floor(nl);
    int tt=sp_canvas_tile_floor(nt);
    int tr=sp_canvas_tile_ceil(nr);
    int tb=sp_canvas_tile_ceil(nb);

    int nh=tr-tl,nv=tb-tt;
    uint8_t* ntiles=(uint8_t*)malloc(nh*nv*sizeof(uint8_t));
    for (int i=tl;i<tr;i++) {
        for (int j=tt;j<tb;j++) {
            int ind=(i-tl)+(j-tt)*nh;
            if ( i >= canvas->tLeft && i < canvas->tRight && j >= canvas->tTop && j < canvas->tBottom ) {
                ntiles[ind]=canvas->tiles[(i-canvas->tLeft)+(j-canvas->tTop)*canvas->tileH];
            } else {
                ntiles[ind]=0;
            }
        }
    }
    if ( canvas->tiles ) free(canvas->tiles);
    canvas->tiles=ntiles;
    canvas->tLeft=tl;
    canvas->tTop=tt;
    canvas->tRight=tr;
    canvas->tBottom=tb;
    canvas->tileH=nh;
    canvas->tileV=nv;
}

void sp_canvas_dirty_rect(SPCanvas* canvas,int nl,int nt,int nr,int nb)
{
    if ( nl >= nr || nt >= nb ) {
        return;
    }
    int tl=sp_canvas_tile_floor(nl);
    int tt=sp_canvas_tile_floor(nt);
    int tr=sp_canvas_tile_ceil(nr);
    int tb=sp_canvas_tile_ceil(nb);
    if ( tl >= canvas->tRight || tr <= canvas->tLeft || tt >= canvas->tBottom || tb <= canvas->tTop ) return;
    if ( tl < canvas->tLeft ) tl=canvas->tLeft;
    if ( tr > canvas->tRight ) tr=canvas->tRight;
    if ( tt < canvas->tTop ) tt=canvas->tTop;
    if ( tb > canvas->tBottom ) tb=canvas->tBottom;
  
    canvas->need_redraw = TRUE;
  
    for (int i=tl;i<tr;i++) {
        for (int j=tt;j<tb;j++) {
            canvas->tiles[(i-canvas->tLeft)+(j-canvas->tTop)*canvas->tileH]=1;
        }
    }
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
