#define __SP_VIEW_C__

/*
 * Abstract base class for all SVG document views
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <time.h>
#include <gtk/gtk.h>
#include "macros.h"
#include "helper/action.h"
#include "helper/sp-marshal.h"
#include "document.h"
#include "view.h"
#include "inkscape.h"
#include "message-stack.h"
#include "message-context.h"

enum {
    SHUTDOWN,
    URI_SET,
    RESIZED,
    POSITION_SET,
    LAST_SIGNAL
};

static void sp_view_class_init(SPViewClass *vc);

static void sp_view_document_uri_set(SPDocument *doc, const guchar *uri, SPView *view);
static void sp_view_document_resized(SPDocument *doc, gdouble width, gdouble height, SPView *view);

static GObjectClass *parent_class;
static guint signals[LAST_SIGNAL] = { 0 };

static GValue shutdown_accumulator_data;

GtkType sp_view_get_type(void)
{
    static GType type = 0;
    
    if (!type) {
        GTypeInfo info = {
            sizeof(SPViewClass),
            NULL, NULL,
            (GClassInitFunc) sp_view_class_init,
            NULL, NULL,
            sizeof(SPView),
            4,
            (GInstanceInitFunc) &SPView::init,
            NULL
        };
        type = g_type_register_static(G_TYPE_OBJECT, "SPView", &info, (GTypeFlags) 0);
    }
    
    return type;
}

/**
 * This signal accumulator is a special callback function for collecting the
 * return values of the various callbacks that are called during the SHUTDOWN 
 * signal emission.  This allows us to see if any of the views being shutdown
 * return a 'cancel' value that needs to be honored.  Without this accumulator
 * only the return value of the last view would be checked.
 *
 * ihint:           Signal invocation hint
 * return_accu:     Accumulator to collect callback return values in, this is 
 *                  the return value of the current signal emission. 
 * handler_return:  Return value of the signal handler this function will check.
 * data:            
 *
 * Returns:         The accumulator function returns whether the signal emission 
 *                  should be aborted. Returning FALSE means to abort the current 
 *                  emission and TRUE is returned for continuation. 
 */
static gboolean
sp_shutdown_accumulator(GSignalInvocationHint *ihint, GValue *return_accu,
                        const GValue *handler_return, gpointer data)
{
    if (G_VALUE_HOLDS_BOOLEAN(handler_return)) {
	/* If the signal returned TRUE, it means the user cancelled the operation 
	   so we return FALSE, indicating we should abort signal emission. 
	*/
	if (g_value_get_boolean(handler_return)) {
	    g_value_set_boolean(return_accu, TRUE);
	}
	return ! g_value_get_boolean(handler_return);
    }
    
    return TRUE;   /* Signal emission can continue */
}

static void sp_view_class_init(SPViewClass *vc)
{
    GObjectClass *object_class = G_OBJECT_CLASS(vc);

    parent_class = (GObjectClass*) g_type_class_peek_parent(vc);

    g_value_init(&shutdown_accumulator_data, G_TYPE_BOOLEAN);
    g_value_set_boolean(&shutdown_accumulator_data, FALSE);

    signals[SHUTDOWN] =     g_signal_new("shutdown",
                                         G_TYPE_FROM_CLASS(vc),
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET(SPViewClass, shutdown),
                                         sp_shutdown_accumulator, 
                                         &shutdown_accumulator_data,
                                         sp_marshal_BOOLEAN__NONE,
                                         G_TYPE_BOOLEAN, 0);
    signals[URI_SET] =      g_signal_new("uri_set",
                                         G_TYPE_FROM_CLASS(vc),
                                         G_SIGNAL_RUN_FIRST,
                                         G_STRUCT_OFFSET(SPViewClass, uri_set),
                                         NULL, NULL,
                                         sp_marshal_NONE__POINTER,
                                         G_TYPE_NONE, 1,
                                         G_TYPE_POINTER);
    signals[RESIZED] =      g_signal_new("resized",
                                         G_TYPE_FROM_CLASS(vc),
                                         G_SIGNAL_RUN_FIRST,
                                         G_STRUCT_OFFSET(SPViewClass, resized),
                                         NULL, NULL,
                                         sp_marshal_NONE__DOUBLE_DOUBLE,
                                         G_TYPE_NONE, 2,
                                         G_TYPE_DOUBLE, G_TYPE_DOUBLE);
    signals[POSITION_SET] = g_signal_new("position_set",
                                         G_TYPE_FROM_CLASS(vc),
                                         G_SIGNAL_RUN_FIRST,
                                         G_STRUCT_OFFSET(SPViewClass, position_set),
                                         NULL, NULL,
                                         sp_marshal_NONE__DOUBLE_DOUBLE,
                                         G_TYPE_NONE, 2,
                                         G_TYPE_DOUBLE, G_TYPE_DOUBLE);
    
    object_class->dispose = &SPView::dispose;
}

void SPView::init(SPView *view)
{
    view->doc = NULL;

    new (&view->_message_changed_connection) sigc::connection();

    view->_message_stack = new Inkscape::MessageStack();

    view->_tips_message_context = new Inkscape::MessageContext(view->_message_stack);
    view->_legacy_message_context = new Inkscape::MessageContext(view->_message_stack);

    view->_message_changed_connection = view->_message_stack->connectChanged(
        sigc::bind(sigc::ptr_fun(&SPView::_set_status_message), view)
        );
}

