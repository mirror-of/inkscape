#ifndef __SP_INTERFACE_H__
#define __SP_INTERFACE_H__

/*
 * Main UI stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "view.h"

void sp_create_window (SPViewWidget *vw, gboolean editable);

void sp_ui_new_view (GtkWidget *widget);
void sp_ui_new_view_preview (GtkWidget *widget);
void sp_ui_close_view (GtkWidget *widget);

unsigned int sp_ui_close_all (void);

/* I am not sure, what is the right place for that (Lauris) */

GtkWidget *sp_ui_main_menu (void);
GtkWidget *sp_ui_generic_menu (SPView *v, SPItem *item);

void sp_menu_append_recent_documents (GtkWidget *menu);

#endif
