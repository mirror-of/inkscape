#define __SP_NODE_CONTEXT_C__

/*
 * Node editing context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain, except stamping code,
 * which is Copyright (C) Masatake Yamato 2002
 */

#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include "macros.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "helper/sp-canvas-util.h"
#include "object-edit.h"
#include "sp-path.h"
#include "rubberband.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "selection.h"
#include "nodepath.h"
#include "knotholder.h"
#include "pixmaps/cursor-node.xpm"
#include "node-context.h"
#include "sp-cursor.h"
#include "pixmaps/cursor-node-m.xpm"
#include "pixmaps/cursor-node-d.xpm"
#include "document.h"

static void sp_node_context_class_init (SPNodeContextClass * klass);
static void sp_node_context_init (SPNodeContext * node_context);
static void sp_node_context_dispose (GObject *object);

static void sp_node_context_setup (SPEventContext *ec);
static gint sp_node_context_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_node_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

static void sp_node_context_selection_changed (SPSelection * selection, gpointer data);
static gboolean sp_node_context_stamp (SPNodeContext * node_context);

static SPEventContextClass * parent_class;
GdkCursor * CursorNodeMouseover = NULL, * CursorNodeDragging = NULL;

GType
sp_node_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPNodeContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_node_context_class_init,
			NULL, NULL,
			sizeof (SPNodeContext),
			4,
			(GInstanceInitFunc) sp_node_context_init,
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPNodeContext", &info, 0);
	}
	return type;
}

static void
sp_node_context_class_init (SPNodeContextClass * klass)
{
	GObjectClass *object_class;
	SPEventContextClass * event_context_class;

	object_class = (GObjectClass *) klass;
	event_context_class = (SPEventContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = sp_node_context_dispose;

	event_context_class->setup = sp_node_context_setup;
	event_context_class->root_handler = sp_node_context_root_handler;
	event_context_class->item_handler = sp_node_context_item_handler;

	// cursors in node context
	CursorNodeMouseover = sp_cursor_new_from_xpm (cursor_node_m_xpm , 1, 1); 
	CursorNodeDragging = sp_cursor_new_from_xpm (cursor_node_d_xpm , 1, 1); 
}

static void
sp_node_context_init (SPNodeContext * node_context)
{
	SPEventContext * event_context;
	
	event_context = SP_EVENT_CONTEXT (node_context);

	event_context->cursor_shape = cursor_node_xpm;
	event_context->hot_x = 1;
	event_context->hot_y = 1;
}

static void
sp_node_context_dispose (GObject *object)
{
	SPNodeContext * nc;

	nc = SP_NODE_CONTEXT (object);

	if (nc->nodepath) {
		sp_nodepath_destroy (nc->nodepath);
		nc->nodepath = NULL;
	}

	if (nc->knot_holder) {
		sp_knot_holder_destroy (nc->knot_holder);
		nc->knot_holder = NULL;
	}

	if (SP_EVENT_CONTEXT_DESKTOP (nc)) {
		sp_signal_disconnect_by_data (SP_DT_SELECTION (SP_EVENT_CONTEXT_DESKTOP (nc)), nc);
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
sp_node_context_setup (SPEventContext *ec)
{
	SPNodeContext *nc;
	SPItem *item;

	nc = SP_NODE_CONTEXT (ec);

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	g_signal_connect (G_OBJECT (SP_DT_SELECTION (ec->desktop)), "changed", G_CALLBACK (sp_node_context_selection_changed), nc);

	item = sp_selection_item (SP_DT_SELECTION (ec->desktop));

	nc->nodepath = NULL;
	nc->knot_holder = NULL;
	if (item) {
		nc->nodepath = sp_nodepath_new (ec->desktop, item);
		if (! nc->nodepath) {
			nc->knot_holder = sp_item_knot_holder (item, ec->desktop);
		}
	}
}

static gint
sp_node_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event)
{
	SPNodeContext *nc;
	SPDesktop * desktop;
	gint ret;

	ret = FALSE;

	desktop = event_context->desktop;

	nc = SP_NODE_CONTEXT (event_context);

	switch (event->type) {
		case GDK_BUTTON_RELEASE:
			if (event->button.button == 1) {
				if (!nc->drag) {
					sp_selection_set_item (SP_DT_SELECTION (desktop), item);
					ret = TRUE;
				}
				break;
			}
			break;
		default:
			break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->item_handler)
			ret = ((SPEventContextClass *) parent_class)->item_handler (event_context, item, event);
	}

	return ret;
}

static gint
sp_node_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
	SPDesktop * desktop;
	SPNodeContext * nc;
	NRPointF p;
	NRRectD b;
	gint ret;

	ret = FALSE;

	desktop = event_context->desktop;
	nc = SP_NODE_CONTEXT (event_context);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
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
			sp_desktop_w2d_xy_point (desktop, &p, event->motion.x, event->motion.y);
			sp_rubberband_move (p.x, p.y);
			nc->drag = TRUE;
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			if (sp_rubberband_rect (&b)) {
				sp_rubberband_stop ();
				if (nc->nodepath) {
					sp_nodepath_select_rect (nc->nodepath, &b, event->button.state & GDK_SHIFT_MASK);
				}
				ret = TRUE;
			}
			nc->drag = FALSE;
			break;
		}
		break;
	case GDK_KEY_PRESS:
		if ((event->key.state & GDK_SHIFT_MASK) && !(event->key.state & GDK_CONTROL_MASK)) {
			switch (event->key.keyval) {
			case GDK_Insert:
			case GDK_KP_Insert:
				sp_node_selected_add_node ();
				ret = TRUE;
				break;
			case GDK_Delete:
			case GDK_KP_Delete:
				sp_node_selected_delete ();
				ret = TRUE;
				break;
			case GDK_C:
				sp_node_selected_set_type (SP_PATHNODE_CUSP);
				ret = TRUE;
				break;
			case GDK_S:
				sp_node_selected_set_type (SP_PATHNODE_SMOOTH);
				ret = TRUE;
				break;
			case GDK_Y:
				sp_node_selected_set_type (SP_PATHNODE_SYMM);
				ret = TRUE;
				break;
			case GDK_B:
				sp_node_selected_break ();
				ret = TRUE;
				break;
			case GDK_J:
				sp_node_selected_join ();
				ret = TRUE;
				break;
			case GDK_L:
				sp_node_selected_set_line_type (ART_LINETO);
				ret = TRUE;
				break;
			case GDK_K:
				sp_node_selected_set_line_type (ART_CURVETO);
				ret = TRUE;
				break;
			default:
				break;
			}
		}
		if (!(event->key.state & GDK_SHIFT_MASK) && !(event->key.state & GDK_CONTROL_MASK)) {
			switch (event->key.keyval) {
			case GDK_space:
				if ((event->key.state & GDK_BUTTON1_MASK)
				    && (nc->nodepath == NULL)) {
					ret = sp_node_context_stamp(nc);
				}
				break;
			default:
				ret = node_key (event);
				break;
			}
		}


		break;
	default:
		break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->root_handler)
			ret = ((SPEventContextClass *) parent_class)->root_handler (event_context, event);
	}

	return ret;
}

