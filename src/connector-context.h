#ifndef SEEN_CONNECTOR_CONTEXT_H
#define SEEN_CONNECTOR_CONTEXT_H

/*
 * Connector creation tool
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "draw-context.h"
#include <libnr/nr-matrix-fns.h>
#include <sigc++/connection.h>


#define SP_TYPE_CONNECTOR_CONTEXT (sp_connector_context_get_type())
#define SP_CONNECTOR_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_CONNECTOR_CONTEXT, SPConnectorContext))
#define SP_CONNECTOR_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_CONNECTOR_CONTEXT, SPConnectorContextClass))
#define SP_IS_CONNECTOR_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_CONNECTOR_CONTEXT))
#define SP_IS_CONNECTOR_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_CONNECTOR_CONTEXT))

struct SPKnot;

enum {
    SP_CONNECTOR_CONTEXT_IDLE,
    SP_CONNECTOR_CONTEXT_DRAGGING,
    SP_CONNECTOR_CONTEXT_CLOSE,
    SP_CONNECTOR_CONTEXT_STOP,
    SP_CONNECTOR_CONTEXT_DRAWING,
    SP_CONNECTOR_CONTEXT_REROUTING
};


// TODO: I've had to replace much of the functionality from the DrawContext
//       that I originally thought I'd be able to use.  I'll eventually rework
//       SPConnectorContext to just extend SPEventContext. (mjwybrow)
//
struct SPConnectorContext : public SPDrawContext {
    NR::Point p[5];

    /** \invar npoints in {0, 2}. */
    gint npoints;

    unsigned int mode : 1;
    unsigned int state : 4;
    unsigned int onlycurves : 1;

    // The new connector
    SPItem *newconn;
    
    // Called when the attached object gets deleted.
    sigc::connection _delete_connection;

    // The active shape
    SPItem *active_shape;
    Inkscape::XML::Node *active_shape_repr;
    Inkscape::XML::Node *active_shape_layer_repr;

    // Same as above, but for the active connector
    SPItem *active_conn;
    Inkscape::XML::Node *active_conn_repr;
    Inkscape::XML::Node *active_conn_layer_repr;
    
    // The activehandle
    SPKnot *active_handle;

    SPItem *clickeditem;
    SPKnot *clickedhandle;
    
    SPKnot *connpthandle;
    SPKnot *endpt_handle[2];
    guint  endpt_handler_id[2];
    gchar *sid;
    gchar *eid;
    SPCanvasItem *c0, *c1, *cl0, *cl1;
};

struct SPConnectorContextClass : public SPEventContextClass { };

GType sp_connector_context_get_type();


#endif /* !SEEN_CONNECTOR_CONTEXT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
