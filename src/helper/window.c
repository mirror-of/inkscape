#define __SP_WINDOW_C__

/*
 * Generic window implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <gtk/gtkwindow.h>

#include "shortcuts.h"

#include "window.h"

static int
sp_window_key_press (GtkWidget *widgt, GdkEventKey *event)
{
	unsigned int shortcut;
	shortcut = event->keyval;
	if (event->state & GDK_SHIFT_MASK) shortcut |= SP_SHORTCUT_SHIFT_MASK;
	if (event->state & GDK_CONTROL_MASK) shortcut |= SP_SHORTCUT_CONTROL_MASK;
	if (event->state & GDK_MOD1_MASK) shortcut |= SP_SHORTCUT_ALT_MASK;
	return sp_shortcut_run (shortcut);
}

GtkWidget *
sp_window_new (const unsigned char *title, unsigned int resizeable)
{
	GtkWidget *window;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title ((GtkWindow *) window, title);
	gtk_window_set_resizable ((GtkWindow *) window, resizeable);
	g_signal_connect_after ((GObject *) window, "key_press_event", (GCallback) sp_window_key_press, NULL);

	return window;
}


