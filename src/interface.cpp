#define __SP_INTERFACE_C__

/**
 * Main UI stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2004 David Turner
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
#include "extension/db.h"
#include "widgets/icon.h"
#include "prefs-utils.h"
#include "path-prefix.h"

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
#include "sp-desktop-widget.h"
#include "sp-item-group.h"
#include "sp-namedview.h"

#include "dir-util.h"
#include "xml/repr-private.h"
#include "helper/action.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"
#include "helper/window.h"

#include "dialogs/dialog-events.h"

#include "message-context.h"

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
		gtk_window_set_default_size ((GtkWindow *) w, 640, 480);
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

void
sp_ui_new_view()
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

/* TODO: not yet working */
/* To be re-enabled (by adding to menu) once it works. */
void
sp_ui_new_view_preview()
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

void
sp_ui_close_view (GtkWidget * widget)
{
    GtkWidget *w;

    if (SP_ACTIVE_DESKTOP == NULL) {
        return;
    }
    w = (GtkWidget*)g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
    if (sp_view_shutdown (SP_VIEW (SP_ACTIVE_DESKTOP))) {
        return;
    }
    gtk_widget_destroy (w);
}


/**
 *  sp_ui_close_all
 *
 *  This function is called to exit the program, and iterates through all
 *  open document view windows, attempting to close each in turn.  If the
 *  view has unsaved information, the user will be prompted to save,
 *  discard, or cancel.
 * 
 *  Returns FALSE if the user cancels the close_all operation, TRUE
 *  otherwise.
 */
unsigned int
sp_ui_close_all (void)
{
    /* Iterate through all the windows, destroying each in the order they
       become active */
    while (SP_ACTIVE_DESKTOP) {
    GtkWidget *w;
    w = (GtkWidget*)g_object_get_data (G_OBJECT (SP_ACTIVE_DESKTOP), "window");
    if (sp_view_shutdown (SP_VIEW (SP_ACTIVE_DESKTOP))) {
        /* The user cancelled the operation, so end doing the close */
        return FALSE;
    }
    gtk_widget_destroy (w);
    }

    return TRUE;
}


static gint
sp_ui_delete (GtkWidget *widget, GdkEvent *event, SPView *view)
{
	return sp_view_shutdown (view);
}

/*
 * Some day when the right-click menus are ready to start working
 * smarter with the verbs, we'll need to change this NULL being
 * sent to sp_action_perform to something useful, or set some kind
 * of global "right-clicked position" variable for actions to
 * investigate when they're called.
 */
static void
sp_ui_menu_activate (void *object, SPAction *action)
{
    sp_action_perform (action, NULL);
}

static void
sp_ui_menu_select_action (void *object, SPAction *action)
{
        action->view->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, action->tip);
}

static void
sp_ui_menu_deselect_action (void *object, SPAction *action)
{
        action->view->tipsMessageContext()->clear();
}

static void
sp_ui_menu_select (gpointer object, gpointer tip)
{
	SPView *view = SP_VIEW (g_object_get_data (G_OBJECT (object), "view"));
        view->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, (gchar *)tip);
}

static void
sp_ui_menu_deselect (gpointer object)
{
	SPView *view = SP_VIEW (g_object_get_data (G_OBJECT (object), "view"));
        view->tipsMessageContext()->clear();
}

/**
 * sp_ui_menuitem_add_icon
 *
 * Creates and attaches a scaled icon to the given menu item.
 *
 */
void
sp_ui_menuitem_add_icon ( GtkWidget *item, gchar * icon_name )
{
    GtkWidget *icon;

    icon = sp_icon_new_scaled (16, icon_name);
    gtk_widget_show (icon);
    gtk_image_menu_item_set_image ((GtkImageMenuItem *) item, icon);
} // end of sp_ui_menu_add_icon

/**
 * sp_ui_menu_append_item
 * 
 * Appends a UI item with specific info for Inkscape/Sodipodi.
 *
 */

