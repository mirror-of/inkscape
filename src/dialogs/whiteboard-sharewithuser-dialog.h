/**
 * Jabber share with user dialog 
 *
 * Authors:
 * 	Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock
 * 	(original authors)
 * 	David Yip <yipdw@rose-hulman.edu> (cleanup, Inkscape integration)
 *
 * 	Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __SP_WHITEBOARD_SHAREWITHUSER_DIALOG_H__
#define __SP_WHITEBOARD_SHAREWITHUSER_DIALOG_H__

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

GtkWidget* sp_whiteboard_sharewithuser_dialog(gchar *msgText);
void selection_made( GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data );
void sp_whiteboard_connect_dialog_sharewithuser(GObject *object, GObject *dlg);

/** 
 * Dialog tracking facilities
 */
static GtkWidget* sharewithuser_dialog = NULL;
static win_data sharewithuser_wd;

static gboolean
sp_whiteboard_sharewithuser_dialog_delete(GtkObject *, GdkEvent *, gpointer data)
{
	return FALSE;
}

static void
sp_whiteboard_sharewithuser_dialog_destroy(GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, object);
    sharewithuser_wd.win = sharewithuser_dialog = NULL;
    sharewithuser_wd.stop = 0;
}

#endif
