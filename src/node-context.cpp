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
#include <string.h>
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
#include "prefs-utils.h"
#include "xml/repr.h"
#include "xml/repr-private.h"

static void sp_node_context_class_init (SPNodeContextClass * klass);
static void sp_node_context_init (SPNodeContext * node_context);
static void sp_node_context_dispose (GObject *object);

static void sp_node_context_setup (SPEventContext *ec);
static gint sp_node_context_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_node_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

static gboolean sp_node_context_stamp (SPNodeContext * node_context);

static void nodepath_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, gpointer data);
static SPReprEventVector nodepath_repr_events = {
	NULL, /* destroy */
	NULL, /* add_child */
	NULL, /* child_added */
	NULL, /* remove_child */
	NULL, /* child_removed */
	NULL, /* change_attr */
	nodepath_event_attr_changed,
	NULL, /* change_list */
	NULL, /* content_changed */
	NULL, /* change_order */
	NULL  /* order_changed */
};

static SPEventContextClass *parent_class;
static GdkCursor *CursorNodeMouseover = NULL, *CursorNodeDragging = NULL;

static gint nodeedit_rb_escaped = 0; // if non-zero, rubberband was canceled by esc, so the next button release should not deselect

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

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
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPNodeContext", &info, (GTypeFlags)0);
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

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

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

	node_context -> leftalt = FALSE;
	node_context -> rightalt = FALSE;
	node_context -> leftctrl = FALSE;
	node_context -> rightctrl = FALSE;
}