static GtkWidget *
sp_ui_menu_append_item ( GtkMenu *menu, const gchar *stock, 
                         const gchar *label, const gchar *tip, SPView *view, GCallback callback, 
                         gpointer data, gboolean with_mnemonic = TRUE )
{
    GtkWidget *item;

    if (stock) {
        item = gtk_image_menu_item_new_from_stock (stock, NULL);
    } else if (label) {
        item = (with_mnemonic) 
            ? gtk_image_menu_item_new_with_mnemonic (label) :      
            gtk_image_menu_item_new_with_label (label);
    } else {
        item = gtk_separator_menu_item_new ();
    }

    gtk_widget_show (item);

    if (callback) {
        g_signal_connect (G_OBJECT (item), "activate", callback, data);
    }

    if (tip && view) {
        g_object_set_data (G_OBJECT (item), "view", (gpointer) view);
        g_signal_connect ( G_OBJECT (item), "select", G_CALLBACK (sp_ui_menu_select), (gpointer) tip );
        g_signal_connect ( G_OBJECT (item), "deselect", G_CALLBACK (sp_ui_menu_deselect), NULL);
    }

    gtk_menu_append (GTK_MENU (menu), item);

    return item;

} // end of sp_ui_menu_append_item()


static void
sp_ui_menu_key_press (GtkMenuItem *item, GdkEventKey *event, void *data)
{
    if (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) {
        unsigned int shortcut;

        shortcut = event->keyval;
        if (event->state & GDK_SHIFT_MASK) shortcut |= SP_SHORTCUT_SHIFT_MASK;
        if (event->state & GDK_CONTROL_MASK) shortcut |= SP_SHORTCUT_CONTROL_MASK;
        if (event->state & GDK_MOD1_MASK) shortcut |= SP_SHORTCUT_ALT_MASK;
        sp_shortcut_set (shortcut, (Inkscape::Verb *)((int)data), true);
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
sp_ui_dialog_title_string (Inkscape::Verb * verb, gchar* c)
{
    SPAction     *action;
    unsigned int shortcut;
    gchar        *s; 
    gchar        key[256];
    gchar        *atitle;

    action = verb->get_action(NULL);
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


/**
 * sp_ui_menu_append_item_from_verb
 * 
 * Appends a custom menu UI from a verb.
 *
 */

static GtkWidget *
sp_ui_menu_append_item_from_verb (GtkMenu *menu, Inkscape::Verb * verb, SPView *view)
{
    SPAction *action;
    GtkWidget *item;

    if (verb->get_code() == SP_VERB_NONE) {

        item = gtk_separator_menu_item_new ();

    } else {
        unsigned int shortcut;

        action = verb->get_action(view);

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
            sp_ui_menuitem_add_icon (item, action->image);
        }
        gtk_widget_set_events (item, GDK_KEY_PRESS_MASK);
        g_signal_connect ( G_OBJECT (item), "activate", 
                           G_CALLBACK (sp_ui_menu_activate), action );
        g_signal_connect ( G_OBJECT (item), "key_press_event", 
                           G_CALLBACK (sp_ui_menu_key_press), (void *) verb);

        g_signal_connect ( G_OBJECT (item), "select", G_CALLBACK (sp_ui_menu_select_action), action );
        g_signal_connect ( G_OBJECT (item), "deselect", G_CALLBACK (sp_ui_menu_deselect_action), action );
    }

    gtk_widget_show (item);
    gtk_menu_append (GTK_MENU (menu), item);

    return item;

} // end of sp_ui_menu_append_item_from_verb



static void
sp_ui_menu_append (GtkMenu *menu, Inkscape::Verb ** verbs, SPView *view)
{
    int i;
    for (i = 0; verbs[i]->get_code() != SP_VERB_LAST; i++) {
        // std::cout << "Verb name: " << verbs[i]->get_id() << " Code: " << verbs[i]->get_code() << std::endl;
        sp_ui_menu_append_item_from_verb (menu, verbs[i], view);
    }
}

static void
sp_ui_menu_append_submenu (GtkMenu *fm, SPView *view, void (*fill_function)(GtkWidget*, SPView *), const gchar *label, const gchar *tip, const gchar *icon)
{
    GtkWidget *item = sp_ui_menu_append_item (fm, NULL, label, tip, view, NULL, NULL);
    if (icon) sp_ui_menuitem_add_icon (item, (gchar *) icon);

    GtkWidget *menu = gtk_menu_new ();
    fill_function (GTK_WIDGET (menu), view);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
}

 static void 
checkitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data) 	 
 { 	 
     const gchar *pref = (const gchar *) user_data;
     SPView *view = (SPView *) g_object_get_data (G_OBJECT (menuitem), "view");

     const gchar *pref_path;
     if (SP_DESKTOP(view)->is_fullscreen)
         pref_path = g_strconcat ("fullscreen.", pref, NULL);
     else 
         pref_path = g_strconcat ("window.", pref, NULL);

     gboolean checked = gtk_check_menu_item_get_active(menuitem); 	 
     prefs_set_int_attribute(pref_path, "state", checked);

     sp_desktop_widget_layout (SP_DESKTOP(view)->owner);
 } 	 
  	 
 static gboolean 
checkitem_update(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) 	 
 { 	 
     GtkCheckMenuItem *menuitem=GTK_CHECK_MENU_ITEM(widget); 	 

     const gchar *pref = (const gchar *) user_data;
     SPView *view = (SPView *) g_object_get_data (G_OBJECT(menuitem), "view");

     const gchar *pref_path;
     if (SP_DESKTOP(view)->is_fullscreen)
         pref_path = g_strconcat ("fullscreen.", pref, NULL);
     else 
         pref_path = g_strconcat ("window.", pref, NULL);

     gint ison = prefs_get_int_attribute_limited (pref_path, "state", 1, 0, 1); 	 

     g_signal_handlers_block_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)checkitem_toggled, user_data); 	 
     gtk_check_menu_item_set_active(menuitem, ison); 	 
     g_signal_handlers_unblock_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)checkitem_toggled, user_data); 	 

     return FALSE; 	 
 }


