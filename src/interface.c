#define __SP_INTERFACE_C__

/*
 * Main UI stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "sodipodi.h"
#ifdef WITH_MODULES
#include "modules/sp-module-sys.h"
#endif /* WITH_MODULES */
#include "widgets/icon.h"

#include "verbs.h"
#include "shortcuts.h"

#include "document.h"
#include "desktop-handles.h"
#include "file.h"
#include "help.h"
#include "interface.h"
#include "toolbox.h"
#include "desktop.h"
#include "object-ui.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "zoom-context.h"
#include "event-broker.h"
#include "svg-view.h"

#include "dir-util.h"
#include "xml/repr-private.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"
#include "helper/window.h"

#include "dialogs/node-edit.h"

static gint sp_ui_delete (GtkWidget *widget, GdkEvent *event, SPView *view);

/* Drag and Drop */
typedef enum {
  URI_LIST
} ui_drop_target_info;
static GtkTargetEntry ui_drop_target_entries [] = {
  {"text/uri-list", 0, URI_LIST},
};

#define ENTRIES_SIZE(n) sizeof(n)/sizeof(n[0]) 
static guint nui_drop_target_entries = ENTRIES_SIZE(ui_drop_target_entries);
static void sp_ui_import_files(gchar * buffer);
static void sp_ui_import_one_file(gchar * filename);
static void sp_ui_import_one_file_with_check(gpointer filename, gpointer unused);
static void sp_ui_drag_data_received (GtkWidget * widget,
				      GdkDragContext * drag_context,
				      gint x, gint y,
				      GtkSelectionData * data,
				      guint info,
				      guint event_time,
				      gpointer user_data);
void
sp_create_window (SPViewWidget *vw, gboolean editable)
{
	GtkWidget *w, *hb;

	g_return_if_fail (vw != NULL);
	g_return_if_fail (SP_IS_VIEW_WIDGET (vw));

	w = sp_window_new ("", TRUE);
	g_object_set_data (G_OBJECT (vw), "window", w);
	g_object_set_data (G_OBJECT (SP_VIEW_WIDGET_VIEW (vw)), "window", w);

	hb = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hb);
	gtk_container_add (GTK_CONTAINER (w), hb);
	g_object_set_data (G_OBJECT (w), "hbox", hb);

	/* fixme: */
	if (editable) {
		gtk_window_set_default_size ((GtkWindow *) w, 400, 400);
		g_object_set_data (G_OBJECT (w), "desktop", SP_DESKTOP_WIDGET (vw)->desktop);
		g_object_set_data (G_OBJECT (w), "desktopwidget", vw);
		g_signal_connect (G_OBJECT (w), "delete_event", G_CALLBACK (sp_ui_delete), vw->view);
		g_signal_connect (G_OBJECT (w), "focus_in_event", G_CALLBACK (sp_desktop_widget_set_focus), vw);
#if 0
		sp_desktop_set_title (desktop);
#endif
	} else {
		gtk_window_set_policy (GTK_WINDOW (w), TRUE, TRUE, TRUE);
	}

	gtk_box_pack_end (GTK_BOX (hb), GTK_WIDGET (vw), TRUE, TRUE, 0);
	gtk_widget_show (GTK_WIDGET (vw));

#if 0
	gnome_window_icon_set_from_default (GTK_WINDOW (w));
#endif
	gtk_drag_dest_set(w, 
			  GTK_DEST_DEFAULT_ALL,
			  ui_drop_target_entries,
			  nui_drop_target_entries,
			  GDK_ACTION_COPY);
	g_signal_connect (G_OBJECT(w),
			   "drag_data_received",
			   G_CALLBACK (sp_ui_drag_data_received),
			   NULL);
	gtk_widget_show (w);
}

void
sp_ui_new_view (GtkWidget * widget)
{
	SPDocument * document;
	SPViewWidget *dtw;

	document = SP_ACTIVE_DOCUMENT;
	if (!document) return;

	dtw = sp_desktop_widget_new (sp_document_namedview (document, NULL));
	g_return_if_fail (dtw != NULL);

	sp_create_window (dtw, TRUE);
}

/* fixme: */
void
sp_ui_new_view_preview (GtkWidget *widget)
{
	SPDocument *document;
	SPViewWidget *dtw;

	document = SP_ACTIVE_DOCUMENT;
	if (!document) return;

	dtw = (SPViewWidget *) sp_svg_view_widget_new (document);
	g_return_if_fail (dtw != NULL);
#if 1
	sp_svg_view_widget_set_resize (SP_SVG_VIEW_WIDGET (dtw), TRUE, 400.0, 400.0);
#endif

	sp_create_window (dtw, FALSE);
}

