#define __SP_MENU_C__

/*
 * Menu with sensible callbacks
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <gtk/gtkmenuitem.h>

#include "helper/sp-marshal.h"

#include "menu.h"

enum {SELECTED, LAST_SIGNAL};

static void sp_menu_class_init (SPMenuClass *klass);
static void sp_menu_init (SPMenu *menu);
static void sp_menu_destroy (GtkObject *object);

static GtkMenuClass *parent_class;
static guint menu_signals[LAST_SIGNAL];

GtkType
sp_menu_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPMenu",
			sizeof (SPMenu),
			sizeof (SPMenuClass),
			(GtkClassInitFunc) sp_menu_class_init,
			(GtkObjectInitFunc) sp_menu_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_MENU, &info);
	}
	return type;
}

static void
sp_menu_class_init (SPMenuClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->destroy = sp_menu_destroy;

	menu_signals[SELECTED] = g_signal_new ("selected",
						G_OBJECT_CLASS_TYPE (object_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (SPMenuClass, selected),
						NULL, NULL,
						sp_marshal_VOID__POINTER,
						G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
sp_menu_init (SPMenu *menu)
{
}

static void
sp_menu_destroy (GtkObject *object)
{
	SPMenu *menu;

	menu = SP_MENU (object);

	/* fixme: Tooltips */

	((GtkObjectClass *) (parent_class))->destroy (object);
}

GtkWidget *
sp_menu_new (void)
{
	SPMenu *menu;

	menu = g_object_new (SP_TYPE_MENU, NULL);

	menu->tt = gtk_tooltips_new ();

	return (GtkWidget *) menu;
}

static void
sp_menu_item_activate (GObject *object, SPMenu *menu)
{
	menu->activedata = g_object_get_data (object, "itemdata");

	g_signal_emit (G_OBJECT (menu), menu_signals[SELECTED], 0, menu->activedata);
}

void
sp_menu_append (SPMenu *menu, const unsigned char *label, const unsigned char *tip, const void *data)
{
	GtkWidget *mi;
	GList *cl;

	mi = gtk_menu_item_new_with_label (label);
	gtk_widget_show (mi);
	g_object_set_data (G_OBJECT (mi), "itemdata", (gpointer) data);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_tooltips_set_tip (menu->tt, GTK_WIDGET (mi), tip, NULL);
	g_signal_connect (G_OBJECT (mi), "activate", G_CALLBACK (sp_menu_item_activate), menu);

	/* Set data if we are the first item */
	cl = gtk_container_get_children (GTK_CONTAINER (menu));
	if (cl && !cl->next) menu->activedata = (gpointer) data;
	g_list_free (cl);
}

