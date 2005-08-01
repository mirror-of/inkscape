/**
 * Jabber share with chatroom dialog
 *
 * Authors:
 * 	David Yip <yipdw@rose-hulman.edu> (cleanup, Inkscape integration)
 *
 * 	Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __SP_WHITEBOARD_SHAREWITHCHAT_DIALOG_H__
#define __SP_WHITEBOARD_SHAREWITHCHAT_DIALOG_H__

#include <gtk/gtk.h>
#include <gtk/gtkstyle.h>
#include <gtkmm/container.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/image.h>
#include <gtkmm/entry.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>

#include <glibmm.h>

#include "inkscape.h"
#include "macros.h"
#include "dialogs/dialog-events.h"

GtkWidget* sp_whiteboard_sharewithchat_dialog(gchar *msgText);
void sp_whiteboard_connect_dialog_sharewithchat(GObject *object, GObject *dlg);

/** 
 * Dialog tracking facilities
 */
static GtkWidget* sharewithchat_dialog = NULL;
static win_data sharewithchat_wd;

static gboolean
sp_whiteboard_sharewithchat_dialog_delete(GtkObject *, GdkEvent *, gpointer data)
{
	return FALSE;
}

static void
sp_whiteboard_sharewithchat_dialog_destroy(GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, object);
    sharewithchat_wd.win = sharewithchat_dialog = NULL;
    sharewithchat_wd.stop = 0;
}

#endif
