#define __SP_DYNA_DRAW_CONTEXT_C__

/*
 * Handwriting-like drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * TODO: Tue Oct  2 22:57:15 2001
 *  - Decide control point behavior when use_calligraphic==1.
 *  - Option dialog box support if it is availabe.
 *  - Decide to use NORMALIZED_COORDINATE or not.
 *  - Remove hard coded style attributes and move to customizable style.
 *  - Bug fix.
 */

#define noDYNA_DRAW_VERBOSE

#include <config.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gdk/gdkkeysyms.h>

#include "svg/svg.h"
#include "helper/sp-intl.h"
#include "helper/curve.h"
#include "helper/canvas-bpath.h"
#include "helper/sodipodi-ctrl.h"
#include "helper/bezier-utils.h"

#include "macros.h"
#include "enums.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-snap.h"
#include "dyna-draw-context.h"

#include <libnr/nr-point-fns.h>

#define DDC_RED_RGBA 0xff0000ff
#define DDC_GREEN_RGBA 0x000000ff

#define SAMPLE_TIMEOUT 10
#define TOLERANCE_LINE 1.0
#define TOLERANCE_CALLIGRAPHIC 3.0
#define DYNA_EPSILON 1.0e-6

#define DYNA_MIN_WIDTH 1.0e-6

#define DRAG_MIN 0.0
#define DRAG_DEFAULT 0.5
#define DRAG_MAX 1.0

static void sp_dyna_draw_context_class_init (SPDynaDrawContextClass *klass);
static void sp_dyna_draw_context_init (SPDynaDrawContext *ddc);
static void sp_dyna_draw_context_dispose (GObject *object);

static void sp_dyna_draw_context_setup (SPEventContext *ec);
static void sp_dyna_draw_context_set (SPEventContext *ec, const gchar *key, const gchar *val);
static gint sp_dyna_draw_context_root_handler (SPEventContext *ec, GdkEvent *event);

static GtkWidget *sp_dyna_draw_context_config_widget (SPEventContext *ec);

static void clear_current (SPDynaDrawContext * dc);
static void set_to_accumulated (SPDynaDrawContext * dc);
static void concat_current_line (SPDynaDrawContext * dc);
static void accumulate_calligraphic (SPDynaDrawContext * dc);

static void fit_and_split (SPDynaDrawContext *ddc, gboolean release);
static void fit_and_split_line (SPDynaDrawContext *ddc, gboolean release);
static void fit_and_split_calligraphics (SPDynaDrawContext *ddc, gboolean release);

static void sp_dyna_draw_reset (SPDynaDrawContext *ddc, NR::Point p);
static NR::Point sp_dyna_draw_get_npoint (const SPDynaDrawContext *ddc, NR::Point v);
static NR::Point sp_dyna_draw_get_vpoint (const SPDynaDrawContext *ddc, NR::Point n);
static NR::Point sp_dyna_draw_get_curr_vpoint (const SPDynaDrawContext *ddc);
static void draw_temporary_box (SPDynaDrawContext *dc);


static SPEventContextClass *parent_class;

GtkType
sp_dyna_draw_context_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPDynaDrawContextClass),
			NULL, NULL,
			(GClassInitFunc) sp_dyna_draw_context_class_init,
			NULL, NULL,
			sizeof (SPDynaDrawContext),
			4,
			(GInstanceInitFunc) sp_dyna_draw_context_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPDynaDrawContext", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_dyna_draw_context_class_init (SPDynaDrawContextClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

	parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

	object_class->dispose = sp_dyna_draw_context_dispose;

	event_context_class->setup = sp_dyna_draw_context_setup;
	event_context_class->set = sp_dyna_draw_context_set;
	event_context_class->root_handler = sp_dyna_draw_context_root_handler;
	event_context_class->config_widget = sp_dyna_draw_context_config_widget;
}

