#define __SP_TOOLBOX_C__

/*
 * Toolbox widget
 *
 * Authors:
 *   Frank Felfe  <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000-2002 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtktogglebutton.h>
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "icon.h"
#include "button.h"
#include "sp-toolbox.h"
#include "../helper/sp-marshal.h"

enum {
	SET_STATE,
	LAST_SIGNAL
};

static void sp_toolbox_class_init (SPToolBoxClass * klass);
static void sp_toolbox_init (SPToolBox * toolbox);
static void sp_toolbox_destroy (GtkObject * object);

static gboolean sp_toolbox_real_set_state (SPToolBox * toolbox, guint state);
static gboolean sp_toolbox_set_state_accumulator (GSignalInvocationHint *ihint, GValue *return_accu, const GValue *handler_return, gpointer data);

static void sp_toolbox_hide (GtkButton * button, gpointer data);
//static void sp_toolbox_separate (GtkButton * button, gpointer data);
static void sp_toolbox_separate (SPButton *button, SPToolBox *toolbox);
#if 0
static void sp_toolbox_close (GtkButton * button, gpointer data);
#endif
static gint sp_toolbox_delete (GtkWidget * widget, GdkEventAny * event, gpointer data);

static GtkVBoxClass * parent_class;
static guint toolbox_signals[LAST_SIGNAL] = {0};

GtkType
sp_toolbox_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPToolBox",
			sizeof (SPToolBox),
			sizeof (SPToolBoxClass),
			(GtkClassInitFunc) sp_toolbox_class_init,
			(GtkObjectInitFunc) sp_toolbox_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_VBOX, &info);
	}
	return type;
}

static void
sp_toolbox_class_init (SPToolBoxClass * klass)
{
	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;
	SPToolBoxClass * toolbox_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	toolbox_class = (SPToolBoxClass *) klass;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	toolbox_signals[SET_STATE] = g_signal_new ("set_state",
		G_TYPE_FROM_CLASS(object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (SPToolBoxClass, set_state),
		sp_toolbox_set_state_accumulator, NULL,
		sp_marshal_BOOLEAN__UINT,
		GTK_TYPE_BOOL, 1,
		GTK_TYPE_UINT);

	object_class->destroy = sp_toolbox_destroy;

/*  	widget_class->size_request = sp_toolbox_size_request; */

	toolbox_class->set_state = sp_toolbox_real_set_state;
}

static void
sp_toolbox_init (SPToolBox * toolbox)
{
	toolbox->state = 7;//SP_TOOLBOX_VISIBLE;

	toolbox->contents = NULL;
	toolbox->window = sp_window_new (NULL, FALSE);
	toolbox->windowvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (toolbox->window), toolbox->windowvbox);

	toolbox->width = 0;
	toolbox->name = NULL;
	toolbox->internalname = NULL;
}