static void
sp_node_context_selection_changed (SPSelection * selection, gpointer data)
{
	SPNodeContext * nc;
	SPDesktop *desktop;
	SPItem * item;

	nc = SP_NODE_CONTEXT (data);

	if (nc->nodepath) sp_nodepath_destroy (nc->nodepath);
	if (nc->knot_holder) sp_knot_holder_destroy (nc->knot_holder);

	item = sp_selection_item (selection);
	
	desktop = selection->desktop;
	nc->nodepath = NULL;
	nc->knot_holder = NULL;
	if (item) {
		nc->nodepath = sp_nodepath_new (desktop, item);
		if (! nc->nodepath)
			nc->knot_holder = sp_item_knot_holder (item, desktop);
	}
}

static gboolean
sp_node_context_stamp (SPNodeContext * nc)
{
	SPItem * original_item, * copy_item;
	SPRepr * original_repr, * copy_repr;
	gchar tstr[80];
	SPEventContext * ec;
	
	ec = SP_EVENT_CONTEXT(nc);
	original_item = sp_selection_item (SP_DT_SELECTION (ec->desktop));
	if (original_item) {
		original_repr = (SPRepr *)(SP_OBJECT (original_item)->repr);
		copy_repr = sp_repr_duplicate (original_repr);
		copy_item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (ec->desktop), 
							     copy_repr);
		
		if (sp_svg_transform_write (tstr, 80, &original_item->transform)) {
			sp_repr_set_attr (copy_repr, "transform", tstr);
		} else {
			sp_repr_set_attr (copy_repr, "transform", NULL);
		}
		
		sp_repr_unref (copy_repr);
		sp_document_done (SP_DT_DOCUMENT (ec->desktop));
	}
	return TRUE;
}