void
sp_ui_menu_append_check_item_from_verb (GtkMenu *menu, SPView *view, const gchar *label, const gchar *tip, const gchar *pref,
                              void (*callback_toggle)(GtkCheckMenuItem *, gpointer user_data),
                              gboolean (*callback_update)(GtkWidget *widget, GdkEventExpose *event, gpointer user_data),
                                        Inkscape::Verb * verb)
{
    GtkWidget *item;

    unsigned int shortcut = 0;
    SPAction *action = NULL;

    if (verb) {
        shortcut = sp_shortcut_get_primary (verb);
        action = verb->get_action(view);
    }

    if (verb && shortcut) {
        gchar c[256];
        sp_ui_shortcut_string (shortcut, c);

        GtkWidget *hb = gtk_hbox_new (FALSE, 16);

        {
            GtkWidget *l = gtk_label_new_with_mnemonic (action ? action->name : label);
            gtk_misc_set_alignment ((GtkMisc *) l, 0.0, 0.5);
            gtk_box_pack_start ((GtkBox *) hb, l, TRUE, TRUE, 0);
        }

        {
            GtkWidget *l = gtk_label_new (c);
            gtk_misc_set_alignment ((GtkMisc *) l, 1.0, 0.5);
            gtk_box_pack_end ((GtkBox *) hb, l, FALSE, FALSE, 0);
        }

        gtk_widget_show_all (hb);

        item = gtk_check_menu_item_new ();
        gtk_container_add ((GtkContainer *) item, hb);
    } else {
        GtkWidget *l = gtk_label_new_with_mnemonic (action ? action->name : label);
        gtk_misc_set_alignment ((GtkMisc *) l, 0.0, 0.5);
        item = gtk_check_menu_item_new ();
        gtk_container_add ((GtkContainer *) item, l);
    }
  	 
    gtk_widget_show(item);
  	 
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item); 	 

    g_object_set_data (G_OBJECT (item), "view", (gpointer) view); 	 
  	 
    g_signal_connect( G_OBJECT(item), "toggled", (GCallback) callback_toggle, (void *) pref); 	 
    g_signal_connect( G_OBJECT(item), "expose_event", (GCallback) callback_update, (void *) pref);
	 
    g_signal_connect ( G_OBJECT (item), "select", G_CALLBACK (sp_ui_menu_select), (gpointer) (action ? action->tip : tip)); 	 
    g_signal_connect ( G_OBJECT (item), "deselect", G_CALLBACK (sp_ui_menu_deselect), NULL); 	 
}

static void
sp_recent_open (GtkWidget *widget, const gchar *uri)
{
	sp_file_open (uri, NULL);
}

static void
sp_file_new_from_template (GtkWidget *widget, const gchar *uri)
{
	sp_file_new (uri);
}

