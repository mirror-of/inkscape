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
#include "helper/sp-marshal.h"
#include "document.h"
#include "view.h"
#include "inkscape.h"

enum {
	SHUTDOWN,
	URI_SET,
	RESIZED,
	POSITION_SET,
	STATUS_SET,
	LAST_SIGNAL
};

static void sp_view_class_init (SPViewClass *klass);
static void sp_view_init (SPView *view);
static void sp_view_dispose (GObject *object);

static void sp_view_document_uri_set (SPDocument *doc, const guchar *uri, SPView *view);
static void sp_view_document_resized (SPDocument *doc, gdouble width, gdouble height, SPView *view);

static GObjectClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GtkType
sp_view_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPViewClass),
			NULL, NULL,
			(GClassInitFunc) sp_view_class_init,
			NULL, NULL,
			sizeof (SPView),
			4,
			(GInstanceInitFunc) sp_view_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPView", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_view_class_init (SPViewClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	parent_class = (GObjectClass*)g_type_class_peek_parent (klass);

	signals[SHUTDOWN] =     g_signal_new ("shutdown",
					      G_TYPE_FROM_CLASS(klass),
					      G_SIGNAL_RUN_LAST,
					      G_STRUCT_OFFSET (SPViewClass, shutdown),
					      NULL, NULL,
					      sp_marshal_BOOLEAN__NONE,
					      G_TYPE_BOOLEAN, 0);
	signals[URI_SET] =      g_signal_new ("uri_set",
					      G_TYPE_FROM_CLASS(klass),
					      G_SIGNAL_RUN_FIRST,
					      G_STRUCT_OFFSET (SPViewClass, uri_set),
					      NULL, NULL,
					      sp_marshal_NONE__POINTER,
					      G_TYPE_NONE, 1,
					      G_TYPE_POINTER);
	signals[RESIZED] =      g_signal_new ("resized",
					      G_TYPE_FROM_CLASS(klass),
					      G_SIGNAL_RUN_FIRST,
					      G_STRUCT_OFFSET (SPViewClass, resized),
					      NULL, NULL,
					      sp_marshal_NONE__DOUBLE_DOUBLE,
					      G_TYPE_NONE, 2,
					      G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	signals[POSITION_SET] = g_signal_new ("position_set",
					      G_TYPE_FROM_CLASS(klass),
					      G_SIGNAL_RUN_FIRST,
					      G_STRUCT_OFFSET (SPViewClass, position_set),
					      NULL, NULL,
					      sp_marshal_NONE__DOUBLE_DOUBLE,
					      G_TYPE_NONE, 2,
					      G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	signals[STATUS_SET] =   g_signal_new ("status_set",
					      G_TYPE_FROM_CLASS(klass),
					      G_SIGNAL_RUN_FIRST,
					      G_STRUCT_OFFSET (SPViewClass, status_set),
					      NULL, NULL,
					      sp_marshal_NONE__POINTER_UINT,
					      G_TYPE_NONE, 2,
					      G_TYPE_POINTER, G_TYPE_UINT);

	object_class->dispose = sp_view_dispose;
}

static void
sp_view_init (SPView *view)
{
	view->doc = NULL;
}

static void
sp_view_dispose (GObject *object)
{
	SPView *view;

	view = SP_VIEW (object);

	if (view->doc) {
		sp_signal_disconnect_by_data (view->doc, view);
		view->doc = sp_document_unref (view->doc);
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

gboolean
sp_view_shutdown (SPView *view)
{
	gboolean result;

	g_return_val_if_fail (view != NULL, TRUE);
	g_return_val_if_fail (SP_IS_VIEW (view), TRUE);

	result = FALSE;

	g_signal_emit (G_OBJECT (view), signals[SHUTDOWN], 0, &result);

	return result;
}

void
sp_view_request_redraw (SPView *view)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->request_redraw)
		((SPViewClass *) G_OBJECT_GET_CLASS(view))->request_redraw (view);
}

void
sp_view_set_document (SPView *view, SPDocument *doc)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));
	g_return_if_fail (!doc || SP_IS_DOCUMENT (doc));

	if (((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_document)
		((SPViewClass *) G_OBJECT_GET_CLASS(view))->set_document (view, doc);

	if (view->doc) {
		sp_signal_disconnect_by_data (view->doc, view);
		view->doc = sp_document_unref (view->doc);
	}

	if (doc) {
		view->doc = sp_document_ref (doc);
		g_signal_connect (G_OBJECT (doc), "uri_set", G_CALLBACK (sp_view_document_uri_set), view);
		g_signal_connect (G_OBJECT (doc), "resized", G_CALLBACK (sp_view_document_resized), view);
	}

	g_signal_emit (G_OBJECT (view), signals[URI_SET], 0, (doc) ? SP_DOCUMENT_URI (doc) : NULL);
}

void
sp_view_emit_resized (SPView *view, gdouble width, gdouble height)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	g_signal_emit (G_OBJECT (view), signals[RESIZED], 0, width, height);
}

void
sp_view_set_position (SPView *view, gdouble x, gdouble y)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	g_signal_emit (G_OBJECT (view), signals[POSITION_SET], 0, x, y);
}

/**
\brief Sets statusbar message permanently. The format string and the rest of arguments
are as in printf(). Not to be used normally, because the message nev er disappears
unless replaced by another message. In the majority of situations, one of the _flash or
_error functions should be used instead.
*/
void
sp_view_set_statusf (SPView *view, const gchar *format, ...)
{
	va_list args;
	va_start (args, format);
	sp_view_set_statusf_va (view, 0, format, args);
	va_end (args);
}

