#define __SP_EVENT_CONTEXT_C__

/*
 * Base class for event processors
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>

#include "helper/sp-canvas.h"
#include "xml/repr-private.h"
#include "sp-cursor.h"

#include "shortcuts.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "selection.h"
#include "event-context.h"
#include "event-broker.h"
#include "sp-item.h"
#include "zoom-context.h"
#include "file.h"
#include "interface.h"
#include "helper/sp-intl.h"
#include "selection-chemistry.h"
#include "dialogs/desktop-properties.h"

static void sp_event_context_class_init (SPEventContextClass *klass);
static void sp_event_context_init (SPEventContext *event_context);
static void sp_event_context_dispose (GObject *object);

static void sp_event_context_private_setup (SPEventContext *ec);
static gint sp_event_context_private_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_event_context_private_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

static void set_event_location (SPDesktop * desktop, GdkEvent * event);

static GObjectClass *parent_class;

GType
sp_event_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPEventContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_event_context_class_init,
			NULL, NULL,
			sizeof (SPEventContext),
			4,
			(GInstanceInitFunc) sp_event_context_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPEventContext", &info, 0);
	}
	return type;
}

static void
sp_event_context_class_init (SPEventContextClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = sp_event_context_dispose;

	klass->setup = sp_event_context_private_setup;
	klass->root_handler = sp_event_context_private_root_handler;
	klass->item_handler = sp_event_context_private_item_handler;
}

static void
sp_event_context_init (SPEventContext *event_context)
{
	event_context->desktop = NULL;
	event_context->cursor = NULL;
}

static void
sp_event_context_dispose (GObject *object)
{
	SPEventContext *ec;

	ec = SP_EVENT_CONTEXT (object);

	if (ec->cursor != NULL) {
		gdk_cursor_unref (ec->cursor);
		ec->cursor = NULL;
	}

	if (ec->desktop) {
		ec->desktop = NULL;
	}

	if (ec->repr) {
		sp_repr_remove_listener_by_data (ec->repr, ec);
		sp_repr_unref (ec->repr);
		ec->repr = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
sp_event_context_private_setup (SPEventContext *ec)
{
	GtkWidget *w;
	GdkBitmap *bitmap, * mask;

	w = GTK_WIDGET (SP_DT_CANVAS (ec->desktop));
	if (w->window) {
		/* fixme: */
		if (ec->cursor_shape) {
			bitmap = NULL;
			mask = NULL;
			sp_cursor_bitmap_and_mask_from_xpm (&bitmap, &mask, ec->cursor_shape);
			if ((bitmap != NULL) && (mask != NULL)) {
				ec->cursor = gdk_cursor_new_from_pixmap (bitmap, mask,
					&w->style->black, &w->style->white,
					ec->hot_x, ec->hot_y);
			}
		}
		gdk_window_set_cursor (w->window, ec->cursor);
	}
}

static gint
sp_event_context_private_root_handler (SPEventContext *event_context, GdkEvent *event)
{
	static NRPointF s;
	static unsigned int panning = 0;
	gint ret;
	SPDesktop * desktop;
	ret = FALSE;

       	desktop = event_context->desktop;

	switch (event->type) {
	case GDK_2BUTTON_PRESS:
		if (panning) {
			panning = 0;
			sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
			ret = TRUE;
		} else {
			/* sp_desktop_dialog (); */
		}
		break;
	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 2:
			s.x = event->button.x;
			s.y = event->button.y;
			panning = 2;
			sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
					     GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
					     NULL, event->button.time);

			ret = TRUE;
			break;
		case 3:
			if (event->button.state & GDK_SHIFT_MASK) {
				s.x = event->button.x;
				s.y = event->button.y;
				panning = 3;
				sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
						     GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
						     NULL, event->button.time);
				ret = TRUE;
			} else {
				/* fixme: */
				sp_event_root_menu_popup (desktop, NULL, event);
			}
			break;
		default:
			break;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (panning) {
			if (((panning == 2) && !(event->motion.state & GDK_BUTTON2_MASK)) ||
			    ((panning == 3) && !(event->motion.state & GDK_BUTTON3_MASK))) {
				/* Gdk seems to lose button release for us sometimes :-( */
				panning = 0;
				sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
				ret = TRUE;
			} else {
				sp_desktop_scroll_world (event_context->desktop, event->motion.x - s.x, event->motion.y - s.y);
				ret = TRUE;
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (panning == event->button.button) {
			panning = 0;
			sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
			ret = TRUE;
		}
		break;
        case GDK_KEY_PRESS:
		switch (event->key.keyval) {
			unsigned int shortcut;
		case GDK_F1:
			/* Grab it away from Gtk */
			shortcut = event->key.keyval;
			if (event->key.state & GDK_SHIFT_MASK) shortcut |= SP_SHORTCUT_SHIFT_MASK;
			if (event->key.state & GDK_CONTROL_MASK) shortcut |= SP_SHORTCUT_CONTROL_MASK;
			if (event->key.state & GDK_MOD1_MASK) shortcut |= SP_SHORTCUT_ALT_MASK;
			ret = sp_shortcut_run (shortcut);
		case GDK_Tab: // disable tab/shift-tab which cycle widget focus
		case GDK_ISO_Left_Tab: // they will get different functions
			ret = TRUE;
			break;
		case GDK_W:
		case GDK_w:
			/* Close view */
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_ui_close_view (NULL);
				ret = TRUE;
			}
			break;
		case GDK_i: // Ctrl i - import file
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_file_import (NULL);
				ret = TRUE;
			}
			break;
		case GDK_q: // Ctrl q - quit
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_file_exit ();
				ret = TRUE;
			}
			break;
		case GDK_Left: // Ctrl Left 
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_desktop_scroll_world (event_context->desktop, 10, 0);
				ret = TRUE;
			}
			break;
		case GDK_Up: // Ctrl Up
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_desktop_scroll_world (event_context->desktop, 0, +10);
				ret = TRUE;
			}
			break;
		case GDK_Right: // Ctrl Right
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_desktop_scroll_world (event_context->desktop, -10, 0);
				ret = TRUE;
			}
			break;
		case GDK_Down: // Ctrl Down
			if (event->key.state & GDK_CONTROL_MASK) {
				sp_desktop_scroll_world (event_context->desktop, 0, -10);
				ret = TRUE;
			}
			break;
		case GDK_space: // Space - root menu
			sp_event_root_menu_popup (desktop, NULL, event);
			ret= TRUE;
			break;
		case GDK_F10:
			if (event->key.state & GDK_SHIFT_MASK) {
				sp_event_root_menu_popup (desktop, NULL, event);
				ret= TRUE;
			}
			break;
		}
