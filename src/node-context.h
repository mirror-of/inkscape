#ifndef __SP_NODE_CONTEXT_H__
#define __SP_NODE_CONTEXT_H__

/*
 * Node editing context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "event-context.h"
#include "nodepath.h"
#include "knotholder.h"

#define SP_TYPE_NODE_CONTEXT            (sp_node_context_get_type ())
#define SP_NODE_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_NODE_CONTEXT, SPNodeContext))
#define SP_NODE_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_NODE_CONTEXT, SPNodeContextClass))
#define SP_IS_NODE_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_NODE_CONTEXT))
#define SP_IS_NODE_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_NODE_CONTEXT))

typedef struct _SPNodeContext SPNodeContext;
typedef struct _SPNodeContextClass SPNodeContextClass;

struct _SPNodeContext {
	SPEventContext event_context;

	guint drag : 1;

	SPNodePath *nodepath;
	SPKnotHolder *knot_holder;
};

struct _SPNodeContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_node_context_get_type (void);

#endif
