#ifndef __SP_DROPPER_CONTEXT_H__
#define __SP_DROPPER_CONTEXT_H__

/*
 * Tool for picking colors from drawing
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-types.h>

#include "helper/helper-forward.h"
#include "event-context.h"

#define SP_TYPE_DROPPER_CONTEXT (sp_dropper_context_get_type ())
#define SP_DROPPER_CONTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_DROPPER_CONTEXT, SPDropperContext))
#define SP_IS_DROPPER_CONTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_DROPPER_CONTEXT))

typedef struct _SPDropperContext SPDropperContext;
typedef struct _SPDropperContextClass SPDropperContextClass;

struct _SPDropperContext {
	SPEventContext event_context;

	unsigned int dragging : 1;

	SPCanvasItem *area;
	NRPointF centre;
};

struct _SPDropperContextClass {
	SPEventContextClass parent_class;
};

GType sp_dropper_context_get_type (void);

#endif