static void
sp_dyna_draw_context_init (SPDynaDrawContext *ddc)
{
	ddc->accumulated = NULL;
	ddc->segments = NULL;
	ddc->currentcurve = NULL;
	ddc->currentshape = NULL;
	ddc->npoints = 0;
	ddc->cal1 = NULL;
	ddc->cal2 = NULL;
	ddc->repr = NULL;

	/* DynaDraw values */
	ddc->cur = NR::Point(0,0);
	ddc->last = NR::Point(0,0);
	ddc->vel = NR::Point(0,0);
	ddc->acc = NR::Point(0,0);
	ddc->ang = NR::Point(0,0);
	ddc->del = NR::Point(0,0);

	/* attributes */
	ddc->fixed_angle = FALSE;
	ddc->use_timeout = FALSE;
	ddc->use_calligraphic = TRUE;
	ddc->timer_id = 0;
	ddc->dragging = FALSE;
	ddc->dynahand = FALSE;

	ddc->mass = 0.3;
	ddc->drag = DRAG_DEFAULT;
	ddc->angle = 30.0;
	ddc->width = 0.2;
}

static void
sp_dyna_draw_context_dispose (GObject *object)
{
	SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT (object);

	if (ddc->accumulated) ddc->accumulated = sp_curve_unref (ddc->accumulated);

	while (ddc->segments) {
		gtk_object_destroy (GTK_OBJECT (ddc->segments->data));
		ddc->segments = g_slist_remove (ddc->segments, ddc->segments->data);
	}

	if (ddc->currentcurve) ddc->currentcurve = sp_curve_unref (ddc->currentcurve);
	if (ddc->cal1) ddc->cal1 = sp_curve_unref (ddc->cal1);
	if (ddc->cal2) ddc->cal2 = sp_curve_unref (ddc->cal2);

	if (ddc->currentshape) {
		gtk_object_destroy (GTK_OBJECT (ddc->currentshape));
		ddc->currentshape = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
sp_dyna_draw_context_setup (SPEventContext *ec)
{
	SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT (ec);

	if (((SPEventContextClass *) parent_class)->setup)
		((SPEventContextClass *) parent_class)->setup (ec);

	ddc->accumulated = sp_curve_new_sized (32);
	ddc->currentcurve = sp_curve_new_sized (4);

	ddc->cal1 = sp_curve_new_sized (32);
	ddc->cal2 = sp_curve_new_sized (32);

	/* style should be changed when dc->use_calligraphc is touched */  
	ddc->currentshape = sp_canvas_item_new (SP_DT_SKETCH (ec->desktop), SP_TYPE_CANVAS_BPATH, NULL);
	sp_canvas_bpath_set_fill (SP_CANVAS_BPATH (ddc->currentshape), DDC_RED_RGBA, SP_WIND_RULE_EVENODD);
	sp_canvas_bpath_set_stroke (SP_CANVAS_BPATH (ddc->currentshape), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
	/* fixme: Cannot we cascade it to root more clearly? */
	g_signal_connect (G_OBJECT (ddc->currentshape), "event", G_CALLBACK (sp_desktop_root_handler), ec->desktop);

	sp_event_context_read (ec, "mass");
	sp_event_context_read (ec, "drag");
	sp_event_context_read (ec, "angle");
	sp_event_context_read (ec, "width");
}

static void
sp_dyna_draw_context_set (SPEventContext *ec, const gchar *key, const gchar *val)
{
	gdouble dval;

	SPDynaDrawContext *ddc = SP_DYNA_DRAW_CONTEXT (ec);

	if (!strcmp (key, "mass")) {
		dval = (val) ? atof (val) : 0.2;
		ddc->mass = CLAMP (dval, -1000.0, 1000.0);
	} else if (!strcmp (key, "drag")) {
		dval = (val) ? atof (val) : DRAG_DEFAULT;
		ddc->drag = CLAMP (dval, DRAG_MIN, DRAG_MAX);
	} else if (!strcmp (key, "angle")) {
		dval = (val) ? atof (val) : 0.5;
		dval = fmod (dval, 360.0);
		ddc->angle = (dval > 0) ? dval : dval + 360.0;
	} else if (!strcmp (key, "width")) {
		dval = (val) ? atof (val) : 0.1;
		ddc->width = CLAMP (dval, -1000.0, 1000.0);
	}

	//g_print ("DDC: %g %g %g %g\n", ddc->mass, ddc->drag, ddc->angle, ddc->width);
}

static double
flerp (double f0, double f1, double p)
{
  return f0 + (f1 - f0) * p;
}

/* Get normalized point */
static NR::Point
sp_dyna_draw_get_npoint (const SPDynaDrawContext *dc, NR::Point v)
{
	NRRect drect;

	sp_desktop_get_display_area (SP_EVENT_CONTEXT(dc)->desktop, &drect);
	return NR::Point((v[NR::X] - drect.x0)/(drect.x1 - drect.x0),
			 (v[NR::Y] - drect.y0)/(drect.y1 - drect.y0));
}

/* Get view point */
static NR::Point
sp_dyna_draw_get_vpoint (const SPDynaDrawContext *dc, NR::Point n)
{
	NRRect drect;

	sp_desktop_get_display_area (SP_EVENT_CONTEXT(dc)->desktop, &drect);
	return NR::Point(n[NR::X] * (drect.x1 - drect.x0) + drect.x0,
			 n[NR::Y] * (drect.y1 - drect.y0) + drect.y0);
}

/* Get current view point */
static NR::Point sp_dyna_draw_get_curr_vpoint (const SPDynaDrawContext * dc)
{
	NRRect drect;

	sp_desktop_get_display_area (SP_EVENT_CONTEXT(dc)->desktop, &drect);
	return NR::Point(dc->cur[NR::X] * (drect.x1 - drect.x0) + drect.x0,
			 dc->cur[NR::Y] * (drect.y1 - drect.y0) + drect.y0);
}

static void
sp_dyna_draw_reset (SPDynaDrawContext * dc, NR::Point p)
{
  dc->last = dc->cur = sp_dyna_draw_get_npoint (dc, p);
  dc->vel = NR::Point(0,0);
  dc->acc = NR::Point(0,0);
  dc->ang = NR::Point(0,0);
  dc->del = NR::Point(0,0);
}

static gboolean
sp_dyna_draw_apply (SPDynaDrawContext * dc, NR::Point p)
{
  double mass, drag;

  NR::Point n = sp_dyna_draw_get_npoint (dc, p);

  /* Calculate mass and drag */
  mass = flerp (1.0, 160.0, dc->mass);
  drag = flerp (0.0, 0.5, dc->drag * dc->drag);

  /* Calculate force and acceleration */
  NR::Point force = n - dc->cur;
  if (NR::L2(force) < DYNA_EPSILON)
    return FALSE;

  dc->acc = force / mass;

  /* Calculate new velocity */
  dc->vel += dc->acc;

  /* Calculate angle of drawing tool */
  if (dc->fixed_angle) {
	  // should this be -sin, cos?
	  dc->ang = NR::Point(cos (dc->angle * M_PI / 180.0),
			      -sin (dc->angle * M_PI / 180.0));
  } else {
	  gdouble mag_vel = NR::L2(dc->vel);
	  if (mag_vel < DYNA_EPSILON)
		  return FALSE;
	  dc->ang = NR::rot90(dc->vel) / mag_vel;
  }

  /* Apply drag */
  dc->vel *= 1.0 - drag;

  /* Update position */
  dc->last = dc->cur;
  dc->cur += dc->vel;

  return TRUE;
}

static void
sp_dyna_draw_brush (SPDynaDrawContext *dc)
{
	g_assert (dc->npoints >= 0 && dc->npoints < SAMPLING_SIZE);

	if (dc->use_calligraphic) {
		/* calligraphics */

		/* fixme: */
		double width = (0.05 - NR::L2(dc->vel)) * dc->width;
		if (width < DYNA_MIN_WIDTH)
			width = DYNA_MIN_WIDTH;
		NR::Point del = width * dc->ang;

		dc->point1[dc->npoints] = sp_dyna_draw_get_vpoint (dc, dc->cur + del);
		dc->point2[dc->npoints] = sp_dyna_draw_get_vpoint (dc, dc->cur - del);

		dc->del = del;
	} else {
		dc->point1[dc->npoints] = sp_dyna_draw_get_curr_vpoint (dc);
	}

	dc->npoints++;
}

static gint
sp_dyna_draw_timeout_handler (gpointer data)
{
	SPDynaDrawContext *dc = SP_DYNA_DRAW_CONTEXT (data);
	SPDesktop *desktop = SP_EVENT_CONTEXT(dc)->desktop;
	SPCanvas *canvas = SP_CANVAS (SP_DT_CANVAS (desktop));

	dc->dragging = TRUE;
	dc->dynahand = TRUE;
  
	int x, y;
	gtk_widget_get_pointer (GTK_WIDGET(canvas), &x, &y);
	NR::Point p = sp_canvas_window_to_world (canvas, NR::Point(x, y));
	p = sp_desktop_w2d_xy_point (desktop, p);
	if (! sp_dyna_draw_apply (dc, p)) {
		return TRUE;
	}
	p = sp_dyna_draw_get_curr_vpoint (dc);
	sp_desktop_free_snap (desktop, p);
	// something's not right here
	if ((dc->cur[NR::X] != dc->last[NR::X]) || (dc->cur[NR::Y] != dc->last[NR::Y])) {
		sp_dyna_draw_brush (dc);
		g_assert (dc->npoints > 0);
		fit_and_split (dc, FALSE);
	}

	return TRUE;
}

gint
sp_dyna_draw_context_root_handler (SPEventContext * event_context,
				   GdkEvent * event)
{
	SPDynaDrawContext *dc = SP_DYNA_DRAW_CONTEXT (event_context);
	SPDesktop *desktop = event_context->desktop;

	gint ret = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			NR::Point p = sp_desktop_w2d_xy_point (desktop,NR::Point(event->button.x, event->button.y));
			sp_dyna_draw_reset (dc, p);
			sp_dyna_draw_apply (dc, p);
			p = sp_dyna_draw_get_curr_vpoint (dc);
			sp_desktop_free_snap (desktop, p);
			sp_curve_reset (dc->accumulated);
			if (dc->repr) {
				dc->repr = NULL;
			}

			/* initialize first point */
			dc->npoints = 0;
         
			if (dc->use_timeout) {
				sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
							GDK_BUTTON_RELEASE_MASK |
							GDK_BUTTON_PRESS_MASK, NULL,
							event->button.time);
			} else {
				sp_canvas_item_grab (SP_CANVAS_ITEM (desktop->acetate),
							GDK_BUTTON_RELEASE_MASK |
							GDK_POINTER_MOTION_MASK |
							GDK_BUTTON_PRESS_MASK, NULL,
							event->button.time);
			}

			if (dc->use_timeout && !dc->timer_id) {
				dc->timer_id = gtk_timeout_add (SAMPLE_TIMEOUT, sp_dyna_draw_timeout_handler, dc);
			}
			ret = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (!dc->use_timeout && (event->motion.state & GDK_BUTTON1_MASK)) {
			dc->dragging = TRUE;
			dc->dynahand = TRUE;

			NR::Point p = sp_desktop_w2d_xy_point (desktop, NR::Point(event->motion.x, event->motion.y));

			if (! sp_dyna_draw_apply (dc, p)) {
				ret = TRUE;
				break;
			}
			p = sp_dyna_draw_get_curr_vpoint (dc);
			
			sp_desktop_free_snap (desktop, p);

			if ((dc->cur[NR::X] != dc->last[NR::X]) || (dc->cur[NR::Y] != dc->last[NR::Y])) {
				sp_dyna_draw_brush (dc);
				g_assert (dc->npoints > 0);
				fit_and_split (dc, FALSE);
			}
			ret = TRUE;
		}
		break;

	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1 &&
		    dc->use_timeout && dc->timer_id != 0) {
			gtk_timeout_remove (dc->timer_id);
			dc->timer_id = 0;
		}
		if (dc->dragging && event->button.button == 1) {
			dc->dragging = FALSE;

			/* release */
			if (dc->dynahand) {
				dc->dynahand = FALSE;
				/* Remove all temporary line segments */
				while (dc->segments) {
					gtk_object_destroy (GTK_OBJECT (dc->segments->data));
					dc->segments = g_slist_remove (dc->segments, dc->segments->data);
				}
				/* Create object */
				fit_and_split (dc, TRUE);
				if (dc->use_calligraphic) {
					accumulate_calligraphic (dc);
				} else {
					concat_current_line (dc);
				}
				set_to_accumulated (dc); /* temporal implementation */
				if (dc->use_calligraphic /* || dc->cinside*/) {
					/* reset accumulated curve */
					sp_curve_reset (dc->accumulated);
					clear_current (dc);
					if (dc->repr) {
						dc->repr = NULL;
					}
				}
				
			}

			sp_canvas_item_ungrab (SP_CANVAS_ITEM (desktop->acetate), event->button.time);
			ret = TRUE;
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
		default:
			break;
		}
	default:
		break;
	}

	if (!ret) {
		if (((SPEventContextClass *) parent_class)->root_handler)
			ret = ((SPEventContextClass *) parent_class)-> root_handler (event_context, event);
	}

	return ret;
}

