#define __SP_DESKTOP_EVENTS_C__

/*
 * Event handlers for SPDesktop
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <gtk/gtksignal.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>
#include "display/guideline.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "libnr/nr-matrix-ops.h"
#include "widgets/icon.h"
#include "inkscape-private.h"
#include "desktop.h"
#include "document.h"
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-affine.h"
#include "desktop-handles.h"
#include "event-context.h"
#include "sp-metrics.h"
#include "sp-item.h"
#include "desktop-events.h"
#include "helper/sp-intl.h"
#include "dialogs/dialog-events.h"
#include "message-context.h"
#include "xml/repr.h"

static void sp_dt_simple_guide_dialog (SPGuide * guide, SPDesktop * desktop);


/* Root item handler */


int
sp_desktop_root_handler (SPCanvasItem * item, GdkEvent * event, SPDesktop * desktop)
{
	return sp_event_context_root_handler (desktop->event_context, event);
}

/*
 * fixme: this conatins a hack, to deal with deleting a view, which is
 * completely on another view, in which case active_desktop will not be updated
 *
 */

int
sp_desktop_item_handler (SPCanvasItem * item, GdkEvent * event, gpointer data)
{
	gpointer ddata = gtk_object_get_data (GTK_OBJECT (item->canvas), "SPDesktop");
	g_return_val_if_fail (ddata != NULL, FALSE);

	SPDesktop *desktop = SP_DESKTOP (ddata);

	return sp_event_context_item_handler (desktop->event_context, SP_ITEM (data), event);
}


// not needed 
gint
sp_desktop_enter_notify (GtkWidget * widget, GdkEventCrossing * event)
{
  	inkscape_activate_desktop (SP_DESKTOP (widget));
	return FALSE;
}

static gint
sp_dt_ruler_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw, gboolean horiz)
{
	static gboolean dragging = FALSE;
	static SPCanvasItem * guide = NULL;
	int wx, wy;

	SPDesktop *desktop = dtw->desktop;
	SPRepr *repr = SP_OBJECT_REPR (desktop->namedview);

	gdk_window_get_pointer (GTK_WIDGET (dtw->canvas)->window, &wx, &wy, NULL);
	NR::Point const event_win(wx, wy);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			dragging = TRUE;
			NR::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
			NR::Point const event_dt(sp_desktop_w2d_xy_point(desktop, event_w));

			// explicitly show guidelines; if I draw a guide, I want them on
			sp_repr_set_boolean (repr, "showguides", TRUE);
			sp_repr_set_boolean (repr, "inkscape:guide-bbox", TRUE);

			double const guide_pos_dt = event_dt[ horiz
							      ? NR::Y
							      : NR::X ];
			guide = sp_guideline_new(desktop->guides, guide_pos_dt, !horiz);
			sp_guideline_set_color (SP_GUIDELINE (guide), desktop->namedview->guidehicolor);
			gdk_pointer_grab (widget->window, FALSE,
					  (GdkEventMask)(GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK),
					  NULL, NULL,
					  event->button.time);
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging) {
			NR::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
			NR::Point const event_dt(sp_desktop_w2d_xy_point(desktop, event_w));
			double const guide_pos_dt = event_dt[ horiz
							      ? NR::Y
							      : NR::X ];
			sp_guideline_set_position(SP_GUIDELINE(guide), guide_pos_dt);
			sp_desktop_set_coordinate_status(desktop, event_dt, ( horiz
									      ? SP_COORDINATES_UNDERLINE_Y
									      : SP_COORDINATES_UNDERLINE_X ));
			sp_view_set_position(SP_VIEW(desktop), event_dt);
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (dragging && event->button.button == 1) {
			gdk_pointer_ungrab (event->button.time);
			NR::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
			NR::Point const event_dt(sp_desktop_w2d_xy_point(desktop, event_w));
			dragging = FALSE;
			gtk_object_destroy (GTK_OBJECT (guide));
			guide = NULL;
			if ( ( horiz
			       ? wy
			       : wx )
			     >= 0 )
			{
				SPRepr *repr = sp_repr_new("sodipodi:guide");
				sp_repr_set_attr (repr, "orientation", (horiz) ? "horizontal" : "vertical");
				double const guide_pos_dt = event_dt[ horiz
								      ? NR::Y
								      : NR::X ];
				sp_repr_set_double(repr, "position", guide_pos_dt);
				sp_repr_append_child (SP_OBJECT_REPR (desktop->namedview), repr);
				sp_repr_unref (repr);
				sp_document_done (SP_DT_DOCUMENT (desktop));
			}
			sp_desktop_set_coordinate_status(desktop, event_dt, 0);
		}
	default:
		break;
	}

	return FALSE;
}

