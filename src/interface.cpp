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
#include "inkscape.h"
#include "inkscape-private.h"
#include "extension/menu.h"
#include "extension/db.h"
#include "widgets/icon.h"
#include "prefs-utils.h"

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
#include "svg-view.h"
#include "sp-namedview.h"

#include "dir-util.h"
#include "xml/repr-private.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"
#include "helper/window.h"

#include "dialogs/dialog-events.h"

/* forward declaration */
static gint sp_ui_delete (GtkWidget *widget, GdkEvent *event, SPView *view);

/* Drag and Drop */
typedef enum {
  URI_LIST,
  SVG_XML_DATA,
  SVG_DATA
} ui_drop_target_info;

static GtkTargetEntry ui_drop_target_entries [] = {
  {"text/uri-list", 0, URI_LIST},
  {"image/svg+xml", 0, SVG_XML_DATA},
  {"image/svg",     0, SVG_DATA},
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
	} else {
		gtk_window_set_policy (GTK_WINDOW (w), TRUE, TRUE, TRUE);
	}

	gtk_box_pack_end (GTK_BOX (hb), GTK_WIDGET (vw), TRUE, TRUE, 0);
	gtk_widget_show (GTK_WIDGET (vw));

	gtk_drag_dest_set(w, 
			  GTK_DEST_DEFAULT_ALL,
			  ui_drop_target_entries,
			  nui_drop_target_entries,
			  GdkDragAction (GDK_ACTION_COPY | GDK_ACTION_MOVE));
	g_signal_connect (G_OBJECT(w),
			   "drag_data_received",
			   G_CALLBACK (sp_ui_drag_data_received),
			   NULL);
	gtk_widget_show (w);

	// needed because the first ACTIVATE_DESKTOP was sent when there was no window yet
	inkscape_reactivate_desktop (SP_DESKTOP_WIDGET (vw)->desktop);
}

static void sp_ui_new_view(GtkWidget * widget)
{
	SPDocument * document;
	SPViewWidget *dtw;

	document = SP_ACTIVE_DOCUMENT;
	if (!document) return;

	dtw = sp_desktop_widget_new (sp_document_namedview (document, NULL));
	g_return_if_fail (dtw != NULL);

	sp_create_window (dtw, TRUE);
      sp_namedview_window_from_document (SP_DESKTOP(dtw->view));
}

#if 0 /* To be re-enabled (and added to menu) once it works. */
/* fixme: not yet working */
static void sp_ui_new_view_preview(GtkWidget *widget)
{
	SPDocument *document;
	SPViewWidget *dtw;

	document = SP_ACTIVE_DOCUMENT;
	if (!document) return;

	dtw = (SPViewWidget *) sp_svg_view_widget_new (document);
	g_return_if_fail (dtw != NULL);
	sp_svg_view_widget_set_resize (SP_SVG_VIEW_WIDGET (dtw), TRUE, 400.0, 400.0);

	sp_create_window (dtw, FALSE);
}
#endif

void
sp_ui_close_view (GtkWidget * widget)
{
	GtkWidget *w;

	if (SP_ACTIVE_DESKTOP == NULL) return;
	w = (GtkWidget*)g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
	if (sp_view_shutdown (SP_VIEW (SP_ACTIVE_DESKTOP))) return;
	gtk_widget_destroy (w);
}

