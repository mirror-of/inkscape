#define __SPW_UTILITIES_C__

/* 
 * Sodipodi Widget Utilities
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
#include "sodipodi.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-item-transform.h"
#include "selection.h"

#include "helper/unit-menu.h"
#include "spw-utilities.h"

void
spw_checkbutton(GtkWidget * dialog, GtkWidget * t,
		const guchar * label, guchar * key, int col, int row,
		int sensitive, GCallback cb) {
  GtkWidget *b;
  b = gtk_check_button_new_with_label (label);
  gtk_widget_show (b);
  gtk_table_attach (GTK_TABLE (t), b, col, col+1, row, row+1,
		    GTK_EXPAND | GTK_FILL, 0, 0, 0);
  gtk_object_set_data (GTK_OBJECT (b), "key", key);
  gtk_object_set_data (GTK_OBJECT (dialog), key, b);
  g_signal_connect (G_OBJECT (b), "toggled",
		    cb, dialog);
  if (sensitive == 1) {
    gtk_widget_set_sensitive (b, FALSE);
  }
}

void
spw_dropdown(GtkWidget * dialog, GtkWidget * t,
	     const guchar * label, guchar * key, int row,
	     GtkWidget * selector
	     )
{
  GtkWidget * l;
  l = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
  gtk_widget_show (l);
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  gtk_widget_show (selector);
  gtk_table_attach (GTK_TABLE (t), selector, 1, 2, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  gtk_object_set_data (GTK_OBJECT (dialog), key, selector);
}

void
spw_unit_selector(GtkWidget * dialog, GtkWidget * t,
		  const guchar * label, guchar * key, int row,
		  GtkWidget * us, GCallback cb)
{
  GtkWidget * l, * sb;
  GtkObject * a;
  l = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
  gtk_widget_show (l);
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  a = gtk_adjustment_new (0.0, -1e6, 1e6, 1.0, 10.0, 10.0);
  gtk_object_set_data (GTK_OBJECT (a), "key", key);
  gtk_object_set_data (GTK_OBJECT (a), "unit_selector", us);
  gtk_object_set_data (GTK_OBJECT (dialog), key, a);
  sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), GTK_ADJUSTMENT (a));
  sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 2);
  gtk_widget_show (sb);
  gtk_table_attach (GTK_TABLE (t), sb, 1, 2, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  g_signal_connect (G_OBJECT (a), "value_changed", cb, dialog);
}


