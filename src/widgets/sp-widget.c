#define __SP_WIDGET_C__

/*
 * Abstract base class for dynamic control widgets
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtksignal.h>
#include "macros.h"
#include "../xml/repr-private.h"
#include "../sodipodi.h"
#include "../desktop.h"
#include "../desktop-handles.h"
#include "../selection.h"
#include "../document.h"
#include "sp-widget.h"

enum {
	CONSTRUCT,
	MODIFY_SELECTION,
	CHANGE_SELECTION,
	SET_SELECTION,
	ATTR_CHANGED,
	SET_DIRTY,
	LAST_SIGNAL
};

static void sp_widget_class_init (SPWidgetClass *klass);
static void sp_widget_init (SPWidget *widget);

static void sp_widget_destroy (GtkObject *object);

static void sp_widget_show (GtkWidget *widget);
static void sp_widget_hide (GtkWidget *widget);
/*  static void sp_widget_draw (GtkWidget *widget, GdkRectangle *area); */
static gint sp_widget_expose (GtkWidget *widget, GdkEventExpose *event);
static void sp_widget_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void sp_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static void sp_widget_modify_selection (Sodipodi *sodipodi, SPSelection *selection, guint flags, SPWidget *spw);
static void sp_widget_change_selection (Sodipodi *sodipodi, SPSelection *selection, SPWidget *spw);
static void sp_widget_set_selection (Sodipodi *sodipodi, SPSelection *selection, SPWidget *spw);

static GtkBinClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

static void
spw_repr_destroy (SPRepr *repr, gpointer data)
{
	g_warning ("Oops! Repr destroyed while SPWidget still present");
}

static void
spw_repr_attr_changed (SPRepr *repr, const guchar *key, const guchar *oldval, const guchar *newval, gpointer data)
{
	g_signal_emit (G_OBJECT (data), signals[ATTR_CHANGED], 0, key, oldval, newval);
}

static void
spw_repr_content_changed (SPRepr *repr, const guchar *oldval, const guchar *newval, gpointer data)
{
	/* Signalling goes here */
}

SPReprEventVector spw_event_vector = {
	spw_repr_destroy,
	NULL, /* Add child */
	NULL, /* Child added */
	NULL, /* Remove child */
	NULL, /* Child removed */
	NULL, /* Change attr */
	spw_repr_attr_changed,
	NULL, /* Change content */
	spw_repr_content_changed,
	NULL, /* Change_order */
	NULL /* Order changed */
};

GtkType
sp_widget_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		static const GtkTypeInfo info = {
			"SPWidget",
			sizeof (SPWidget),
			sizeof (SPWidgetClass),
			(GtkClassInitFunc) sp_widget_class_init,
			(GtkObjectInitFunc) sp_widget_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_BIN, &info);
	}
	return type;
}

static void
sp_widget_class_init (SPWidgetClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_BIN);

	object_class->destroy = sp_widget_destroy;

	signals[CONSTRUCT] =        gtk_signal_new ("construct",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, construct),
						    gtk_marshal_NONE__NONE,
						    GTK_TYPE_NONE, 0);
	signals[CHANGE_SELECTION] = gtk_signal_new ("change_selection",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, change_selection),
						    gtk_marshal_NONE__POINTER,
						    GTK_TYPE_NONE, 1,
						    GTK_TYPE_POINTER);
	signals[MODIFY_SELECTION] = gtk_signal_new ("modify_selection",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, modify_selection),
						    gtk_marshal_NONE__POINTER_UINT,
						    GTK_TYPE_NONE, 2,
						    GTK_TYPE_POINTER, GTK_TYPE_UINT);
	signals[SET_SELECTION] =    gtk_signal_new ("set_selection",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, set_selection),
						    gtk_marshal_NONE__POINTER,
						    GTK_TYPE_NONE, 1,
						    GTK_TYPE_POINTER);
	signals[ATTR_CHANGED] =     gtk_signal_new ("attr_changed",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, attr_changed),
						    gtk_marshal_NONE__POINTER_POINTER_POINTER,
						    GTK_TYPE_NONE, 3,
						    GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
	signals[SET_DIRTY] =        gtk_signal_new ("set_dirty",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, set_dirty),
						    gtk_marshal_NONE__BOOL,
						    GTK_TYPE_NONE, 1,
						    GTK_TYPE_BOOL);

	widget_class->show = sp_widget_show;
	widget_class->hide = sp_widget_hide;
