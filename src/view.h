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

typedef struct _SPView SPView;
typedef struct _SPViewClass SPViewClass;

#define SP_TYPE_VIEW (sp_view_get_type ())
#define SP_VIEW(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_VIEW, SPView))
#define SP_IS_VIEW(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_VIEW))

#include <gtk/gtkeventbox.h>
#include "forward.h"

struct _SPView {
	GObject object;

	SPDocument *doc;
};

struct _SPViewClass {
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
	void (* status_set) (SPView *view, const guchar *status, gboolean isdefault);
};

GType sp_view_get_type (void);

#define SP_VIEW_DOCUMENT(v) (SP_VIEW (v)->doc)

void sp_view_set_document (SPView *view, SPDocument *doc);

void sp_view_emit_resized (SPView *view, gdouble width, gdouble height);
void sp_view_set_position (SPView *view, gdouble x, gdouble y);
void sp_view_set_status (SPView *view, const guchar *status, gboolean isdefault);

gboolean sp_view_shutdown (SPView *view);
void sp_view_request_redraw (SPView *view);

/* SPViewWidget */

typedef struct _SPViewWidget SPViewWidget;
typedef struct _SPViewWidgetClass SPViewWidgetClass;

#define SP_TYPE_VIEW_WIDGET (sp_view_widget_get_type ())
#define SP_VIEW_WIDGET(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_VIEW_WIDGET, SPViewWidget))
#define SP_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_VIEW_WIDGET, SPViewWidgetClass))
#define SP_IS_VIEW_WIDGET(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_VIEW_WIDGET))
#define SP_IS_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_VIEW_WIDGET))

struct _SPViewWidget {
	GtkEventBox eventbox;

	SPView *view;
};

struct _SPViewWidgetClass {
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

#endif