unsigned int
sp_ui_close_all (void)
{
	while (SP_ACTIVE_DESKTOP) {
		GtkWidget *w;
		w = (GtkWidget*)g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
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
sp_ui_menu_append_item (GtkMenu *menu, const gchar *stock, const gchar *label, GCallback callback, gpointer data, gboolean with_mnemonic = TRUE)
{
	GtkWidget *item;

	if (stock) {
		item = gtk_image_menu_item_new_from_stock (stock, NULL);
	} else if (label) {
		item = (with_mnemonic) ? gtk_image_menu_item_new_with_mnemonic (label) : gtk_image_menu_item_new_with_label (label);
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
	sp_action_perform (action, NULL);
}

static void
sp_ui_menu_key_press (GtkMenuItem *item, GdkEventKey *event, void *data)
{
	if (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) {
		unsigned int shortcut;

		shortcut = event->keyval;
		if (event->state & GDK_SHIFT_MASK) shortcut |= SP_SHORTCUT_SHIFT_MASK;
		if (event->state & GDK_CONTROL_MASK) shortcut |= SP_SHORTCUT_CONTROL_MASK;
		if (event->state & GDK_MOD1_MASK) shortcut |= SP_SHORTCUT_ALT_MASK;
		sp_shortcut_set (shortcut, (sp_verb_t)((int)data), true);
	}
}

/**
\brief  a wrapper around gdk_keyval_name producing (when possible) characters, not names
 */
static gchar const *
sp_key_name (guint keyval)
{
	gchar const *n = gdk_keyval_name (gdk_keyval_to_upper (keyval));
	
	if      (!strcmp (n, "asciicircum"))  return "^";
	else if (!strcmp (n, "parenleft"  ))  return "(";
	else if (!strcmp (n, "parenright" ))  return ")";
	else if (!strcmp (n, "plus"       ))  return "+";
	else if (!strcmp (n, "minus"      ))  return "-";
	else if (!strcmp (n, "asterisk"   ))  return "*";
	else if (!strcmp (n, "KP_Multiply"))  return "*";
	else if (!strcmp (n, "Delete"     ))  return "Del";
	else if (!strcmp (n, "Page_Up"    ))  return "PgUp";
	else if (!strcmp (n, "Page_Down"  ))  return "PgDn";
	else if (!strcmp (n, "grave"      ))  return "`";
	else if (!strcmp (n, "numbersign" ))  return "#";
	else if (!strcmp (n, "bar" ))  return "|";
	else if (!strcmp (n, "slash" ))  return "/";
	else return n;
}

void
sp_ui_shortcut_string (unsigned int shortcut, gchar* c)
{
	const gchar *altStr, *ctrlStr, *shiftStr;

	altStr   = (shortcut & SP_SHORTCUT_ALT_MASK    ) ? "Alt+"   : "";
	ctrlStr  = (shortcut & SP_SHORTCUT_CONTROL_MASK) ? "Ctrl+"  : "";
	shiftStr = (shortcut & SP_SHORTCUT_SHIFT_MASK  ) ? "Shift+" : "";

	g_snprintf (c, 256, "%s%s%s%s", shiftStr, ctrlStr, altStr,
         sp_key_name (shortcut & 0xffffff));
}

void
sp_ui_dialog_title_string (sp_verb_t verb, gchar* c)
{
	SPAction     *action;
	unsigned int shortcut;
	gchar        *s; 
	gchar        key[256];
	gchar        *atitle;

	action = sp_verb_get_action (verb, NULL);
	if (!action)
        return; 
	
	atitle = sp_action_get_title (action);
	
	s = g_stpcpy (c, atitle);
	
	g_free (atitle);

	shortcut = sp_shortcut_get_primary (verb);
	if (shortcut) {
		s = g_stpcpy (s, " (");
		sp_ui_shortcut_string (shortcut, key);
		s = g_stpcpy (s, key);
		s = g_stpcpy (s, ")");
	}
}

static GtkWidget *
sp_ui_menu_append_item_from_verb (GtkMenu *menu, sp_verb_t verb, SPView *view)
{
	SPAction *action;
	GtkWidget *item, *icon;

	if (verb == SP_VERB_NONE) {
		item = gtk_separator_menu_item_new ();
	} else {
		unsigned int shortcut;

		action = sp_verb_get_action (verb, view);
		if (!action) return NULL;

		shortcut = sp_shortcut_get_primary (verb);
		if (shortcut) {
			gchar c[256];
			GtkWidget *hb, *l;
			sp_ui_shortcut_string (shortcut, c);
			hb = gtk_hbox_new (FALSE, 16);
			l = gtk_label_new_with_mnemonic (action->name);
			gtk_misc_set_alignment ((GtkMisc *) l, 0.0, 0.5);
			gtk_box_pack_start ((GtkBox *) hb, l, TRUE, TRUE, 0);
			l = gtk_label_new (c);
			gtk_misc_set_alignment ((GtkMisc *) l, 1.0, 0.5);
			gtk_box_pack_end ((GtkBox *) hb, l, FALSE, FALSE, 0);
			gtk_widget_show_all (hb);
			item = gtk_image_menu_item_new ();
			gtk_container_add ((GtkContainer *) item, hb);
		} else {
			item = gtk_image_menu_item_new_with_mnemonic (action->name);
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
sp_ui_menu_append (GtkMenu *menu, const sp_verb_t *verbs, SPView *view)
{
	int i;
	for (i = 0; verbs[i] != SP_VERB_LAST; i++) {
		sp_ui_menu_append_item_from_verb (menu, verbs[i], view);
	}
}

static void
sp_ui_file_menu (GtkMenu *fm, SPDocument *doc, SPView *view)
{
 	GtkWidget *item_recent, *menu_recent;
	static const sp_verb_t file_verbs_one[] = {
		SP_VERB_FILE_NEW, SP_VERB_FILE_OPEN, SP_VERB_LAST
        };

	static const sp_verb_t file_verbs_two[] = {
		SP_VERB_FILE_SAVE,
		SP_VERB_FILE_SAVE_AS,
		SP_VERB_NONE,
		SP_VERB_FILE_IMPORT, 
		SP_VERB_FILE_EXPORT,
		SP_VERB_NONE,
		SP_VERB_FILE_PRINT,
		SP_VERB_LAST
	};

	sp_ui_menu_append (fm, file_verbs_one, view);

 	item_recent = sp_ui_menu_append_item (fm, NULL, _("Open _Recent"), NULL, NULL);
 	menu_recent = gtk_menu_new ();
 	sp_menu_append_recent_documents (GTK_WIDGET (menu_recent));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_recent), menu_recent);

	sp_ui_menu_append (fm, file_verbs_two, view);

#ifdef WIN32
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_DIRECT, view);
#endif
#ifdef WITH_GNOME_PRINT
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_DIRECT, view);
#endif
/* commented out until implemented */
/*	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_PRINT_PREVIEW, view);*/

	sp_ui_menu_append_item_from_verb (fm, SP_VERB_NONE, view);

	sp_ui_menu_append_item (fm, GTK_STOCK_CLOSE, _("Close View"), G_CALLBACK (sp_ui_close_view), NULL);
	sp_ui_menu_append_item_from_verb (fm, SP_VERB_FILE_QUIT, view);
}

static void
sp_ui_edit_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
	static const sp_verb_t edit_verbs[] = {
		SP_VERB_EDIT_UNDO, 
		SP_VERB_EDIT_REDO,
		SP_VERB_NONE,
		SP_VERB_EDIT_CUT, 
		SP_VERB_EDIT_COPY, 
		SP_VERB_EDIT_PASTE, 
		SP_VERB_EDIT_PASTE_IN_PLACE, 
		SP_VERB_EDIT_PASTE_STYLE,
		SP_VERB_NONE,
		SP_VERB_EDIT_DUPLICATE, 
		SP_VERB_EDIT_DELETE,
		SP_VERB_NONE,
		SP_VERB_EDIT_SELECT_ALL,
		SP_VERB_EDIT_DESELECT,
		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, edit_verbs, view);
}

static void
sp_ui_object_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
	static const sp_verb_t selection[] = {
		SP_VERB_SELECTION_GROUP, 
		SP_VERB_SELECTION_UNGROUP,

		SP_VERB_NONE,
		SP_VERB_SELECTION_TO_FRONT,
		SP_VERB_SELECTION_TO_BACK,
		SP_VERB_SELECTION_RAISE,
		SP_VERB_SELECTION_LOWER,

		SP_VERB_NONE,
		//		SP_VERB_OBJECT_FLATTEN,
		SP_VERB_OBJECT_ROTATE_90,
		SP_VERB_OBJECT_FLIP_HORIZONTAL,
		SP_VERB_OBJECT_FLIP_VERTICAL,

		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, selection, view);
}