static void
clear_current (SPDynaDrawContext * dc)
{
	/* reset bpath */
	sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (dc->currentshape), NULL);
	/* reset curve */
	sp_curve_reset (dc->currentcurve);
	sp_curve_reset (dc->cal1);
	sp_curve_reset (dc->cal2);
	/* reset points */
	dc->npoints = 0;
}

static void
set_to_accumulated (SPDynaDrawContext * dc)
{
  SPDesktop *desktop = SP_EVENT_CONTEXT (dc)->desktop;

  if (!sp_curve_empty (dc->accumulated))
    {
      ArtBpath *abp;
      gdouble d2doc[6];
      gchar *str;

      if (!dc->repr)
	{
	  SPRepr *repr, *style;
	  SPCSSAttr *css;
	  /* Create object */
	  repr = sp_repr_new ("path");
	  /* Set style */

          if (dc->use_calligraphic)
            style = inkscape_get_repr (INKSCAPE, "tools.calligraphic");
          else
            style = inkscape_get_repr (INKSCAPE, "tools.freehand");

	  if (style)
	    {
	      css = sp_repr_css_attr_inherited (style, "style");
	      sp_repr_css_set (repr, css, "style");
	      sp_repr_css_attr_unref (css);
	    }
	  dc->repr = repr;

	  sp_document_add_repr (SP_DT_DOCUMENT (desktop), dc->repr);
	  sp_repr_unref (dc->repr);
	  sp_selection_set_repr (SP_DT_SELECTION (desktop), dc->repr);
	}
      sp_desktop_dt2root_affine (desktop, (NRMatrix *) d2doc);
      abp = art_bpath_affine_transform (sp_curve_first_bpath (dc->accumulated), d2doc);
      str = sp_svg_write_path (abp);
      g_assert (str != NULL);
      art_free (abp);
      sp_repr_set_attr (dc->repr, "d", str);
      g_free (str);
    }
  else
    {
      if (dc->repr)
	sp_repr_unparent (dc->repr);
      dc->repr = NULL;
    }

  sp_document_done (SP_DT_DOCUMENT (desktop));
}