void
sp_menu_append_new_templates (GtkWidget *menu, SPView *view)
{
    // the Default must be there even if the templates dir is unreadable or empty
    sp_ui_menu_append_item_from_verb (GTK_MENU (menu), Inkscape::Verb::get(SP_VERB_FILE_NEW), view);

    GDir *dir = g_dir_open (INKSCAPE_TEMPLATESDIR, 0, NULL);
    if (!dir)
        return;
    for (const gchar *file = g_dir_read_name (dir); file != NULL; file = g_dir_read_name (dir)) {
        if (!g_str_has_suffix (file, ".svg"))
            continue; // skip non-svg files
        if (g_str_has_suffix (file, "default.svg"))
            continue; // skip default.svg - it's in the menu already

        const gchar *filepath = g_build_filename(INKSCAPE_TEMPLATESDIR, file, NULL);
        const gchar *filename =  g_filename_to_utf8(g_strndup (file, strlen(file) - 4),  -1, NULL, NULL, NULL);

        GtkWidget *item = gtk_menu_item_new_with_label (filename);
        gtk_widget_show(item);
        g_signal_connect(G_OBJECT(item),
                         "activate",
                         G_CALLBACK(sp_file_new_from_template),
                         (gpointer) filepath);

        if (view) {
            // set null tip for now; later use a description from the template file
            g_object_set_data (G_OBJECT (item), "view", (gpointer) view);
            g_signal_connect ( G_OBJECT (item), "select", G_CALLBACK (sp_ui_menu_select), (gpointer) NULL );
            g_signal_connect ( G_OBJECT (item), "deselect", G_CALLBACK (sp_ui_menu_deselect), NULL);
        }

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    g_dir_close (dir);
}


void
sp_menu_append_recent_documents (GtkWidget *menu, SPView* /* view */)
{
	const gchar ** recent;

	recent = prefs_get_recent_files();
	if (recent) {
            int i;

            for (i = 0; recent[i] != NULL; i += 2) {
                const gchar *uri = recent[i];
                const gchar *name = recent[i + 1];

                GtkWidget *item = gtk_menu_item_new_with_label (name);
                gtk_widget_show(item);
                g_signal_connect(G_OBJECT(item),
                                "activate",
                                G_CALLBACK(sp_recent_open),
                                (gpointer)uri);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
            }

            g_free(recent);
	} else {
		GtkWidget *item = gtk_menu_item_new_with_label(_("None"));
		gtk_widget_show(item);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}
}

static void
sp_ui_file_menu (GtkMenu *fm, SPDocument *doc, SPView *view)
{
    sp_ui_menu_append_submenu (fm, view, sp_menu_append_new_templates, _("_New"), _("Create new document"), NULL);

    static Inkscape::Verb * file_verbs_one[] = {
	Inkscape::Verb::get(SP_VERB_FILE_OPEN),
	Inkscape::Verb::get(SP_VERB_LAST)
    };

    sp_ui_menu_append (fm, file_verbs_one, view);

    sp_ui_menu_append_submenu (fm, view, sp_menu_append_recent_documents, _("Open _Recent"), _("Open one of the recently visited documents"), "file_open_recent");

    static Inkscape::Verb * file_verbs_two[] = {
	Inkscape::Verb::get(SP_VERB_FILE_REVERT),
        Inkscape::Verb::get(SP_VERB_FILE_SAVE),
        Inkscape::Verb::get(SP_VERB_FILE_SAVE_AS),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_FILE_IMPORT),
        Inkscape::Verb::get(SP_VERB_FILE_EXPORT),

        Inkscape::Verb::get(SP_VERB_NONE),
        /* commented out until implemented */
	// Inkscape::Verb::get(SP_VERB_FILE_PRINT_PREVIEW),
        Inkscape::Verb::get(SP_VERB_FILE_PRINT),
#if defined(WIN32) || defined(WITH_GNOME_PRINT)
	Inkscape::Verb::get(SP_VERB_FILE_PRINT_DIRECT),
#endif

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_FILE_VACUUM),

        Inkscape::Verb::get(SP_VERB_NONE),
       Inkscape::Verb::get(SP_VERB_DIALOG_NAMEDVIEW),
       Inkscape::Verb::get(SP_VERB_DIALOG_DISPLAY),

        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_FILE_CLOSE_VIEW),
	Inkscape::Verb::get(SP_VERB_FILE_QUIT),
        Inkscape::Verb::get(SP_VERB_LAST)
    };

    sp_ui_menu_append (fm, file_verbs_two, view);
}



