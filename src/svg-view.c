#define __SP_SVG_VIEW_C__

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

#include <libart_lgpl/art_affine.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkscrolledwindow.h>
#include "display/canvas-arena.h"
#include "document.h"
#include "sp-item.h"
#include "svg-view.h"

static void sp_svg_view_class_init (SPSVGViewClass *klass);
static void sp_svg_view_init (SPSVGView *view);
static void sp_svg_view_dispose (GObject *object);

static void sp_svg_view_set_document (SPView *view, SPDocument *doc);
static void sp_svg_view_document_resized (SPView *view, SPDocument *doc, gdouble width, gdouble height);

static void sp_svg_view_rescale (SPSVGView *svgview, gboolean event);

static SPViewClass *parent_class;

GtkType
sp_svg_view_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPSVGViewClass),
			NULL, NULL,
			(GClassInitFunc) sp_svg_view_class_init,
			NULL, NULL,
			sizeof (SPSVGView),
			4,
			(GInstanceInitFunc) sp_svg_view_init,
		};
		type = g_type_register_static (SP_TYPE_VIEW, "SPSVGView", &info, 0);
	}
	return type;
}

static void
sp_svg_view_class_init (SPSVGViewClass *klass)
{
	GObjectClass *object_class;
	SPViewClass *view_class;

	object_class = G_OBJECT_CLASS (klass);
	view_class = (SPViewClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = sp_svg_view_dispose;

	view_class->set_document = sp_svg_view_set_document;
	view_class->document_resized = sp_svg_view_document_resized;
}

static void
sp_svg_view_init (SPSVGView *view)
{
	view->hscale = 1.0;
	view->vscale = 1.0;
	view->rescale = FALSE;
	view->keepaspect = FALSE;
	view->width = 0.0;
	view->height = 0.0;

	view->dkey = 0;
}

static void
sp_svg_view_dispose (GObject *object)
{
	SPSVGView *svgview;

	svgview = SP_SVG_VIEW (object);

	if (svgview->drawing) {
		sp_item_invoke_hide (SP_ITEM (sp_document_root (SP_VIEW_DOCUMENT (svgview))), svgview->dkey);
		svgview->drawing = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

/* fixme: */

static gint
arena_handler (SPCanvasArena *arena, NRArenaItem *ai, GdkEvent *event, SPSVGView *svgview)
{
	static gdouble x, y;
	static gboolean active = FALSE;
	SPItem *spitem;
	SPEvent spev;

	spitem = (ai) ? NR_ARENA_ITEM_GET_DATA (ai) : NULL;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			active = TRUE;
			x = event->button.x;
			y = event->button.y;
		}
		break;
	case GDK_BUTTON_RELEASE:
		if (event->button.button == 1) {
			if (active && (event->button.x == x) && (event->button.y == y)) {
				spev.type = SP_EVENT_ACTIVATE;
				sp_item_event (spitem, &spev);
			}
		}
		active = FALSE;
		break;
	case GDK_MOTION_NOTIFY:
		active = FALSE;
		break;
	case GDK_ENTER_NOTIFY:
		spev.type = SP_EVENT_MOUSEOVER;
		spev.data = svgview;
		sp_item_event (spitem, &spev);
		break;
	case GDK_LEAVE_NOTIFY:
		spev.type = SP_EVENT_MOUSEOUT;
		spev.data = svgview;
		sp_item_event (spitem, &spev);
		break;
	default:
		break;
	}

	return TRUE;
}

static void
sp_svg_view_set_document (SPView *view, SPDocument *doc)
{
	SPSVGView *svgview;

	svgview = SP_SVG_VIEW (view);

	if (view->doc) {
		sp_item_invoke_hide (SP_ITEM (sp_document_root (SP_VIEW_DOCUMENT (view))), svgview->dkey);
	}

	if (!svgview->drawing) {
		svgview->drawing = sp_canvas_item_new (svgview->parent, SP_TYPE_CANVAS_ARENA, NULL);
		g_signal_connect (G_OBJECT (svgview->drawing), "arena_event", G_CALLBACK (arena_handler), svgview);
	}

	if (doc) {
		NRArenaItem *ai;
		ai = sp_item_invoke_show (SP_ITEM (sp_document_root (doc)), SP_CANVAS_ARENA (svgview->drawing)->arena,
					  svgview->dkey, SP_ITEM_SHOW_PRINT);
		if (ai) {
			nr_arena_item_add_child (SP_CANVAS_ARENA (svgview->drawing)->root, ai, NULL);
			nr_arena_item_unref (ai);
		}
		sp_svg_view_rescale (svgview, !svgview->rescale);
	}
}

static void
sp_svg_view_document_resized (SPView *view, SPDocument *doc, gdouble width, gdouble height)
{
	SPSVGView *svgview;

	svgview = SP_SVG_VIEW (view);

	sp_svg_view_rescale (svgview, !svgview->rescale);
}

SPView *
sp_svg_view_new (SPCanvasGroup *parent)
{
	SPSVGView *svgview;

	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);

	svgview = g_object_new (SP_TYPE_SVG_VIEW, NULL);

	svgview->parent = parent;

	return (SPView *) svgview;
}

