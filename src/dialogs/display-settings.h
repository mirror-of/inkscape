#ifndef __SP_DISPLAY_SETTINGS_H__
#define __SP_DISPLAY_SETTINGS_H__

/**
 * \brief Display settings dialog
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>



#include <gtk/gtkwidget.h>

void sp_display_dialog       ( void );
void sp_display_dialog_apply ( GtkWidget * widget );
void sp_display_dialog_close ( GtkWidget * widget );



#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