static void
sp_ui_edit_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
    static Inkscape::Verb * edit_verbs[] = {
        Inkscape::Verb::get(SP_VERB_EDIT_UNDO),
        Inkscape::Verb::get(SP_VERB_EDIT_REDO),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_EDIT_CUT),
        Inkscape::Verb::get(SP_VERB_EDIT_COPY),
        Inkscape::Verb::get(SP_VERB_EDIT_PASTE),
        Inkscape::Verb::get(SP_VERB_EDIT_PASTE_IN_PLACE),
        Inkscape::Verb::get(SP_VERB_EDIT_PASTE_STYLE),

        Inkscape::Verb::get(SP_VERB_NONE),
       Inkscape::Verb::get(SP_VERB_DIALOG_FIND),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_EDIT_DUPLICATE),
        Inkscape::Verb::get(SP_VERB_EDIT_CLONE),
        Inkscape::Verb::get(SP_VERB_EDIT_UNLINK_CLONE),
        Inkscape::Verb::get(SP_VERB_EDIT_CLONE_ORIGINAL),
        Inkscape::Verb::get(SP_VERB_SELECTION_CREATE_BITMAP),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_EDIT_TILE),
        Inkscape::Verb::get(SP_VERB_EDIT_UNTILE),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_EDIT_DELETE),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_EDIT_SELECT_ALL),
        Inkscape::Verb::get(SP_VERB_EDIT_DESELECT),

        Inkscape::Verb::get(SP_VERB_NONE),
       Inkscape::Verb::get(SP_VERB_DIALOG_XML_EDITOR),

        Inkscape::Verb::get(SP_VERB_LAST
    )};
    sp_ui_menu_append (menu, edit_verbs, view);
} // end of sp_ui_edit_menu


static void
sp_ui_layer_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
    static Inkscape::Verb * layer_verbs[] = {
        Inkscape::Verb::get(SP_VERB_LAYER_NEW),

        Inkscape::Verb::get(SP_VERB_NONE),

        Inkscape::Verb::get(SP_VERB_LAYER_NEXT),
        Inkscape::Verb::get(SP_VERB_LAYER_PREV),

        Inkscape::Verb::get(SP_VERB_NONE),
        
        Inkscape::Verb::get(SP_VERB_LAYER_RAISE),
        Inkscape::Verb::get(SP_VERB_LAYER_LOWER),
        Inkscape::Verb::get(SP_VERB_LAYER_TO_TOP),
        Inkscape::Verb::get(SP_VERB_LAYER_TO_BOTTOM),

        Inkscape::Verb::get(SP_VERB_NONE),

        Inkscape::Verb::get(SP_VERB_LAYER_DELETE),

        Inkscape::Verb::get(SP_VERB_LAST)
    };
    sp_ui_menu_append (menu, layer_verbs, view);
}

static void
sp_ui_object_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
    static Inkscape::Verb * selection[] = {
        Inkscape::Verb::get(SP_VERB_DIALOG_FILL_STROKE),
        Inkscape::Verb::get(SP_VERB_DIALOG_ITEM),
        Inkscape::Verb::get(SP_VERB_NONE),

        Inkscape::Verb::get(SP_VERB_SELECTION_GROUP),
        Inkscape::Verb::get(SP_VERB_SELECTION_UNGROUP),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_SELECTION_RAISE),
        Inkscape::Verb::get(SP_VERB_SELECTION_LOWER),
        Inkscape::Verb::get(SP_VERB_SELECTION_TO_FRONT),
        Inkscape::Verb::get(SP_VERB_SELECTION_TO_BACK),

        Inkscape::Verb::get(SP_VERB_NONE),
        //		Inkscape::Verb::get(SP_VERB_OBJECT_FLATTEN),
        Inkscape::Verb::get(SP_VERB_OBJECT_ROTATE_90_CW),
        Inkscape::Verb::get(SP_VERB_OBJECT_ROTATE_90_CCW),
        Inkscape::Verb::get(SP_VERB_OBJECT_FLIP_HORIZONTAL),
        Inkscape::Verb::get(SP_VERB_OBJECT_FLIP_VERTICAL),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_DIALOG_TRANSFORM),
        Inkscape::Verb::get(SP_VERB_DIALOG_ALIGN_DISTRIBUTE),

        Inkscape::Verb::get(SP_VERB_LAST)
    };
    sp_ui_menu_append (menu, selection, view);
} // end of sp_ui_object_menu