#if 0
		g_print ("What a funny key: %d \n", event->key.keyval);
#endif
		break;
	case GDK_SCROLL:
		/* ctrl + wheel, pan left--right */
		if (event->scroll.state & GDK_CONTROL_MASK) {
			switch (event->scroll.direction) {
			case GDK_SCROLL_UP:
				sp_desktop_scroll_world (desktop, SP_MOUSEMOVE_STEP, 0);
				break;
			case GDK_SCROLL_DOWN:
				sp_desktop_scroll_world (desktop, -SP_MOUSEMOVE_STEP, 0);
				break;
			default:
				break;
			}
			/* shift + wheel, zoom in--out */
		} else if (event->scroll.state & GDK_SHIFT_MASK) {
			NRRectF d;
			switch (event->scroll.direction) {
			case GDK_SCROLL_UP:
				sp_desktop_get_display_area (desktop, &d);
				sp_desktop_zoom_relative (desktop, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, SP_DESKTOP_ZOOM_INC);
				break;
			case GDK_SCROLL_DOWN:
				sp_desktop_get_display_area (desktop, &d);
				sp_desktop_zoom_relative (desktop, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, 1 / SP_DESKTOP_ZOOM_INC);
				break;
			default:
				break;
			}
			/* no modifier, pan up--down (left--right on multiwheel mice?) */
		} else {
			switch (event->scroll.direction) {
			case GDK_SCROLL_UP:
				sp_desktop_scroll_world (desktop, 0, SP_MOUSEMOVE_STEP);
				break;
			case GDK_SCROLL_DOWN:
				sp_desktop_scroll_world (desktop, 0, -SP_MOUSEMOVE_STEP);
				break;
			case GDK_SCROLL_LEFT:
				sp_desktop_scroll_world (desktop, SP_MOUSEMOVE_STEP, 0);
				break;
			case GDK_SCROLL_RIGHT:
				sp_desktop_scroll_world (desktop, -SP_MOUSEMOVE_STEP, 0);
				break;
			}
		}
		break;
	default:
		break;
	}

	return ret;
}

/* fixme: do context sensitive popup menu on items */

