#ifndef __SP_SVG_VIEW_H__
#define __SP_SVG_VIEW_H__

/*
 * Generic SVG view and widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

typedef struct _SPSVGView SPSVGView;
typedef struct _SPSVGViewClass SPSVGViewClass;

#define SP_TYPE_SVG_VIEW (sp_svg_view_get_type ())
#define SP_SVG_VIEW(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_SVG_VIEW, SPSVGView))
#define SP_SVG_VIEW_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_SVG_VIEW, SPSVGViewClass))
#define SP_IS_SVG_VIEW(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_SVG_VIEW))
#define SP_IS_SVG_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_SVG_VIEW))

#include "helper/helper-forward.h"
#include "view.h"

struct _SPSVGView {
	SPView view;

	unsigned int dkey;

	SPCanvasGroup *parent;
	SPCanvasItem *drawing;

	/* Horizontal and vertical scale */
	gdouble hscale, vscale;
	/* Whether to rescale automatically */
	gboolean rescale, keepaspect;
	gdouble width, height;
};

struct _SPSVGViewClass {
	SPViewClass parent_class;
};

GtkType sp_svg_view_get_type (void);

SPView *sp_svg_view_new (SPCanvasGroup *parent);

void sp_svg_view_set_scale (SPSVGView *view, gdouble hscale, gdouble vscale);
void sp_svg_view_set_rescale (SPSVGView *view, gboolean rescale, gboolean keepaspect, gdouble width, gdouble height);

/* SPSVGViewWidget */

typedef struct _SPSVGViewWidget SPSVGViewWidget;
typedef struct _SPSVGViewWidgetClass SPSVGViewWidgetClass;

#define SP_TYPE_SVG_VIEW_WIDGET (sp_svg_view_widget_get_type ())
#define SP_SVG_VIEW_WIDGET(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_SVG_VIEW_WIDGET, SPSVGViewWidget))
#define SP_SVG_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_SVG_VIEW_WIDGET, SPSVGViewWidgetClass))
#define SP_IS_SVG_VIEW_WIDGET(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_SVG_VIEW_WIDGET))
#define SP_IS_SVG_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_SVG_VIEW_WIDGET))

struct _SPSVGViewWidget {
	SPViewWidget widget;

	GtkWidget *sw;
	GtkWidget *canvas;

	/* Whether to resize automatically */
	gboolean resize;
	gdouble maxwidth, maxheight;
};

struct _SPSVGViewWidgetClass {
	SPViewWidgetClass parent_class;
};

GtkType sp_svg_view_widget_get_type (void);

GtkWidget *sp_svg_view_widget_new (SPDocument *doc);

void sp_svg_view_widget_set_resize (SPSVGViewWidget *vw, gboolean resize, gdouble width, gdouble height);

#endif
