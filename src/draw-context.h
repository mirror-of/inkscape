#ifndef __SP_DRAW_CONTEXT_H__
#define __SP_DRAW_CONTEXT_H__

/*
 * Generic drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include "helper/curve.h"
#include "event-context.h"

/* Freehand context */

#define SP_TYPE_DRAW_CONTEXT (sp_draw_context_get_type ())
#define SP_DRAW_CONTEXT(o) (GTK_CHECK_CAST ((o), SP_TYPE_DRAW_CONTEXT, SPDrawContext))
#define SP_DRAW_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_DRAW_CONTEXT, SPDrawContextClass))
#define SP_IS_DRAW_CONTEXT(o) (GTK_CHECK_TYPE ((o), SP_TYPE_DRAW_CONTEXT))
#define SP_IS_DRAW_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_DRAW_CONTEXT))

typedef struct _SPDrawContext SPDrawContext;
typedef struct _SPDrawContextClass SPDrawContextClass;
typedef struct _SPDrawAnchor SPDrawAnchor;

#define SP_DRAW_POINTS_MAX 16

struct _SPDrawContext {
	SPEventContext event_context;

	SPSelection *selection;
	SPCanvasItem *grab;

	guint attach : 1;

	guint32 red_color;
	guint32 blue_color;
	guint32 green_color;

	/* Red */
	SPCanvasItem *red_bpath;
	SPCurve *red_curve;

	/* Blue */
	SPCanvasItem *blue_bpath;
	SPCurve *blue_curve;

	/* Green */
	GSList *green_bpaths;
	SPCurve *green_curve;
	SPDrawAnchor *green_anchor;

	/* White */
	SPItem *white_item;
	GSList *white_curves;
	GSList *white_anchors;

	/* Start anchor */
	SPDrawAnchor *sa;
	/* End anchor */
	SPDrawAnchor *ea;

	NRPointF p[SP_DRAW_POINTS_MAX];
	gint npoints;
};

struct _SPDrawContextClass {
	SPEventContextClass parent_class;
};

GtkType sp_draw_context_get_type (void);

/* Pencil context */

#define SP_TYPE_PENCIL_CONTEXT (sp_pencil_context_get_type ())
#define SP_PENCIL_CONTEXT(o) (GTK_CHECK_CAST ((o), SP_TYPE_PENCIL_CONTEXT, SPPencilContext))
#define SP_PENCIL_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_PENCIL_CONTEXT, SPPencilContextClass))
#define SP_IS_PENCIL_CONTEXT(o) (GTK_CHECK_TYPE ((o), SP_TYPE_PENCIL_CONTEXT))
#define SP_IS_PENCIL_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_PENCIL_CONTEXT))

typedef struct _SPPencilContext SPPencilContext;
typedef struct _SPPencilContextClass SPPencilContextClass;

enum {
	SP_PENCIL_CONTEXT_IDLE,
	SP_PENCIL_CONTEXT_ADDLINE,
	SP_PENCIL_CONTEXT_FREEHAND
};

struct _SPPencilContext {
	SPDrawContext draw_context;

	guint state : 2;
};

struct _SPPencilContextClass {
	SPEventContextClass parent_class;
};

GtkType sp_pencil_context_get_type (void);

/* Pen context */

#define SP_TYPE_PEN_CONTEXT (sp_pen_context_get_type ())
#define SP_PEN_CONTEXT(o) (GTK_CHECK_CAST ((o), SP_TYPE_PEN_CONTEXT, SPPenContext))
#define SP_PEN_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_PEN_CONTEXT, SPPenContextClass))
#define SP_IS_PEN_CONTEXT(o) (GTK_CHECK_TYPE ((o), SP_TYPE_PEN_CONTEXT))
#define SP_IS_PEN_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_PEN_CONTEXT))

typedef struct _SPPenContext SPPenContext;
typedef struct _SPPenContextClass SPPenContextClass;

enum {
	SP_PEN_CONTEXT_POINT,
	SP_PEN_CONTEXT_CONTROL,
	SP_PEN_CONTEXT_CLOSE,
	SP_PEN_CONTEXT_STOP
};

enum {
	SP_PEN_CONTEXT_MODE_CLICK,
	SP_PEN_CONTEXT_MODE_DRAG
};

struct _SPPenContext {
	SPDrawContext draw_context;

	unsigned int mode : 1;
	unsigned int state : 2;
	unsigned int onlycurves : 1;

	SPCanvasItem *c0, *c1, *cl0, *cl1;
};

struct _SPPenContextClass {
	SPEventContextClass parent_class;
};

GtkType sp_pen_context_get_type (void);

#endif