static void
sp_ui_path_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
    static Inkscape::Verb * selection[] = {
        Inkscape::Verb::get(SP_VERB_OBJECT_TO_CURVE),
        Inkscape::Verb::get(SP_VERB_SELECTION_OUTLINE),
        Inkscape::Verb::get(SP_VERB_SELECTION_POTRACE),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_SELECTION_UNION),
        Inkscape::Verb::get(SP_VERB_SELECTION_DIFF),
        Inkscape::Verb::get(SP_VERB_SELECTION_INTERSECT),
        Inkscape::Verb::get(SP_VERB_SELECTION_SYMDIFF),
        Inkscape::Verb::get(SP_VERB_SELECTION_CUT),
        Inkscape::Verb::get(SP_VERB_SELECTION_SLICE),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_SELECTION_COMBINE),
        Inkscape::Verb::get(SP_VERB_SELECTION_BREAK_APART),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_SELECTION_INSET),
        Inkscape::Verb::get(SP_VERB_SELECTION_OFFSET),
        Inkscape::Verb::get(SP_VERB_SELECTION_DYNAMIC_OFFSET),
        Inkscape::Verb::get(SP_VERB_SELECTION_LINKED_OFFSET),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_SELECTION_SIMPLIFY),
        Inkscape::Verb::get(SP_VERB_SELECTION_REVERSE),
        Inkscape::Verb::get(SP_VERB_SELECTION_CLEANUP),
        Inkscape::Verb::get(SP_VERB_LAST)
    };
    sp_ui_menu_append (menu, selection, view);
} // end of sp_ui_path_menu

static void
sp_ui_view_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
    static Inkscape::Verb * view_verbs1[] = {
	Inkscape::Verb::get(SP_VERB_ZOOM_1_1),
	Inkscape::Verb::get(SP_VERB_ZOOM_1_2),
	Inkscape::Verb::get(SP_VERB_ZOOM_2_1),

        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_ZOOM_SELECTION),
	Inkscape::Verb::get(SP_VERB_ZOOM_DRAWING),
	Inkscape::Verb::get(SP_VERB_ZOOM_PAGE),
	Inkscape::Verb::get(SP_VERB_ZOOM_PAGE_WIDTH),

        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_ZOOM_PREV),
	Inkscape::Verb::get(SP_VERB_ZOOM_NEXT),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_LAST)
    };

    static Inkscape::Verb * view_verbs2[] = {
	Inkscape::Verb::get(SP_VERB_DIALOG_TOGGLE),

        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_TOGGLE_GRID),
	Inkscape::Verb::get(SP_VERB_TOGGLE_GUIDES),

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_FULLSCREEN),
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */

        Inkscape::Verb::get(SP_VERB_NONE),
      Inkscape::Verb::get(SP_VERB_DIALOG_DEBUG),
        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_FILE_PREV_DESKTOP),
	Inkscape::Verb::get(SP_VERB_FILE_NEXT_DESKTOP),
        Inkscape::Verb::get(SP_VERB_NONE),
	Inkscape::Verb::get(SP_VERB_VIEW_NEW),
	// Inkscape::Verb::get(SP_VERB_VIEW_NEW_PREVIEW),
        Inkscape::Verb::get(SP_VERB_LAST)
    };

    sp_ui_menu_append (menu, view_verbs1, view);

    GtkWidget *item_showhide = sp_ui_menu_append_item (menu, NULL, _("_Show/Hide"), _("Show or hide parts of the document window (differently for normal and fullscreen modes)"), view, NULL, NULL);
    GtkMenu *m = (GtkMenu *) gtk_menu_new ();

//    sp_ui_menu_append_check_item_from_verb (m, view, _("_Menu"), _("Show or hide the menu bar"), "menu",
//                                  checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb (m, view, _("Commands Bar"), _("Show or hide the Commands bar (under the menu)"), "commands",
                                  checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb (m, view, _("Tool Controls"), _("Show or hide the Tool Controls panel"), "toppanel",
                                  checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb (m, view, _("_Toolbox"), _("Show or hide the main toolbox (on the left)"), "toolbox",
                                  checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb (m, view, NULL, NULL, "rulers",
                                  checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_RULERS));
    sp_ui_menu_append_check_item_from_verb (m, view, NULL, NULL, "scrollbars",
                                  checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_SCROLLBARS));
    sp_ui_menu_append_check_item_from_verb (m, view, _("_Statusbar"), _("Show or hide the statusbar (at the bottom of the window)"), "statusbar",
                                  checkitem_toggled, checkitem_update, 0);

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_showhide), GTK_WIDGET (m));

    sp_ui_menu_append (menu, view_verbs2, view);
}