void
sp_ui_close_view (GtkWidget * widget)
{
	GtkWidget *w;

	if (SP_ACTIVE_DESKTOP == NULL) return;
	w = g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
	if (sp_view_shutdown (SP_VIEW (SP_ACTIVE_DESKTOP))) return;
	gtk_widget_destroy (w);
}

unsigned int
sp_ui_close_all (void)
{
	while (SP_ACTIVE_DESKTOP) {
		GtkWidget *w;
		w = g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
		if (sp_view_shutdown (SP_VIEW (SP_ACTIVE_DESKTOP))) return FALSE;
		gtk_widget_destroy (w);
	}

	return TRUE;
}

static gint
sp_ui_delete (GtkWidget *widget, GdkEvent *event, SPView *view)
{
	return sp_view_shutdown (view);
}

static GtkWidget *
sp_ui_menu_append_item (GtkMenu *menu, const guchar *stock, const guchar *label, GCallback callback, gpointer data)
{
	GtkWidget *item;

	if (stock) {
		item = gtk_image_menu_item_new_from_stock (stock, NULL);
	} else if (label) {
		item = gtk_image_menu_item_new_with_label (label);
	} else {
		item = gtk_separator_menu_item_new ();
	}
	gtk_widget_show (item);
	if (callback) {
		g_signal_connect (G_OBJECT (item), "activate", callback, data);
	}
	gtk_menu_append (GTK_MENU (menu), item);

	return item;
}

static void
sp_ui_menu_activate (void *object, SPAction *action)
{
	sp_action_perform (action);
}

static void
sp_ui_menu_key_press (GtkMenuItem *item, GdkEventKey *event, void *data)
{
	if (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) {
		unsigned int shortcut, verb;
		shortcut = event->keyval;
		if (event->state & GDK_SHIFT_MASK) shortcut |= SP_SHORTCUT_SHIFT_MASK;
		if (event->state & GDK_CONTROL_MASK) shortcut |= SP_SHORTCUT_CONTROL_MASK;
		if (event->state & GDK_MOD1_MASK) shortcut |= SP_SHORTCUT_ALT_MASK;
		verb = (unsigned int) data;
		sp_shortcut_set_verb (shortcut, verb, TRUE);
	}
}

static GtkWidget *
sp_ui_menu_append_item_from_verb (GtkMenu *menu, unsigned int verb)
{
	SPAction *action;
	GtkWidget *item, *icon;

	if (verb == SP_VERB_NONE) {
		item = gtk_separator_menu_item_new ();
	} else {
		action = sp_verb_get_action (verb);
		if (!action) return NULL;
		if (action->shortcut) {
			unsigned char c[256];
			unsigned char *as, *cs, *ss;
			GtkWidget *hb, *l;
			as = (action->shortcut & SP_SHORTCUT_ALT_MASK) ? "Alt+" : "";
			cs = (action->shortcut & SP_SHORTCUT_CONTROL_MASK) ? "Ctrl+" : "";
			ss = (action->shortcut & SP_SHORTCUT_SHIFT_MASK) ? "Shift+" : "";
			g_snprintf (c, 256, "%s%s%s%s", as, cs, ss, gdk_keyval_name (action->shortcut & 0xffffff));
			hb = gtk_hbox_new (FALSE, 16);
			l = gtk_label_new (action->name);
			gtk_misc_set_alignment ((GtkMisc *) l, 0.0, 0.5);
			gtk_box_pack_start ((GtkBox *) hb, l, TRUE, TRUE, 0);
			l = gtk_label_new (c);
			gtk_misc_set_alignment ((GtkMisc *) l, 1.0, 0.5);
			gtk_box_pack_end ((GtkBox *) hb, l, FALSE, FALSE, 0);
			gtk_widget_show_all (hb);
			item = gtk_image_menu_item_new ();
			gtk_container_add ((GtkContainer *) item, hb);
		} else {
			item = gtk_image_menu_item_new_with_label (action->name);
		}
		if (action->image) {
			icon = sp_icon_new_scaled (16, action->image);
			gtk_widget_show (icon);
			gtk_image_menu_item_set_image ((GtkImageMenuItem *) item, icon);
		}
		gtk_widget_set_events (item, GDK_KEY_PRESS_MASK);
		g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_ui_menu_activate), action);
		g_signal_connect (G_OBJECT (item), "key_press_event", G_CALLBACK (sp_ui_menu_key_press), (void *) verb);
	}
	gtk_widget_show (item);
	gtk_menu_append (GTK_MENU (menu), item);

	return item;
}

