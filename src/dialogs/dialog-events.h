#ifndef __DIALOG_EVENTS_H__
#define __DIALOG_EVENTS_H__

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

gboolean sp_dialog_event_handler (GtkWindow *win, GdkEvent *event, gpointer data);
void sp_transientize (GtkWidget *win);
void sp_transientize_callback (Inkscape *inkscape, SPDesktop *desktop, GtkWidget *dialog);

#endif
