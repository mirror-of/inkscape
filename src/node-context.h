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

#include <sigc++/sigc++.h>
#include "event-context.h"
#include "forward.h"
#include "nodepath.h"
#include "knotholder.h"

class SPSelection;

#define SP_TYPE_NODE_CONTEXT            (sp_node_context_get_type ())
#define SP_NODE_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_NODE_CONTEXT, SPNodeContext))
#define SP_NODE_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_NODE_CONTEXT, SPNodeContextClass))
#define SP_IS_NODE_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_NODE_CONTEXT))
#define SP_IS_NODE_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_NODE_CONTEXT))

class SPNodeContext;
class SPNodeContextClass;

struct SPNodeContext {
	SPEventContext event_context;

	guint drag : 1;

	Path::Path *nodepath;
	SPKnotHolder *knot_holder;

	gboolean leftalt;
	gboolean rightalt;
	gboolean leftctrl;
	gboolean rightctrl;

	sigc::connection sel_changed_connection;

	Inkscape::MessageContext *_node_message_context;
};

struct SPNodeContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_node_context_get_type (void);

void sp_node_context_selection_changed (SPSelection * selection, gpointer data);

#endif