static void
sp_ui_menu_append (GtkMenu *menu, const unsigned int *verbs)
{
	int i;
	for (i = 0; verbs[i] < SP_VERB_LAST; i++) {
		sp_ui_menu_append_item_from_verb (menu, verbs[i]);
	}
}

static void
sp_ui_file_menu (GtkMenu *fm, SPDocument *doc)
{
	static const unsigned int file_verbs_one[] = {
		SP_VERB_FILE_NEW, SP_VERB_FILE_OPEN, SP_VERB_LAST
        };

	static const unsigned int file_verbs_two[] = {
		SP_VERB_FILE_SAVE, SP_VERB_FILE_SAVE_AS,
		SP_VERB_NONE,
		SP_VERB_FILE_IMPORT, SP_VERB_FILE_EXPORT,
		SP_VERB_NONE,
		SP_VERB_FILE_PRINT,
		SP_VERB_LAST
	};

	sp_ui_menu_append (fm, file_verbs_one);

 	GtkWidget *item_recent = sp_ui_menu_append_item (fm, NULL, _("Open Recent"), NULL, NULL);
 	GtkWidget *menu_recent = gtk_menu_new ();
 	sp_menu_append_recent_documents (GTK_WIDGET (menu_recent));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_recent), menu_recent);

	sp_ui_menu_append (fm, file_verbs_two);

#ifdef WIN32
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_DIRECT);
#endif
#ifdef WITH_KDE
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_DIRECT);
#endif
#ifdef WITH_GNOME_PRINT
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_DIRECT);
#endif
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_PREVIEW);

	sp_ui_menu_append_item_from_verb (fm, SP_VERB_NONE);

	sp_ui_menu_append_item (fm, GTK_STOCK_CLOSE, _("Close View"), G_CALLBACK (sp_ui_close_view), NULL);
	sp_ui_menu_append_item (fm, GTK_STOCK_QUIT, _("Exit Program"), G_CALLBACK (sp_file_exit), NULL);
	sp_ui_menu_append_item (fm, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item (fm, NULL, _("About Sodipodi"), G_CALLBACK(sp_help_about), NULL);
#ifdef WITH_MODULES
	/* Modules need abouts too */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(sp_ui_menu_append_item (GTK_MENU (fm), NULL, _("About Modules"), NULL, NULL)),
			                   GTK_WIDGET(sp_modulesys_menu_about()));
#endif /* WITH_MODULES */
}

static void
sp_ui_edit_menu (GtkMenu *menu, SPDocument *doc)
{
	static const unsigned int edit_verbs[] = {
		SP_VERB_EDIT_CUT, SP_VERB_EDIT_COPY, SP_VERB_EDIT_PASTE,
		SP_VERB_NONE,
		SP_VERB_EDIT_DUPLICATE, SP_VERB_EDIT_DELETE,
		SP_VERB_NONE,
		SP_VERB_EDIT_CLEAR_ALL,
		SP_VERB_EDIT_SELECT_ALL,
		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, edit_verbs);
	sp_ui_menu_append_item (menu, NULL, _("Cleanup"), G_CALLBACK (sp_edit_cleanup), NULL);
}

static void
sp_ui_selection_menu (GtkMenu *menu, SPDocument *doc)
{
	static const unsigned int select_verbs[] = {
		SP_VERB_SELECTION_GROUP, SP_VERB_SELECTION_UNGROUP,
		SP_VERB_NONE,
		SP_VERB_SELECTION_COMBINE, SP_VERB_SELECTION_BREAK_APART,
		SP_VERB_NONE,
		SP_VERB_SELECTION_TO_FRONT,
		SP_VERB_SELECTION_TO_BACK,
		SP_VERB_SELECTION_RAISE,
		SP_VERB_SELECTION_LOWER,
		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, select_verbs);
}

static void
sp_ui_view_show_toolbox (GObject *object, gpointer data)
{
	sp_maintoolbox_create_toplevel ();
}