static void
concat_current_line (SPDynaDrawContext * dc)
{
  if (!sp_curve_empty (dc->currentcurve))
    {
      ArtBpath *bpath;
      if (sp_curve_empty (dc->accumulated))
	{
	  bpath = sp_curve_first_bpath (dc->currentcurve);
	  g_assert (bpath->code == ART_MOVETO_OPEN);
	  sp_curve_moveto (dc->accumulated, bpath->x3, bpath->y3);
	}
      bpath = sp_curve_last_bpath (dc->currentcurve);
      if (bpath->code == ART_CURVETO)
	{
	  sp_curve_curveto (dc->accumulated, bpath->x1, bpath->y1, bpath->x2,
			    bpath->y2, bpath->x3, bpath->y3);
	}
      else if (bpath->code == ART_LINETO)
	{
	  sp_curve_lineto (dc->accumulated, bpath->x3, bpath->y3);
	}
      else
	{
	  g_assert_not_reached ();
	}
    }
}

static void
accumulate_calligraphic (SPDynaDrawContext * dc)
{
  if (!sp_curve_empty (dc->cal1) && !sp_curve_empty(dc->cal2))
    {
      sp_curve_reset (dc->accumulated); /*  Is this required ?? */
      SPCurve *rev_cal2 = sp_curve_reverse (dc->cal2);
      sp_curve_append (dc->accumulated, dc->cal1, FALSE);
      sp_curve_append (dc->accumulated, rev_cal2, TRUE);
      sp_curve_closepath(dc->accumulated);

      sp_curve_unref (rev_cal2);

      sp_curve_reset (dc->cal1);
      sp_curve_reset (dc->cal2);
    }
}

