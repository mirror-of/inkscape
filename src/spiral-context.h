#ifndef __SP_SPIRAL_CONTEXT_H__
#define __SP_SPIRAL_CONTEXT_H__

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

#include "knot.h"
#include "event-context.h"

#define SP_TYPE_SPIRAL_CONTEXT            (sp_spiral_context_get_type ())
#define SP_SPIRAL_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_SPIRAL_CONTEXT, SPSpiralContext))
#define SP_SPIRAL_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_SPIRAL_CONTEXT, SPSpiralContextClass))
#define SP_IS_SPIRAL_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_SPIRAL_CONTEXT))
#define SP_IS_SPIRAL_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_SPIRAL_CONTEXT))

typedef struct _SPSpiralContext SPSpiralContext;
typedef struct _SPSpiralContextClass SPSpiralContextClass;

struct _SPSpiralContext {
	SPEventContext event_context;
	SPItem * item;
	NRPointF center;
	gdouble revo;
	gdouble exp;
	gdouble t0;
};

struct _SPSpiralContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_spiral_context_get_type (void);

#endif