static void
sp_ui_view_dock_toolbox (GObject *object, gpointer data)
{
	GObject *w;
	GtkWidget *hb, *tb, *f;

	if (!SP_ACTIVE_DESKTOP) return;
	w = g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
	if (!w) return;
	tb = g_object_get_data (w, "toolbox");
	if (tb) return;
	hb = g_object_get_data (w, "hbox");
	if (!hb) return;
	f = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_OUT);
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (hb), f, FALSE, FALSE, 0);
	tb = sp_maintoolbox_new ();
	gtk_widget_show (tb);
	gtk_container_add (GTK_CONTAINER (f), tb);
	g_object_set_data (w, "toolbox", f);
}

static void
sp_ui_view_remove_toolbox (GObject *object, gpointer data)
{
	GObject *w;
	GtkWidget *tb;

	if (!SP_ACTIVE_DESKTOP) return;
	w = g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
	if (!w) return;
	tb = g_object_get_data (w, "toolbox");
	if (!tb) return;
	gtk_widget_destroy (tb);
	g_object_set_data (w, "toolbox", NULL);
}

static void
sp_ui_view_menu (GtkMenu *menu, SPDocument *doc)
{
	GtkWidget *zm, *zi;

	/* View:Zoom */
	zi = sp_ui_menu_append_item (menu, NULL, _("Zoom"), NULL, NULL);
	zm = gtk_menu_new ();
	gtk_widget_show (zm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (zi), zm);
	sp_ui_menu_append_item_from_verb (GTK_MENU (zm), SP_VERB_ZOOM_SELECTION);
	sp_ui_menu_append_item_from_verb (GTK_MENU (zm), SP_VERB_ZOOM_DRAWING);
	sp_ui_menu_append_item_from_verb (GTK_MENU (zm), SP_VERB_ZOOM_PAGE);
	sp_ui_menu_append_item (GTK_MENU (zm), NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item_from_verb (GTK_MENU (zm), SP_VERB_ZOOM_1_2);
	sp_ui_menu_append_item_from_verb (GTK_MENU (zm), SP_VERB_ZOOM_1_1);
	sp_ui_menu_append_item_from_verb (GTK_MENU (zm), SP_VERB_ZOOM_2_1);
	/* View:New View*/
	sp_ui_menu_append_item (menu, NULL, _("New View"), G_CALLBACK(sp_ui_new_view), NULL);
	/* View:New Preview*/
	sp_ui_menu_append_item (menu, NULL, _("New Preview"), G_CALLBACK(sp_ui_new_view_preview), NULL);
	sp_ui_menu_append_item (menu, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item (menu, NULL, _("New Toplevel Toolbox"), G_CALLBACK (sp_ui_view_show_toolbox), NULL);
	sp_ui_menu_append_item (menu, NULL, _("New Docked Toolbox"), G_CALLBACK (sp_ui_view_dock_toolbox), NULL);
	sp_ui_menu_append_item (menu, NULL, _("Remove Docked Toolbox"), G_CALLBACK (sp_ui_view_remove_toolbox), NULL);
}

static void
sp_ui_event_context_menu (GtkMenu *menu, SPDocument *doc)
{
	static const unsigned int context_verbs[] = {
		SP_VERB_CONTEXT_SELECT, SP_VERB_CONTEXT_NODE,
		SP_VERB_CONTEXT_RECT, SP_VERB_CONTEXT_ARC, SP_VERB_CONTEXT_STAR, SP_VERB_CONTEXT_SPIRAL,
		SP_VERB_CONTEXT_PEN, SP_VERB_CONTEXT_PENCIL, SP_VERB_CONTEXT_CALLIGRAPHIC,
		SP_VERB_CONTEXT_TEXT, SP_VERB_CONTEXT_ZOOM,
		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, context_verbs);
}

static void
sp_ui_dialog_menu (GtkMenu *menu, SPDocument *doc)
{
	static const unsigned int verbs[] = {
		SP_VERB_DIALOG_FILL_STROKE,
		SP_VERB_DIALOG_TEXT,
		SP_VERB_DIALOG_SIZE_POSITION,
		SP_VERB_DIALOG_TRANSFORM,
		SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
		SP_VERB_DIALOG_ITEM,
		SP_VERB_DIALOG_XML_EDITOR,
		SP_VERB_DIALOG_DOCUMENT,
		SP_VERB_DIALOG_NAMEDVIEW,
		SP_VERB_DIALOG_TOOL_OPTIONS,
		SP_VERB_DIALOG_TOOL_ATTRIBUTES,
		SP_VERB_DIALOG_DISPLAY,
		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, verbs);
}

/* Menus */

static void
sp_ui_populate_main_menu(GtkWidget *m)
{
	sp_ui_menu_append_item (GTK_MENU (m), GTK_STOCK_NEW, _("New"), G_CALLBACK(sp_file_new), NULL);
	sp_ui_menu_append_item (GTK_MENU (m), GTK_STOCK_OPEN, _("Open"), G_CALLBACK(sp_file_open_dialog), NULL);
        
	GtkWidget *item_recent = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("Open Recent"), NULL, NULL);
	GtkWidget *menu_recent = gtk_menu_new ();

	sp_menu_append_recent_documents (GTK_WIDGET (menu_recent));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_recent), menu_recent); 

	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item (GTK_MENU (m), NULL, _("About Sodipodi"), G_CALLBACK(sp_help_about), NULL);
