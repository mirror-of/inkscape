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

#include <config.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include "helper/sp-canvas.h"
#include "sp-spiral.h"
#include "sodipodi.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "pixmaps/cursor-spiral.xpm"
#include "spiral-context.h"
#include "sp-metrics.h"
#include "helper/sp-intl.h"

static void sp_spiral_context_class_init (SPSpiralContextClass * klass);
static void sp_spiral_context_init (SPSpiralContext * spiral_context);
static void sp_spiral_context_dispose (GObject *object);
static void sp_spiral_context_setup (SPEventContext *ec);
static void sp_spiral_context_set (SPEventContext *ec, const guchar *key, const guchar *val);

static gint sp_spiral_context_root_handler (SPEventContext * event_context, GdkEvent * event);

static GtkWidget *sp_spiral_context_config_widget (SPEventContext *ec);

static void sp_spiral_drag (SPSpiralContext * sc, double x, double y, guint state);
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
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPSpiralContext", &info, 0);
	}
	return type;
}

static void
sp_spiral_context_class_init (SPSpiralContextClass * klass)
{
	GObjectClass * object_class;
	SPEventContextClass * event_context_class;

	object_class = (GObjectClass *) klass;
	event_context_class = (SPEventContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = sp_spiral_context_dispose;

	event_context_class->setup = sp_spiral_context_setup;
	event_context_class->set = sp_spiral_context_set;
	event_context_class->root_handler = sp_spiral_context_root_handler;
	event_context_class->config_widget = sp_spiral_context_config_widget;
}

static void
sp_spiral_context_init (SPSpiralContext * spiral_context)
{
	SPEventContext * event_context;
	
	event_context = SP_EVENT_CONTEXT (spiral_context);

	event_context->cursor_shape = cursor_spiral_xpm;
	event_context->hot_x = 4;
	event_context->hot_y = 4;

	spiral_context->item = NULL;

	spiral_context->revo = 3.0;
	spiral_context->exp = 1.0;
	spiral_context->t0 = 0.0;
}

static void
sp_spiral_context_dispose (GObject *object)
{
	SPSpiralContext * sc;

	sc = SP_SPIRAL_CONTEXT (object);

	/* fixme: This is necessary because we do not grab */
	if (sc->item) sp_spiral_finish (sc);

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
sp_spiral_context_setup (SPEventContext *ec)
{
	SPSpiralContext * sc;

	sc = SP_SPIRAL_CONTEXT (ec);

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	sp_event_context_read (ec, "expansion");
	sp_event_context_read (ec, "revolution");
	sp_event_context_read (ec, "t0");
}

static void
sp_spiral_context_set (SPEventContext *ec, const guchar *key, const guchar *val)
{
	SPSpiralContext *sc;

	sc = SP_SPIRAL_CONTEXT (ec);

	if (!strcmp (key, "expansion")) {
		sc->exp = (val) ? atof (val) : 1.0;
		sc->exp = CLAMP (sc->exp, 0.0, 1000.0);
	} else if (!strcmp (key, "revolution")) {
		sc->revo = (val) ? atof (val) : 3.0;
		sc->revo = CLAMP (sc->revo, 0.05, 20.0);
	} else if (!strcmp (key, "t0")) {
		sc->t0 = (val) ? atof (val) : 0.0;
		sc->t0 = CLAMP (sc->t0, 0.0, 0.999);
	}
}

static gint
sp_spiral_context_root_handler (SPEventContext * event_context, GdkEvent * event)
{
	static gboolean dragging;
	SPSpiralContext * sc;
	gint ret;
	SPDesktop * desktop;

	desktop = event_context->desktop;
	sc = SP_SPIRAL_CONTEXT (event_context);
	ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			NRPointF fp;
			dragging = TRUE;
			/* Position center */
			sp_desktop_w2d_xy_point (event_context->desktop, &fp, event->button.x, event->button.y);
			sc->center.x = fp.x;
			sc->center.y = fp.y;
			/* Snap center to nearest magnetic point */
			sp_desktop_free_snap (event_context->desktop, &sc->center);
			sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
						GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (dragging && event->motion.state && GDK_BUTTON1_MASK) {
			NRPointF p;
			sp_desktop_w2d_xy_point (event_context->desktop, &p, event->motion.x, event->motion.y);
			sp_spiral_drag (sc, p.x, p.y, event->motion.state);
			ret = TRUE;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			dragging = FALSE;
			sp_spiral_finish (sc);
			ret = TRUE;
			sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
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
sp_spiral_drag (SPSpiralContext * sc, double x, double y, guint state)
{
	SPSpiral *spiral;
	SPDesktop *desktop;
	NRPointF p0, p1;
	gdouble dx, dy, rad, arg;
	GString *xs, *ys;
	gchar status[80];
	NRPointF fp;

	desktop = SP_EVENT_CONTEXT (sc)->desktop;

	if (!sc->item) {
		SPRepr * repr, * style;
		SPCSSAttr * css;
		/* Create object */
		repr = sp_repr_new ("path");
                sp_repr_set_attr (repr, "sodipodi:type", "spiral");
		/* Set style */
		style = sodipodi_get_repr (SODIPODI, "tools.shapes.spiral");
		if (style) {
			css = sp_repr_css_attr_inherited (style, "style");
			sp_repr_css_set (repr, css, "style");
			sp_repr_css_attr_unref (css);
		}
                else
                  g_print ("sp_spiral_drag: cant get style\n");

		sc->item = (SPItem *) sp_document_add_repr (SP_DT_DOCUMENT (desktop), repr);
		sp_repr_unref (repr);
	}

	/* This is bit ugly, but so we are */

/*  	if (state & GDK_CONTROL_MASK) { */
/*  	} else if (state & GDK_SHIFT_MASK) { */

	/* Free movement for corner point */
	sp_desktop_dt2root_xy_point (desktop, &fp, sc->center.x, sc->center.y);
	p0.x = fp.x;
	p0.y = fp.y;
	sp_desktop_dt2root_xy_point (desktop, &fp, x, y);
	p1.x = fp.x;
	p1.y = fp.y;
	sp_desktop_free_snap (desktop, &p1);
	
	spiral = SP_SPIRAL (sc->item);

	dx = p1.x - p0.x;
	dy = p1.y - p0.y;
	rad = hypot (dx, dy);
	arg = atan2 (dy, dx) - 2.0*M_PI*spiral->revo;
	
        /* Fixme: these parameters should be got from dialog box */
	sp_spiral_position_set (spiral, p0.x, p0.y,
		       /*expansion*/ sc->exp,
		       /*revolution*/ sc->revo,
		       rad, arg,
		       /*t0*/ sc->t0);
	
	/* status text */
	xs = SP_PT_TO_METRIC_STRING (fabs(p0.x), SP_DEFAULT_METRIC);
	ys = SP_PT_TO_METRIC_STRING (fabs(p0.y), SP_DEFAULT_METRIC);
	sprintf (status, "Draw spiral at (%s,%s)", xs->str, ys->str);
	sp_view_set_status (SP_VIEW (desktop), status, FALSE);
	g_string_free (xs, FALSE);
	g_string_free (ys, FALSE);
}

static void
sp_spiral_finish (SPSpiralContext * sc)
{
	if (sc->item != NULL) {
		SPDesktop *desktop;
		SPSpiral  *spiral;

		desktop = SP_EVENT_CONTEXT (sc)->desktop;
		spiral = SP_SPIRAL (sc->item);

		sp_shape_set_shape(SP_SHAPE(spiral));
		sp_object_invoke_write (SP_OBJECT (spiral), NULL, SP_OBJECT_WRITE_SODIPODI);

		sp_selection_set_item (SP_DT_SELECTION (desktop), sc->item);
		sp_document_done (SP_DT_DOCUMENT (desktop));

		sc->item = NULL;
	}
}

/* Gtk stuff */

static void
sp_sc_revolution_value_changed (GtkAdjustment *adj, SPSpiralContext *sc)
{
	sp_repr_set_int (SP_EVENT_CONTEXT_REPR (sc), "revolution", (gint) adj->value);
}

static void
sp_sc_expansion_value_changed (GtkAdjustment *adj, SPSpiralContext *sc)
{
	sp_repr_set_double (SP_EVENT_CONTEXT_REPR (sc), "expansion", adj->value);
}

static void
sp_sc_t0_value_changed (GtkAdjustment *adj, SPSpiralContext *sc)
{
	sp_repr_set_double (SP_EVENT_CONTEXT_REPR (sc), "t0", adj->value);
}

static void
sp_sc_defaults (GtkWidget *widget, GtkObject *obj)
{
	GtkAdjustment *adj;

	adj = gtk_object_get_data (obj, "revolution");
	gtk_adjustment_set_value (adj, 3.0);
	adj = gtk_object_get_data (obj, "expansion");
	gtk_adjustment_set_value (adj, 1.0);
	adj = gtk_object_get_data (obj, "t0");
	gtk_adjustment_set_value (adj, 0.0);
}

static GtkWidget *
sp_spiral_context_config_widget (SPEventContext *ec)
{
	SPSpiralContext *sc;
	GtkWidget *tbl, *l, *sb, *b;
	GtkObject *a;

	sc = SP_SPIRAL_CONTEXT (ec);

	tbl = gtk_table_new (4, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (tbl), 4);
	gtk_table_set_row_spacings (GTK_TABLE (tbl), 4);

	/* Revolution */
	l = gtk_label_new (_("Revolution:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 0, 1, 0, 0, 0, 0);
	a = gtk_adjustment_new (sc->revo, 0.05, 20.0, 1.0, 1.0, 1.0);
	gtk_object_set_data (GTK_OBJECT (tbl), "revolution", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_sc_revolution_value_changed), sc);

	/* Expansion */
	l = gtk_label_new (_("Expansion:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 1, 2, 0, 0, 0, 0);
	a = gtk_adjustment_new (sc->exp, 0.0, 1000.0, 0.1, 1.0, 1.0);
	gtk_object_set_data (GTK_OBJECT (tbl), "expansion", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_sc_expansion_value_changed), sc);

	/* T0 */
	l = gtk_label_new (_("Inner radius:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 2, 3, 0, 0, 0, 0);
	a = gtk_adjustment_new (sc->t0, 0.0, 0.999, 0.1, 1.0, 1.0);
	gtk_object_set_data (GTK_OBJECT (tbl), "t0", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_sc_t0_value_changed), sc);

	/* Reset */
	b = gtk_button_new_with_label (_("Defaults"));
	gtk_widget_show (b);
	gtk_table_attach (GTK_TABLE (tbl), b, 0, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_sc_defaults), tbl);

	return tbl;
}

