#ifndef __SP_ITEM_PROPERTIES_H__
#define __SP_ITEM_PROPERTIES_H__

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
#include "../forward.h"

GtkWidget *sp_item_widget_new (void);

void sp_item_dialog (void);

G_END_DECLS

#endif
