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

#include "forward.h"
#include <verbs.h>


/**
 *  Create a new document window.
 */
void sp_create_window (SPViewWidget *vw, gboolean editable);

/**
 *
 */
void sp_ui_new_view (GtkWidget *widget);

/**
 *
 */
void sp_ui_new_view_preview (GtkWidget *widget);

/**
 *
 */
void sp_ui_close_view (GtkWidget *widget);

/**
 *
 */
unsigned int sp_ui_close_all (void);

/**
 *
 */
GtkWidget *sp_ui_main_menubar (SPView *view);

/**
 *
 */
GtkWidget *sp_ui_context_menu (SPView *v, SPItem *item);


/**
 *
 */
void sp_menu_append_recent_documents (GtkWidget *menu);


/**
 *
 */
void sp_ui_dialog_title_string (sp_verb_t verb, gchar* c);


/**
 *
 */
void sp_ui_error_dialog (const gchar * message);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
