/*
 * This code builds all of the menus for the modules.  It creates them
 * so that calling code doesn't have to know that much about the menus
 * at all.  Really kinda simple, makes good use of the db functions.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <helper/sp-intl.h>

#include <extension.h>

#include "menu.h"
#include "system.h"
#include "db.h"

static void sp_module_menu_open_internal    (Inkscape::Extension::Extension * in_plug,
                                             gpointer   data);
static void sp_module_menu_save_internal    (Inkscape::Extension::Extension * in_plug,
                                             gpointer   data);
static void sp_module_menu_about_internal   (Inkscape::Extension::Extension * in_plug,
                                             gpointer   data);
static void sp_module_menu_filter_internal  (Inkscape::Extension::Extension * in_plug,
                                             gpointer   data);

/**
	\return   A completed menu with all the input options
	\brief    This function creates the menu that is on the open dialog.

	This function creates a new menu, and then populates it with all
	of the input modules.  First it puts in the 'Autodetect' case, and
	then a sperator.  Then it calls the module database foreach function
	with an internal function to build the rest of the menu.
*/
SPMenu *
sp_module_menu_open (void)
{
	SPMenu * menu;

	menu = SP_MENU(sp_menu_new());

	sp_menu_append(menu, _("Autodetect"), _("Use the extension of the file to choose a filetype"), SP_MODULE_KEY_AUTODETECT);
	sp_menu_append(menu, NULL, NULL, NULL);
	sp_module_db_foreach(sp_module_menu_open_internal, (gpointer)menu);

	return menu;
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an input module, then add it to the open menu.
	\param     in_plug  Module to be examined
	\param     data     The menu to be attached to

	The first thing that is checked is if this module is an input
	module.  If it is, then it is turned into a menu item.  The
	filetypename is used if possible, otherwise the function reverts
	to using the name of the module.  Also, if there is a filetypetooltip
	that is used, otherwise the tooltip is NULL.  The menu item is
	added using the sp_menu_append function.
*/
static void
sp_module_menu_open_internal (Inkscape::Extension::Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Inkscape::Extension::Input *>(in_plug)) {
		const gchar * name;
		const gchar * tooltip;
		Inkscape::Extension::Input * imod;

		imod = dynamic_cast<Inkscape::Extension::Input *>(in_plug);

		name = imod->get_filetypename();
		if (name == NULL) {
			name = in_plug->get_name();
		}

		tooltip = imod->get_filetypetooltip();

		sp_menu_append (SP_MENU(data),
						name,
						tooltip,
				        in_plug->get_id());
	}

	return;
}

/**
	\return   Menu with all of the output filters
	\brief    A function to find all the output modules and put them
	          in a nice little menu.

	This function first creates the menu, and then tags the autodetect
	key at the top.  Finally it calls the module DB function to step
	through all of them with the internal function which actually
	adds all the modules into the menu.  The created menu is returned.
*/
SPMenu *
sp_module_menu_save (void)
{
	SPMenu * menu;

	menu = SP_MENU(sp_menu_new());

	sp_menu_append(menu, _("Autodetect"), _("Use the extension of the file to choose a filetype"), SP_MODULE_KEY_AUTODETECT);
	sp_menu_append(menu, NULL, NULL, NULL);
	sp_module_db_foreach(sp_module_menu_save_internal, (gpointer)menu);

	return menu;
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an output module, then add it to the save menu.
	\param     in_plug  Module to be examined
	\param     data     The menu to be attached to

	The first thing that is checked is if this module is an output
	module.  If it is, then it is turned into a menu item.  The
	filetypename is used if possible, otherwise the function reverts
	to using the name of the module.  Also, if there is a filetypetooltip
	that is used, otherwise the tooltip is NULL.  The menu item is
	added using the sp_menu_append function.
*/
static void
sp_module_menu_save_internal (Inkscape::Extension::Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Inkscape::Extension::Output *>(in_plug)) {
		const gchar * name;
		const gchar * tooltip;
		Inkscape::Extension::Output * omod;

		omod = dynamic_cast<Inkscape::Extension::Output *>(in_plug);

		name = omod->get_filetypename();
		if (name == NULL) {
			name = in_plug->get_name();
		}

		tooltip = omod->get_filetypetooltip();

		sp_menu_append (SP_MENU(data),
						name,
						tooltip,
				        in_plug->get_id());
	}

	return;
}

/**
	\return   A menu with all the modules
	\brief    This function puts all of the modules into a menu.

	This function just creates a menu and then calls the module DB
	function to step through all of the modules.  Pretty boring really.
*/
SPMenu *
sp_module_menu_about (void)
{
	SPMenu * menu;

	menu = SP_MENU(sp_menu_new());
	sp_module_db_foreach(sp_module_menu_about_internal, (gpointer)menu);

	return menu;
}

/**
	\return   None
	\brief    Tacks the passed in module to the passed in menu.
	\param    in_plug  Incomming module
	\param    data     Menu for it to be tacked on.

	This function just calls sp_menu_append, basically it shuffles the
	parameters around.  It will do more in the future.
*/
static void
sp_module_menu_about_internal (Inkscape::Extension::Extension * in_plug, gpointer data)
{
	sp_menu_append (SP_MENU(data),
					in_plug->get_name(),
					NULL,
					in_plug->get_id());

	return;
}

/**
	\return   Filter menu
	\brief    This function creates the menu for all of the filter
	          modules.

	The first thing that happens is that the menu gets created.  Then
	there are two items added for the last used filter.  One of them
	will work to have preferences, the other won't.  Then a list of
	the modules is tacked on to the end, and then the whole menu is
	returned.
*/
GtkWidget *
sp_module_menu_filter (void)
{
	GtkWidget *     menu;
	GtkWidget *   item;

	menu = gtk_menu_new();

	/* TODO: Implement this */
	item = gtk_menu_item_new_with_mnemonic("_Filter");
	gtk_widget_show(GTK_WIDGET(item));
	gtk_menu_append(menu, GTK_WIDGET(item));
	gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);

	/* TODO: Implement this */
	item = gtk_menu_item_new_with_mnemonic("_Last Filter...");
	gtk_widget_show(GTK_WIDGET(item));
	gtk_menu_append(menu, GTK_WIDGET(item));
	gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);

	item = gtk_separator_menu_item_new ();
	gtk_widget_show(GTK_WIDGET(item));
	gtk_menu_append(menu, GTK_WIDGET(item));

	sp_module_db_foreach(sp_module_menu_filter_internal, (gpointer)menu);

	return menu;
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an filter module, then add it to the filter menu.
	\param     in_plug  Module to be examined
	\param     data     The menu to be attached to

	The first thing that is checked is if this module is an filter
	module.  If it is, then it is turned into a menu item.  The label
	is the name of the filter.  Then a signal to call the function
	sp_module_system_filter upon activiation is attached.  As a parameter
	it is passed the key of the module.
*/
static void
sp_module_menu_filter_internal (Inkscape::Extension::Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Inkscape::Extension::Filter *>(in_plug)) {
		GtkWidget * item;

		item = gtk_menu_item_new_with_label(in_plug->get_name());
		gtk_widget_show(GTK_WIDGET(item));
		gtk_menu_append(GTK_MENU(data), GTK_WIDGET(item));
		g_signal_connect(G_OBJECT(item),
		                 "activate",
                         GTK_SIGNAL_FUNC(sp_module_system_filter),
                         in_plug->get_id());
	}

	return;
}