static void
fit_and_split (SPDynaDrawContext *dc,
               gboolean           release)
{
	if (dc->use_calligraphic) {
		fit_and_split_calligraphics (dc, release);
	} else {
		fit_and_split_line (dc, release);
	}
}

static void
fit_and_split_line (SPDynaDrawContext *dc,
                    gboolean           release)
{
	NR::Point b[4];
	gdouble tolerance;

	tolerance = SP_EVENT_CONTEXT (dc)->desktop->w2d[0] * TOLERANCE_LINE;
	tolerance = tolerance * tolerance;

	if (sp_bezier_fit_cubic (b, dc->point1, dc->npoints, tolerance) > 0
	    && dc->npoints < SAMPLING_SIZE)
	{
		/* Fit and draw and reset state */
#ifdef DYNA_DRAW_VERBOSE
		g_print ("%d", dc->npoints);
#endif
		sp_curve_reset (dc->currentcurve);
		sp_curve_moveto (dc->currentcurve, b[0]);
		sp_curve_curveto (dc->currentcurve, b[1], b[2], b[3]);
		sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (dc->currentshape), dc->currentcurve);
	}
	else
	{
		SPCurve *curve;
		SPCanvasItem *cbp;
		/* Fit and draw and copy last point */
#ifdef DYNA_DRAW_VERBOSE
		g_print ("[%d]Yup\n", dc->npoints);
#endif
		g_assert (!sp_curve_empty (dc->currentcurve));
		concat_current_line (dc);

		cbp = sp_canvas_item_new (SP_DT_SKETCH (SP_EVENT_CONTEXT (dc)->desktop), SP_TYPE_CANVAS_BPATH, NULL);
		curve = sp_curve_copy (dc->currentcurve);
		sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (cbp), curve);
		sp_curve_unref (curve);
		/* fixme: We have to parse style color somehow */
		sp_canvas_bpath_set_fill (SP_CANVAS_BPATH (cbp), DDC_GREEN_RGBA, SP_WIND_RULE_EVENODD);
		sp_canvas_bpath_set_stroke (SP_CANVAS_BPATH (cbp), 0x000000ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
		/* fixme: Cannot we cascade it to root more clearly? */
		g_signal_connect (G_OBJECT (cbp), "event", G_CALLBACK (sp_desktop_root_handler), SP_EVENT_CONTEXT (dc)->desktop);

		dc->segments = g_slist_prepend (dc->segments, cbp);
		dc->point1[0] = dc->point1[dc->npoints - 2];
		dc->npoints = 1;
	}
}

