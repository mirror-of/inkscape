#define __SP_SELECT_CONTEXT_C__

/*
 * Selection and transformation context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "macros.h"
#include "display/sp-canvas-util.h"
#include "rubberband.h"
#include "inkscape-private.h"
#include "document.h"
#include "selection.h"
#include "desktop-affine.h"
#include "seltrans-handles.h"
#include "sp-cursor.h"
#include "pixmaps/cursor-select-m.xpm"
#include "pixmaps/cursor-select-d.xpm"
#include "pixmaps/handles.xpm"
#include "helper/sp-intl.h"
#include "widgets/spw-utilities.h"

#include "select-context.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "desktop.h"
#include "dialogs/object-properties.h"
#include "sp-metrics.h"
#include "sp-item.h"
#include "desktop-snap.h"
#include "prefs-utils.h"

static void sp_select_context_class_init (SPSelectContextClass * klass);
static void sp_select_context_init (SPSelectContext * select_context);
static void sp_select_context_dispose (GObject *object);

static void sp_select_context_setup (SPEventContext *ec);
static void sp_select_context_set (SPEventContext *ec, const gchar *key, const gchar *val);
static gint sp_select_context_root_handler (SPEventContext * event_context, GdkEvent * event);
static gint sp_select_context_item_handler (SPEventContext * event_context, SPItem * item, GdkEvent * event);

static void sp_select_context_update_statusbar(SPSelectContext *sc);

static void sp_selection_moveto(SPSelTrans *seltrans, NR::Point const &xy, guint state);

static SPEventContextClass * parent_class;

static GdkCursor *CursorSelectMouseover = NULL;
static GdkCursor *CursorSelectDragging = NULL;
GdkPixbuf * handles[13];

static gint rb_escaped = 0; // if non-zero, rubberband was canceled by esc, so the next button release should not deselect
static gint drag_escaped = 0; // if non-zero, drag was canceled by esc

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

GtkType
sp_select_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPSelectContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_select_context_class_init,
			NULL, NULL,
			sizeof (SPSelectContext),
			4,
			(GInstanceInitFunc) sp_select_context_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPSelectContext", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_select_context_class_init (SPSelectContextClass * klass)
{
	GObjectClass *object_class;
	SPEventContextClass * event_context_class;

	object_class = (GObjectClass *) klass;
	event_context_class = (SPEventContextClass *) klass;

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

	object_class->dispose = sp_select_context_dispose;

	event_context_class->setup = sp_select_context_setup;
	event_context_class->set = sp_select_context_set;
	event_context_class->root_handler = sp_select_context_root_handler;
	event_context_class->item_handler = sp_select_context_item_handler;

	// cursors in select context
	CursorSelectMouseover = sp_cursor_new_from_xpm (cursor_select_m_xpm , 1, 1); 
	CursorSelectDragging = sp_cursor_new_from_xpm (cursor_select_d_xpm , 1, 1); 
	// selection handles
      	handles[0]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_scale_nw_xpm);
	handles[1]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_scale_ne_xpm);
	handles[2]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_scale_h_xpm);
	handles[3]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_scale_v_xpm);
	handles[4]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_nw_xpm);
	handles[5]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_n_xpm);
	handles[6]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_ne_xpm);
	handles[7]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_e_xpm);
	handles[8]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_se_xpm);
	handles[9]  = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_s_xpm);
	handles[10] = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_sw_xpm);
	handles[11] = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_rotate_w_xpm);
	handles[12] = gdk_pixbuf_new_from_xpm_data ((const gchar **)handle_center_xpm);

}

static void
sp_select_context_init (SPSelectContext * sc)
{
	sc->dragging = FALSE;
	sc->moved = FALSE;
	sc->button_press_shift = FALSE;
}

static void
sp_select_context_dispose (GObject *object)
{
	SPSelectContext *sc;

	sc = SP_SELECT_CONTEXT (object);

	if (sc->grabbed) {
		sp_canvas_item_ungrab (sc->grabbed, GDK_CURRENT_TIME);
		sc->grabbed = NULL;
	}

	sp_sel_trans_shutdown (&sc->seltrans);

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
sp_select_context_setup (SPEventContext *ec)
{
	SPSelectContext * select_context;

	select_context = SP_SELECT_CONTEXT (ec);

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	sp_sel_trans_init (&select_context->seltrans, ec->desktop);

	sp_event_context_read (ec, "show");
	sp_event_context_read (ec, "transform");
	sp_event_context_read (ec, "cue");

	sp_sel_trans_update_item_bboxes (&select_context->seltrans);

	sp_select_context_update_statusbar(select_context);
}

static void
sp_select_context_set (SPEventContext *ec, const gchar *key, const gchar *val)
{
	SPSelectContext *sc;

	sc = SP_SELECT_CONTEXT (ec);

	if (!strcmp (key, "show")) {
		if (val && !strcmp (val, "outline")) {
			sc->seltrans.show = SP_SELTRANS_SHOW_OUTLINE;
		} else {
			sc->seltrans.show = SP_SELTRANS_SHOW_CONTENT;
		}
	} else if (!strcmp (key, "cue")) {
               if (val) {
                   if (!strcmp (val, "none")) {
                       sc->seltrans.cue = SP_SELTRANS_CUE_NONE;
                   } else if (!strcmp (val, "bbox")) {
                       sc->seltrans.cue = SP_SELTRANS_CUE_BBOX;
                   } else {
                       sc->seltrans.cue = SP_SELTRANS_CUE_MARK;
                   }
               }
	}
}

static gint
sp_select_context_item_handler (SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
	SPDesktop *desktop;
	SPSelectContext *sc;
	SPSelTrans *seltrans;
	SPSelection *selection;
	GdkCursor *cursor;
	gint ret = FALSE;

	desktop = event_context->desktop;
	sc = SP_SELECT_CONTEXT (event_context);
	seltrans = &sc->seltrans;
	selection = SP_DT_SELECTION (desktop);

	tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);

	switch (event->type) {
	case GDK_2BUTTON_PRESS:
		if (event->button.button == 1) {
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			/* Left mousebutton */

			// save drag origin
			xp = (gint) event->button.x; 
			yp = (gint) event->button.y;
			within_tolerance = true;

			if (!(event->button.state & GDK_SHIFT_MASK || event->button.state & GDK_CONTROL_MASK)) {
				// if shift or ctrl was pressed, do not move objects; 
				// pass the event to root handler which will perform rubberband, shift-click, ctrl-click, ctrl-drag

				sc->dragging = TRUE;
				sc->moved = FALSE;
				sc->item = item;

				rb_escaped = drag_escaped = 0;

 				sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->drawing),
 						 GDK_KEY_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
 						 GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
 						 NULL, event->button.time);
 				sc->grabbed = SP_CANVAS_ITEM (desktop->drawing);
			
				ret = TRUE;
			}
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (event->motion.state & GDK_BUTTON1_MASK) {
			/* Left mousebutton */
			if (sc->dragging) {
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

				NR::Point const motion_pt(event->motion.x, event->motion.y);
				NR::Point const p(sp_desktop_w2d_xy_point(desktop, motion_pt));
				if (!sc->moved) {
					NR::Point const button_pt(event->button.x, event->button.y);
					SPItem *item_at_point = sp_desktop_item_at_point(desktop, button_pt, TRUE);
					SPItem *group_at_point = sp_desktop_group_at_point(desktop, button_pt);
					// if neither a group nor an item (possibly in a group) at point are selected, set selection to the item passed with the event
					if ((!item_at_point || !selection->includesItem(item_at_point)) && 
					    (!group_at_point || !selection->includesItem(group_at_point))) {
						// have to select here since selecting is done when releasing
						sp_sel_trans_reset_state (seltrans);
						if (!selection->includesItem(sc->item))
							selection->setItem(sc->item);
					} // otherwise, do not change selection so that dragging selected-within-group items is possible
					sp_sel_trans_grab(seltrans, p, -1, -1, FALSE);
					sc->moved = TRUE;
				}
				sp_selection_moveto(seltrans, p, event->button.state);
				if (sp_desktop_scroll_to_point (desktop, &p))
					// unfortunately in complex drawings, gobbling results in losing grab of the object, for some mysterious reason
					; //gobble_motion_events (GDK_BUTTON1_MASK);
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
		xp = yp = 0; 
		if (event->button.button == 1) {
			if (sc->moved) {
				// item has been moved
				sp_sel_trans_ungrab (seltrans);
				sc->moved = FALSE;
				sp_select_context_update_statusbar(sc);
			} else {
				// item has not been moved -> do selecting
				if (!selection->isEmpty()) {
					if (event->button.state & GDK_SHIFT_MASK) {
						sp_sel_trans_reset_state (seltrans);
						if (selection->includesItem(sc->item)) {
							selection->removeItem(sc->item);
						} else {
							selection->addItem(sc->item);
						}
					} else {
						if (selection->includesItem(sc->item)) {
							sp_sel_trans_increase_state (seltrans);
						} else {
							sp_sel_trans_reset_state (seltrans);
							selection->setItem(sc->item);
						}
					}
				} else {
					sp_sel_trans_reset_state (seltrans);
					selection->setItem(sc->item);
				}
			}
			sc->dragging = FALSE;
			sc->item = NULL;
			if (sc->grabbed) {
				sp_canvas_item_ungrab (sc->grabbed, event->button.time);
				sc->grabbed = NULL;
			}
			ret = TRUE;
		} 
		break;
	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_FLEUR);
		gdk_window_set_cursor (GTK_WIDGET (SP_DT_CANVAS (desktop))->window, cursor);
		gdk_cursor_destroy (cursor);
		break;
	case GDK_LEAVE_NOTIFY:
		gdk_window_set_cursor (GTK_WIDGET (SP_DT_CANVAS (desktop))->window, event_context->cursor);
		break;
	case GDK_KEY_PRESS:
		if (event->key.keyval == GDK_space) {
			if (sc->dragging && sc->grabbed) {
			/* stamping mode: show content mode moving */
				sp_sel_trans_stamp(seltrans);
				ret = TRUE;
			}
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
sp_select_context_root_handler (SPEventContext *event_context, GdkEvent * event)
{
	SPDesktop *desktop;
	SPSelectContext *sc;
	SPSelTrans *seltrans;
	SPSelection *selection;
	SPItem *item = NULL, *group = NULL;
	SPItem *item_at_point = NULL, *group_at_point = NULL, *item_in_group = NULL;
	gint ret = FALSE;
	NRRect b;
	GSList *l;
	gdouble nudge;
	gdouble offset;

	desktop = event_context->desktop;
	sc = SP_SELECT_CONTEXT (event_context);
	seltrans = &sc->seltrans;
	selection = SP_DT_SELECTION (desktop);
	nudge = prefs_get_double_attribute_limited ("options.nudgedistance", "value", 2.8346457, 0, 1000); // default is 1 mm
	offset = prefs_get_double_attribute_limited ("options.defaultscale", "value", 2, 0, 1000); 
	tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);
	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {

			// save drag origin
			xp = (gint) event->button.x; 
			yp = (gint) event->button.y;
			within_tolerance = true;

			NR::Point const button_pt(event->button.x, event->button.y);
			NR::Point const p(sp_desktop_w2d_xy_point(desktop, button_pt));
			sp_rubberband_start(desktop, p);
			sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
					     GDK_KEY_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK,
					     NULL, event->button.time);
			sc->grabbed = SP_CANVAS_ITEM (desktop->acetate);
			
			// remember that shift or ctrl was on before button press
			// (originally intended by lauris for partial selects and then abandoned)
			sc->button_press_shift = (event->button.state & GDK_SHIFT_MASK) ? TRUE : FALSE;
			sc->button_press_ctrl = (event->button.state & GDK_CONTROL_MASK) ? TRUE : FALSE;

			sc->moved = FALSE;

			rb_escaped = drag_escaped = 0;

			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (event->motion.state & GDK_BUTTON1_MASK) {
			NR::Point const motion_pt(event->motion.x, event->motion.y);
			NR::Point const p(sp_desktop_w2d_xy_point(desktop, motion_pt));

			if ( within_tolerance
			     && ( abs( (gint) event->motion.x - xp ) < tolerance )
			     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
				break; // do not drag if we're within tolerance from origin
			}
			// Once the user has moved farther than tolerance from the original location 
			// (indicating they intend to move the object, not click), then always process the 
			// motion notify coordinates as given (no snapping back to origin)
			within_tolerance = false; 

			if (sc->button_press_ctrl) // if ctrl was pressed and we're away from the origin, we want to ctrl-drag rather than click
				sc->dragging = TRUE;

			if (sc->dragging) {
				/* User has dragged fast, so we get events on root (lauris)*/
				// not only that; we will end up here when ctrl-dragging as well
				sp_rubberband_stop ();
				item_at_point = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), FALSE);
				if (item_at_point || sc->moved) { // drag only if starting from a point, or if something is already grabbed
					if (!sc->moved) {
						item_in_group = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);
						group_at_point = sp_desktop_group_at_point (desktop, NR::Point(event->button.x, event->button.y));
						// if neither a group nor an item (possibly in a group) at point are selected, set selection to the item at point
						if ((!item_in_group || !selection->includesItem(item_in_group)) && 
								(!group_at_point || !selection->includesItem(group_at_point))) {
							// have to select here since selecting is done when releasing
							sp_sel_trans_reset_state (seltrans);
							// when simply ctrl-dragging, we don't want to go into groups
							if (item_at_point && !selection->includesItem(item_at_point))
								selection->setItem(item_at_point);
						} // otherwise, do not change selection so that dragging selected-within-group items is possible
						sp_sel_trans_grab(seltrans, p, -1, -1, FALSE);
						sc->moved = TRUE;
					}
					sp_selection_moveto(seltrans, p, event->button.state);
					if (sp_desktop_scroll_to_point (desktop, &p)) 
						// unfortunately in complex drawings, gobbling results in losing grab of the object, for some mysterious reason
						; //gobble_motion_events (GDK_BUTTON1_MASK);
					ret = TRUE;
				} else {
					sc->dragging = FALSE;
				}
			} else {
				sp_rubberband_move(p);
				gobble_motion_events (GDK_BUTTON1_MASK);
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
		xp = yp = 0; 
		if ((event->button.button == 1) && (sc->grabbed)) {
			if (sc->dragging) {
				if (sc->moved) {
					// item has been moved
					sp_sel_trans_ungrab (seltrans);
					sc->moved = FALSE;
					sp_select_context_update_statusbar(sc);
				} else {
					// item has not been moved -> simply a click, do selecting
					if (!selection->isEmpty()) {
						if (event->button.state & GDK_SHIFT_MASK) { 
							// with shift, toggle selection
							sp_sel_trans_reset_state (seltrans);
							if (selection->includesItem(sc->item)) {
								selection->removeItem(sc->item);
							} else {
								selection->addItem (sc->item);
							}
						} else {
							// without shift, increase state (i.e. toggle scale/rotation handles)
							if (selection->includesItem(sc->item)) {
								sp_sel_trans_increase_state (seltrans);
							} else {
								sp_sel_trans_reset_state (seltrans);
								selection->setItem(sc->item);
							}
						}
					} else {
						sp_sel_trans_reset_state (seltrans);
						selection->setItem(sc->item);
					}
				}
				sc->dragging = FALSE;
				sc->item = NULL;
			} else {
				if (sp_rubberband_rect (&b) && !within_tolerance) {
					// this was a rubberband drag
					sp_rubberband_stop ();
					sp_sel_trans_reset_state (seltrans);
					// find out affected items:
					l = sp_document_items_in_box (SP_DT_DOCUMENT (desktop), &b);
					if (event->button.state & GDK_SHIFT_MASK) {
						// with shift, add to selection
						while (l) {
							item = SP_ITEM (l->data);
							if (selection->includesItem(item)) {
								// uncomment if you want toggle behavior for shift-rubberband
								//	selection->removeItem(item);
							} else {
								selection->addItem(item);
							}
							l = g_slist_remove (l, item);
						}
					} else {
						// without shift, simply select anew
						selection->setItemList(l);
					}
				} else { // it was just a click, or a too small rubberband
					sp_rubberband_stop ();
					if (sc->button_press_shift && !rb_escaped && !drag_escaped) {
						// this was a shift-click, select what was clicked upon

						sc->button_press_shift = FALSE;

						if (sc->button_press_ctrl) {
							item = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);
							group = sp_desktop_group_at_point (desktop, NR::Point(event->button.x, event->button.y));
							sc->button_press_ctrl = FALSE;
						} else {
							item = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), FALSE);
						}
						// if there's both a group and an item at point, deselect group to prevent double selection
						if (group) {
							if (selection->includesItem(group)) {
								selection->removeItem(group);
							}
						}
						if (item) {
							// toggle selected status
							if (selection->includesItem(item)) {
								selection->removeItem(item);
							} else {
								selection->addItem(item);
							}
							item = NULL;
						}

					} else if (sc->button_press_ctrl && !rb_escaped && !drag_escaped) { // ctrl-click 

						sc->button_press_ctrl = FALSE;

						item = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);

						if (item) {
							if (selection->includesItem(item)) {
								sp_sel_trans_increase_state (seltrans);
							} else {
								sp_sel_trans_reset_state (seltrans);
								selection->setItem(item);
							}
							item = NULL;
						}

					} else { // click without shift, simply deselect
						if (!selection->isEmpty()) {
							if (!(rb_escaped) && !(drag_escaped)) // unless something was cancelled
								sp_selection_empty (selection);
							rb_escaped = 0;
							ret = TRUE;
						}
					}
				}
				ret = TRUE;
			}
			if (sc->grabbed) {
				sp_canvas_item_ungrab (sc->grabbed, event->button.time);
				sc->grabbed = NULL;
			}
		} 
		sc->button_press_shift = FALSE;
		break;
	case GDK_KEY_PRESS: // keybindings for select context
		switch (event->key.keyval) {  
		case GDK_Left: // move selection left
		case GDK_KP_Left: 
		case GDK_KP_4: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_selection_move_screen (-10, 0); // shift
					else sp_selection_move_screen (-1, 0); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_selection_move (-10*nudge, 0); // shift
					else sp_selection_move (-nudge, 0); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Up: // move selection up
		case GDK_KP_Up: 
		case GDK_KP_8: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_selection_move_screen (0, 10); // shift
					else sp_selection_move_screen (0, 1); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_selection_move (0, 10*nudge); // shift
					else sp_selection_move (0, nudge); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Right: // move selection right
		case GDK_KP_Right: 
		case GDK_KP_6: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_selection_move_screen (10, 0); // shift
					else sp_selection_move_screen (1, 0); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_selection_move (10*nudge, 0); // shift
					else sp_selection_move (nudge, 0); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Down: // move selection down
		case GDK_KP_Down: 
		case GDK_KP_2: 
			if (!MOD__CTRL) { // not ctrl
				if (MOD__ALT) { // alt
					if (MOD__SHIFT) sp_selection_move_screen (0, -10); // shift
					else sp_selection_move_screen (0, -1); // no shift
				}
				else { // no alt
					if (MOD__SHIFT) sp_selection_move (0, -10*nudge); // shift
					else sp_selection_move (0, -nudge); // no shift
				}
				ret = TRUE;
			}
			break;
		case GDK_Escape:
			if (sc->dragging) {
				if (sc->moved) { // cancel dragging an object
					sp_sel_trans_ungrab (seltrans);
					sc->moved = FALSE;
					sc->dragging = FALSE;
					sc->item = NULL;
					sp_document_undo (SP_DT_DOCUMENT (desktop));
					drag_escaped = 1;
					sp_select_context_update_statusbar(sc);
					sp_view_set_statusf_flash (SP_VIEW(SP_EVENT_CONTEXT(sc)->desktop), _("Move cancelled."));
				}
			} else {
				if (sp_rubberband_rect (&b)) { // cancel rubberband
					sp_rubberband_stop ();
					rb_escaped = 1;
					sp_select_context_update_statusbar(sc);
					sp_view_set_statusf_flash (SP_VIEW(SP_EVENT_CONTEXT(sc)->desktop), _("Selection cancelled."));
				} else {
					sp_selection_empty (selection); // deselect
				}
			}
			ret = TRUE;
			break;
		case GDK_a:
		case GDK_A:
			if (MOD__CTRL_ONLY) {
				sp_edit_select_all();
				ret = TRUE;
			}
			break;
		case GDK_Tab: // Tab - cycle selection forward
			if (!(MOD__CTRL_ONLY || (MOD__CTRL && MOD__SHIFT))) {
				sp_selection_item_next ();
				ret = TRUE;
			}
			break;
		case GDK_ISO_Left_Tab: // Shift Tab - cycle selection backward
			if (!(MOD__CTRL_ONLY || (MOD__CTRL && MOD__SHIFT))) {
				sp_selection_item_prev ();
				ret = TRUE;
			} 
			break;
		case GDK_space:
		/* stamping mode: show outline mode moving */
		/* FIXME: Is next condition ok? (lauris) */
			if (sc->dragging && sc->grabbed) {
				sp_sel_trans_stamp(seltrans);
				ret = TRUE;
			}
			break;
 		case GDK_x:
 		case GDK_X:
 			if (MOD__ALT_ONLY) {
 				gpointer hb = sp_search_by_data_recursive (desktop->owner->aux_toolbox, (gpointer) "altx");
 				if (hb && GTK_IS_WIDGET(hb)) {
					gtk_widget_grab_focus (GTK_WIDGET (hb));
				}
 				ret = TRUE;
 			}
			break;
 		case GDK_bracketleft:
			if (MOD__ALT) { 
				sp_selection_rotate_screen (selection, 1);
			} else if (MOD__CTRL) {
				sp_selection_rotate (selection, 90);
			} else if (snaps) {
				sp_selection_rotate (selection, 180/snaps);
			}
			ret = TRUE;
			break;
 		case GDK_bracketright:
			if (MOD__ALT) { 
				sp_selection_rotate_screen (selection, -1);
			} else if (MOD__CTRL) {
				sp_selection_rotate (selection, -90);
			} else if (snaps) {
				sp_selection_rotate (selection, -180/snaps);
			}
			ret = TRUE;
			break;
 		case GDK_less:
 		case GDK_comma:
			if (MOD__ALT) { 
				sp_selection_scale_screen (selection, -2);
			} else if (MOD__CTRL) {
				sp_selection_scale_times (selection, 0.5);
			} else {
				sp_selection_scale (selection, -offset);
			}
			ret = TRUE;
			break;
 		case GDK_greater:
 		case GDK_period:
			if (MOD__ALT) { 
				sp_selection_scale_screen (selection, 2);
			} else if (MOD__CTRL) {
				sp_selection_scale_times (selection, 2);
			} else {
				sp_selection_scale (selection, offset);
			}
			ret = TRUE;
			break;
		default:
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

static void sp_selection_moveto(SPSelTrans *seltrans, NR::Point const &xy, guint state)
{
	using NR::X;
	using NR::Y;

	SPDesktop *desktop = seltrans->desktop;

	NR::Point dxy = xy - sp_sel_trans_point_desktop(seltrans);
	if (state & GDK_MOD1_MASK) {
		dxy /= 10;
	}

	for(unsigned dim = 0 ; dim < 2 ; ++dim) {
		dxy[dim] = sp_desktop_dim_snap_list(desktop, seltrans->spp, seltrans->spp_length, dxy[dim], dim);
	}

	/* N.B. If we ever implement angled guides, then we'll want to make sure that the movement
	   respects both the snapped-to guide and the h-or-v constraint implied by control mask.
	   This should be a special case of snapping to h-or-v followed by snapping to whatever
	   real guides there are. */
	if (state & GDK_CONTROL_MASK) {
		if( fabs(dxy[X]) > fabs(dxy[Y]) ) {
			dxy[Y] = 0.0;
		} else {
			dxy[X] = 0.0;
		}
	}

	NR::Matrix const move((NR::translate(dxy)));
	NR::Point const norm(0, 0);
	sp_sel_trans_transform(seltrans, move, norm);

	// status text
	GString *xs = SP_PT_TO_METRIC_STRING(dxy[X], SP_DEFAULT_METRIC);
	GString *ys = SP_PT_TO_METRIC_STRING(dxy[Y], SP_DEFAULT_METRIC);
	gchar *status = g_strdup_printf(_("Move by %s, %s"), xs->str, ys->str);
	g_string_free(xs, TRUE);
	g_string_free(ys, TRUE);
	sp_view_set_status (SP_VIEW (seltrans->desktop), status, FALSE);
	g_free(status);
}

static void sp_select_context_update_statusbar(SPSelectContext *sc) {
	SPEventContext *ec=SP_EVENT_CONTEXT(sc);
        char const *when_selected = _("Click selection to toggle scale/rotation handles");
	GSList const *items=SP_DT_SELECTION(ec->desktop)->itemList();
	if (!items) { // no items
		sp_view_set_statusf(SP_VIEW (ec->desktop), _("No objects selected. Click, Shift+click, drag around objects to select."));
	} else if (!items->next) { // one item
		sp_view_set_statusf(SP_VIEW (ec->desktop), "%s. %s.", sp_item_description (SP_ITEM (items->data)), when_selected);
	} else { // multiple items
		sp_view_set_statusf(SP_VIEW (ec->desktop), _("%i objects selected. %s."), g_slist_length((GSList *)items), when_selected);
	}
}