void
sp_svg_view_set_scale (SPSVGView *view, gdouble hscale, gdouble vscale)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_SVG_VIEW (view));

	if (!view->rescale && ((hscale != view->hscale) || (vscale != view->vscale))) {
		view->hscale = hscale;
		view->vscale = vscale;
		sp_svg_view_rescale (view, TRUE);
	}
}

void
sp_svg_view_set_rescale (SPSVGView *view, gboolean rescale, gboolean keepaspect, gdouble width, gdouble height)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (SP_IS_SVG_VIEW (view));
	g_return_if_fail (!rescale || (width >= 0.0));
	g_return_if_fail (!rescale || (height >= 0.0));

	view->rescale = rescale;
	view->keepaspect = keepaspect;
	view->width = width;
	view->height = height;

	sp_svg_view_rescale (view, TRUE);
}

static void
sp_svg_view_rescale (SPSVGView *svgview, gboolean event)
{
	SPDocument *doc;

	doc = SP_VIEW_DOCUMENT (svgview);

	if (!doc) return;
	if (sp_document_width (doc) < 1e-9) return;
	if (sp_document_height (doc) < 1e-9) return;

	if (svgview->rescale) {
		gdouble hscale, vscale;
		hscale = svgview->width / (sp_document_width (doc) * 1.25);
		vscale = svgview->height / (sp_document_height (doc) * 1.25);
		if (svgview->keepaspect) {
			if (hscale > vscale) {
				hscale = vscale;
			} else {
				vscale = hscale;
			}
		}
		svgview->hscale = hscale;
		svgview->vscale = vscale;
	}

	if (svgview->drawing) {
		gdouble affine[6];
		art_affine_scale (affine, svgview->hscale, svgview->vscale);
		sp_canvas_item_affine_absolute (svgview->drawing, affine);
	}

	if (event) {
		sp_view_emit_resized (SP_VIEW (svgview),
				      sp_document_width (doc) * 1.25 * svgview->hscale,
				      sp_document_height (doc) * 1.25 * svgview->vscale);
	}
}

/* SPSVGViewWidget */

static void sp_svg_view_widget_class_init (SPSVGViewWidgetClass *klass);
static void sp_svg_view_widget_init (SPSVGViewWidget *widget);
static void sp_svg_view_widget_destroy (GtkObject *object);

static void sp_svg_view_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void sp_svg_view_widget_size_request (GtkWidget *widget, GtkRequisition *req);

static void sp_svg_view_widget_view_resized (SPViewWidget *vw, SPView *view, gdouble width, gdouble height);

static SPViewWidgetClass *widget_parent_class;

GtkType
sp_svg_view_widget_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPSVGViewWidget",
			sizeof (SPSVGViewWidget),
			sizeof (SPSVGViewWidgetClass),
			(GtkClassInitFunc) sp_svg_view_widget_class_init,
			(GtkObjectInitFunc) sp_svg_view_widget_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (SP_TYPE_VIEW_WIDGET, &info);
	}
	return type;
}

static void
sp_svg_view_widget_class_init (SPSVGViewWidgetClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPViewWidgetClass *vw_class;

	object_class = GTK_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);
	vw_class = SP_VIEW_WIDGET_CLASS (klass);

	widget_parent_class = gtk_type_class (SP_TYPE_VIEW_WIDGET);

	object_class->destroy = sp_svg_view_widget_destroy;

	widget_class->size_allocate = sp_svg_view_widget_size_allocate;
	widget_class->size_request = sp_svg_view_widget_size_request;

	vw_class->view_resized = sp_svg_view_widget_view_resized;
}

static void
sp_svg_view_widget_init (SPSVGViewWidget *vw)
{
	GtkStyle *style;
	SPCanvasItem *parent;
	SPView *view;

	/* Settings */
	vw->resize = FALSE;
	vw->maxwidth = 400.0;
	vw->maxheight = 400.0;

	/* ScrolledWindow */
	vw->sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (vw->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (vw), vw->sw);
	gtk_widget_show (vw->sw);

	/* Canvas */
	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	vw->canvas = sp_canvas_new_aa ();
#if 0
	sp_canvas_set_dither (SP_CANVAS (vw->canvas), GDK_RGB_DITHER_MAX);
#endif
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	style = gtk_style_copy (vw->canvas->style);
	style->bg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (vw->canvas, style);
#if 0
	sp_canvas_set_scroll_region (SP_CANVAS (vw->canvas), 0, 0, 200, 200);
#endif
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (vw->sw), vw->canvas);
	gtk_widget_show (vw->canvas);

	/* View */
	parent = sp_canvas_item_new (sp_canvas_root (SP_CANVAS (vw->canvas)), SP_TYPE_CANVAS_GROUP, NULL);
	view = sp_svg_view_new (SP_CANVAS_GROUP (parent));
