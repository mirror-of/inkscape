/*
 * Common callbacks for spinbuttons
 *
 * Authors:
 *   bulia byak <bulia@users.sourceforge.net>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

gboolean spinbutton_focus_in (GtkWidget *w, GdkEventKey *event, gpointer data);
void spinbutton_undo (GtkWidget *w);
gboolean spinbutton_keypress (GtkWidget *w, GdkEventKey *event, gpointer data);
void spinbutton_defocus (GtkObject *container);
