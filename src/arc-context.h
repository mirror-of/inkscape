#ifndef __SP_ARC_CONTEXT_H__
#define __SP_ARC_CONTEXT_H__

/*
 * Ellipse drawing context
 *
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Mitsuru Oka
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"
#include "knotholder.h"

#define SP_TYPE_ARC_CONTEXT            (sp_arc_context_get_type ())
#define SP_ARC_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_ARC_CONTEXT, SPArcContext))
#define SP_ARC_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_ARC_CONTEXT, SPArcContextClass))
#define SP_IS_ARC_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_ARC_CONTEXT))
#define SP_IS_ARC_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_ARC_CONTEXT))

class SPArcContext;
class SPArcContextClass;

struct SPArcContext {
	SPEventContext event_context;
	SPItem * item;
	NR::Point center;

	SigC::Connection sel_changed_connection;

    SPKnotHolder *knot_holder;
    SPRepr *repr;

};

struct SPArcContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_arc_context_get_type (void);

#endif
