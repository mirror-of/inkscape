#define __SP_SPIRAL_CONTEXT_C__

/*
 * Spiral drawing context
 *
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2001-2002 Mitsuru Oka
 *
 * Released under GNU GPL
 */

#include "config.h"

#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-spiral.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "desktop.h"
#include "pixmaps/cursor-spiral.xpm"
#include "spiral-context.h"
#include "sp-metrics.h"
#include "helper/sp-intl.h"
#include "object-edit.h"
#include "knotholder.h"
#include "xml/repr.h"
#include "xml/repr-private.h"
#include "prefs-utils.h"
#include "widgets/spw-utilities.h"
#include <libnr/nr-point-fns.h>

static void sp_spiral_context_class_init (SPSpiralContextClass * klass);
static void sp_spiral_context_init (SPSpiralContext * spiral_context);
static void sp_spiral_context_dispose (GObject *object);
static void sp_spiral_context_setup (SPEventContext *ec);
static void sp_spiral_context_set (SPEventContext *ec, const gchar *key, const gchar *val);

static gint sp_spiral_context_root_handler (SPEventContext * event_context, GdkEvent * event);

static void sp_spiral_drag (SPSpiralContext * sc, NR::Point p, guint state);
static void sp_spiral_finish (SPSpiralContext * sc);

static SPEventContextClass * parent_class;

GtkType
sp_spiral_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPSpiralContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_spiral_context_class_init,
			NULL, NULL,
			sizeof (SPSpiralContext),
			4,
			(GInstanceInitFunc) sp_spiral_context_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPSpiralContext", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_spiral_context_class_init (SPSpiralContextClass * klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

	object_class->dispose = sp_spiral_context_dispose;

	event_context_class->setup = sp_spiral_context_setup;
	event_context_class->set = sp_spiral_context_set;
	event_context_class->root_handler = sp_spiral_context_root_handler;
}

static void
sp_spiral_context_init (SPSpiralContext * spiral_context)
{
	SPEventContext *event_context = SP_EVENT_CONTEXT (spiral_context);

	event_context->cursor_shape = cursor_spiral_xpm;
	event_context->hot_x = 4;
	event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

	spiral_context->item = NULL;

	spiral_context->revo = 3.0;
	spiral_context->exp = 1.0;
	spiral_context->t0 = 0.0;

	new (&spiral_context->sel_changed_connection) SigC::Connection();
}

static void
sp_spiral_context_dispose (GObject *object)
{
	SPSpiralContext *sc = SP_SPIRAL_CONTEXT (object);

    SPEventContext *ec = SP_EVENT_CONTEXT (object);

    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection.~Connection();

	/* fixme: This is necessary because we do not grab */
	if (sc->item) sp_spiral_finish (sc);

    if (sc->knot_holder) {
        sp_knot_holder_destroy (sc->knot_holder);
        sc->knot_holder = NULL;
    }

    if (sc->repr) { // remove old listener
        sp_repr_remove_listener_by_data (sc->repr, ec);
        sp_repr_unref (sc->repr);
        sc->repr = 0;
    }

    sp_sel_cue_shutdown(&(ec->selcue));

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void shape_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data)
{
    SPSpiralContext *sc;
    SPEventContext *ec;

    sc = SP_SPIRAL_CONTEXT (data);
    ec = SP_EVENT_CONTEXT (sc);

    if (sc->knot_holder) {
        sp_knot_holder_destroy (sc->knot_holder);
    }
    sc->knot_holder = NULL;

    SPDesktop *desktop = ec->desktop;
    SPItem *item = SP_DT_SELECTION(desktop)->singleItem();

    if (item) {
        sc->knot_holder = sp_item_knot_holder (item, desktop);
    }
}

