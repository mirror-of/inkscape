#ifndef __SP_RECT_CONTEXT_H__
#define __SP_RECT_CONTEXT_H__

/*
 * Rectangle drawing context
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

#include "event-context.h"

#define SP_TYPE_RECT_CONTEXT            (sp_rect_context_get_type ())
#define SP_RECT_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_RECT_CONTEXT, SPRectContext))
#define SP_RECT_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_RECT_CONTEXT, SPRectContextClass))
#define SP_IS_RECT_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_RECT_CONTEXT))
#define SP_IS_RECT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_RECT_CONTEXT))

typedef struct _SPRectContext SPRectContext;
typedef struct _SPRectContextClass SPRectContextClass;

struct _SPRectContext {
	SPEventContext event_context;
	SPItem *item;
	NRPointF center;
	
  	gdouble rx_ratio;	/* roundness ratio (x direction) */
  	gdouble ry_ratio;	/* roundness ratio (y direction) */
};

struct _SPRectContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_rect_context_get_type (void);

#endif