static void
sp_toolbox_destroy (GtkObject * object)
{
	SPToolBox * toolbox;

	toolbox = (SPToolBox *) object;

	if (toolbox->internalname) {
		g_free (toolbox->internalname);
		toolbox->internalname = NULL;
	}
	if (toolbox->name) {
		g_free (toolbox->name);
		toolbox->name = NULL;
	}
	if (toolbox->window) {
		gtk_widget_destroy (toolbox->window);
		toolbox->window = NULL;
		toolbox->windowvbox = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

void
sp_toolbox_set_state (SPToolBox * toolbox, guint state)
{
	gboolean consumed;

	g_return_if_fail (toolbox != NULL);
	g_return_if_fail (SP_IS_TOOLBOX (toolbox));

	if (state == toolbox->state) return;

	gtk_signal_emit (GTK_OBJECT (toolbox), toolbox_signals[SET_STATE], state, &consumed);
}

static gboolean
sp_toolbox_set_state_accumulator (GSignalInvocationHint *ihint, GValue *return_accu, const GValue *handler_return, gpointer data)
{
	gboolean b;

	b = g_value_get_boolean (handler_return);

	return b ? FALSE : TRUE;
}

static gboolean
sp_toolbox_real_set_state (SPToolBox * toolbox, guint state)
{
	int consumed;

	if (state == toolbox->state) return TRUE; /* consumed */

	gtk_object_ref (GTK_OBJECT (toolbox));

	consumed = FALSE;

#if 0
	/* fixme: Why that whole consumed thing? (Lauris) */
	gtk_signal_emit (GTK_OBJECT (toolbox), toolbox_signals[SET_STATE], state, &consumed);
#endif

	if (!consumed) {
		if ((state & SP_TOOLBOX_STANDALONE) && (!(toolbox->state & SP_TOOLBOX_STANDALONE))) {
			gtk_widget_reparent (toolbox->contents, toolbox->windowvbox);
		} else if ((!(state & SP_TOOLBOX_STANDALONE)) && (toolbox->state & SP_TOOLBOX_STANDALONE)) {
			gtk_widget_reparent (toolbox->contents, GTK_WIDGET (toolbox));
			gtk_widget_hide (toolbox->window);
		}
		if (state & SP_TOOLBOX_VISIBLE) {
			gtk_widget_show (toolbox->contents);
			if (state & SP_TOOLBOX_STANDALONE) gtk_widget_show (toolbox->window);
		} else {
			gtk_widget_hide (toolbox->contents);
			if (state & SP_TOOLBOX_STANDALONE) gtk_widget_hide (toolbox->window);
		}
		if ((state & SP_TOOLBOX_VISIBLE) && (!(state & SP_TOOLBOX_STANDALONE))) {
			gtk_arrow_set (GTK_ARROW (toolbox->arrow), GTK_ARROW_DOWN, GTK_SHADOW_OUT);
		} else {
			gtk_arrow_set (GTK_ARROW (toolbox->arrow), GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
		}
		if (state & SP_TOOLBOX_STANDALONE) {
			sp_button_toggle_set_down (SP_BUTTON (toolbox->standalonetoggle), TRUE, FALSE);
#if 0
			gtk_button_set_relief (GTK_BUTTON (toolbox->standalonetoggle), GTK_RELIEF_NORMAL);
#endif
		} else {
			sp_button_toggle_set_down (SP_BUTTON (toolbox->standalonetoggle), FALSE, FALSE);
#if 0
			gtk_button_set_relief (GTK_BUTTON (toolbox->standalonetoggle), GTK_RELIEF_NONE);
#endif
		}
		toolbox->state = state;
	}

	if ((state & SP_TOOLBOX_STANDALONE) && (!(toolbox->state & SP_TOOLBOX_STANDALONE))) {
       		gtk_widget_reparent (toolbox->contents, toolbox->windowvbox);
       	} else if ((!(state & SP_TOOLBOX_STANDALONE)) && (toolbox->state & SP_TOOLBOX_STANDALONE)) {
		gtk_widget_reparent (toolbox->contents, GTK_WIDGET (toolbox));
		gtk_widget_hide (toolbox->window);
	}

	if (state & SP_TOOLBOX_VISIBLE) {
		gtk_widget_show (toolbox->contents);
		if (state & SP_TOOLBOX_STANDALONE) gtk_widget_show (toolbox->window);
	} else {
		gtk_widget_hide (toolbox->contents);
		if (state & SP_TOOLBOX_STANDALONE) gtk_widget_hide (toolbox->window);
	}

	if ((state & SP_TOOLBOX_VISIBLE) && (!(state & SP_TOOLBOX_STANDALONE))) {
		gtk_arrow_set (GTK_ARROW (toolbox->arrow), GTK_ARROW_DOWN, GTK_SHADOW_OUT);
	} else {
		gtk_arrow_set (GTK_ARROW (toolbox->arrow), GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
	}

	if (state & SP_TOOLBOX_STANDALONE) {
		sp_button_toggle_set_down (SP_BUTTON (toolbox->standalonetoggle), TRUE, FALSE);
#if 0
		gtk_button_set_relief (GTK_BUTTON (toolbox->standalonetoggle), GTK_RELIEF_NORMAL);
#endif
	} else {
		sp_button_toggle_set_down (SP_BUTTON (toolbox->standalonetoggle), FALSE, FALSE);
#if 0
		gtk_button_set_relief (GTK_BUTTON (toolbox->standalonetoggle), GTK_RELIEF_NONE);
#endif
	}
	toolbox->state = state;

	gtk_object_unref (GTK_OBJECT (toolbox));

	return TRUE;		/* consumed */
}

GtkWidget *
sp_toolbox_new (GtkWidget * contents, const gchar * name, const gchar * internalname, const gchar * pixmapname,
		GtkTooltips *tt)
{
	SPToolBox * t;
	GtkWidget * hbox, * hbb, * b, * w;
	gchar c[256];

	g_return_val_if_fail (contents != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (contents), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (internalname != NULL, NULL);
	g_return_val_if_fail (pixmapname != NULL, NULL);
	
	if (!tt)
	  tt = gtk_tooltips_new ();
	
	t = gtk_type_new (SP_TYPE_TOOLBOX);

	t->contents = contents;
	t->name = g_strdup (name);
	t->internalname = g_strdup (internalname);

	/* Main widget */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (t), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	/* Hide button */
	b = gtk_button_new  ();
	gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, TRUE, 0);
	gtk_widget_show (b);
	hbb = gtk_hbox_new (FALSE,0);
	gtk_container_add (GTK_CONTAINER (b), hbb);
	gtk_widget_show (hbb);
	w = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (hbb), w, FALSE, FALSE, 2);
	gtk_widget_show (w);
	t->arrow = w;
	w = sp_icon_new (SP_ICON_SIZE_TITLEBAR, pixmapname);
	gtk_box_pack_start (GTK_BOX (hbb), w, FALSE, FALSE, 0);
	gtk_widget_show (w);
	w = gtk_label_new (t->name);
	gtk_box_pack_start (GTK_BOX (hbb), w, FALSE, FALSE, 4);
	gtk_widget_show (w);
	w = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (hbb), w, TRUE, TRUE, 0);
	gtk_widget_show (w);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_toolbox_hide), t);
	/* Separate button */
	b = sp_button_new_from_data (SP_ICON_SIZE_TITLEBAR,
				     SP_BUTTON_TYPE_TOGGLE,
				     "seperate_tool",
				     _("Toggle separate window and main toolbox placement"),
				     tt);
#if 0
        b = gtk_toggle_button_new ();
#endif
	gtk_box_pack_start (GTK_BOX (hbox), b, FALSE, FALSE, 0);
	gtk_widget_show (b);
        t->standalonetoggle = b;
#if 0
	w = sp_icon_new (SP_ICON_SIZE_TITLEBAR, "separate_tool");
	gtk_container_add (GTK_CONTAINER (b), w);
	gtk_widget_show (w);
#endif
        gtk_signal_connect (GTK_OBJECT (b), "toggled", GTK_SIGNAL_FUNC (sp_toolbox_separate), t);
	/* Contents */
	gtk_box_pack_start (GTK_BOX (t), contents, TRUE, TRUE, 0);
	gtk_widget_show (contents);

	/* Window */
	gtk_window_set_title (GTK_WINDOW (t->window), t->name);
	gtk_window_set_policy (GTK_WINDOW (t->window), FALSE, FALSE, FALSE);
	g_snprintf (c, 256, "toolbox_%s", t->internalname);
	gtk_window_set_wmclass (GTK_WINDOW (t->window), c, "Sodipodi");
/*  	gnome_window_icon_set_from_default (GTK_WINDOW (t->window)); */
	gtk_signal_connect (GTK_OBJECT (t->window), "delete_event", GTK_SIGNAL_FUNC (sp_toolbox_delete), t);
	/* Window vbox */
	gtk_widget_show (t->windowvbox);
#if 0
	/* Close button */
	b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_box_pack_end (GTK_BOX (t->windowvbox), b, TRUE, TRUE, 0);
	gtk_widget_show (b);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_toolbox_close), t);
