#ifndef __SP_DISPLAY_SETTINGS_H__
#define __SP_DISPLAY_SETTINGS_H__

/*
 * Display settings dialog
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#include <gtk/gtkwidget.h>

void sp_display_dialog (void);
void sp_display_dialog_apply (GtkWidget * widget);
void sp_display_dialog_close (GtkWidget * widget);

G_END_DECLS

#endif
