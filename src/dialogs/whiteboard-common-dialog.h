/** 
 * Common routines for whiteboard dialogs
 * Authors:
 * 	Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock
 * 	(original authors)
 * 	David Yip <yipdw@rose-hulman.edu> (cleanup, Inkscape integration)
 *
 * 	Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __SP_WHITEBOARD_COMMON_DIALOG_H__
#define __SP_WHITEBOARD_COMMON_DIALOG_H__

#include <gtk/gtk.h>
#include <gtk/gtkstyle.h>
#include <gtkmm/container.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/image.h>
#include <gtkmm/entry.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>

#include <glibmm.h>

void sp_whiteboard_connect_dialog_Cancel(GObject *object, GObject *dlg);
void sp_whiteboard_connect_dialog_reset (GObject *, GObject *dlg);
void sp_whiteboard_connect_reset_searchfield (GObject *dlg, const gchar *field);
void close_dialog (GtkWidget *target);

#define WHITEBOARDCONNECT_LABELWIDTH 80

#endif
