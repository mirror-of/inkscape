#ifndef __SP_FILL_STYLE_H__
#define __SP_FILL_STYLE_H__

/*
 * Fill style configuration
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include <gtk/gtkwidget.h>

#include "forward.h"

GtkWidget *sp_fill_style_widget_new (void);

void sp_fill_style_dialog (void);

void sp_fill_style_widget_system_color_set (GtkWidget *widget, SPColor *color, float opacity);

#endif