static void
sp_ui_path_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
	static const sp_verb_t selection[] = {
		SP_VERB_OBJECT_TO_CURVE,
		SP_VERB_SELECTION_OUTLINE,

		SP_VERB_NONE,
		SP_VERB_SELECTION_UNION,
		SP_VERB_SELECTION_DIFF,
		SP_VERB_SELECTION_INTERSECT,
		SP_VERB_SELECTION_SYMDIFF,
		SP_VERB_SELECTION_CUT,
		SP_VERB_SELECTION_SLICE,

		SP_VERB_NONE,
		SP_VERB_SELECTION_COMBINE,
		SP_VERB_SELECTION_BREAK_APART,

		SP_VERB_NONE,
		SP_VERB_SELECTION_INSET,
		SP_VERB_SELECTION_OFFSET,
		SP_VERB_SELECTION_DYNAMIC_OFFSET,
		SP_VERB_SELECTION_LINKED_OFFSET,

		SP_VERB_NONE,
		SP_VERB_SELECTION_SIMPLIFY,
		SP_VERB_LAST
	};
	sp_ui_menu_append (menu, selection, view);
	sp_ui_menu_append_item (menu, NULL, _("Cl_eanup"), G_CALLBACK (sp_edit_cleanup), NULL);
}

static void
sp_ui_view_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_1_1, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_1_2, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_2_1, view);
	sp_ui_menu_append_item (GTK_MENU (menu), NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_SELECTION, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_DRAWING, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_PAGE, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_PAGE_WIDTH, view);

	sp_ui_menu_append_item (menu, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_NEXT, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_ZOOM_PREV, view);

	sp_ui_menu_append_item (menu, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_TOGGLE_GRID, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_TOGGLE_GUIDES, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_TOGGLE_RULERS, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_TOGGLE_SCROLLBARS, view);

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
	sp_ui_menu_append_item (menu, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_FULLSCREEN, view);
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
	
	sp_ui_menu_append_item (menu, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_FILE_NEXT_DESKTOP, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (menu), SP_VERB_FILE_PREV_DESKTOP, view);

	sp_ui_menu_append_item (menu, NULL, NULL, NULL, NULL);
	sp_ui_menu_append_item (menu, NULL, _("_New View"), G_CALLBACK(sp_ui_new_view), NULL);
	//	sp_ui_menu_append_item (menu, NULL, _("New P_review"), G_CALLBACK(sp_ui_new_view_preview), NULL);
}

