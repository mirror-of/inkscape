#ifndef __SP_DYNA_DRAW_CONTEXT_H__
#define __SP_DYNA_DRAW_CONTEXT_H__

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

#include "helper/curve.h"
#include "event-context.h"

#define SP_TYPE_DYNA_DRAW_CONTEXT (sp_dyna_draw_context_get_type ())
#define SP_DYNA_DRAW_CONTEXT(o) (GTK_CHECK_CAST ((o), SP_TYPE_DYNA_DRAW_CONTEXT, SPDynaDrawContext))
#define SP_DYNA_DRAW_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_DYNA_DRAW_CONTEXT, SPDynaDrawContextClass))
#define SP_IS_DYNA_DRAW_CONTEXT(o) (GTK_CHECK_TYPE ((o), SP_TYPE_DYNA_DRAW_CONTEXT))
#define SP_IS_DYNA_DRAW_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_DYNA_DRAW_CONTEXT))

typedef struct _SPDynaDrawContext SPDynaDrawContext;
typedef struct _SPDynaDrawContextClass SPDynaDrawContextClass;

#define SAMPLING_SIZE 16        /* fixme: ?? */

struct _SPDynaDrawContext
{
	SPEventContext event_context;

	SPCurve *accumulated;
	GSList *segments;
	/* current shape and curves */
	SPCanvasItem *currentshape;
	SPCurve *currentcurve;
	SPCurve *cal1;
	SPCurve *cal2;
	/* temporary work area */
	NRPointF point1[SAMPLING_SIZE];
	NRPointF point2[SAMPLING_SIZE];
	gint npoints;

	/* repr */
	SPRepr *repr;

	/* time_id if use timeout */
	gint timer_id;

	/* DynaDraw */
	double curx, cury;
	double velx, vely, vel;
	double accx, accy, acc;
	double angx, angy;
	double lastx, lasty;
	double delx, dely;
	/* attributes */
	/* fixme: shuld be merge dragging and dynahand ?? */
	guint dragging : 1;           /* mouse state: mouse is dragging */
	guint dynahand : 1;           /* mouse state: mouse is in draw */
	guint use_timeout : 1;
	guint use_calligraphic : 1;
	guint fixed_angle : 1;
	double mass, drag;
	double angle;
	double width;
};

struct _SPDynaDrawContextClass
{
	SPEventContextClass parent_class;
};

GtkType sp_dyna_draw_context_get_type (void);

#endif