/*  	widget_class->draw = sp_widget_draw; */
	widget_class->expose_event = sp_widget_expose;
	widget_class->size_request = sp_widget_size_request;
	widget_class->size_allocate = sp_widget_size_allocate;
}

static void
sp_widget_init (SPWidget *spw)
{
	spw->sodipodi = NULL;
	spw->repr = NULL;

	spw->dirty = FALSE;
	spw->autoupdate = TRUE;
}

static void
sp_widget_destroy (GtkObject *object)
{
	SPWidget *spw;

	spw = (SPWidget *) object;

	if (spw->sodipodi) {
#if 1
		/* This happens in ::hide (Lauris) */
		/* It seems it does not (Lauris) */
		/* Disconnect signals */
		sp_signal_disconnect_by_data (sodipodi, spw);
#endif
		spw->sodipodi = NULL;
	}

	if (spw->repr) {
#if 1
		/* This happens in ::hide (Lauris) */
		/* It seems it does not (Lauris) */
		sp_repr_remove_listener_by_data (spw->repr, spw);
#endif
		sp_repr_unref (spw->repr);
		spw->repr = NULL;
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

static void
sp_widget_show (GtkWidget *widget)
{
	SPWidget *spw;

	spw = SP_WIDGET (widget);

	if (spw->sodipodi) {
		/* Connect signals */
		g_signal_connect (G_OBJECT (sodipodi), "modify_selection", G_CALLBACK (sp_widget_modify_selection), spw);
		g_signal_connect (G_OBJECT (sodipodi), "change_selection", G_CALLBACK (sp_widget_change_selection), spw);
		g_signal_connect (G_OBJECT (sodipodi), "set_selection", G_CALLBACK (sp_widget_set_selection), spw);
	}

	if (spw->repr) {
		sp_repr_add_listener (spw->repr, &spw_event_vector, spw);
	}

	if (((GtkWidgetClass *) parent_class)->show)
		(* ((GtkWidgetClass *) parent_class)->show) (widget);
}

static void
sp_widget_hide (GtkWidget *widget)
{
	SPWidget *spw;

	spw = SP_WIDGET (widget);

	if (spw->sodipodi) {
		/* Disconnect signals */
		sp_signal_disconnect_by_data (sodipodi, spw);
	}

	if (spw->repr) {
		sp_repr_remove_listener_by_data (spw->repr, spw);
	}

	if (((GtkWidgetClass *) parent_class)->hide)
		(* ((GtkWidgetClass *) parent_class)->hide) (widget);
}

#if 0
static void
sp_widget_draw (GtkWidget *widget, GdkRectangle *area)
{
	if (((GtkBin *) widget)->child)
		gtk_widget_draw (((GtkBin *) widget)->child, area);
}
#endif

static gint
sp_widget_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkBin *bin;

	bin = GTK_BIN (widget);

	gtk_container_propagate_expose (GTK_CONTAINER(widget), bin->child, event);
	/*
	if ((bin->child) && (GTK_WIDGET_NO_WINDOW (bin->child))) {
		GdkEventExpose ce;
		ce = *event;
		gtk_widget_event (bin->child, (GdkEvent *) &ce);
	}
	*/

	return FALSE;
}

static void
sp_widget_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	if (((GtkBin *) widget)->child)
		gtk_widget_size_request (((GtkBin *) widget)->child, requisition);
}

static void
sp_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	widget->allocation = *allocation;

	if (((GtkBin *) widget)->child)
		gtk_widget_size_allocate (((GtkBin *) widget)->child, allocation);
}

/* Methods */

GtkWidget *
sp_widget_new_global (Sodipodi *sodipodi)
{
	SPWidget *spw;

	spw = gtk_type_new (SP_TYPE_WIDGET);

	if (!sp_widget_construct_global (spw, sodipodi)) {
		gtk_object_unref (GTK_OBJECT (spw));
		return NULL;
	}

	return (GtkWidget *) spw;
}

GtkWidget *
sp_widget_new_repr (SPRepr *repr)
{
	SPWidget *spw;

	g_return_val_if_fail (repr != NULL, NULL);

	spw = gtk_type_new (SP_TYPE_WIDGET);

	if (!sp_widget_construct_repr (spw, repr)) {
		gtk_object_unref (GTK_OBJECT (spw));
		return NULL;
	}

	return (GtkWidget *) spw;
}