#ifndef WIN32
static void window_policy_toggled(GtkCheckMenuItem *menuitem,
                                  gpointer user_data)
{
	gboolean checked = gtk_check_menu_item_get_active(menuitem);
	prefs_set_int_attribute("options.transientpolicy", "value", checked);
}
 
static gboolean window_policy_update(GtkWidget *widget,
                                     GdkEventExpose *event,
                                     gpointer user_data)
{
	GtkCheckMenuItem *menuitem=GTK_CHECK_MENU_ITEM(widget);
	gint policy = prefs_get_int_attribute_limited("options.transientpolicy", "value", 1, 0, 2);
	g_signal_handlers_block_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)window_policy_toggled, user_data);
	gtk_check_menu_item_set_active(menuitem, policy);
	g_signal_handlers_unblock_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)window_policy_toggled, user_data);
	return FALSE;
}
#endif

static void
sp_ui_dialogs_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
        static const sp_verb_t dialog_verbs[] = {
                SP_VERB_DIALOG_FILL_STROKE,
                SP_VERB_DIALOG_TEXT,
								//                SP_VERB_DIALOG_SIZE_POSITION,
                SP_VERB_DIALOG_TRANSFORM,
                SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                SP_VERB_DIALOG_ITEM,
                SP_VERB_DIALOG_XML_EDITOR,
								//                SP_VERB_DIALOG_DOCUMENT,
                SP_VERB_DIALOG_NAMEDVIEW,
                SP_VERB_DIALOG_TOOL_OPTIONS,
								//                SP_VERB_DIALOG_TOOL_ATTRIBUTES,
                SP_VERB_DIALOG_DISPLAY,

		   SP_VERB_NONE,
		   SP_VERB_DIALOG_TOGGLE,

                SP_VERB_LAST
        };

	sp_ui_menu_append (menu, dialog_verbs, view);

#ifndef WIN32
	GtkWidget *window_policy_check = gtk_check_menu_item_new_with_label(_("Autoraise Dialogs"));
	gtk_widget_show(window_policy_check);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), window_policy_check);

	g_signal_connect(G_OBJECT(window_policy_check), "toggled", (GCallback)window_policy_toggled, NULL);
	g_signal_connect(G_OBJECT(window_policy_check), "expose_event", (GCallback)window_policy_update, NULL);
#endif
}

/* Menus */

static void
sp_ui_help_menu(GtkWidget *m)
{
	sp_ui_menu_append_item (GTK_MENU (m), NULL, _("_Tutorial"), G_CALLBACK(sp_help_tutorial), NULL);
	sp_ui_menu_append_item (GTK_MENU (m), NULL, _("_Keys and Mouse"), G_CALLBACK(sp_help_keys), NULL);
	sp_ui_menu_append_item (GTK_MENU (m), NULL, _("_About Inkscape"), G_CALLBACK(sp_help_about), NULL);

#ifdef WITH_MODULES
	/* TODO: Modules need abouts too
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(sp_ui_menu_append_item (GTK_MENU (m), NULL, _("About Modules"), NULL, NULL)),
			                   GTK_WIDGET(sp_module_menu_about()));
    */
#endif /* WITH_MODULES */
}