#ifdef lalaWITH_MODULES
	/* Modules need abouts too */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(sp_ui_menu_append_item (GTK_MENU (m), NULL, _("About Modules"), NULL, NULL)),
			                   GTK_WIDGET(sp_modulesys_menu_about()));
#endif /* WITH_MODULES */

	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item (GTK_MENU (m), GTK_STOCK_QUIT, _("Exit Program"), G_CALLBACK(sp_file_exit), NULL);
}

static void
sp_ui_remove_child (GtkWidget *child, gpointer parent)
{
	gtk_container_remove (GTK_CONTAINER ((GtkWidget *)parent), child);
}

static void
sp_ui_remove_all(GtkWidget *m)
{
	gtk_container_foreach (GTK_CONTAINER (m), sp_ui_remove_child, m);
}

GtkWidget *
sp_ui_main_menu (void)
{
	GtkWidget *m;

	m = gtk_menu_new ();

	g_signal_connect (G_OBJECT (m), "show", (GCallback)sp_ui_populate_main_menu, NULL);
	g_signal_connect (G_OBJECT (m), "hide", (GCallback)sp_ui_remove_all, NULL);

	return m;
}

GtkWidget *
sp_ui_generic_menu (SPView *v, SPItem *item)
{
	GtkWidget *m, *i, *sm;
	SPDesktop *dt;

	dt = (SP_IS_DESKTOP (v)) ? SP_DESKTOP (v) : NULL;

	m = gtk_menu_new ();

	/* Undo and Redo */
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_UNDO);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_REDO);
	/* Separator */
	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);

	/* Item menu */
	if (item) {
		sp_object_menu ((SPObject *) item, dt, GTK_MENU (m));
		/* Separator */
		sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);
	}

#if 0
	/* Desktop menu */
	if (vw) {
		sp_view_widget_menu (vw, m);
		i = gtk_menu_item_new ();
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
	}
#endif

	/* Generic menu */
	/* File submenu */
	i = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("File"), NULL, NULL);
	sm = gtk_menu_new ();
	sp_ui_file_menu (GTK_MENU (sm), NULL);
	gtk_widget_show (sm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), sm);
	/* Edit submenu */
	i = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("Edit"), NULL, NULL);
	sm = gtk_menu_new ();
	sp_ui_edit_menu (GTK_MENU (sm), NULL);
	gtk_widget_show (sm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), sm);
	/* Selection submenu */
	i = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("Selection"), NULL, NULL);
	sm = gtk_menu_new ();
	sp_ui_selection_menu (GTK_MENU (sm), NULL);
	gtk_widget_show (sm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), sm);
	/* View submenu */
	i = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("View"), NULL, NULL);
	sm = gtk_menu_new ();
	sp_ui_view_menu (GTK_MENU (sm), NULL);
	gtk_widget_show (sm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), sm);
	/* Drawing mode submenu */
	i = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("Drawing Mode"), NULL, NULL);
	sm = gtk_menu_new ();
	sp_ui_event_context_menu (GTK_MENU (sm), NULL);
	gtk_widget_show (sm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), sm);
	/* Dialog submenu */
	i = sp_ui_menu_append_item (GTK_MENU (m), NULL, _("Dialogs"), NULL, NULL);
	sm = gtk_menu_new ();
	sp_ui_dialog_menu (GTK_MENU (sm), NULL);
	gtk_widget_show (sm);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), sm);
	/* Filters submenu */
#ifdef lalaWITH_MODULES
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(sp_ui_menu_append_item (GTK_MENU (m), NULL, _("Filters"), NULL, NULL)),
			                   GTK_WIDGET(sp_modulesys_menu_filter()));
#endif /* WITH_MODULES */

	return m;
}

static void
sp_recent_open (GtkWidget *widget, const guchar *uri)
{
	sp_file_open (uri, NULL);
}