void
sp_view_set_statusf_timeout (SPView *view, guint msec, const gchar *format, ...)
{
	va_list args;
	va_start (args, format);
	sp_view_set_statusf_va (view, msec, format, args);
	va_end (args);
}

//TODO: make user-settable
#define STATUSBAR_FLASH_MSEC 1000
#define STATUSBAR_ERROR_MSEC 5000

/**
\brief Displays message temporarily for 1 second; when the message disappears, the one
that was in the statusbar before is restored. The format string and the rest of
arguments are as in printf(). Use this function for short helpful hints or explanations
of non-trivial actions. Examples: "No-break space" when you press ctrl-space in text
object; "Closing path" when hitting the anchor in freehand; "Selection cancelled" when
pressing Esc cancelled rubberband selection.
*/
void
sp_view_set_statusf_flash (SPView *view, const gchar *format, ...)
{
	va_list args;
	va_start (args, format);
	sp_view_set_statusf_va (view, STATUSBAR_FLASH_MSEC, format, args);
	va_end (args);
}

/**
\brief Displays message temporarily for 5 seconds; when the message disappears, the one
that was in the statusbar before it is restored. The format string and the rest of
arguments are as in printf(). Use this function for most non-fatal error messages
(unless they are so important as to warrant a modal messagebox). Examples: "To join, you
must have two endnodes selected" when you try to join nodes that are not exactly two
endpoints; "To subtract, you must have two objects selected" for the boolean difference
operation.
*/
void
sp_view_set_statusf_error (SPView *view, const gchar *format, ...)
{
	va_list args;
	va_start (args, format);
	sp_view_set_statusf_va (view, STATUSBAR_ERROR_MSEC, format, args);
	va_end (args);
}

void
sp_view_set_statusf_va (SPView *view, guint msec, const gchar *format, va_list args)
{
	gchar *status;

	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	status = g_strdup_vprintf (format, args);

	g_return_if_fail (status != NULL);

	g_signal_emit (G_OBJECT (view), signals[STATUS_SET], 0, status, msec);
	g_free (status);
}

/**
\brief Clears statusbar message permanently. Not to be used normally. In the majority of
situations, one of the _flash or _error functions should be used instead.
*/
void
sp_view_clear_status (SPView *view)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	g_signal_emit (G_OBJECT (view), signals[STATUS_SET], 0, NULL, 0);
}

//deprecated, eliminate
void
sp_view_set_status (SPView *view, const gchar *status, gboolean isdefault)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	g_signal_emit (G_OBJECT (view), signals[STATUS_SET], 0, status, 0);
}


static void
sp_view_document_uri_set (SPDocument *doc, const guchar *uri, SPView *view)
{
	g_signal_emit (G_OBJECT (view), signals[URI_SET], 0, uri);
}

static void
sp_view_document_resized (SPDocument *doc, gdouble width, gdouble height, SPView *view)
{
	if (((SPViewClass *) G_OBJECT_GET_CLASS (view))->document_resized)
		((SPViewClass *) G_OBJECT_GET_CLASS (view))->document_resized (view, doc, width, height);
}

/* SPViewWidget */

static void sp_view_widget_class_init (SPViewWidgetClass *klass);
static void sp_view_widget_init (SPViewWidget *widget);
static void sp_view_widget_destroy (GtkObject *object);

static void sp_view_widget_view_resized (SPView *view, gdouble width, gdouble height, SPViewWidget *vw);

static GtkEventBoxClass *widget_parent_class;

GtkType
sp_view_widget_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPViewWidget",
			sizeof (SPViewWidget),
			sizeof (SPViewWidgetClass),
			(GtkClassInitFunc) sp_view_widget_class_init,
			(GtkObjectInitFunc) sp_view_widget_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_EVENT_BOX, &info);
	}
	return type;
}

static void
sp_view_widget_class_init (SPViewWidgetClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);

	widget_parent_class = (GtkEventBoxClass*)gtk_type_class (GTK_TYPE_EVENT_BOX);

	object_class->destroy = sp_view_widget_destroy;
}

static void
sp_view_widget_init (SPViewWidget *vw)
{
	vw->view = NULL;
}

static void
sp_view_widget_destroy (GtkObject *object)
{
	SPViewWidget *vw;

	vw = SP_VIEW_WIDGET (object);

	if (vw->view) {
		sp_signal_disconnect_by_data (vw->view, vw);
		g_object_unref (G_OBJECT (vw->view));
		vw->view = NULL;
	}

	if (((GtkObjectClass *) (widget_parent_class))->destroy)
		(* ((GtkObjectClass *) (widget_parent_class))->destroy) (object);
}

void
sp_view_widget_set_view (SPViewWidget *vw, SPView *view)
{
	g_return_if_fail (vw != NULL);
	g_return_if_fail (SP_IS_VIEW_WIDGET (vw));
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_VIEW (view));

	g_return_if_fail (vw->view == NULL);

	vw->view = view;
	g_object_ref (G_OBJECT (view));
	g_signal_connect (G_OBJECT (view), "resized", G_CALLBACK (sp_view_widget_view_resized), vw);

	if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->set_view)
		((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->set_view (vw, view);
}

gboolean
sp_view_widget_shutdown (SPViewWidget *vw)
{
	g_return_val_if_fail (vw != NULL, TRUE);
	g_return_val_if_fail (SP_IS_VIEW_WIDGET (vw), TRUE);

	if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->shutdown)
		return ((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->shutdown (vw);

	return FALSE;
}

static void
sp_view_widget_view_resized (SPView *view, gdouble width, gdouble height, SPViewWidget *vw)
{
	if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->view_resized)
		((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->view_resized (vw, view, width, height);
}