static gint
sp_event_context_private_item_handler (SPEventContext *ec, SPItem *item, GdkEvent *event)
{
	int ret;

	ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if ((event->button.button == 3) && !(event->button.state & GDK_SHIFT_MASK)) {
			sp_event_root_menu_popup (ec->desktop, item, event);
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;
}

static void
sp_ec_repr_destroy (SPRepr *repr, gpointer data)
{
	g_warning ("Oops! Repr destroyed while event context still present");
}

static unsigned int
sp_ec_repr_change_attr (SPRepr *repr, const guchar *key, const guchar *oldval, const guchar *newval, gpointer data)
{
	SPEventContext *ec;

	ec = SP_EVENT_CONTEXT (data);

	/* In theory we could verify values here */

	return TRUE;
}

static void
sp_ec_repr_attr_changed (SPRepr *repr, const guchar *key, const guchar *oldval, const guchar *newval, gpointer data)
{
	SPEventContext *ec;

	ec = SP_EVENT_CONTEXT (data);

	if (((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->set)
		((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set (ec, key, newval);
}

SPReprEventVector sp_ec_event_vector = {
	sp_ec_repr_destroy,
	NULL, /* Add child */
	NULL, /* Child added */
	NULL, /* Remove child */
	NULL, /* Child removed */
	sp_ec_repr_change_attr,
	sp_ec_repr_attr_changed,
	NULL, /* Change content */
	NULL, /* Content changed */
	NULL, /* Change_order */
	NULL /* Order changed */
};

SPEventContext *
sp_event_context_new (GType type, SPDesktop *desktop, SPRepr *repr, unsigned int key)
{
	SPEventContext *ec;

	g_return_val_if_fail (g_type_is_a (type, SP_TYPE_EVENT_CONTEXT), NULL);
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	ec = g_object_new (type, NULL);

	ec->desktop = desktop;
	ec->key = key;
	ec->repr = repr;
	if (ec->repr) {
		sp_repr_ref (ec->repr);
		sp_repr_add_listener (ec->repr, &sp_ec_event_vector, ec);
	}

	if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->setup)
		((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->setup (ec);

	return ec;
}

void
sp_event_context_finish (SPEventContext *ec)
{
	g_return_if_fail (ec != NULL);
	g_return_if_fail (SP_IS_EVENT_CONTEXT (ec));

	if (ec->next) {
		g_warning ("Finishing event context with active link\n");
	}

	if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->finish)
		((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->finish (ec);
}

void
sp_event_context_read (SPEventContext *ec, const guchar *key)
{
	g_return_if_fail (ec != NULL);
	g_return_if_fail (SP_IS_EVENT_CONTEXT (ec));
	g_return_if_fail (key != NULL);

	if (ec->repr) {
		const guchar *val;
		val = sp_repr_attr (ec->repr, key);
		if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set)
			((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set (ec, key, val);
	}
}

void
sp_event_context_activate (SPEventContext *ec)
{
	g_return_if_fail (ec != NULL);
	g_return_if_fail (SP_IS_EVENT_CONTEXT (ec));

	if (((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->activate)
		((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->activate (ec);
}

void
sp_event_context_desactivate (SPEventContext *ec)
{
	g_return_if_fail (ec != NULL);
	g_return_if_fail (SP_IS_EVENT_CONTEXT (ec));

	if (((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->activate)
		((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->activate (ec);
}

gint
sp_event_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
	gint ret;

	ret = ((SPEventContextClass *) G_OBJECT_GET_CLASS (event_context))->root_handler (event_context, event);

	set_event_location (event_context->desktop, event);

	return ret;
}

gint
sp_event_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event)
{
	gint ret;

	ret = ((SPEventContextClass *) G_OBJECT_GET_CLASS (event_context))->item_handler (event_context, item, event);

	if (! ret) {
		ret = sp_event_context_root_handler (event_context, event);
	} else {
		set_event_location (event_context->desktop, event);
	}

	return ret;
}

GtkWidget *
sp_event_context_config_widget (SPEventContext *ec)
{
	g_return_val_if_fail (ec != NULL, NULL);
	g_return_val_if_fail (SP_IS_EVENT_CONTEXT (ec), NULL);

	if (((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->config_widget)
		return ((SPEventContextClass *) G_OBJECT_GET_CLASS (ec))->config_widget (ec);

	return NULL;
}

static void
set_event_location (SPDesktop * desktop, GdkEvent * event)
{
	NRPointF p;

	if (event->type != GDK_MOTION_NOTIFY)
		return;

	sp_desktop_w2d_xy_point (desktop, &p, event->button.x, event->button.y);
	sp_view_set_position (SP_VIEW (desktop), p.x, p.y);
	sp_desktop_set_coordinate_status (desktop, p.x, p.y, 0);
}

void
sp_event_root_menu_popup (SPDesktop *desktop, SPItem *item, GdkEvent *event)
{
	GtkWidget *menu;

	/* fixme: This is not what I want but works for now (Lauris) */
	if (event->type == GDK_KEY_PRESS) {
		item = sp_selection_item (SP_DT_SELECTION (desktop));
	}
	menu = sp_ui_generic_menu (SP_VIEW (desktop), item);
	gtk_widget_show (menu);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, 0, NULL, event->button.button, event->button.time);
		break;
	case GDK_KEY_PRESS:
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, 0, NULL, 0, event->key.time);
		break;
	default:
		break;
	}
}

#if 0
static void
sp_event_grab_item_destroy (GtkObject * object, gpointer data)
{
	gtk_menu_item_remove_submenu (GTK_MENU_ITEM (data));
	gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
}
#endif