static void
fit_and_split_calligraphics (SPDynaDrawContext *dc, gboolean release)
{
	gdouble tolerance = SP_EVENT_CONTEXT (dc)->desktop->w2d[0] * TOLERANCE_CALLIGRAPHIC;
	tolerance = tolerance * tolerance;

#ifdef DYNA_DRAW_VERBOSE
	g_print ("[F&S:R=%c]", release?'T':'F');
#endif

	g_assert (dc->npoints > 0 && dc->npoints < SAMPLING_SIZE);

	if (dc->npoints == SAMPLING_SIZE-1 || release) {
#define BEZIER_SIZE       4
#define BEZIER_MAX_DEPTH  4
#define BEZIER_MAX_LENGTH (BEZIER_SIZE << (BEZIER_MAX_DEPTH-1))
		SPCurve *curve;
		NR::Point b1[BEZIER_MAX_LENGTH], b2[BEZIER_MAX_LENGTH];
		gint nb1, nb2;            /* number of blocks */

#ifdef DYNA_DRAW_VERBOSE
		g_print ("[F&S:#] dc->npoints:%d, release:%s\n",
			 dc->npoints, release ? "TRUE" : "FALSE");
#endif

		/* Current calligraphic */
		if (dc->cal1->end==0 || dc->cal2->end==0) {
			/* dc->npoints > 0 */
			/* g_print ("calligraphics(1|2) reset\n"); */
			sp_curve_reset (dc->cal1);
			sp_curve_reset (dc->cal2);
          
			sp_curve_moveto (dc->cal1, dc->point1[0]);
			sp_curve_moveto (dc->cal2, dc->point2[0]);
		}

		nb1 = sp_bezier_fit_cubic_r (b1, dc->point1, dc->npoints,
					     tolerance, BEZIER_MAX_DEPTH);
		g_assert (nb1 * BEZIER_SIZE <= gint(G_N_ELEMENTS(b1)));
		nb2 = sp_bezier_fit_cubic_r (b2, dc->point2, dc->npoints,
					     tolerance, BEZIER_MAX_DEPTH);
		g_assert (nb2 * BEZIER_SIZE <= gint(G_N_ELEMENTS(b2)));
		if (nb1 != -1 && nb2 != -1) {
			NR::Point *bp1, *bp2;
			/* Fit and draw and reset state */
#ifdef DYNA_DRAW_VERBOSE
			g_print ("nb1:%d nb2:%d\n", nb1, nb2);
#endif
			/* CanvasShape */
			if (! release) {
				sp_curve_reset (dc->currentcurve);
				sp_curve_moveto (dc->currentcurve, b1[0]);
				for (bp1 = b1; bp1 < b1 + BEZIER_SIZE*nb1; bp1 += BEZIER_SIZE) {
					sp_curve_curveto (dc->currentcurve, bp1[1],
							  bp1[2], bp1[3]);
				}
				sp_curve_lineto (dc->currentcurve,
						 b2[BEZIER_SIZE*(nb2-1) + 3]);
				for (bp2 = b2 + BEZIER_SIZE*(nb2-1); bp2 >= b2; bp2 -= BEZIER_SIZE) {
					sp_curve_curveto (dc->currentcurve, bp2[2], bp2[1], bp2[0]);
				}
				sp_curve_closepath (dc->currentcurve);
				sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (dc->currentshape), dc->currentcurve);
			}
          
			/* Current calligraphic */
			for (bp1 = b1; bp1 < b1 + BEZIER_SIZE*nb1; bp1 += BEZIER_SIZE) {
				sp_curve_curveto (dc->cal1, bp1[1], bp1[2], bp1[3]);
			}
			for (bp2 = b2; bp2 < b2 + BEZIER_SIZE*nb2; bp2 += BEZIER_SIZE) {
				sp_curve_curveto (dc->cal2, bp2[1], bp2[2], bp2[3]);
			}
		} else {
			gint  i;
			/* fixme: ??? */
#ifdef DYNA_DRAW_VERBOSE
			g_print ("[fit_and_split_calligraphics] failed to fit-cubic.\n");
#endif
			draw_temporary_box (dc);

			for (i = 1; i < dc->npoints; i++)
				sp_curve_lineto (dc->cal1, dc->point1[i]);
			for (i = 1; i < dc->npoints; i++)
				sp_curve_lineto (dc->cal2, dc->point2[i]);
		}

		/* Fit and draw and copy last point */
#ifdef DYNA_DRAW_VERBOSE
		g_print ("[%d]Yup\n", dc->npoints);
#endif
		if (!release) {
			g_assert (!sp_curve_empty (dc->currentcurve));

			SPCanvasItem *cbp = sp_canvas_item_new (SP_DT_SKETCH (SP_EVENT_CONTEXT (dc)->desktop), SP_TYPE_CANVAS_BPATH, NULL);
			curve = sp_curve_copy (dc->currentcurve);
			sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (cbp), curve);
			sp_curve_unref (curve);
			sp_canvas_bpath_set_fill (SP_CANVAS_BPATH (cbp), 0x000000ff, SP_WIND_RULE_EVENODD);
			sp_canvas_bpath_set_stroke (SP_CANVAS_BPATH (cbp), 0x00000000, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
			/* fixme: Cannot we cascade it to root more clearly? */
			g_signal_connect (G_OBJECT (cbp), "event", G_CALLBACK (sp_desktop_root_handler), SP_EVENT_CONTEXT (dc)->desktop);

			dc->segments = g_slist_prepend (dc->segments, cbp);
		}

		dc->point1[0] = dc->point1[dc->npoints - 1];
		dc->point2[0] = dc->point2[dc->npoints - 1];
		dc->npoints = 1;
	} else {
		draw_temporary_box (dc);
	}
}

