#define __SP_ZOOM_CONTEXT_C__

/*
 * Handy zooming tool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <stdlib.h>

#include <gtk/gtkeditable.h>
#include <gtk/gtkeditable.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "rubberband.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "sp-item.h"
#include "pixmaps/cursor-zoom.xpm"
#include "prefs-utils.h"

#include "zoom-context.h"

static void sp_zoom_context_class_init (SPZoomContextClass * klass);
static void sp_zoom_context_init (SPZoomContext * zoom_context);
static void sp_zoom_context_dispose (GObject * object);

static gint sp_zoom_context_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_zoom_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

void sp_zoom_string (const gchar * zoom_str);

static SPEventContextClass * parent_class;

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

GType
sp_zoom_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPZoomContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_zoom_context_class_init,
			NULL, NULL,
			sizeof (SPZoomContext),
			4,
			(GInstanceInitFunc) sp_zoom_context_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPZoomContext", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_zoom_context_class_init (SPZoomContextClass * klass)
{
	GObjectClass * object_class;
	SPEventContextClass * event_context_class;

	object_class = (GObjectClass *) klass;
	event_context_class = (SPEventContextClass *) klass;

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

	object_class->dispose = sp_zoom_context_dispose;

	event_context_class->root_handler = sp_zoom_context_root_handler;
	event_context_class->item_handler = sp_zoom_context_item_handler;
}

static void
sp_zoom_context_init (SPZoomContext * zoom_context)
{
	SPEventContext * event_context;
	
	event_context = SP_EVENT_CONTEXT (zoom_context);

	event_context->cursor_shape = cursor_zoom_xpm;
	event_context->hot_x = 6;
	event_context->hot_y = 6;
}

static void
sp_zoom_context_dispose (GObject *object)
{
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gint
sp_zoom_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event)
{
	gint ret;

	ret = FALSE;

	if (((SPEventContextClass *) parent_class)->item_handler)
		ret = ((SPEventContextClass *) parent_class)->item_handler (event_context, item, event);

	return ret;
}

static gint
sp_zoom_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
	SPDesktop * desktop;
	NRPoint p;
	NRRect b;
	gint ret;

	ret = FALSE;

	desktop = event_context->desktop;

	tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);
	gdouble zoom_inc = prefs_get_double_attribute_limited ("options.zoomincrement", "value", 1.414213562, 1.01, 10);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:

			// save drag origin
			xp = (gint) event->button.x; 
			yp = (gint) event->button.y;
			within_tolerance = true;

			sp_desktop_w2d_xy_point (desktop, &p, event->button.x, event->button.y);
			sp_rubberband_start (desktop, p.x, p.y);

			ret = TRUE;
			break;
		default:
			break;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (event->motion.state & GDK_BUTTON1_MASK) {
			ret = TRUE;

			if ( within_tolerance
			     && ( abs( (gint) event->motion.x - xp ) < tolerance )
			     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
				break; // do not drag if we're within tolerance from origin
			}
			// Once the user has moved farther than tolerance from the original location 
			// (indicating they intend to move the object, not click), then always process the 
			// motion notify coordinates as given (no snapping back to origin)
			within_tolerance = false; 

			sp_desktop_w2d_xy_point (desktop, &p, event->motion.x, event->motion.y);
			sp_rubberband_move (p.x, p.y);
		}
		break;
	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			if (sp_rubberband_rect (&b) && !within_tolerance) {
				sp_desktop_set_display_area (desktop, b.x0, b.y0, b.x1, b.y1, 10);
			} else {
				sp_desktop_w2d_xy_point (desktop, &p, event->button.x, event->button.y);
				if (event->button.state & GDK_SHIFT_MASK) {
					sp_desktop_zoom_relative_keep_point (desktop, p.x, p.y, 1/zoom_inc);
				} else {
					sp_desktop_zoom_relative_keep_point (desktop, p.x, p.y, zoom_inc);
				}
			}
			ret = TRUE;
			break;
		default:
			break;
		}
		sp_rubberband_stop ();
		xp = yp = 0; 
		break;
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_Up: 
		case GDK_Down: 
			// prevent the zoom field from activation
			if (!MOD__CTRL_ONLY)
				ret = TRUE;
			break;
		default:
			break;
		}
	default:
		break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->root_handler)
			ret = ((SPEventContextClass *) parent_class)->root_handler (event_context, event);
	}

	return ret;
}

void
sp_zoom_string (const gchar * zoom_str) {
	SPDesktop * desktop;
	gchar * zoom_str2;
	NRRect d;
	gdouble any;
	
	desktop = SP_ACTIVE_DESKTOP;
	if (desktop == NULL) return;
	
	zoom_str2 = g_strndup(zoom_str, 5); /* g_new'ed 5+1 gchar */
	zoom_str2[5] = '\0';
	any = strtod(zoom_str2,NULL) /100;
	g_free (zoom_str2);
	if (any < 0.001) return;
	
	sp_desktop_get_display_area (SP_ACTIVE_DESKTOP, &d);
	sp_desktop_zoom_absolute (SP_ACTIVE_DESKTOP, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, any);
}