#endif

	return GTK_WIDGET (t);
}

static void
sp_toolbox_hide (GtkButton * button, gpointer data)
{
	SPToolBox * toolbox;
	guint newstate;

	toolbox = SP_TOOLBOX (data);

	//if (toolbox->state & SP_TOOLBOX_STANDALONE) {
	//	newstate = SP_TOOLBOX_VISIBLE;
	if (toolbox->state == SP_TOOLBOX_VISIBLE) {
		newstate = 0;
	} else {
        //	newstate = toolbox->state ^ SP_TOOLBOX_VISIBLE;
		newstate = SP_TOOLBOX_VISIBLE;
	}

	sp_toolbox_set_state (toolbox, newstate);
}

static void
//sp_toolbox_separate (GtkButton * button, gpointer data)
sp_toolbox_separate (SPButton *button, SPToolBox *toolbox)
{
	guint newstate;

	//if ((toolbox->state & SP_TOOLBOX_STANDALONE) && (toolbox->state & SP_TOOLBOX_VISIBLE)) {
	if (toolbox->state & SP_TOOLBOX_STANDALONE) {
		newstate = 0;
	} else {
		//newstate = SP_TOOLBOX_VISIBLE | SP_TOOLBOX_STANDALONE;
		newstate = SP_TOOLBOX_STANDALONE | SP_TOOLBOX_VISIBLE;
	}

	sp_toolbox_set_state (toolbox, newstate);
}

#if 0
static void
sp_toolbox_close (GtkButton * button, gpointer data)
{
	SPToolBox * toolbox;

	toolbox = SP_TOOLBOX (data);

	sp_toolbox_set_state (toolbox, 0);
}
#endif

static gint
sp_toolbox_delete (GtkWidget * widget, GdkEventAny * event, gpointer data)
{
	SPToolBox * toolbox;

	toolbox = SP_TOOLBOX (data);

	sp_toolbox_set_state (toolbox, 0);

	return TRUE;
}