static void
draw_temporary_box (SPDynaDrawContext *dc)
{
	sp_curve_reset (dc->currentcurve);
	sp_curve_moveto (dc->currentcurve, dc->point1[0]);
	for (gint i = 1; i < dc->npoints; i++) {
		sp_curve_lineto (dc->currentcurve, dc->point1[i]);
	}
	for (gint i = dc->npoints-1; i >= 0; i--) {
		sp_curve_lineto (dc->currentcurve, dc->point2[i]);
	}
	sp_curve_closepath (dc->currentcurve);
	sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (dc->currentshape), dc->currentcurve);
}

/* Gtk stuff */

static void
sp_ddc_mass_value_changed (GtkAdjustment *adj, SPDynaDrawContext *ddc)
{
	sp_repr_set_double (SP_EVENT_CONTEXT_REPR (ddc), "mass", adj->value);
}

static void
sp_ddc_drag_value_changed (GtkAdjustment *adj, SPDynaDrawContext *ddc)
{
	sp_repr_set_double (SP_EVENT_CONTEXT_REPR (ddc), "drag", adj->value);
}

static void
sp_ddc_angle_value_changed (GtkAdjustment *adj, SPDynaDrawContext *ddc)
{
	sp_repr_set_double (SP_EVENT_CONTEXT_REPR (ddc), "angle", adj->value);
}

