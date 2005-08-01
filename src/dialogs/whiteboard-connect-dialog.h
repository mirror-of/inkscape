#ifndef __SP_WHITEBOARD_CONNECT_DIALOG_H__
#define __SP_WHITEBOARD_CONNECT_DIALOG_H__

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

GtkWidget * sp_whiteboard_connect_dialog(gchar *msgText);

void sp_whiteboard_connect_new_button (GtkWidget *dlg, GtkWidget *hb, const gchar *label, GtkTooltips *tt, const gchar *tip, void (*function) (GObject *, GObject *));
void sp_whiteboard_connect_new_searchfield (GtkWidget *dlg, GtkWidget *vb, const gchar *label, const gchar *id, GtkTooltips *tt, const gchar *tip);
void sp_whiteboard_connect_dialog_do_connect (GObject *object, GObject *dlg);

/** 
 * Connection dialog tracking facilities
 */
static GtkWidget *connect_dialog = NULL;
static win_data connect_dialog_wd;

static gboolean
sp_whiteboard_connect_dialog_delete(GtkObject *, GdkEvent *, gpointer data)
{
	return FALSE;
}

static void
sp_whiteboard_connect_dialog_destroy(GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, object);
    connect_dialog_wd.win = connect_dialog = NULL;
    connect_dialog_wd.stop = 0;
}


#endif