GtkWidget *
sp_ui_main_menubar (SPView *view)
{
	GtkWidget *mbar, *mitem;
	GtkWidget *menu;

	mbar = gtk_menu_bar_new ();

	mitem = gtk_menu_item_new_with_mnemonic (_("_File"));
	menu = gtk_menu_new ();
	sp_ui_file_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	mitem = gtk_menu_item_new_with_mnemonic (_("_Edit"));
	menu = gtk_menu_new ();
	sp_ui_edit_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	mitem = gtk_menu_item_new_with_mnemonic (_("_View"));
	menu = gtk_menu_new ();
	sp_ui_view_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);
	
	mitem = gtk_menu_item_new_with_mnemonic (_("_Object"));
	menu = gtk_menu_new ();
	sp_ui_object_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	mitem = gtk_menu_item_new_with_mnemonic (_("_Path"));
	menu = gtk_menu_new ();
	sp_ui_path_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	mitem = gtk_menu_item_new_with_mnemonic (_("_Dialogs"));
	menu = gtk_menu_new ();
	sp_ui_dialogs_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	mitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
	menu = gtk_menu_new ();
	sp_ui_help_menu (menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	return mbar;
}

GtkWidget *
sp_ui_context_menu (SPView *view, SPItem *item)
{
	GtkWidget *m;
	SPDesktop *dt;

	dt = (SP_IS_DESKTOP (view)) ? SP_DESKTOP (view) : NULL;

	m = gtk_menu_new ();

	/* Undo and Redo */
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_UNDO, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_REDO, view);

	/* Separator */
	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);

	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_CUT, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_COPY, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_PASTE, view);

	/* Separator */
	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);

	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_DUPLICATE, view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), SP_VERB_EDIT_DELETE, view);

	/* Item menu */
	if (item) {
		sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL);
		sp_object_menu ((SPObject *) item, dt, GTK_MENU (m));
	}

	return m;
}

static void
sp_recent_open (GtkWidget *widget, const gchar *uri)
{
	sp_file_open (uri, NULL);
}

void
sp_menu_append_recent_documents (GtkWidget *menu)
{
	SPRepr *recent;
	recent = inkscape_get_repr (INKSCAPE, "documents.recent");
	if (recent) {
		SPRepr *child;
		for (child = recent->children; child != NULL; child = child->next) {
			GtkWidget *item;
			const gchar *uri, *name;
			uri = sp_repr_attr (child, "uri");
			name = sp_repr_attr (child, "name");

			item = gtk_menu_item_new_with_label (name);
			gtk_widget_show(item);
			g_signal_connect(G_OBJECT(item),
					"activate",
					G_CALLBACK(sp_recent_open),
					(gpointer)uri);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		}
	} else {
		GtkWidget *item = gtk_menu_item_new_with_label(_("None"));
		gtk_widget_show(item);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
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
	gchar * uri, * svgdata;
	SPRepr * rdoc, * repr, * newgroup, * child;
	SPReprDoc * rnewdoc;
	SPDocument * doc;
	const gchar *style;
	
	switch(info) {
	case SVG_DATA:
	case SVG_XML_DATA: 
		svgdata = (gchar *)data->data;

		doc = SP_ACTIVE_DOCUMENT;

		/* fixme: put into a function (sp_mem_import?) */
		rdoc = sp_document_repr_root (doc);
		
		rnewdoc = sp_repr_read_mem (svgdata, data->length, SP_SVG_NS_URI);

		if (rnewdoc == NULL) {
			sp_ui_error_dialog (_("Could not parse SVG data"));
			return;
		}
	
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
		
		break;
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
		if (strlen((gchar*)filename) > 2)
			sp_ui_import_one_file((gchar*)filename);
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
		// TODO: bulia, please look over
		gsize bytesRead = 0;
		gsize bytesWritten = 0;
		GError* error = NULL;
		gchar* localFilename = g_filename_from_utf8 ( filename,
													  -1,
													  &bytesRead,
													  &bytesWritten,
													  &error);
		pb = gdk_pixbuf_new_from_file (localFilename, NULL);
		if (pb) {
			/* We are readable */
			repr = sp_repr_new ("image");
			sp_repr_set_attr (repr, "xlink:href", relname);
			sp_repr_set_attr (repr, "sodipodi:absref", filename);
			sp_repr_set_double (repr, "width", gdk_pixbuf_get_width (pb));
			sp_repr_set_double (repr, "height", gdk_pixbuf_get_height (pb));
			sp_document_add_repr (doc, repr);
			sp_repr_unref (repr);
			sp_document_done (doc);
			gdk_pixbuf_unref (pb);
		}
		if ( localFilename != NULL )
		{
			g_free (localFilename);
		}
	}
}

void
sp_ui_error_dialog (const gchar * message)
{
	GtkWidget *dlg;
	
	dlg = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, 
			GTK_BUTTONS_CLOSE, message);
	sp_transientize (dlg);
	gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
	gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
}