static void
sp_ddc_width_value_changed (GtkAdjustment *adj, SPDynaDrawContext *ddc)
{
	sp_repr_set_double (SP_EVENT_CONTEXT_REPR (ddc), "width", adj->value);
}

static void
sp_ddc_defaults (GtkWidget *widget, GtkObject *obj)
{
	GtkAdjustment *adj;

	adj = (GtkAdjustment*)gtk_object_get_data (obj, "mass");
	gtk_adjustment_set_value (adj, 0.3);
	adj = (GtkAdjustment*)gtk_object_get_data (obj, "drag");
	gtk_adjustment_set_value (adj, DRAG_DEFAULT);
	adj = (GtkAdjustment*)gtk_object_get_data (obj, "angle");
	gtk_adjustment_set_value (adj, 30.0);
	adj = (GtkAdjustment*)gtk_object_get_data (obj, "width");
	gtk_adjustment_set_value (adj, 0.2);
}

static GtkWidget *
sp_dyna_draw_context_config_widget (SPEventContext *ec)
{
	SPDynaDrawContext *ddc;
	GtkWidget *tbl, *l, *sb, *b;
	GtkObject *a;

	ddc = SP_DYNA_DRAW_CONTEXT (ec);

	tbl = gtk_table_new (5, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (tbl), 4);
	gtk_table_set_row_spacings (GTK_TABLE (tbl), 4);

	/* Mass */
	l = gtk_label_new (_("Mass:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 0, 1, 
			  (GtkAttachOptions)0, 
			  (GtkAttachOptions)0, 
			  0, 0);
	a = gtk_adjustment_new (ddc->mass, 0.0, 1.0, 0.01, 0.1, 0.1);
	gtk_object_set_data (GTK_OBJECT (tbl), "mass", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 0, 1, 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)0,
			  0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_ddc_mass_value_changed), ddc);

	/* Drag */
	l = gtk_label_new (_("Drag:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 1, 2, 
			  (GtkAttachOptions)0, 
			  (GtkAttachOptions)0, 
			  0, 0);
	a = gtk_adjustment_new (ddc->drag, DRAG_MIN, DRAG_MAX, 0.01, 0.1, 0.1);
	gtk_object_set_data (GTK_OBJECT (tbl), "drag", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 1, 2, 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)0, 
			  0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_ddc_drag_value_changed), ddc);

	/* Angle */
	l = gtk_label_new (_("Angle:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 2, 3,
			  (GtkAttachOptions)0, 
			  (GtkAttachOptions)0, 
			  0, 0);
	a = gtk_adjustment_new (ddc->angle, 0.0, 360.0, 1.0, 10.0, 10.0);
	gtk_object_set_data (GTK_OBJECT (tbl), "angle", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 2, 3, 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)0, 
			  0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_ddc_angle_value_changed), ddc);

	/* Width */
	l = gtk_label_new (_("Width:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), l, 0, 1, 3, 4, 
			  (GtkAttachOptions)0, 
			  (GtkAttachOptions)0, 
			  0, 0);
	a = gtk_adjustment_new (ddc->width, 0.01, 1.0, 0.01, 0.1, 0.1);
	gtk_object_set_data (GTK_OBJECT (tbl), "width", a);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
	gtk_widget_show (sb);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 3, 4, 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  (GtkAttachOptions)0, 
			  0, 0);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_ddc_width_value_changed), ddc);

	/* Reset */
	b = gtk_button_new_with_label (_("Defaults"));
	gtk_widget_show (b);
	gtk_table_attach (GTK_TABLE (tbl), b, 0, 2, 4, 5, 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			  0, 0);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_ddc_defaults), tbl);

	return tbl;
}