void SPView::dispose(GObject *object)
{
    SPView *view = SP_VIEW(object);

    view->_message_changed_connection.disconnect();

    delete view->_tips_message_context;
    view->_tips_message_context = NULL;

    delete view->_legacy_message_context;
    view->_legacy_message_context = NULL;

    Inkscape::Refcounted::release(view->_message_stack);
    view->_message_stack = NULL;

    if (view->doc) {
        sp_signal_disconnect_by_data(view->doc, view);
        view->doc = sp_document_unref(view->doc);
    }
    
    view->_message_changed_connection.~connection();
    
    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
 *  Closes the given 'view' by issuing the 'SHUTDOWN' signal to it.
 *  Returns the result that is returned by the signal handler, which
 *  is 0 (FALSE) if the user cancels the close, or non-zero otherwise.
 */
gboolean sp_view_shutdown(SPView *view)
{
    gboolean result = FALSE;

    g_return_val_if_fail(view != NULL, TRUE);
    g_return_val_if_fail(SP_IS_VIEW(view), TRUE);

    g_signal_emit(G_OBJECT(view), signals[SHUTDOWN], 0, &result);

    return result;
}

void SPView::_set_status_message(Inkscape::MessageType type, gchar const *message, SPView *view)
{
    if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_status_message) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_status_message(view, type, message);
    }
}

void sp_view_request_redraw(SPView *view)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));

    if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->request_redraw) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(view))->request_redraw(view);
    }
}

void sp_view_set_document(SPView *view, SPDocument *doc)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));
    g_return_if_fail(!doc || SP_IS_DOCUMENT(doc));
    
    if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_document) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_document(view, doc);
    }

    if (view->doc) {
        sp_signal_disconnect_by_data(view->doc, view);
        view->doc = sp_document_unref(view->doc);
    }

    if (doc) {
        view->doc = sp_document_ref(doc);
        g_signal_connect(G_OBJECT(doc), "uri_set", G_CALLBACK(sp_view_document_uri_set), view);
        g_signal_connect(G_OBJECT(doc), "resized", G_CALLBACK(sp_view_document_resized), view);
    }

    g_signal_emit(G_OBJECT(view), signals[URI_SET], 0, (doc) ? SP_DOCUMENT_URI(doc) : NULL);
}

void sp_view_emit_resized(SPView *view, gdouble width, gdouble height)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW (view));

    g_signal_emit(G_OBJECT(view), signals[RESIZED], 0, width, height);
}

void sp_view_set_position(SPView *view, gdouble x, gdouble y)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));

    g_signal_emit(G_OBJECT(view), signals[POSITION_SET], 0, x, y);
}

static void sp_view_document_uri_set(SPDocument *doc, const guchar *uri, SPView *view)
{
    g_signal_emit(G_OBJECT(view), signals[URI_SET], 0, uri);
}

static void sp_view_document_resized(SPDocument *doc, gdouble width, gdouble height, SPView *view)
{
    if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->document_resized) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(view))->document_resized(view, doc, width, height);
    }
}


/* SPViewWidget */

static void sp_view_widget_class_init(SPViewWidgetClass *vwc);
static void sp_view_widget_init(SPViewWidget *widget);
static void sp_view_widget_destroy(GtkObject *object);

static void sp_view_widget_view_resized(SPView *view, gdouble width, gdouble height, SPViewWidget *vw);

static GtkEventBoxClass *widget_parent_class;

GtkType sp_view_widget_get_type(void)
{
    static GtkType type = 0;
    
    if (!type) {
        GtkTypeInfo info = {
            "SPViewWidget",
            sizeof(SPViewWidget),
            sizeof(SPViewWidgetClass),
            (GtkClassInitFunc) sp_view_widget_class_init,
            (GtkObjectInitFunc) sp_view_widget_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique(GTK_TYPE_EVENT_BOX, &info);
    }
    
    return type;
}

static void sp_view_widget_class_init(SPViewWidgetClass *vwc)
{
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(vwc);

    widget_parent_class = (GtkEventBoxClass*) gtk_type_class(GTK_TYPE_EVENT_BOX);
    
    object_class->destroy = sp_view_widget_destroy;
}

static void sp_view_widget_init(SPViewWidget *vw)
{
    vw->view = NULL;
}

static void sp_view_widget_destroy(GtkObject *object)
{
    SPViewWidget *vw = SP_VIEW_WIDGET(object);

    if (vw->view) {
        sp_signal_disconnect_by_data(vw->view, vw);
        g_object_unref(G_OBJECT(vw->view));
        vw->view = NULL;
    }

    if (((GtkObjectClass *) (widget_parent_class))->destroy) {
        (* ((GtkObjectClass *) (widget_parent_class))->destroy)(object);
    }
}

void sp_view_widget_set_view(SPViewWidget *vw, SPView *view)
{
    g_return_if_fail(vw != NULL);
    g_return_if_fail(SP_IS_VIEW_WIDGET(vw));
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));
    
    g_return_if_fail(vw->view == NULL);
    
    vw->view = view;
    g_object_ref(G_OBJECT(view));
    g_signal_connect(G_OBJECT(view), "resized", G_CALLBACK(sp_view_widget_view_resized), vw);

    if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->set_view) {
        ((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->set_view(vw, view);
    }
}

gboolean sp_view_widget_shutdown(SPViewWidget *vw)
{
    g_return_val_if_fail(vw != NULL, TRUE);
    g_return_val_if_fail(SP_IS_VIEW_WIDGET(vw), TRUE);

    if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->shutdown) {
        return ((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->shutdown(vw);
    }

    return FALSE;
}

static void sp_view_widget_view_resized(SPView *view, gdouble width, gdouble height, SPViewWidget *vw)
{
    if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->view_resized) {
        ((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->view_resized(vw, view, width, height);
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
