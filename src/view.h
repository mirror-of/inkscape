#ifndef __SP_VIEW_H__
#define __SP_VIEW_H__

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

#include <libnr/nr-point.h>

#define SP_TYPE_VIEW (sp_view_get_type ())
#define SP_VIEW(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_VIEW, SPView))
#define SP_IS_VIEW(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_VIEW))

#include <sigc++/sigc++.h>
#include <gtk/gtkeventbox.h>

#include "helper/action.h"
#include "forward.h"
#include "message.h"
#include "message-stack.h"
#include "message-context.h"

namespace Inkscape {
	class MessageContext;
	class MessageStack;
}

struct SPView {
	GObject object;

	SPDocument *doc;

	static void init(SPView *view);
	static void dispose(GObject *obj);

	Inkscape::MessageStack *messageStack() {
		return _message_stack;
	}

	Inkscape::MessageContext *tipsMessageContext() {
		return _tips_message_context;
	}

	/* don't use this in new code */
	Inkscape::MessageContext *legacyMessageContext() {
		return _legacy_message_context;
	}

private:
	static void _set_status_message(Inkscape::MessageType type, gchar const *message, SPView *view);

	Inkscape::MessageStack *_message_stack;
	Inkscape::MessageContext *_tips_message_context;
	Inkscape::MessageContext *_legacy_message_context;

	sigc::connection _message_changed_connection;
};

struct SPViewClass {
	GObjectClass parent_class;

	/* Request shutdown */
	gboolean (* shutdown) (SPView *view);
	/* Request redraw of visible area */
	void (* request_redraw) (SPView *view);
	/* Virtual method to set/change/remove document link */
	void (* set_document) (SPView *view, SPDocument *doc);
	/* Virtual method about document size change */
	void (* document_resized) (SPView *view, SPDocument *doc, gdouble width, gdouble height);

	/* Signal of view uri change */
	void (* uri_set) (SPView *view, const guchar *uri);
	/* Signal of view size change */
	void (* resized) (SPView *view, gdouble width, gdouble height);
	/* Cursor position */
	void (* position_set) (SPView *view, gdouble x, gdouble y);
	/* Status */
	void (* set_status_message) (SPView *view, Inkscape::MessageType type, gchar const *message);
};

GType sp_view_get_type (void);

#define SP_VIEW_DOCUMENT(v) (SP_VIEW (v)->doc)

void sp_view_set_document (SPView *view, SPDocument *doc);

void sp_view_emit_resized (SPView *view, gdouble width, gdouble height);
void sp_view_set_position (SPView *view, gdouble x, gdouble y);

inline void sp_view_set_position(SPView *view, NR::Point const &p)
{
	sp_view_set_position(view, p[NR::X], p[NR::Y]);
}

gboolean sp_view_shutdown (SPView *view);
void sp_view_request_redraw (SPView *view);

/* SPViewWidget */

#define SP_TYPE_VIEW_WIDGET (sp_view_widget_get_type ())
#define SP_VIEW_WIDGET(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_VIEW_WIDGET, SPViewWidget))
#define SP_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_VIEW_WIDGET, SPViewWidgetClass))
#define SP_IS_VIEW_WIDGET(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_VIEW_WIDGET))
#define SP_IS_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_VIEW_WIDGET))

struct SPViewWidget {
	GtkEventBox eventbox;

	SPView *view;
};

struct SPViewWidgetClass {
	GtkEventBoxClass parent_class;

	/* Vrtual method to set/change/remove view */
	void (* set_view) (SPViewWidget *vw, SPView *view);
	/* Virtual method about view size change */
	void (* view_resized) (SPViewWidget *vw, SPView *view, gdouble width, gdouble height);

	gboolean (* shutdown) (SPViewWidget *vw);
};

GType sp_view_widget_get_type (void);

#define SP_VIEW_WIDGET_VIEW(w) (SP_VIEW_WIDGET (w)->view)
#define SP_VIEW_WIDGET_DOCUMENT(w) (SP_VIEW_WIDGET (w)->view ? ((SPViewWidget *) (w))->view->doc : NULL)

void sp_view_widget_set_view (SPViewWidget *vw, SPView *view);

/* Allows presenting 'save changes' dialog, FALSE - continue, TRUE - cancel */
gboolean sp_view_widget_shutdown (SPViewWidget *vw);

inline __attribute__((deprecated)) void sp_view_set_statusf(SPView *view, gchar const *format, ...) {
	va_list args;
	va_start(args, format);
	view->legacyMessageContext()->setVF(Inkscape::NORMAL_MESSAGE, format, args);
	va_end(args);
}

inline __attribute__((deprecated)) void sp_view_pop_statusf(SPView *view) {
	view->legacyMessageContext()->clear();
}

inline __attribute__((deprecated)) void sp_view_set_statusf_timeout(SPView *view, guint msec, gchar const *format, ...)
{
	va_list args;
	va_start(args, format);
	if (msec) {
		view->messageStack()->flashVF(Inkscape::NORMAL_MESSAGE, format, args);
	} else {
		view->legacyMessageContext()->setVF(Inkscape::NORMAL_MESSAGE, format, args);
	}
	va_end(args);
}

inline __attribute__((deprecated)) void sp_view_set_statusf_flash(SPView *view, gchar const *format, ...)
{
	va_list args;
	va_start(args, format);
	view->messageStack()->flashVF(Inkscape::NORMAL_MESSAGE, format, args);
	va_end(args);
}

inline __attribute__((deprecated)) void sp_view_set_statusf_error(SPView *view, gchar const *format, ...)
{
	va_list args;
	va_start(args, format);
	view->messageStack()->flashVF(Inkscape::ERROR_MESSAGE, format, args);
	va_end(args);
}

inline __attribute__((deprecated)) void sp_view_set_statusf_va(SPView *view, gchar const *format, va_list args)
{
	view->legacyMessageContext()->setVF(Inkscape::NORMAL_MESSAGE, format, args);
}

inline __attribute__((deprecated)) void sp_view_clear_status(SPView *view) {
	view->legacyMessageContext()->clear();
}

inline __attribute__((deprecated)) void sp_view_set_status(SPView *view, gchar const *status, gboolean)
{
	view->legacyMessageContext()->set(Inkscape::NORMAL_MESSAGE, status);
}


/* Create a new SPViewWidget (which happens to be a SPDesktopWidget). */
SPViewWidget *sp_desktop_widget_new (SPNamedView *namedview);

#endif