int
sp_dt_hruler_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
	return sp_dt_ruler_event (widget, event, dtw, TRUE);
}

int
sp_dt_vruler_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
	return sp_dt_ruler_event (widget, event, dtw, FALSE);
}

/* Guides */

gint
sp_dt_guide_event (SPCanvasItem * item, GdkEvent * event, gpointer data)
{
	static gboolean dragging = FALSE, moved = FALSE;
	gint ret = FALSE;

	SPGuide *guide = SP_GUIDE(data);
	SPDesktop *desktop = SP_DESKTOP(gtk_object_get_data(GTK_OBJECT(item->canvas), "SPDesktop"));

	switch (event->type) {
	case GDK_2BUTTON_PRESS:
		if (event->button.button == 1) {
			dragging = FALSE;
			sp_canvas_item_ungrab (item, event->button.time);
			sp_dt_simple_guide_dialog (guide, desktop);
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			dragging = TRUE;
			sp_canvas_item_grab(item,
					    ( GDK_BUTTON_RELEASE_MASK  |
					      GDK_BUTTON_PRESS_MASK    |
					      GDK_POINTER_MOTION_MASK  |
					      GDK_POINTER_MOTION_HINT_MASK ),
					    NULL,
					    event->button.time);
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging) {
			NR::Point const motion_w(event->motion.x,
						 event->motion.y);
			NR::Point const motion_dt( motion_w * desktop->w2d );
			sp_guide_moveto(*guide, sp_guide_position_from_pt(guide, motion_dt), false);
			moved = TRUE;
			sp_desktop_set_coordinate_status(desktop, motion_dt, 0);
			sp_view_set_position(SP_VIEW(desktop), motion_dt);
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (dragging && event->button.button == 1) {
			if (moved) {
				NR::Point const event_w(event->button.x,
							event->button.y);
				NR::Point const event_dt( event_w * desktop->w2d );
				if (sp_canvas_world_pt_inside_window(item->canvas, event_w)) {
					sp_guide_moveto(*guide, sp_guide_position_from_pt(guide, event_dt), true);
				} else {
					/* Undo movement of any attached shapes. */
					sp_guide_moveto(*guide, guide->position, false);
					sp_guide_remove(guide);
				}
				moved = FALSE;
				sp_document_done (SP_DT_DOCUMENT (desktop));
				sp_desktop_set_coordinate_status(desktop, event_dt, 0);
				sp_view_set_position(SP_VIEW(desktop), event_dt);
			}
			dragging = FALSE;
			sp_canvas_item_ungrab (item, event->button.time);
			ret=TRUE;
		}
	case GDK_ENTER_NOTIFY:
	{
		
		sp_guideline_set_color (SP_GUIDELINE (item), guide->hicolor);

		GString *position_string = SP_PT_TO_METRIC_STRING (guide->position, SP_DEFAULT_METRIC);
		char *guide_description = sp_guide_description(guide);

		desktop->guidesMessageContext()->setF(Inkscape::NORMAL_MESSAGE, _("%s at %s"), guide_description, position_string->str);

		g_free(guide_description);
		g_string_free(position_string, TRUE);
       		break;
	}
	case GDK_LEAVE_NOTIFY:
		sp_guideline_set_color (SP_GUIDELINE (item), guide->color);
		desktop->guidesMessageContext()->clear();
		break;
	default:
		break;
	}

	return ret;
}



/*
 * simple guideline dialog
 */

static GtkWidget *d = NULL, *l1, *l2, * e , *u, * m;
static gdouble oldpos;
static gboolean mode;
static gpointer g;


static void
guide_dialog_mode_changed (GtkWidget * widget)
{
  if (mode) {
    gtk_label_set_text (GTK_LABEL (m), _(" relative by "));
    mode = FALSE;
  } else {
    gtk_label_set_text (GTK_LABEL (m), _(" absolute to "));
    mode = TRUE;
  }
}

static void
guide_dialog_close (GtkWidget * widget, gpointer data)
{
	gtk_widget_hide (GTK_WIDGET(d));
}

static void guide_dialog_apply(SPGuide &guide)
{
	gdouble const raw_dist = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(e));
	SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(u));
	gdouble const points = sp_convert_distance_full(raw_dist, unit, sp_unit_get_by_id(SP_UNIT_PT), 1.0);
	gdouble const newpos = ( mode
				 ? points
				 : guide.position + points );
	sp_guide_moveto(guide, newpos, true);
	sp_document_done(SP_OBJECT_DOCUMENT(&guide));
}

