#ifndef __SP_EVENT_CONTEXT_H__
#define __SP_EVENT_CONTEXT_H__

/*
 * Base class for event processors
 *
 * This is per desktop object, which (its derivatives) implements
 * different actions bound to mouse events.
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

#include <glib-object.h>
#include <gtk/gtkwidget.h>
#include "xml/repr.h"
#include "forward.h"

struct _SPEventContext {
	GObject object;
	/* Desktop eventcontext stack */
	SPEventContext *next;
	unsigned int key;
	SPDesktop *desktop;
	SPRepr *repr;
	gchar **cursor_shape;
	gint hot_x, hot_y;
	GdkCursor *cursor;
};

struct _SPEventContextClass {
	GObjectClass parent_class;
	void (* setup) (SPEventContext *ec);
	void (* finish) (SPEventContext *ec);
	void (* set) (SPEventContext *ec, const guchar *key, const guchar *val);
	void (* activate) (SPEventContext *ec);
	void (* desactivate) (SPEventContext *ec);
	gint (* root_handler) (SPEventContext *ec, GdkEvent *event);
	gint (* item_handler) (SPEventContext *ec, SPItem *item, GdkEvent *event);
	/* fixme: I do not like Gtk+ stuff here (Lauris) */
	GtkWidget * (* config_widget) (SPEventContext *ec);
};

#define SP_EVENT_CONTEXT_DESKTOP(e) (SP_EVENT_CONTEXT (e)->desktop)
#define SP_EVENT_CONTEXT_REPR(e) (SP_EVENT_CONTEXT (e)->repr)

#define SP_EVENT_CONTEXT_STATIC 0

SPEventContext *sp_event_context_new (GType type, SPDesktop *desktop, SPRepr *repr, unsigned int key);
void sp_event_context_finish (SPEventContext *ec);
void sp_event_context_read (SPEventContext *ec, const guchar *key);
void sp_event_context_activate (SPEventContext *ec);
void sp_event_context_desactivate (SPEventContext *ec);

gint sp_event_context_root_handler (SPEventContext *ec, GdkEvent *event);
gint sp_event_context_item_handler (SPEventContext *ec, SPItem *item, GdkEvent *event);

GtkWidget *sp_event_context_config_widget (SPEventContext *ec);

void sp_event_root_menu_popup (SPDesktop *desktop, SPItem *item, GdkEvent *event);

#endif
