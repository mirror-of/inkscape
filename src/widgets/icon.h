#ifndef __SP_ICON_H__
#define __SP_ICON_H__

/*
 * Generic icon widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>



typedef struct _SPIcon SPIcon;
typedef struct _SPIconClass SPIconClass;

#define SP_TYPE_ICON (sp_icon_get_type ())
#define SP_ICON(o) (GTK_CHECK_CAST ((o), SP_TYPE_ICON, SPIcon))
#define SP_IS_ICON(o) (GTK_CHECK_TYPE ((o), SP_TYPE_ICON))

#define SP_ICON_SIZE_BORDER 8
#define SP_ICON_SIZE_BUTTON 16
#define SP_ICON_SIZE_MENU 12
#define SP_ICON_SIZE_TITLEBAR 12
#define SP_ICON_SIZE_NOTEBOOK 20

#include <gtk/gtkwidget.h>

#define SP_ICON_FLAG_STATIC_DATA (1 << 24)

struct _SPIcon {
	GtkWidget widget;

	int size;

	guchar *px;
};

struct _SPIconClass {
	GtkWidgetClass parent_class;
};

GType sp_icon_get_type (void);

GtkWidget *sp_icon_new (unsigned int size, const gchar *name);
GtkWidget *sp_icon_new_scaled (unsigned int size, const gchar *name);
GtkWidget *sp_icon_new_from_data (unsigned int size, const guchar *px);

/* This is unrelated, but can as well be here */

guchar *sp_icon_image_load (const gchar *name, unsigned int size, unsigned int scale);
guchar *sp_icon_image_load_gtk (GtkWidget *widget, const gchar *name, unsigned int size, unsigned int scale);



#endif
