#ifndef __SP_BUTTON_H__
#define __SP_BUTTON_H__

/*
 * Generic button widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

typedef struct _SPButton SPButton;
typedef struct _SPButtonClass SPButtonClass;

#define SP_TYPE_BUTTON (sp_button_get_type ())
#define SP_BUTTON(o) (GTK_CHECK_CAST ((o), SP_TYPE_BUTTON, SPButton))
#define SP_IS_BUTTON(o) (GTK_CHECK_TYPE ((o), SP_TYPE_BUTTON))

#include <gtk/gtkwidget.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtktooltips.h>

#include <helper/action.h>

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	SP_BUTTON_TYPE_NORMAL,
	SP_BUTTON_TYPE_TOGGLE
} SPButtonType;

struct _SPBChoiceData {
	guchar *px;
};

struct _SPButton {
	GtkToggleButton widget;
	SPButtonType type;
	unsigned int size;
	SPAction *action;
	GtkTooltips *tooltips;
};

struct _SPButtonClass {
	GtkToggleButtonClass parent_class;
};

#define SP_BUTTON_IS_DOWN(b) gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))

GType sp_button_get_type (void);

GtkWidget *sp_button_new (unsigned int size, SPButtonType type, SPAction *action, GtkTooltips *tooltips);

void sp_button_toggle_set_down (SPButton *button, gboolean down);

GtkWidget *sp_button_new_from_data (unsigned int size, SPButtonType type,
				    const gchar *name,
				    const gchar *tip,
				    GtkTooltips *tooltips);

G_END_DECLS

#endif