#if 0
	sp_svg_view_set_rescale (SP_SVG_VIEW (view), TRUE, TRUE, 200.0, 200.0);
#endif
	sp_view_widget_set_view (SP_VIEW_WIDGET (vw), view);
	g_object_unref (G_OBJECT (view));
}

static void
sp_svg_view_widget_destroy (GtkObject *object)
{
	SPSVGViewWidget *vw;

	vw = SP_SVG_VIEW_WIDGET (object);

	vw->canvas = NULL;

	if (((GtkObjectClass *) (widget_parent_class))->destroy)
		(* ((GtkObjectClass *) (widget_parent_class))->destroy) (object);
}

static void
sp_svg_view_widget_size_request (GtkWidget *widget, GtkRequisition *req)
{
	SPSVGViewWidget *vw;
	SPView *v;

	vw = SP_SVG_VIEW_WIDGET (widget);
	v = SP_VIEW_WIDGET_VIEW (widget);

	if (((GtkWidgetClass *) (widget_parent_class))->size_request)
		(* ((GtkWidgetClass *) (widget_parent_class))->size_request) (widget, req);

	if (v->doc) {
		SPSVGView *svgv;
		GtkPolicyType hpol, vpol;
		gdouble width, height;

		svgv = SP_SVG_VIEW (v);
		width = sp_document_width (v->doc) * 1.25 * svgv->hscale;
		height = sp_document_height (v->doc) * 1.25 * svgv->vscale;

		if (width <= vw->maxwidth) {
			hpol = GTK_POLICY_NEVER;
			req->width = (gint) (width + 0.5);
		} else {
			hpol = GTK_POLICY_AUTOMATIC;
			req->width = (gint) (vw->maxwidth + 0.5);
		}
		if (height <= vw->maxheight) {
			vpol = GTK_POLICY_NEVER;
			req->height = (gint) (height + 8.0);
		} else {
			vpol = GTK_POLICY_AUTOMATIC;
			req->height = (gint) (vw->maxheight + 2.0);
		}
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (vw->sw), hpol, vpol);
	}
}

static void
sp_svg_view_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	SPSVGViewWidget *svgvw;

	svgvw = SP_SVG_VIEW_WIDGET (widget);

	if (((GtkWidgetClass *) (widget_parent_class))->size_allocate)
		(* ((GtkWidgetClass *) (widget_parent_class))->size_allocate) (widget, allocation);

	if (!svgvw->resize) {
		sp_svg_view_set_rescale (SP_SVG_VIEW (SP_VIEW_WIDGET_VIEW (svgvw)),
					 TRUE, TRUE,
					 (gdouble) allocation->width - 1.0, (gdouble) allocation->height - 1.0);
	}
}

static void
sp_svg_view_widget_view_resized (SPViewWidget *vw, SPView *view, gdouble width, gdouble height)
{
	SPSVGViewWidget *svgvw;

	svgvw = SP_SVG_VIEW_WIDGET (vw);

	if (svgvw->canvas) {
#if 0
		g_print ("set scroll %g %g\n", width, height);
		sp_canvas_set_scroll_region (SP_CANVAS (svgvw->canvas), 0, 0, MAX (width, 1.0), MAX (height, 1.0));
#endif
	}

	if (svgvw->resize) {
		gtk_widget_set_usize (svgvw->canvas, width, height);
		gtk_widget_queue_resize (GTK_WIDGET (vw));
	}
}

GtkWidget *
sp_svg_view_widget_new (SPDocument *doc)
{
	GtkWidget *widget;

	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (SP_IS_DOCUMENT (doc), NULL);

	widget = gtk_type_new (SP_TYPE_SVG_VIEW_WIDGET);

	sp_view_set_document (SP_VIEW_WIDGET_VIEW (widget), doc);

	return widget;
}

void
sp_svg_view_widget_set_resize (SPSVGViewWidget *vw, gboolean resize, gdouble width, gdouble height)
{
	g_return_if_fail (vw != NULL);
	g_return_if_fail (SP_IS_SVG_VIEW_WIDGET (vw));
	g_return_if_fail (!resize || (width > 0.0));
	g_return_if_fail (!resize || (height > 0.0));

	vw->resize = resize;
	vw->maxwidth = width;
	vw->maxheight = height;

	if (resize) {
		gtk_widget_queue_resize (GTK_WIDGET (vw));
	}
}


