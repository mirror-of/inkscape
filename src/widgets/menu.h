#ifndef __SP_MENU_H__
#define __SP_MENU_H__

/*
 * Menu with sensible callbacks
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <glib.h>



#define SP_TYPE_MENU (sp_menu_get_type ())
#define SP_MENU(o) (GTK_CHECK_CAST ((o), SP_TYPE_MENU, SPMenu))
#define SP_IS_MENU(o) (GTK_CHECK_TYPE ((o), SP_TYPE_MENU))

#include <gtk/gtkmenu.h>
#include <gtk/gtktooltips.h>

struct SPMenu {
	GtkMenu menu;

	gpointer activedata;

	GtkTooltips *tt;
};

struct SPMenuClass {
	GtkMenuClass parent_class;

	void (* selected) (SPMenu *menu, gpointer itemdata);
};

GType sp_menu_get_type (void);

GtkWidget *sp_menu_new (void);
void sp_menu_append (SPMenu *menu, const gchar *name, const gchar *tip, const void *data);



#endif