GtkWidget *
sp_widget_construct_global (SPWidget *spw, Sodipodi *sodipodi)
{
	g_return_val_if_fail (!spw->sodipodi, NULL);

	if (spw->repr) {
		if (GTK_WIDGET_VISIBLE (spw)) {
			sp_repr_remove_listener_by_data (spw->repr, spw);
		}
		sp_repr_unref (spw->repr);
		spw->repr = NULL;
	}

	spw->sodipodi = sodipodi;
	if (GTK_WIDGET_VISIBLE (spw)) {
		g_signal_connect (G_OBJECT (sodipodi), "modify_selection", G_CALLBACK (sp_widget_modify_selection), spw);
		g_signal_connect (G_OBJECT (sodipodi), "change_selection", G_CALLBACK (sp_widget_change_selection), spw);
		g_signal_connect (G_OBJECT (sodipodi), "set_selection", G_CALLBACK (sp_widget_set_selection), spw);
	}

	g_signal_emit (G_OBJECT (spw), signals[CONSTRUCT], 0);

	return (GtkWidget *) spw;
}

GtkWidget *
sp_widget_construct_repr (SPWidget *spw, SPRepr *repr)
{
	g_return_val_if_fail (spw != NULL, NULL);
	g_return_val_if_fail (SP_IS_WIDGET (spw), NULL);
	g_return_val_if_fail (repr != NULL, NULL);

	if (spw->repr) {
		if (repr == spw->repr) return (GtkWidget *) spw;
		if (GTK_WIDGET_VISIBLE (spw)) {
			sp_repr_remove_listener_by_data (spw->repr, spw);
		}
		sp_repr_unref (spw->repr);
		spw->repr = NULL;
	}
	if (spw->sodipodi) {
		if (GTK_WIDGET_VISIBLE (spw)) {
			sp_signal_disconnect_by_data (sodipodi, spw);
		}
		spw->sodipodi = NULL;
	}
	spw->repr = repr;
	sp_repr_ref (spw->repr);
	if (GTK_WIDGET_VISIBLE (spw)) {
		sp_repr_add_listener (spw->repr, &spw_event_vector, spw);
	}

	g_signal_emit (G_OBJECT (spw), signals[CONSTRUCT], 0);

	return (GtkWidget *) spw;
}

static void
sp_widget_modify_selection (Sodipodi *sodipodi, SPSelection *selection, guint flags, SPWidget *spw)
{
	g_signal_emit (G_OBJECT (spw), signals[MODIFY_SELECTION], 0, selection, flags);
}

static void
sp_widget_change_selection (Sodipodi *sodipodi, SPSelection *selection, SPWidget *spw)
{
	g_signal_emit (G_OBJECT (spw), signals[CHANGE_SELECTION], 0, selection);
}

static void
sp_widget_set_selection (Sodipodi *sodipodi, SPSelection *selection, SPWidget *spw)
{
	/* Emit "set_selection" signal */
	g_signal_emit (G_OBJECT (spw), signals[SET_SELECTION], 0, selection);
	/* Sodipodi will force "change_selection" anyways */
}

void
sp_widget_set_dirty (SPWidget *spw, gboolean dirty)
{
	if (dirty && !spw->dirty) {
		spw->dirty = TRUE;
		if (!spw->autoupdate) {
			g_signal_emit (G_OBJECT (spw), signals[SET_DIRTY], 0, TRUE);
		}
	} else if (!dirty && spw->dirty) {
		spw->dirty = FALSE;
		if (!spw->autoupdate) {
			g_signal_emit (G_OBJECT (spw), signals[SET_DIRTY], 0, FALSE);
		}
	}
}

void
sp_widget_set_autoupdate (SPWidget *spw, gboolean autoupdate)
{
	if (autoupdate && spw->dirty) spw->dirty = FALSE;
	spw->autoupdate = autoupdate;
}

const GSList *
sp_widget_get_item_list (SPWidget *spw)
{
	g_return_val_if_fail (spw != NULL, NULL);
	g_return_val_if_fail (SP_IS_WIDGET (spw), NULL);

	if (spw->sodipodi) {
		return sp_selection_item_list (SP_DT_SELECTION (SP_ACTIVE_DESKTOP));
	}

	return NULL;
}



