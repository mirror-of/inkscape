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

#include <gtk/gtksignal.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>
#include "helper/guideline.h"
#include "helper/unit-menu.h"
#include "widgets/icon.h"
#include "sodipodi-private.h"
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
	gpointer ddata;
	SPDesktop * desktop;

	ddata = gtk_object_get_data (GTK_OBJECT (item->canvas), "SPDesktop");
	g_return_val_if_fail (ddata != NULL, FALSE);

	desktop = SP_DESKTOP (ddata);

	return sp_event_context_item_handler (desktop->event_context, SP_ITEM (data), event);
}


// not needed 
gint
sp_desktop_enter_notify (GtkWidget * widget, GdkEventCrossing * event)
{
  	sodipodi_activate_desktop (SP_DESKTOP (widget));
	return FALSE;
}

static gint
sp_dt_ruler_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw, gboolean horiz)
{
	static gboolean dragging = FALSE;
	static SPCanvasItem * guide = NULL;
	SPDesktop *desktop;
	NRPointF p;
	double px, py;
	int wx, wy;

	desktop = dtw->desktop;

	gdk_window_get_pointer (GTK_WIDGET (dtw->canvas)->window, &wx, &wy, NULL);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			dragging = TRUE;
			sp_canvas_window_to_world (dtw->canvas, wx, wy, &px, &py);
			sp_desktop_w2d_xy_point (desktop, &p, px, py);
			guide = sp_guideline_new (desktop->guides, (horiz) ? p.y : p.x, !horiz);
			sp_guideline_set_color (SP_GUIDELINE (guide), desktop->namedview->guidehicolor);
			gdk_pointer_grab (widget->window, FALSE,
					  GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
					  NULL, NULL,
					  event->button.time);
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging) {
			sp_canvas_window_to_world (dtw->canvas, wx, wy, &px, &py);
			sp_desktop_w2d_xy_point (desktop, &p, px, py);
			sp_guideline_set_position (SP_GUIDELINE (guide), (horiz) ? p.y : p.x);
			if (horiz) {
				sp_desktop_set_coordinate_status (desktop, p.x, p.y, SP_COORDINATES_UNDERLINE_Y);
			} else {
				sp_desktop_set_coordinate_status (desktop, p.x, p.y, SP_COORDINATES_UNDERLINE_X);
			}
			sp_view_set_position (SP_VIEW (desktop), p.x, p.y);
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (dragging && event->button.button == 1) {
			NRPointF p;
		        gdk_pointer_ungrab (event->button.time);
			sp_canvas_window_to_world (dtw->canvas, wx, wy, &px, &py);
			sp_desktop_w2d_xy_point (desktop, &p, px, py);
			dragging = FALSE;
			gtk_object_destroy (GTK_OBJECT (guide));
			guide = NULL;
			if ((horiz && (wy >= 0)) || (!horiz && (wx >= 0))) {
				SPRepr *repr;
				repr = sp_repr_new ("sodipodi:guide");
				sp_repr_set_attr (repr, "orientation", (horiz) ? "horizontal" : "vertical");
				sp_repr_set_double (repr, "position", horiz ? p.y : p.x);
				sp_repr_append_child (SP_OBJECT_REPR (desktop->namedview), repr);
				sp_repr_unref (repr);
				sp_document_done (SP_DT_DOCUMENT (desktop));
			}
			sp_desktop_set_coordinate_status (desktop, p.x, p.y, 0);
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
	SPGuide * guide;
	SPDesktop * desktop;
	gint ret = FALSE;
	GString * msg;

	guide = SP_GUIDE (data);
	desktop = SP_DESKTOP (gtk_object_get_data (GTK_OBJECT (item->canvas), "SPDesktop"));

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
			sp_canvas_item_grab (item, GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
						NULL, event->button.time);
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging) {
			NRPointF p;
			sp_desktop_w2d_xy_point (desktop, &p, event->motion.x, event->motion.y);
			sp_guide_moveto (guide, p.x, p.y);
			moved = TRUE;
			switch (guide->orientation){
			case SP_GUIDE_HORIZONTAL:
				sp_desktop_set_coordinate_status (desktop, p.x, p.y, SP_COORDINATES_UNDERLINE_Y);
				break;
			case SP_GUIDE_VERTICAL:
				sp_desktop_set_coordinate_status (desktop, p.x, p.y, SP_COORDINATES_UNDERLINE_X);
				break;
			}
			sp_view_set_position (SP_VIEW (desktop), p.x, p.y);
			ret=TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (dragging && event->button.button == 1) {
			if (moved) {
				GtkWidget *w;
				double winx, winy;
				NRPointF p;
				w = GTK_WIDGET (item->canvas);
				sp_canvas_world_to_window (item->canvas, event->button.x, event->button.y, &winx, &winy);
				sp_desktop_w2d_xy_point (desktop, &p, event->button.x, event->button.y);
				if ((winx >= 0) && (winy >= 0) &&
				    (winx < w->allocation.width) &&
				    (winy < w->allocation.height)) {
					sp_guide_position_set (guide, p.x, p.y);
				} else {
					sp_guide_remove(guide);
				}
				moved = FALSE;
				sp_document_done (SP_DT_DOCUMENT (desktop));
				sp_desktop_set_coordinate_status (desktop, p.x, p.y, 0);
				sp_view_set_position (SP_VIEW (desktop), p.x, p.y);
			}
			dragging = FALSE;
			sp_canvas_item_ungrab (item, event->button.time);
			ret=TRUE;
		}
	case GDK_ENTER_NOTIFY:
		sp_guideline_set_color (SP_GUIDELINE (item), guide->hicolor);
		msg = SP_PT_TO_METRIC_STRING (guide->position, SP_DEFAULT_METRIC);
		switch (guide->orientation) {
		case SP_GUIDE_HORIZONTAL:
			g_string_prepend(msg, "horizontal guideline at ");
			break;
		case SP_GUIDE_VERTICAL:
			g_string_prepend(msg, "vertical guideline at ");
			break;
		}
		sp_view_set_status (SP_VIEW (desktop), msg->str, FALSE);
		g_string_free (msg, FALSE);
       		break;
	case GDK_LEAVE_NOTIFY:
		sp_guideline_set_color (SP_GUIDELINE (item), guide->color);
		sp_view_set_status (SP_VIEW (desktop), NULL, FALSE);
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

#if 0
static gdouble
get_document_len (SPGuideOrientation orientation)
{
  SPDocument * document = NULL;
  gdouble len =0;

  document = SP_ACTIVE_DOCUMENT;
  g_return_val_if_fail (SP_IS_DOCUMENT (document),0);
  switch (orientation) {
  case SP_GUIDE_VERTICAL:
    len = sp_document_width (document);
    break;
  case SP_GUIDE_HORIZONTAL:
    len = sp_document_height (document);
    break;
  }    
  return len;
}
#endif

#if 0
static void
guide_dialog_unit_changed (SPUnitMenu * u, SPSVGUnit system, SPMetric metric, SPGuide * guide)
{
  gdouble len = 0, pos = 0;
  GString * st = NULL;

  switch (system) {
  case SP_SVG_UNIT_ABSOLUTE:
    st = SP_PT_TO_METRIC_STRING (oldpos, metric);
    g_string_prepend (st,"from ");
    break;
  case SP_SVG_UNIT_PERCENT:
    len = get_document_len (guide->orientation);
    pos = 100 * oldpos / len;
    st = g_string_new ("");
    g_string_sprintf (st, "from %0.2f %s", pos, "%");    
    break;
  default:
    g_print("unit not allowed (should not happen)\n");
    break;
  }
  gtk_label_set (GTK_LABEL (l2), st->str);
  g_string_free (st, TRUE);
}
#endif

static void
guide_dialog_mode_changed (GtkWidget * widget)
{
  if (mode) {
    gtk_label_set_text (GTK_LABEL (m), " relative by ");
    mode = FALSE;
  } else {
    gtk_label_set_text (GTK_LABEL (m), " absolute to ");
    mode = TRUE;
  }
}

static void
guide_dialog_close (GtkWidget * widget, GtkDialog * d)
{
	gtk_object_destroy (GTK_OBJECT(d));
}

static void
guide_dialog_apply (GtkWidget * widget, SPGuide ** g)
{
	gdouble distance;
	const SPUnit *unit;
	gdouble newpos;
	SPGuide * guide;
  
	guide = *g;

	distance = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (e));
	unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (u));
	sp_convert_distance_full (&distance, unit, sp_unit_get_identity (SP_UNIT_ABSOLUTE), 1.0, 1.0);
	if (mode) {
		newpos = distance;
	} else {
		newpos = guide->position + distance;
	}
	sp_guide_position_set (guide, newpos, newpos);
	sp_document_done (SP_OBJECT_DOCUMENT (guide));
}

