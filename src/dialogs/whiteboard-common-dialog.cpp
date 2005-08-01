/** 
 * Common routines for whiteboard dialogs
 * Authors:
 * 	Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock
 * 	(original authors)
 * 	David Yip <yipdw@rose-hulman.edu> (cleanup, Inkscape integration)
 *
 * 	Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "dialogs/whiteboard-common-dialog.h"

#include "dialogs/dockable.h"
#include "dialogs/docker.h"

#include "helper/window.h"
#include "helper/unit-menu.h"

#include "widgets/icon.h"

#include "macros.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "dialog-events.h"
#include "document.h"
#include "inkscape.h"
#include "interface.h"
#include "prefs-utils.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "verbs.h"


void
sp_whiteboard_connect_dialog_reset (GObject *, GObject *dlg)
{
    sp_whiteboard_connect_reset_searchfield (dlg, "server");
    sp_whiteboard_connect_reset_searchfield (dlg, "username");
    sp_whiteboard_connect_reset_searchfield (dlg, "password");
}

//This triggers a Delete event for the target dialog. It is useful in cases in which dialogs
//should shut down after performing a task.
void
close_dialog (GtkWidget *target)
{
	GdkEventAny event;
    event.type = GDK_DELETE;
    event.window = target->window;
    event.send_event = TRUE;
    g_object_ref (G_OBJECT (event.window));
    gtk_main_do_event ((GdkEvent*)&event);
    g_object_unref (G_OBJECT (event.window));
}

void
sp_whiteboard_connect_reset_searchfield (GObject *dlg, const gchar *field)
{
    GtkWidget *widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), field));
    gtk_entry_set_text (GTK_ENTRY(widget), "");
}



void
sp_whiteboard_connect_dialog_Cancel(GObject *object, GObject *dlg)
{
	close_dialog(GTK_WIDGET(dlg));    
}

