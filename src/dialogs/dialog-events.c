#define __DIALOG_EVENTS_C__

/*
 * Event handler for dialog windows
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <time.h>
#include "helper/window.h"
#include "widgets/sp-widget.h"
#include "macros.h"
#include "inkscape.h"
#include <gtk/gtk.h>
#include "desktop-events.h"
#include "desktop.h"
#include "inkscape-private.h"

#include "dialog-events.h"

// this function is called to zero the transientize semaphore by a timeout
gboolean 
sp_allow_again (gpointer *wd) 
{
	((win_data *) wd)->stop = 0;
	return FALSE; // so that it is only called once
}

gboolean
sp_dialog_event_handler (GtkWindow *win, GdkEvent *event, gpointer data)
{
	gboolean ret = FALSE; 
	GtkWindow *w;
	SPDesktopWidget *dtw;

	switch (event->type) {
	case GDK_KEY_PRESS:
		switch (event->key.keyval) {
		case GDK_Escape: 
			// send focus to the canvas
			if (w = gtk_window_get_transient_for ((GtkWindow *) win)) {
				gtk_window_present (w);
			}
			ret = TRUE; 
			break;
		case GDK_F4:
		case GDK_w:
		case GDK_W:
			// close dialog
			if (MOD__CTRL_ONLY) {
				// this code sends a delete_event to the dialog, instead of just destroying it,
				// so that the dialog can do some housekeeping, such as remember its position
				GdkEventAny event;
				GtkWidget *widget = (GtkWidget *) win;
				event.type = GDK_DELETE;
				event.window = widget->window;
				event.send_event = TRUE;
				g_object_ref (G_OBJECT (event.window));
				gtk_main_do_event ((GdkEvent*)&event);
				g_object_unref (G_OBJECT (event.window));

				ret = TRUE; 
			}
			break;
		default: // pass keypress to the canvas
			if (w = gtk_window_get_transient_for ((GtkWindow *) win)) {
				dtw = (SPDesktopWidget *)g_object_get_data (G_OBJECT (w), "desktopwidget");
				inkscape_activate_desktop (dtw->desktop);
				gtk_propagate_event (GTK_WIDGET (dtw->canvas), event);
				ret = TRUE; 
			}
			break;
		}
	}
	return ret; 
}

void
sp_transientize (GtkWidget *dialog)
{
	// if there's an active canvas, attach dialog to it as a transient:
	if (SP_ACTIVE_DESKTOP && g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window")) 
		gtk_window_set_transient_for ((GtkWindow *) dialog, (GtkWindow *) g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window"));
}

void
sp_transientize_callback (Inkscape *inkscape, SPDesktop *desktop, win_data *wd)
{
	if (wd->stop) { // if retransientizing of this dialog is still forbidden after previous call
		// warning turned off because it was confusingly fired when loading many files from command line
		//		g_warning("Retranzientize aborted! You're switching windows too fast!"); 
		return;
	}
	GtkWindow *w;

	w = (GtkWindow *) g_object_get_data (G_OBJECT (desktop), "window"); 
	if (w && wd->win) {
		wd->stop = 1; // disallow other attempts to retranzientize this dialog
		gtk_window_set_transient_for ((GtkWindow *) wd->win, w);
		gtk_window_present (w); // without this, a transient window not always emerges on top
	}
	// we're done, allow next retransientizing not sooner than after 10 msec
	gtk_timeout_add (6, (GtkFunction) sp_allow_again, (gpointer) wd);  
}