void
sp_menu_append_recent_documents (GtkWidget *menu)
{
	SPRepr *recent;
	recent = sodipodi_get_repr (SODIPODI, "documents.recent");
	if (recent) {
		SPRepr *child;
		for (child = recent->children; child != NULL; child = child->next) {
			const guchar *uri, *name;
			uri = sp_repr_attr (child, "uri");
			name = sp_repr_attr (child, "name");
			/* fixme: I am pretty sure this is safe, but double check (Lauris) */
			sp_ui_menu_append_item (GTK_MENU (menu), NULL, name, G_CALLBACK(sp_recent_open), (gpointer) uri);
		}
	} else {
		GtkWidget *item = sp_ui_menu_append_item (GTK_MENU (menu), NULL, "None", NULL, NULL);
		gtk_widget_set_sensitive(item, FALSE);
	}
}

/* Drag and Drop */
void 
sp_ui_drag_data_received (GtkWidget * widget,
			  GdkDragContext * drag_context,
			  gint x, gint y,
			  GtkSelectionData * data,
			  guint info,
			  guint event_time,
			  gpointer user_data)
{
	gchar * uri;
	
	switch(info) {
	case URI_LIST:
		uri = (gchar *)data->data;
		sp_ui_import_files(uri);
		break;
	}
}

static void
sp_ui_import_files(gchar * buffer)
{
	GList * list = gnome_uri_list_extract_filenames(buffer);
	if (!list)
		return;
	g_list_foreach (list, sp_ui_import_one_file_with_check, NULL);
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
}

static void
sp_ui_import_one_file_with_check(gpointer filename, gpointer unused)
{
	if (filename) {
		if (strlen(filename) > 2)
			sp_ui_import_one_file(filename);
	}
}

/* Cut&Paste'ed from file.c:file_import_ok */
static void
sp_ui_import_one_file(gchar * filename)
{
	SPDocument * doc;
	SPRepr * rdoc;
	const gchar * e, * docbase, * relname;
	SPRepr * repr;
	SPReprDoc * rnewdoc;

	doc = SP_ACTIVE_DOCUMENT;
	if (!SP_IS_DOCUMENT(doc)) return;
	
	if (filename == NULL) return;  

	rdoc = sp_document_repr_root (doc);

	docbase = sp_repr_attr (rdoc, "sodipodi:docbase");
	relname = sp_relative_path_from_path (filename, docbase);
	/* fixme: this should be implemented with mime types */
	e = sp_extension_from_path (filename);

	if ((e == NULL) || (strcmp (e, "svg") == 0) || (strcmp (e, "xml") == 0)) {
		SPRepr * newgroup;
		const gchar * style;
		SPRepr * child;

		rnewdoc = sp_repr_read_file (filename, SP_SVG_NS_URI);
		if (rnewdoc == NULL) return;
		repr = sp_repr_document_root (rnewdoc);
		style = sp_repr_attr (repr, "style");

		newgroup = sp_repr_new ("g");
		sp_repr_set_attr (newgroup, "style", style);

		for (child = repr->children; child != NULL; child = child->next) {
			SPRepr * newchild;
			newchild = sp_repr_duplicate (child);
			sp_repr_append_child (newgroup, newchild);
		}

		sp_repr_document_unref (rnewdoc);

		sp_document_add_repr (doc, newgroup);
		sp_repr_unref (newgroup);
		sp_document_done (doc);
		return;
	}

	if ((strcmp (e, "png") == 0) ||
	    (strcmp (e, "jpg") == 0) ||
	    (strcmp (e, "jpeg") == 0) ||
	    (strcmp (e, "bmp") == 0) ||
	    (strcmp (e, "gif") == 0) ||
	    (strcmp (e, "tiff") == 0) ||
	    (strcmp (e, "xpm") == 0)) {
		/* Try pixbuf */
		GdkPixbuf *pb;
		pb = gdk_pixbuf_new_from_file (filename, NULL);
		if (pb) {
			/* We are readable */
			repr = sp_repr_new ("image");
			sp_repr_set_attr (repr, "xlink:href", relname);
			sp_repr_set_attr (repr, "sodipodi:absref", filename);
			sp_repr_set_double_attribute (repr, "width", gdk_pixbuf_get_width (pb));
			sp_repr_set_double_attribute (repr, "height", gdk_pixbuf_get_height (pb));
			sp_document_add_repr (doc, repr);
			sp_repr_unref (repr);
			sp_document_done (doc);
			gdk_pixbuf_unref (pb);
		}
	}
}