static void
sp_node_context_dispose (GObject *object)
{
	SPNodeContext * nc;
	SPEventContext * ec;

	nc = SP_NODE_CONTEXT (object);
	ec = SP_EVENT_CONTEXT (object);

	if (nc->nodepath) {
		sp_repr_remove_listener_by_data (nc->nodepath->repr, ec);
		sp_repr_unref (nc->nodepath->repr);
		sp_nodepath_destroy (nc->nodepath);
		nc->nodepath = NULL;
	}

	if (nc->knot_holder) {
		sp_repr_remove_listener_by_data (nc->knot_holder->repr, ec);
		sp_repr_unref (nc->knot_holder->repr);
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
	SPRepr *repr;

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	g_signal_connect (G_OBJECT (SP_DT_SELECTION (ec->desktop)), 
		"changed", G_CALLBACK (sp_node_context_selection_changed), nc);

	item = sp_selection_item (SP_DT_SELECTION (ec->desktop));

	nc->nodepath = NULL;
	nc->knot_holder = NULL;

	if (item) {
		nc->nodepath = sp_nodepath_new (ec->desktop, item);
		if ( nc->nodepath) {
			//point pack to parent in case nodepath is deleted
			nc->nodepath->nodeContext = nc;
		}
		else {
			nc->knot_holder = sp_item_knot_holder (item, ec->desktop);
		}


		// setting listener
		repr = SP_OBJECT (item)->repr;
		if (repr) {
			sp_repr_ref (repr);
			sp_repr_add_listener (repr, &nodepath_repr_events, ec);
			sp_repr_synthesize_events (repr, &nodepath_repr_events, ec);
		}
	}

	sp_nodepath_update_statusbar (nc->nodepath);
}

/**
\brief  Callback that processes the "changed" signal on the selection; 
destroys old and creates new nodepath and reassigns listeners to the new selected item's repr
*/
void
sp_node_context_selection_changed (SPSelection * selection, gpointer data)
{
	SPNodeContext * nc;
	SPEventContext * ec;
	SPDesktop *desktop;
	SPItem * item;
	SPRepr *old_repr = NULL, *repr;

	nc = SP_NODE_CONTEXT (data);
	ec = SP_EVENT_CONTEXT (nc);

	if (nc->nodepath) {
		old_repr = nc->nodepath->repr;
		sp_nodepath_destroy (nc->nodepath);
	} 
	if (nc->knot_holder) {
		old_repr = nc->knot_holder->repr;
		sp_knot_holder_destroy (nc->knot_holder);
	}

	if (old_repr) { // remove old listener
		sp_repr_remove_listener_by_data (old_repr, ec);
		sp_repr_unref (old_repr);
	}

	item = sp_selection_item (selection);
	
	desktop = selection->desktop;
	nc->nodepath = NULL;
	nc->knot_holder = NULL;
	if (item) {
		nc->nodepath = sp_nodepath_new (desktop, item);
		if (! nc->nodepath) {
			nc->knot_holder = sp_item_knot_holder (item, desktop);
		}
		// setting new listener
		repr = SP_OBJECT (item)->repr;
		if (repr) {
			sp_repr_ref (repr);
			sp_repr_add_listener (repr, &nodepath_repr_events, ec);
			sp_repr_synthesize_events (repr, &nodepath_repr_events, ec);
		}
	}
	sp_nodepath_update_statusbar (nc->nodepath);
}

/**
\brief  Regenerates nodepath when the item's repr was change outside of node edit
(e.g. by undo, or xml editor, or edited in another view). The item is assumed to be the same 
(otherwise sp_node_context_selection_changed() would have been called), so repr and listeners
are not changed.
*/
void
sp_nodepath_update_from_item (SPNodeContext *nc, SPItem *item)
{
	g_assert(nc);

	SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP (SP_EVENT_CONTEXT (nc));
	g_assert(desktop);

	if (nc->nodepath) {
		sp_nodepath_destroy (nc->nodepath);
	}

	if (nc->knot_holder) {
		sp_knot_holder_destroy (nc->knot_holder);
	}

	item = sp_selection_item (SP_DT_SELECTION (desktop));

	nc->nodepath = NULL;
	nc->knot_holder = NULL;
	if (item) {
		nc->nodepath = sp_nodepath_new (desktop, item);
		if (! nc->nodepath) {
			nc->knot_holder = sp_item_knot_holder (item, desktop);
		}
	}
	sp_nodepath_update_statusbar (nc->nodepath);
}

/**
\brief  Callback that is fired whenever an attribute of the selected item (which we have in the nodepath) changes
*/
static void 
nodepath_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, gpointer data)
{
	SPItem *item = NULL;
	const char *newd = NULL, *newtypestr = NULL;
	gboolean changed = FALSE;

	g_assert (data);
	SPNodeContext *nc = ((SPNodeContext *) data);
	g_assert(nc);
	Path::Path *np = nc->nodepath;
	SPKnotHolder *kh = nc->knot_holder;

	if (np) {
		item = SP_ITEM (np->path);
		if (!strcmp(name, "d")) {
			newd = new_value;
			changed = nodepath_repr_d_changed (np, new_value);
		} else if (!strcmp(name, "sodipodi:nodetypes")) {
			newtypestr = new_value;
			changed = nodepath_repr_typestr_changed (np, new_value);
		} else return; 	// with paths, we only need to act if one of the path-affecting attributes has changed	
	} else if (kh) {
		item = SP_ITEM (kh->item);
		changed = !(kh->local_change);
		kh->local_change = FALSE;
	}
	if (np && changed) {
		GList *saved = NULL;
		SPDesktop *desktop = np->desktop;
		g_assert(desktop);
		SPSelection *selection = desktop->selection;
		g_assert(selection);

		saved = save_nodepath_selection (nc->nodepath);
		sp_nodepath_update_from_item (nc, item);
 		if (nc->nodepath && saved) restore_nodepath_selection (nc->nodepath, saved);

	} else if (kh && changed) {
		sp_nodepath_update_from_item (nc, item);
	}

	sp_nodepath_update_statusbar (nc->nodepath);
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
					ret = FALSE;
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
	SPDesktop *desktop = event_context->desktop;
	SPNodeContext *nc = SP_NODE_CONTEXT(event_context);
	double const nudge = prefs_get_double_attribute_limited("options.nudgedistance", "value", 2.8346457, 0, 1000); // default is 1 mm
	tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);
	int const snaps = prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12);
	double const offset = prefs_get_double_attribute_limited("options.defaultoffsetwidth", "value", 2, 0, 1000);

	gint ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			// save drag origin
			xp = (gint) event->button.x; 
			yp = (gint) event->button.y;
			within_tolerance = true;

			NR::Point const button_w(event->button.x,
						 event->button.y);
			NR::Point const button_dt(sp_desktop_w2d_xy_point(desktop, button_w));
			sp_rubberband_start(desktop, button_dt);
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (event->motion.state & GDK_BUTTON1_MASK) {

			if ( within_tolerance
			     && ( abs( (gint) event->motion.x - xp ) < tolerance )
			     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
				break; // do not drag if we're within tolerance from origin
			}
			// Once the user has moved farther than tolerance from the original location 
			// (indicating they intend to move the object, not click), then always process the 
			// motion notify coordinates as given (no snapping back to origin)
			within_tolerance = false; 

			NR::Point const motion_w(event->motion.x,
						 event->motion.y);
			NR::Point const motion_dt(sp_desktop_w2d_xy_point(desktop, motion_w));
			sp_rubberband_move(motion_dt);
			nc->drag = TRUE;
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		xp = yp = 0; 
		if (event->button.button == 1) {
			NRRect b;
			if (sp_rubberband_rect (&b) && !within_tolerance) { // drag
				if (nc->nodepath) {
					sp_nodepath_select_rect (nc->nodepath, &b, event->button.state & GDK_SHIFT_MASK);
				}
			} else {
				if (!(nodeedit_rb_escaped)) // unless something was cancelled
					sp_nodepath_deselect (nc->nodepath); 
			}
			ret = TRUE;
			sp_rubberband_stop ();
			nodeedit_rb_escaped = 0;
			nc->drag = FALSE;
			break;
		}
		break;
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_Insert:
		case GDK_KP_Insert:
			// with any modifiers
			sp_node_selected_add_node ();
			ret = TRUE;
			break;
		case GDK_Delete:
		case GDK_KP_Delete:
			// with any modifiers
			sp_node_selected_delete ();
			ret = TRUE;
			break;
		case GDK_C:
		case GDK_c:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_set_type (Path::NODE_CUSP);
				ret = TRUE;
			}
			break;
		case GDK_S:
		case GDK_s:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_set_type (Path::NODE_SMOOTH);
				ret = TRUE;
			}
			break;
		case GDK_Y:
		case GDK_y:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_set_type (Path::NODE_SYMM);
				ret = TRUE;
			}
			break;
		case GDK_B:
		case GDK_b:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_break ();
				ret = TRUE;
			}
			break;
		case GDK_J:
		case GDK_j:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_join ();
				ret = TRUE;
			}
			break;
		case GDK_D:
		case GDK_d:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_duplicate ();
				ret = TRUE;
			}
			break;
		case GDK_L:
		case GDK_l:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_set_line_type (ART_LINETO);
				ret = TRUE;
			}
			break;
		case GDK_K:
		case GDK_k:
			if (MOD__SHIFT_ONLY) {
				sp_node_selected_set_line_type (ART_CURVETO);
				ret = TRUE;
			}
			break;
		case GDK_space:
			// stamping when node-editing is a strange idea - the entire shape is duplicated 
			// it may be useful however, so let it be 
			if ((event->key.state & GDK_BUTTON1_MASK)
					&& (nc->nodepath == NULL)) {
				ret = sp_node_context_stamp(nc);
			}
			break;

			// arrow keys:
			// this was copied over from select-context.c, 
			// with "sp_selection_" replaced by "sp_node_selected"; 
			// maybe we could make this a shared function?
		case GDK_Left: // move selection left
		case GDK_KP_Left: 
		case GDK_KP_4: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_node_selected_move_screen (-10, 0); // shift
					else sp_node_selected_move_screen (-1, 0); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_node_selected_move (-10*nudge, 0); // shift
					else sp_node_selected_move (-nudge, 0); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Up: // move selection up
		case GDK_KP_Up: 
		case GDK_KP_8: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_node_selected_move_screen (0, 10); // shift
					else sp_node_selected_move_screen (0, 1); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_node_selected_move (0, 10*nudge); // shift
					else sp_node_selected_move (0, nudge); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Right: // move selection right
		case GDK_KP_Right: 
		case GDK_KP_6: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_node_selected_move_screen (10, 0); // shift
					else sp_node_selected_move_screen (1, 0); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_node_selected_move (10*nudge, 0); // shift
					else sp_node_selected_move (nudge, 0); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Down: // move selection down
		case GDK_KP_Down: 
		case GDK_KP_2: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_node_selected_move_screen (0, -10); // shift
					else sp_node_selected_move_screen (0, -1); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_node_selected_move (0, -10*nudge); // shift
					else sp_node_selected_move (0, -nudge); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Tab: // Tab - cycle selection forward
			if (!(MOD__CTRL_ONLY || (MOD__CTRL && MOD__SHIFT))) {
				sp_nodepath_select_next (nc->nodepath);
				ret = TRUE;
			} 
			break;
		case GDK_ISO_Left_Tab:  // Shift Tab - cycle selection backward
			if (!(MOD__CTRL_ONLY || (MOD__CTRL && MOD__SHIFT))) {
				sp_nodepath_select_prev (nc->nodepath);
				ret = TRUE;
			}
			break;
		case GDK_Escape:
		{
			NRRect b;
			if (sp_rubberband_rect (&b)) { // cancel rubberband
				sp_rubberband_stop ();
				nodeedit_rb_escaped = 1;
			} else {
				sp_nodepath_deselect (nc->nodepath); // deselect
			}
			ret = TRUE;
			break;
		}

 		case GDK_bracketleft:
			if ( MOD__CTRL && !MOD__ALT && ( snaps != 0 ) ) {
				if (nc->leftctrl)
					sp_nodepath_selected_nodes_rotate (nc->nodepath, M_PI/snaps, -1);
				if (nc->rightctrl)
					sp_nodepath_selected_nodes_rotate (nc->nodepath, M_PI/snaps, 1);
			} else if ( MOD__ALT && !MOD__CTRL ) {
				if (nc->leftalt && nc->rightalt)
					sp_nodepath_selected_nodes_rotate_screen (nc->nodepath, 1, 0);
				else {
					if (nc->leftalt)
						sp_nodepath_selected_nodes_rotate_screen (nc->nodepath, 1, -1);
					if (nc->rightalt)
						sp_nodepath_selected_nodes_rotate_screen (nc->nodepath, 1, 1);
				}
			} else if ( snaps != 0 ) {
				sp_nodepath_selected_nodes_rotate (nc->nodepath, M_PI/snaps, 0);
			}
			ret = TRUE;
			break;
 		case GDK_bracketright:
			if ( MOD__CTRL && !MOD__ALT && ( snaps != 0 ) ) {
				if (nc->leftctrl)
					sp_nodepath_selected_nodes_rotate (nc->nodepath, -M_PI/snaps, -1);
				if (nc->rightctrl)
					sp_nodepath_selected_nodes_rotate (nc->nodepath, -M_PI/snaps, 1);
			} else if ( MOD__ALT && !MOD__CTRL ) {
				if (nc->leftalt && nc->rightalt)
					sp_nodepath_selected_nodes_rotate_screen (nc->nodepath, -1, 0);
				else {
					if (nc->leftalt)
						sp_nodepath_selected_nodes_rotate_screen (nc->nodepath, -1, -1);
					if (nc->rightalt)
						sp_nodepath_selected_nodes_rotate_screen (nc->nodepath, -1, 1);
				}
			} else if ( snaps != 0 ) {
				sp_nodepath_selected_nodes_rotate (nc->nodepath, -M_PI/snaps, 0);
			}
			ret = TRUE;
			break;
 		case GDK_less:
 		case GDK_comma:
			if (MOD__CTRL) {
				if (nc->leftctrl)
					sp_nodepath_selected_nodes_scale (nc->nodepath, -offset, -1);
				if (nc->rightctrl)
					sp_nodepath_selected_nodes_scale (nc->nodepath, -offset, 1);
			} else if (MOD__ALT) {
				if (nc->leftalt && nc->rightalt)
					sp_nodepath_selected_nodes_scale_screen (nc->nodepath, -1, 0);
				else {
					if (nc->leftalt)
						sp_nodepath_selected_nodes_scale_screen (nc->nodepath, -1, -1);
					if (nc->rightalt)
						sp_nodepath_selected_nodes_scale_screen (nc->nodepath, -1, 1);
				}
			} else {
				sp_nodepath_selected_nodes_scale (nc->nodepath, -offset, 0);
			}
			ret = TRUE;
			break;
 		case GDK_greater:
 		case GDK_period:
			if (MOD__CTRL) {
				if (nc->leftctrl)
					sp_nodepath_selected_nodes_scale (nc->nodepath, offset, -1);
				if (nc->rightctrl)
					sp_nodepath_selected_nodes_scale (nc->nodepath, offset, 1);
			} else if (MOD__ALT) {
				if (nc->leftalt && nc->rightalt)
					sp_nodepath_selected_nodes_scale_screen (nc->nodepath, 1, 0);
				else {
					if (nc->leftalt)
						sp_nodepath_selected_nodes_scale_screen (nc->nodepath, 1, -1);
					if (nc->rightalt)
						sp_nodepath_selected_nodes_scale_screen (nc->nodepath, 1, 1);
				}
			} else {
				sp_nodepath_selected_nodes_scale (nc->nodepath, offset, 0);
			}
			ret = TRUE;
			break;

		case GDK_Alt_L:
			nc->leftalt = TRUE;
			break;
		case GDK_Alt_R:
			nc->rightalt = TRUE;
			break;
		case GDK_Control_L:
			nc->leftctrl = TRUE;
			break;
		case GDK_Control_R:
			nc->rightctrl = TRUE;
			break;
		default:
			ret = node_key (event);
			break;
		}
		break;
	case GDK_KEY_RELEASE:
		switch (event->key.keyval) {
		case GDK_Alt_L:
			nc->leftalt = FALSE;
			break;
		case GDK_Alt_R:
			nc->rightalt = FALSE;
			break;
		case GDK_Control_L:
			nc->leftctrl = FALSE;
			break;
		case GDK_Control_R:
			nc->rightctrl = FALSE;
			break;
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