static void
guide_dialog_ok (GtkWidget * widget, gpointer g)
{
	SPGuide &guide = **static_cast<SPGuide**>(g);
	guide_dialog_apply(guide);
	guide_dialog_close(NULL, GTK_DIALOG(widget));
}

static void
guide_dialog_delete (GtkWidget *widget, SPGuide **guide)
{
	SPDocument *doc = SP_OBJECT_DOCUMENT (*guide);
	sp_guide_remove (*guide);
	sp_document_done (doc);
	guide_dialog_close (NULL, GTK_DIALOG (widget));
}

static void
guide_dialog_response (GtkDialog *dialog, gint response, gpointer data)
{
	GtkWidget *widget = GTK_WIDGET(dialog);

	switch (response) {
	case GTK_RESPONSE_OK:
		guide_dialog_ok (widget, data);
		break;
	case -12:
		guide_dialog_delete (widget, (SPGuide**)data);
		break;
	case GTK_RESPONSE_CLOSE:
		guide_dialog_close (widget, (GtkDialog*)data);
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
/*	case GTK_RESPONSE_APPLY:
		guide_dialog_apply (widget, data);
		break;
*/
	default:
		g_assert_not_reached ();
	}
}

static void
sp_dt_simple_guide_dialog (SPGuide *guide, SPDesktop *desktop)
{
	GtkWidget * pix, * b1, * b2, * b3, * b4,* but;

	if (!GTK_IS_WIDGET (d)) {
		GtkObject *a;
		// create dialog
		d = gtk_dialog_new_with_buttons (_("Guideline"),
						 NULL,
						 GTK_DIALOG_MODAL,
						 GTK_STOCK_OK,
						 GTK_RESPONSE_OK,
						 GTK_STOCK_DELETE,
						 -12, /* DELETE */
						 GTK_STOCK_CLOSE,
						 GTK_RESPONSE_CLOSE,
						 NULL);
		sp_transientize (d);
		gtk_widget_hide (d);
    
		b1 = gtk_hbox_new (FALSE,4);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (d)->vbox), b1, FALSE, FALSE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (b1), 4);
		gtk_widget_show (b1);

		b2 = gtk_vbox_new (FALSE,4);
		gtk_box_pack_end (GTK_BOX (b1), b2, TRUE, TRUE, 0);
		gtk_widget_show (b2);
    
		//pixmap
		pix = sp_icon_new (32, "guide_dialog");
		gtk_box_pack_start (GTK_BOX (b1), pix, TRUE, TRUE, 0);
		gtk_widget_show (pix);
		//labels
		b3 = gtk_hbox_new (FALSE,4);
		gtk_box_pack_start (GTK_BOX (b2), b3, TRUE, TRUE, 0);
		gtk_widget_show (b3);

		l1 = gtk_label_new ("foo1");
		gtk_box_pack_start (GTK_BOX (b3), l1, TRUE, TRUE, 0);
		gtk_misc_set_alignment (GTK_MISC (l1), 1.0, 0.5);
		gtk_widget_show (l1);

		l2 = gtk_label_new ("foo2");
		gtk_box_pack_start (GTK_BOX (b3), l2, TRUE, TRUE, 0);
		gtk_misc_set_alignment (GTK_MISC (l2), 0.0, 0.5);
		gtk_widget_show (l2);
    
		b4 = gtk_hbox_new (FALSE,4);
		gtk_box_pack_start (GTK_BOX (b2), b4, FALSE, FALSE, 0);
		gtk_widget_show (b4);
		// mode button
		but = gtk_button_new ();
		gtk_button_set_relief (GTK_BUTTON (but), GTK_RELIEF_NONE);
		gtk_box_pack_start (GTK_BOX (b4), but, FALSE, TRUE, 0);
		gtk_signal_connect_while_alive (GTK_OBJECT (but), "clicked", GTK_SIGNAL_FUNC (guide_dialog_mode_changed), 
						NULL , GTK_OBJECT(but));
		gtk_widget_show (but);
		m = gtk_label_new (_(" absolute to "));
		mode = TRUE;
		gtk_container_add (GTK_CONTAINER (but), m);
		gtk_widget_show (m);
    
		// spinbutton
		a = gtk_adjustment_new (0.0, -SP_DESKTOP_SCROLL_LIMIT, SP_DESKTOP_SCROLL_LIMIT, 1.0, 10.0, 10.0);
		e = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0 , 2);
		gtk_widget_show (e);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (e), TRUE);
		gtk_box_pack_start (GTK_BOX (b4), e, TRUE, TRUE, 0);
		gtk_signal_connect_object(GTK_OBJECT(e), "activate",
					  GTK_SIGNAL_FUNC(gtk_window_activate_default), 
					  GTK_OBJECT(d));
