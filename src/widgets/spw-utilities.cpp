#define __SPW_UTILITIES_C__

/* 
 * Inkscape Widget Utilities
 * 
 * Authors:
 *   Bryce W. Harrington <brycehar@bryceharrington.com>
 *
 * Copyright (C) 2003 Bryce W. Harrington
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <math.h>
#include <stdlib.h>
#include <libnr/nr-macros.h>

#include <gtk/gtksignal.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtklabel.h>

#include <glib.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkhscale.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkframe.h>
 
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "widgets/button.h"
#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-item-transform.h"
#include "selection.h"

#include "helper/unit-menu.h"
#include "spw-utilities.h"

/** 
 * Creates a label widget with the given text, at the given col, row 
 * position in the table.
 */
GtkWidget *
spw_label(GtkWidget * table, const gchar *label_text, int col, int row)
{
  GtkWidget *label_widget;

  label_widget = gtk_label_new (label_text);
  g_assert(label_widget != NULL);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 1.0, 0.5);
  gtk_widget_show (label_widget);
  gtk_table_attach (GTK_TABLE (table), label_widget, col, col+1, row, row+1, 
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 4, 0);
  return label_widget;
}

/**
 * Creates a horizontal layout manager with 4-pixel spacing between children
 * and space for 'width' columns.
 */
GtkWidget *
spw_hbox(GtkWidget * table, int width, int col, int row)
{
  GtkWidget *hb;
  /* Create a new hbox with a 4-pixel spacing between children */
  hb = gtk_hbox_new (FALSE, 4);
  g_assert(hb != NULL);
  gtk_widget_show (hb);
  gtk_table_attach (GTK_TABLE (table), hb, col, col+width, row, row+1, 
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  return hb;
}

/**
 * Creates a checkbutton widget.  This is a compound widget that includes a
 * label.
 */
GtkWidget *
spw_checkbutton(GtkWidget * dialog, GtkWidget * table,
		const gchar * label, gchar * key, int col, int row,
		int sensitive, GCallback cb)
{
  GtkWidget *b;

  g_assert(dialog != NULL);
  g_assert(table  != NULL);

  b = gtk_check_button_new_with_label (label);
  g_assert(b != NULL);
  gtk_widget_show (b);
  gtk_table_attach (GTK_TABLE (table), b, col, col+1, row, row+1,
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  gtk_object_set_data (GTK_OBJECT (b), "key", key);
  gtk_object_set_data (GTK_OBJECT (dialog), key, b);
  g_signal_connect (G_OBJECT (b), "toggled",
		    cb, dialog);
  if (sensitive == 1) {
    gtk_widget_set_sensitive (b, FALSE);
  }
  return b;
}

/**
 * Creates a dropdown widget.  This is a compound widget that includes
 * a label as well as the dropdown.
 */
GtkWidget *
spw_dropdown(GtkWidget * dialog, GtkWidget * table,
	     const gchar * label_text, gchar * key, int row,
	     GtkWidget * selector
	     )
{
  g_assert(dialog   != NULL);
  g_assert(table    != NULL);
  g_assert(selector != NULL);

  spw_label(table, label_text, 0, row);

  gtk_widget_show (selector);
  gtk_table_attach (GTK_TABLE (table), selector, 1, 2, row, row+1, 
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  gtk_object_set_data (GTK_OBJECT (dialog), key, selector);
  return selector;
}

/**
 * Creates a unit selector widget, used for selecting whether one wishes
 * to measure screen elements in millimeters, points, etc.  This is a 
 * compound unit that includes a label as well as the dropdown selector.
 */
GtkWidget *
spw_unit_selector(GtkWidget * dialog, GtkWidget * table,
		  const gchar * label_text, gchar * key, int row,
		  GtkWidget * us, GCallback cb)
{
  GtkWidget * sb;
  GtkObject * a;

  g_assert(dialog != NULL);
  g_assert(table  != NULL);
  g_assert(us     != NULL);

  spw_label(table, label_text, 0, row);

  a = gtk_adjustment_new (0.0, -1e6, 1e6, 1.0, 10.0, 10.0);
  g_assert(a != NULL);
  gtk_object_set_data (GTK_OBJECT (a), "key", key);
  gtk_object_set_data (GTK_OBJECT (a), "unit_selector", us);
  gtk_object_set_data (GTK_OBJECT (dialog), key, a);
  sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), GTK_ADJUSTMENT (a));
  sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 2);
  g_assert(sb != NULL);
  gtk_widget_show (sb);
  gtk_table_attach (GTK_TABLE (table), sb, 1, 2, row, row+1, 
		    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
  g_signal_connect (G_OBJECT (a), "value_changed", cb, dialog);
  return sb;
}

void
sp_set_font_size_recursive (GtkWidget *w, gpointer font)
{
	guint size = GPOINTER_TO_UINT (font);

	PangoFontDescription* pan = pango_font_description_new ();
	pango_font_description_set_size (pan, size);

	gtk_widget_modify_font (w, pan);

	if (GTK_IS_CONTAINER(w)) {
		gtk_container_foreach (GTK_CONTAINER(w), (GtkCallback) sp_set_font_size_recursive, font);
	}

	pango_font_description_free (pan);
}

void
sp_set_font_size (GtkWidget *w, guint font)
{
	sp_set_font_size_recursive (w, GUINT_TO_POINTER(font));
}

/**
\brief  Returns the descendant of w which has the data with the given key, or NULL if there's none
*/
gpointer
sp_search_by_data_recursive (GtkWidget *w, gpointer key)
{
	gpointer r = NULL;

	if (w && GTK_IS_OBJECT(w)) {
		r = gtk_object_get_data (GTK_OBJECT(w), (gchar *) key);
	}
	if (r) return r;
	
	if (GTK_IS_CONTAINER(w)) {
		GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
		for (GList *i = ch; i != NULL; i = i->next) {
			r = sp_search_by_data_recursive(GTK_WIDGET(i->data), key);
			if (r) return r;
		}
	}

	return NULL;
}