static void
guide_dialog_ok (GtkWidget * widget, gpointer g)
{
	guide_dialog_apply (NULL, g);
	guide_dialog_close (NULL, GTK_DIALOG(widget));
}

static void
guide_dialog_delete (GtkWidget *widget, SPGuide **guide)
{
	SPDocument *doc;
  
	doc = SP_OBJECT_DOCUMENT (*guide);
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
		guide_dialog_delete (widget, data);
		break;
	case GTK_RESPONSE_CLOSE:
		guide_dialog_close (widget, data);
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
	gdouble val = 0;
	GtkWidget * pix, * b1, * b2, * b3, * b4,* but;
	const SPUnit *unit;

	if (!GTK_IS_WIDGET (d)) {
		GtkObject *a;
		// create dialog
		d = gtk_dialog_new_with_buttons ("Guideline",
						 NULL,
						 GTK_DIALOG_MODAL,
						 GTK_STOCK_OK,
						 GTK_RESPONSE_OK,
						 GTK_STOCK_DELETE,
						 -12, /* DELETE */
						 GTK_STOCK_CLOSE,
						 GTK_RESPONSE_CLOSE,
						 NULL);
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
		m = gtk_label_new (" absolute to ");
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
#if 0
		gtk_signal_connect_while_alive (GTK_OBJECT (u), "set_unit", GTK_SIGNAL_FUNC (guide_dialog_unit_changed), 
						guide , GTK_OBJECT(u));
#endif
		// dialog
		gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
		gtk_signal_connect (GTK_OBJECT(d), "response", GTK_SIGNAL_FUNC(guide_dialog_response), &g);
	}

	// initialize dialog
	g = guide;
	oldpos = guide->position;
	switch (guide->orientation) {
	case SP_GUIDE_VERTICAL:
		gtk_label_set (GTK_LABEL (l1), "Move vertical guideline");
		break;
	case SP_GUIDE_HORIZONTAL:
		gtk_label_set (GTK_LABEL (l1), "Move horizontal guideline");
		break;
	}

	val = oldpos;
	unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (u));
	sp_convert_distance_full (&val, sp_unit_get_identity (SP_UNIT_ABSOLUTE), unit, 1.0, 1.0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (e), val);
#if 0
	switch (system) {
	case SP_SVG_UNIT_ABSOLUTE:
		val = SP_PT_TO_METRIC (oldpos, metric);
		break;
	case SP_SVG_UNIT_PERCENT:
		len = get_document_len (guide->orientation);
		val = 100 * guide->position /len;
		break;
	default:
		g_print("unit not allowed (should not happen\n");
		break;
	}
#endif
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (e), val);
	gtk_widget_grab_focus (e);
	gtk_editable_select_region (GTK_EDITABLE (e), 0, 20);
	gtk_window_set_position (GTK_WINDOW (d), GTK_WIN_POS_MOUSE);

	gtk_widget_show (d);
}


