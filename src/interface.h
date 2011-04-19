#ifndef SEEN_SP_INTERFACE_H
#define SEEN_SP_INTERFACE_H

/*
 * Main UI stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkstyle.h>

#include "forward.h"
#include "sp-item.h"

/* Drag and Drop */
typedef enum {
    URI_LIST,
    SVG_XML_DATA,
    SVG_DATA,
    PNG_DATA,
    JPEG_DATA,
    IMAGE_DATA,
    APP_X_INKY_COLOR,
    APP_X_COLOR,
    APP_OSWB_COLOR,
} ui_drop_target_info;

/**
 *  Create a new document window.
 */
void sp_create_window (SPViewWidget *vw, gboolean editable);

/**
 *
 */
void sp_ui_close_view (GtkWidget *widget);

/**
 *
 */
void sp_ui_new_view (void);
void sp_ui_new_view_preview (void);

/**
 *
 */
unsigned int sp_ui_close_all (void);

/**
 *
 */
GtkWidget *sp_ui_main_menubar (Inkscape::UI::View::View *view);

/**
 *
 */
GtkWidget *sp_ui_context_menu (Inkscape::UI::View::View *v, SPItem *item);


/**
 *
 */
void sp_menu_append_recent_documents (GtkWidget *menu);


/**
 *
 */
void sp_ui_dialog_title_string (Inkscape::Verb * verb, gchar* c);


/**
 *
 */
void sp_ui_error_dialog (const gchar * message);
bool sp_ui_overwrite_file (const gchar * filename);

#endif // SEEN_SP_INTERFACE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