static SPReprEventVector shape_repr_events = {
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    shape_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void
sp_spiral_context_selection_changed (SPSelection * selection, gpointer data)
{
    SPSpiralContext *sc;
    SPEventContext *ec;

    sc = SP_SPIRAL_CONTEXT (data);
    ec = SP_EVENT_CONTEXT (sc);

    if (sc->knot_holder) { // desktroy knotholder
        sp_knot_holder_destroy (sc->knot_holder);
        sc->knot_holder = NULL;
    }

    if (sc->repr) { // remove old listener
        sp_repr_remove_listener_by_data (sc->repr, ec);
        sp_repr_unref (sc->repr);
        sc->repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        sc->knot_holder = sp_item_knot_holder (item, ec->desktop);
        SPRepr *repr = SP_OBJECT_REPR (item);
        if (repr) {
            sc->repr = repr;
            sp_repr_ref (repr);
            sp_repr_add_listener (repr, &shape_repr_events, ec);
            sp_repr_synthesize_events (repr, &shape_repr_events, ec);
        }


    }
}

static void
sp_spiral_context_setup (SPEventContext *ec)
{
    SPSpiralContext *sc = SP_SPIRAL_CONTEXT (ec);

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	sp_event_context_read (ec, "expansion");
	sp_event_context_read (ec, "revolution");
	sp_event_context_read (ec, "t0");

	SPSelection *selection = SP_DT_SELECTION(ec->desktop);

	SPItem *item = selection->singleItem();
      if (item) {
            sc->knot_holder = sp_item_knot_holder (item, ec->desktop);
            SPRepr *repr = SP_OBJECT_REPR (item);
            if (repr) {
                sc->repr = repr;
                sp_repr_ref (repr);
                sp_repr_add_listener (repr, &shape_repr_events, ec);
                sp_repr_synthesize_events (repr, &shape_repr_events, ec);
            }
      }

	sc->sel_changed_connection.disconnect();
	sc->sel_changed_connection = selection->connectChanged(SigC::bind(SigC::slot(&sp_spiral_context_selection_changed), (gpointer)sc));

	if (prefs_get_int_attribute("tools.shapes", "selcue", 0) != 0)
		sp_sel_cue_init(&(ec->selcue), ec->desktop);
}

static void
sp_spiral_context_set (SPEventContext *ec, const gchar *key, const gchar *val)
{
	SPSpiralContext *sc = SP_SPIRAL_CONTEXT (ec);

	if (!strcmp (key, "expansion")) {
		sc->exp = (val) ? g_ascii_strtod (val, NULL) : 1.0;
		sc->exp = CLAMP (sc->exp, 0.0, 1000.0);
	} else if (!strcmp (key, "revolution")) {
		sc->revo = (val) ? g_ascii_strtod (val, NULL) : 3.0;
		sc->revo = CLAMP (sc->revo, 0.05, 40.0);
	} else if (!strcmp (key, "t0")) {
		sc->t0 = (val) ? g_ascii_strtod (val, NULL) : 0.0;
		sc->t0 = CLAMP (sc->t0, 0.0, 0.999);
	}
}

static gint
sp_spiral_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
	static gboolean dragging;

	SPDesktop *desktop = event_context->desktop;
	SPSpiralContext *sc = SP_SPIRAL_CONTEXT (event_context);

    event_context->tolerance = prefs_get_int_attribute_limited ("options.dragtolerance", "value", 0, 0, 100);

	gint ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
            // save drag origin
            event_context->xp = (gint) event->button.x;
            event_context->yp = (gint) event->button.y;
            event_context->within_tolerance = true;

           // remember clicked item, disregarding groups
            event_context->item_to_select = sp_desktop_item_at_point (desktop, NR::Point(event->button.x, event->button.y), TRUE);

			dragging = TRUE;
			/* Position center */
			sc->center = sp_desktop_w2d_xy_point (event_context->desktop, NR::Point(event->button.x, event->button.y));
			/* Snap center to nearest magnetic point */
			namedview_free_snap (event_context->desktop->namedview, Snapper::SNAP_POINT, sc->center);
			sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
						GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging && event->motion.state && GDK_BUTTON1_MASK) {

			if ( event_context->within_tolerance
					 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
					 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
				break; // do not drag if we're within tolerance from origin
			}
			// Once the user has moved farther than tolerance from the original location
			// (indicating they intend to draw, not click), then always process the
			// motion notify coordinates as given (no snapping back to origin)
			event_context->within_tolerance = false;

			NR::Point const motion_w(event->motion.x, event->motion.y);
			NR::Point const motion_dt(sp_desktop_w2d_xy_point(event_context->desktop, motion_w));
			sp_spiral_drag (sc, motion_dt, event->motion.state);
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
		if (event->button.button == 1) {
			dragging = FALSE;
            if (!event_context->within_tolerance) {
                // we've been dragging, finish the spiral
			sp_spiral_finish (sc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                SP_DT_SELECTION(desktop)->setItem(event_context->item_to_select);
            } else {
                // click in an empty space
                SP_DT_SELECTION(desktop)->clear();
            }

            event_context->item_to_select = NULL;
			ret = TRUE;
			sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
		}
		break;
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_Up: 
		case GDK_Down: 
		case GDK_KP_Up: 
		case GDK_KP_Down: 
			// prevent the zoom field from activation
			if (!MOD__CTRL_ONLY)
				ret = TRUE;
			break;
 		case GDK_x:
 		case GDK_X:
 			if (MOD__ALT_ONLY) {
 				gpointer hb = sp_search_by_data_recursive (desktop->owner->aux_toolbox, (gpointer) "altx-spiral");
 				if (hb && GTK_IS_WIDGET(hb)) {
					gtk_widget_grab_focus (GTK_WIDGET (hb));
				}
 				ret = TRUE;
 			}
			break;
		case GDK_Escape:
			SP_DT_SELECTION(desktop)->clear();
			//TODO: make dragging escapable by Esc
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

static void
sp_spiral_drag (SPSpiralContext * sc, NR::Point p, guint state)
{
	SPDesktop *desktop = SP_EVENT_CONTEXT (sc)->desktop;

	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	if (!sc->item) {
		/* Create object */
		SPRepr *repr = sp_repr_new ("path");
                sp_repr_set_attr (repr, "sodipodi:type", "spiral");
		/* Set style */
		SPRepr *style = inkscape_get_repr (INKSCAPE, "tools.shapes.spiral");
		if (style) {
			SPCSSAttr *css = sp_repr_css_attr_inherited (style, "style");
			sp_repr_css_set (repr, css, "style");
			sp_repr_css_attr_unref (css);
		}
                else
                  g_print ("sp_spiral_drag: cant get style\n");

		sc->item = (SPItem *) desktop->currentLayer()->appendChildRepr(repr);
		sp_repr_unref (repr);
	}

	/* Free movement for corner point */
	NR::Point p0 = sp_desktop_dt2root_xy_point (desktop, sc->center);
	NR::Point p1 = sp_desktop_dt2root_xy_point (desktop, p);
	namedview_free_snap (desktop->namedview, Snapper::SNAP_POINT, p1);
	
	SPSpiral *spiral = SP_SPIRAL (sc->item);

	NR::Point delta = p1 - p0;
	gdouble rad = NR::L2 (delta);

	gdouble arg = NR::atan2 (delta) - 2.0*M_PI*spiral->revo;

  	if (state & GDK_CONTROL_MASK) { 
		arg = sp_round(arg, M_PI/snaps);
	} 
	
        /* Fixme: these parameters should be got from dialog box */
	sp_spiral_position_set (spiral, p0[NR::X], p0[NR::Y],
		       /*expansion*/ sc->exp,
		       /*revolution*/ sc->revo,
		       rad, arg,
		       /*t0*/ sc->t0);
	
	/* status text */
	GString *xs, *ys;
	gchar status[80];
	xs = SP_PT_TO_METRIC_STRING (fabs(p0[NR::X]), SP_DEFAULT_METRIC);
	ys = SP_PT_TO_METRIC_STRING (fabs(p0[NR::Y]), SP_DEFAULT_METRIC);
	sprintf (status, _("Draw spiral at (%s,%s)"), xs->str, ys->str);
	sp_view_set_status (SP_VIEW (desktop), status, FALSE);
	g_string_free (xs, FALSE);
	g_string_free (ys, FALSE);
}

static void
sp_spiral_finish (SPSpiralContext * sc)
{
	if (sc->item != NULL) {
		SPDesktop *desktop = SP_EVENT_CONTEXT (sc)->desktop;
		SPSpiral  *spiral = SP_SPIRAL (sc->item);

		sp_shape_set_shape(SP_SHAPE(spiral));
		SP_OBJECT (spiral)->updateRepr(NULL, SP_OBJECT_WRITE_EXT);

		SP_DT_SELECTION(desktop)->setItem(sc->item);
		sp_document_done (SP_DT_DOCUMENT (desktop));

		sc->item = NULL;
	}
}