/*  		gnome_dialog_editable_enters (GNOME_DIALOG (d), GTK_EDITABLE (e));  */
		// unitmenu
		/* fixme: We should allow percents here too */
		u = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
		gtk_widget_show (u);
		gtk_box_pack_start (GTK_BOX (b4), u, FALSE, FALSE, 0);
		sp_unit_selector_set_unit (SP_UNIT_SELECTOR (u), sp_unit_get_identity (SP_UNIT_ABSOLUTE));
		sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (u), GTK_ADJUSTMENT (a));
		// dialog
		gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
		gtk_signal_connect (GTK_OBJECT(d), "response", GTK_SIGNAL_FUNC(guide_dialog_response), &g);
		gtk_signal_connect (GTK_OBJECT(d), "delete_event", GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), GTK_WIDGET(d));
	}

	// initialize dialog
	g = guide;
	oldpos = guide->position;
	{
		char *guide_description = sp_guide_description(guide);
		char *label = g_strdup_printf(_("Move %s"), guide_description);
		g_free(guide_description);
		gtk_label_set (GTK_LABEL (l1), label);
		g_free(label);
	}

	SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(u));
	gdouble const val = sp_convert_distance_full(oldpos, sp_unit_get_by_id(SP_UNIT_PT), unit, 1.0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (e), val);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (e), val);
	gtk_widget_grab_focus (e);
	gtk_editable_select_region (GTK_EDITABLE (e), 0, 20);
	gtk_window_set_position (GTK_WINDOW (d), GTK_WIN_POS_MOUSE);

	gtk_widget_show (d);
}


