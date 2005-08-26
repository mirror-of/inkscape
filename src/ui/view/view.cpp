#define __SP_VIEW_C__

/** \file
 * Static SPView functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"
#include "macros.h"
#include "helper/sp-marshal.h"
#include "document.h"
#include "view.h"
#include "message-stack.h"
#include "message-context.h"
#include "verbs.h"

/// Possible signals on an SPView.
enum {
    SHUTDOWN,
    URI_SET,
    RESIZED,
    POSITION_SET,
    LAST_SIGNAL
};

using namespace Inkscape::UI::View;

static void sp_view_class_init(SPViewClass *vc);

static void sp_view_document_uri_set(gchar const *uri, View *view);
static void sp_view_document_resized(gdouble width, gdouble height, SPView *view);

static GObjectClass *parent_class;
static guint signals[LAST_SIGNAL] = { 0 };

static GValue shutdown_accumulator_data;

/**
 * Registers the SPView class with Glib and returns its type number.
 */
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
        type = g_type_register_static(G_TYPE_OBJECT, "View", &info, (GTypeFlags) 0);
    }
    
    return type;
}

/**
 * Accumulator callback for collecting return values during shutdown.
 * 
 * This signal accumulator is a special callback function for collecting the
 * return values of the various callbacks that are called during the SHUTDOWN 
 * signal emission.  This allows us to see if any of the views being shutdown
 * return a 'cancel' value that needs to be honored.  Without this accumulator
 * only the return value of the last view would be checked.
 *
 * \param ihint          Signal invocation hint
 * \param return_accu    Accumulator to collect callback return values in. 
 * This is the return value of the current signal emission. 
 * \param handler_return Return value of the signal handler this function 
 * will check.
 *
 * \ret The accumulator function returns whether the signal emission 
 * should be aborted. Returning FALSE means to abort the current emission 
 * and TRUE is returned for continuation. 
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

/**
 * View vtable initialization and signal registration callback.
 */
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

/**
 * Callback to initialize the view's message stacks and to connect it 
 * to various signals.
 */
void SPView::init(SPView *view)
{
    view->doc = NULL;

    new (&view->_message_changed_connection) sigc::connection();
    new (&view->_document_uri_set_connection) sigc::connection();
    new (&view->_document_resized_connection) sigc::connection();

    view->_message_stack = new Inkscape::MessageStack();

    view->_tips_message_context = new Inkscape::MessageContext(view->_message_stack);

    view->_message_changed_connection = view->_message_stack->connectChanged(
        sigc::bind(sigc::ptr_fun(&SPView::_set_status_message), view)
    );


}

/**
 * Callback to delete and null all SPView message stacks and to disconnect it 
 * from signals.
 */
void SPView::dispose(GObject *object)
{
    SPView *view = SP_VIEW(object);

    view->_message_changed_connection.disconnect();

    delete view->_tips_message_context;
    view->_tips_message_context = NULL;

    Inkscape::GC::release(view->_message_stack);
    view->_message_stack = NULL;

    if (view->doc) {
        view->_document_uri_set_connection.disconnect();
        view->_document_resized_connection.disconnect();
        sp_document_unref(view->doc);
        view->doc = NULL;
    }
    
    view->_message_changed_connection.~connection();
    view->_document_uri_set_connection.~connection();
    view->_document_resized_connection.~connection();

    Inkscape::Verb::delete_all_view(view);
    
    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
 *  Closes the given 'view' by issuing the 'SHUTDOWN' signal to it.
 *  
 *  \return The result that is returned by the signal handler, which
 *  is 0 (FALSE) if the user cancels the close, or non-zero otherwise.
 */
gboolean SPView::shutdown() {
    gboolean result = FALSE;

    g_signal_emit(G_OBJECT(this), signals[SHUTDOWN], 0, &result);

    return result;
}

gboolean sp_view_shutdown(SPView *view)
{
    g_return_val_if_fail(view != NULL, TRUE);
    g_return_val_if_fail(SP_IS_VIEW(view), TRUE);

    return view->shutdown();
}

/**
 * Calls the virtual function set_status_message() of the view.
 */
void SPView::_set_status_message(Inkscape::MessageType type, gchar const *message, SPView *view)
{
    if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_status_message) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_status_message(view, type, message);
    }
}

void SPView::requestRedraw() {
    if (((SPViewClass *) G_OBJECT_GET_CLASS(this))->request_redraw) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(this))->request_redraw(this);
    }    
}

/**
 * Calls the virtual function request_redraw() of the view.
 */
void sp_view_request_redraw(SPView *view)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));

    view->requestRedraw();
}

/**
 * Calls the view's virtual function set_document(), disconnects the view 
 * from its old document, connects it to a new one, and emits the signal
 * "uri_set".
 * 
 * \param doc The new document to connect the view to.
 */
void SPView::setDocument(SPDocument *doc) {
    g_return_if_fail(doc != NULL);

    if (((SPViewClass *) G_OBJECT_GET_CLASS(this))->set_document) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(this))->set_document(this, doc);
    }

    if (this->doc) {
        this->_document_uri_set_connection.disconnect();
        this->_document_resized_connection.disconnect();
        sp_document_unref(this->doc);
        this->doc = NULL;
    }

    if (doc) {
        this->doc = sp_document_ref(doc);
        this->_document_uri_set_connection = doc->connectURISet(sigc::bind(sigc::ptr_fun(&sp_view_document_uri_set), this));
        this->_document_resized_connection = doc->connectResized(sigc::bind(sigc::ptr_fun(&sp_view_document_resized), this));
    }

    g_signal_emit(G_OBJECT(this), signals[URI_SET], 0, (doc) ? SP_DOCUMENT_URI(doc) : NULL);
}

void sp_view_set_document(SPView *view, SPDocument *doc)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));

    view->setDocument(doc);
}
 
void SPView::emitResized(gdouble width, gdouble height) {
    g_signal_emit(G_OBJECT(this), signals[RESIZED], 0, width, height);
}

/**
 * Emit the "resized" signal on the view.
 */
void sp_view_emit_resized(SPView *view, gdouble width, gdouble height)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW (view));

    view->emitResized(width, height);
}

void SPView::setPosition(gdouble x, gdouble y) {
    g_signal_emit(G_OBJECT(this), signals[POSITION_SET], 0, x, y);
}

/**
 * Emit the "position_set" signal on the view.
 */
void sp_view_set_position(SPView *view, gdouble x, gdouble y)
{
    g_return_if_fail(view != NULL);
    g_return_if_fail(SP_IS_VIEW(view));

    view->setPosition(x, y);
}

/**
 * Emit the "uri_set" signal on the view.
 */
static void sp_view_document_uri_set(gchar const *uri, SPView *view)
{
    g_signal_emit(G_OBJECT(view), signals[URI_SET], 0, uri);
}

/**
 * Calls the virtual function document_resized() of the view.
 */
static void sp_view_document_resized(gdouble width, gdouble height, SPView *view)
{
    if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->document_resized) {
        ((SPViewClass *) G_OBJECT_GET_CLASS(view))->document_resized(view, view->doc, width, height);
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