static void
sp_ui_text_menu (GtkMenu *menu, SPDocument *doc, SPView *view)
{
    static Inkscape::Verb * text_verbs[] = {
        Inkscape::Verb::get(SP_VERB_DIALOG_TEXT),

        Inkscape::Verb::get(SP_VERB_NONE),
        Inkscape::Verb::get(SP_VERB_SELECTION_TEXTTOPATH),

        Inkscape::Verb::get(SP_VERB_LAST)
    };

    sp_ui_menu_append (menu, text_verbs, view);
}

static void
sp_ui_help_menu (GtkMenu *fm, SPDocument *doc, SPView *view)
{
    GtkWidget *item_tutorials, *menu_tutorials;

    static Inkscape::Verb * help_verbs_one[] = {
        Inkscape::Verb::get(SP_VERB_HELP_KEYS), Inkscape::Verb::get(SP_VERB_LAST)
    };

    static Inkscape::Verb * tutorial_verbs[] = {
        Inkscape::Verb::get(SP_VERB_TUTORIAL_BASIC),
	Inkscape::Verb::get(SP_VERB_TUTORIAL_ADVANCED),
	Inkscape::Verb::get(SP_VERB_TUTORIAL_DESIGN),
	Inkscape::Verb::get(SP_VERB_TUTORIAL_TIPS),
	
	Inkscape::Verb::get(SP_VERB_LAST)
    };

    static Inkscape::Verb * help_verbs_two[] = {
        Inkscape::Verb::get(SP_VERB_HELP_ABOUT), Inkscape::Verb::get(SP_VERB_LAST)
    };

    sp_ui_menu_append (fm, help_verbs_one, view);

    /* There isn't a way to handle "sub menus" (which could really be seen as a
     * list of verb arguments) in the verb system right now, so we have to build
     * the submenu by hand.  Luckily, we can populate it using the same verb system.
     */
    item_tutorials = sp_ui_menu_append_item (fm, NULL, _("_Tutorials"), _("Interactive Inkscape tutorials"), view, NULL, NULL);
    /* should sp_ui_menu_append_item be modified to take an image name? */
    sp_ui_menuitem_add_icon (item_tutorials, "help_tutorials");
    menu_tutorials = gtk_menu_new ();
    sp_ui_menu_append (GTK_MENU (menu_tutorials), tutorial_verbs, view);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item_tutorials), menu_tutorials);

    sp_ui_menu_append (fm, help_verbs_two, view);

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

	mitem = gtk_menu_item_new_with_mnemonic (_("_Layer"));
	menu = gtk_menu_new ();
	sp_ui_layer_menu (GTK_MENU (menu), NULL, view);
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

	mitem = gtk_menu_item_new_with_mnemonic (_("_Text"));
	menu = gtk_menu_new ();
	sp_ui_text_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	mitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
	menu = gtk_menu_new ();
	sp_ui_help_menu (GTK_MENU (menu), NULL, view);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), GTK_WIDGET (menu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mbar), mitem);

	return mbar;
}

static void leave_group(GtkMenuItem *, SPDesktop *desktop) {
    desktop->setCurrentLayer(SP_OBJECT_PARENT(desktop->currentLayer()));
}

static void leave_all_groups(GtkMenuItem *, SPDesktop *desktop) {
    desktop->setCurrentLayer(desktop->currentRoot());
}

static void enter_group(GtkMenuItem *mi, SPDesktop *desktop) {
    desktop->setCurrentLayer(reinterpret_cast<SPObject *>(g_object_get_data(G_OBJECT(mi), "group")));
}

GtkWidget *
sp_ui_context_menu (SPView *view, SPItem *item)
{
	GtkWidget *m;
	SPDesktop *dt;

	dt = (SP_IS_DESKTOP (view)) ? SP_DESKTOP (view) : NULL;

	m = gtk_menu_new ();

	/* Undo and Redo */
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_UNDO), view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_REDO), view);

	/* Separator */
	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL, NULL, NULL);

	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_CUT), view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_COPY), view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_PASTE), view);

	/* Separator */
	sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL, NULL, NULL);

	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_DUPLICATE), view);
	sp_ui_menu_append_item_from_verb (GTK_MENU (m), Inkscape::Verb::get(SP_VERB_EDIT_DELETE), view);

	/* Item menu */
	if (item) {
		sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL, NULL, NULL);
		sp_object_menu ((SPObject *) item, dt, GTK_MENU (m));
	}

        /* layer menu */
        SPGroup *group=NULL;
        if (item) {
            if (SP_IS_GROUP(item)) {
                group = SP_GROUP(item);
            } else if ( item != dt->currentRoot() && SP_IS_GROUP(SP_OBJECT_PARENT(item)) ) {
                group = SP_GROUP(SP_OBJECT_PARENT(item));
            }
        }

        if ( group && group != dt->currentLayer() ) {
            sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL, NULL, NULL);
            /* TRANSLATORS: #%s is the id of the group e.g. <g id="#g7">, not a number. */
            gchar *label=g_strdup_printf(_("Edit group #%s"), SP_OBJECT_ID(group));
            GtkWidget *w = gtk_menu_item_new_with_label(label);
            g_free(label);
            g_object_set_data(G_OBJECT(w), "group", group);
            g_signal_connect(G_OBJECT(w), "activate", GCallback(enter_group), dt);
            gtk_widget_show(w);
            gtk_menu_shell_append(GTK_MENU_SHELL(m), w);
        }

        if ( dt->currentLayer() != dt->currentRoot() ) {
            sp_ui_menu_append_item (GTK_MENU (m), NULL, NULL, NULL, NULL, NULL, NULL);
            if ( SP_OBJECT_PARENT(dt->currentLayer()) != dt->currentRoot() ) {
                GtkWidget *w = gtk_menu_item_new_with_label(_("Edit parent group"));
                g_signal_connect(G_OBJECT(w), "activate", GCallback(leave_group), dt);
                gtk_widget_show(w);
                gtk_menu_shell_append(GTK_MENU_SHELL(m), w);

            }
            GtkWidget *w = gtk_menu_item_new_with_label(_("Edit root"));
            g_signal_connect(G_OBJECT(w), "activate", GCallback(leave_all_groups), dt);
            gtk_widget_show(w);
            gtk_menu_shell_append(GTK_MENU_SHELL(m), w);
        }

	return m;
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

                SP_DOCUMENT_ROOT(doc)->appendChildRepr(newgroup);
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

                SP_DOCUMENT_ROOT(doc)->appendChildRepr(newgroup);
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
                        SP_DOCUMENT_ROOT(doc)->appendChildRepr(repr);
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

bool
sp_ui_overwrite_file (const gchar * filename)
{
	bool return_value = FALSE;
	GtkWidget * dialog;
	GtkWidget * hbox;
	GtkWidget * boxdata;
	gchar * title;
	gchar * text;

	if (g_file_test(filename, G_FILE_TEST_EXISTS)) {

		title = g_strdup_printf(_("Overwrite %s"), filename);
		dialog = gtk_dialog_new_with_buttons (title,
											  NULL,
											  (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
											  GTK_STOCK_NO,
											  GTK_RESPONSE_NO,
											  GTK_STOCK_YES,
											  GTK_RESPONSE_YES,
											  NULL);
		gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);

		sp_transientize (dialog);
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

		hbox = gtk_hbox_new(FALSE, 5);
		boxdata = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
		gtk_widget_show(boxdata);
		gtk_box_pack_start(GTK_BOX(hbox), boxdata, TRUE, TRUE, 5);
		text = g_strdup_printf(_("The file %s already exists.  Do you want to overwrite that file with the current document?"), filename);
		boxdata = gtk_label_new(text);
		gtk_label_set_line_wrap(GTK_LABEL(boxdata), TRUE);
		gtk_widget_show(boxdata);
		gtk_box_pack_start(GTK_BOX(hbox), boxdata, FALSE, FALSE, 5);
		gtk_widget_show(hbox);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE, 5);

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
			return_value = TRUE;
		} else {
			return_value = FALSE;
		}

		gtk_widget_destroy(dialog);
		g_free(title);
		g_free(text);
	} else {
		return_value = TRUE;
	}

	return return_value;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
