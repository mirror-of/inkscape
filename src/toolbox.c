#define __SP_MAINTOOLBOX_C__

/*
 * Main toolbox
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
#include <gtk/gtksignal.h>
#include <gtk/gtkselection.h>
#include <gtk/gtktable.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtklabel.h>

#include "macros.h"
#include "helper/window.h"
#include "widgets/icon.h"
#include "widgets/button.h"

#include "prefs-utils.h"
#include "inkscape-stock.h"
#include "verbs.h"
#include "file.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "inkscape-private.h"
#include "document.h"
#include "inkscape.h"
#include "extension.h"
#include "sp-item-transform.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "interface.h"
#include "toolbox.h"
#include "xml/repr-private.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"

#include "dialogs/object-properties.h"
#include "dialogs/transformation.h"
#include "dialogs/text-edit.h"
#include "dialogs/align.h"
#include "dialogs/export.h"
#include "dialogs/node-edit.h"
#include "dialogs/dialog-events.h"

#define TOOL_BUTTON_SIZE 24
#define COMMAND_BUTTON_SIZE 24

static GtkWidget * sp_select_toolbox_new ();
static GtkWidget * sp_node_toolbox_new ();
static GtkWidget * sp_zoom_toolbox_new ();

static const struct {
	const gchar *type_name;
	const gchar *data_name;
	unsigned int verb;
} tools[] = {
	{ "SPSelectContext", "select_tool", SP_VERB_CONTEXT_SELECT },
	{ "SPNodeContext", "node_tool", SP_VERB_CONTEXT_NODE },
	{ "SPZoomContext", "zoom_tool", SP_VERB_CONTEXT_ZOOM },
	{ "SPRectContext", "rect_tool", SP_VERB_CONTEXT_RECT },
	{ "SPArcContext", "arc_tool", SP_VERB_CONTEXT_ARC },
	{ "SPStarContext", "star_tool", SP_VERB_CONTEXT_STAR },
	{ "SPSpiralContext", "spiral_tool", SP_VERB_CONTEXT_SPIRAL },
	{ "SPPencilContext", "pencil_tool", SP_VERB_CONTEXT_PENCIL },
	{ "SPPenContext", "pen_tool", SP_VERB_CONTEXT_PEN },
	{ "SPDynaDrawContext", "dyna_draw_tool", SP_VERB_CONTEXT_CALLIGRAPHIC },
	{ "SPTextContext", "text_tool", SP_VERB_CONTEXT_TEXT },
	{ "SPDropperContext", "dropper_tool", SP_VERB_CONTEXT_DROPPER },
	{ NULL, 0 }
};

static const struct {
	const gchar *type_name;
	const gchar *data_name;
	GtkWidget *(*create_func)(void);
} aux_toolboxes[] = {
	{ "SPSelectContext", "select_toolbox", sp_select_toolbox_new },
	{ "SPNodeContext", "node_toolbox", sp_node_toolbox_new },
	{ "SPZoomContext", "zoom_toolbox", sp_zoom_toolbox_new },
	{ NULL, NULL, NULL }
};

static void toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, void (*update_func)(SPDesktop * desktop, SPEventContext * eventcontext, GtkWidget *toolbox));
static void update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);
static void update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static GtkWidget *
sp_toolbox_button_new (GtkWidget *t, unsigned int size, const gchar *pxname, GtkSignalFunc handler, GtkTooltips *tt, const gchar *tip)
{
	GtkWidget *b;

	b = sp_button_new_from_data (size, SP_BUTTON_TYPE_NORMAL, pxname, tip, tt);
	gtk_widget_show (b);
	if (handler) gtk_signal_connect (GTK_OBJECT (b), "clicked", handler, NULL);
	gtk_box_pack_start (GTK_BOX (t), b, FALSE, FALSE, 0);

	return b;
}

static GtkWidget *
sp_toolbox_button_new_from_verb (GtkWidget *t, unsigned int size, unsigned int type, unsigned int verb, GtkTooltips *tt)
{
	SPAction *action;
	GtkWidget *b;

	action = sp_verb_get_action (verb);
	if (!action) return NULL;
	/* fixme: Handle sensitive/unsensitive */
	/* fixme: Implement sp_button_new_from_action */
	b = sp_button_new (size, type, action, tt);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (t), b, FALSE, FALSE, 0);
	return b;
}

GtkWidget *
sp_tool_toolbox_new ()
{
	GtkWidget *tb;
	GtkTooltips *tt;
	int i;
	
	tt = gtk_tooltips_new ();
	tb = gtk_vbox_new (FALSE, 0);

	for ( i = 0 ; tools[i].type_name ; i++ ) {
		GtkWidget *button;
		button = sp_toolbox_button_new_from_verb (tb, TOOL_BUTTON_SIZE, SP_BUTTON_TYPE_TOGGLE, tools[i].verb, tt);
		g_object_set_data (G_OBJECT (tb), tools[i].data_name, (gpointer)button);
	}

	g_object_set_data (G_OBJECT (tb), "desktop", NULL);

	update_tool_toolbox (NULL, NULL, tb);
	gtk_widget_set_sensitive (tb, FALSE);

	return tb;
}

GtkWidget *
sp_aux_toolbox_new ()
{
	GtkWidget *tb;
	int i;

	tb = gtk_vbox_new (FALSE, 0);

	for ( i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
		GtkWidget *sub_toolbox;
		sub_toolbox = aux_toolboxes[i].create_func();
		gtk_container_add (GTK_CONTAINER (tb), sub_toolbox);
		g_object_set_data (G_OBJECT (tb), aux_toolboxes[i].data_name, sub_toolbox);
	}

	g_object_set_data (G_OBJECT (tb), "desktop", NULL);

	update_aux_toolbox (NULL, NULL, tb);
	gtk_widget_set_sensitive (tb, FALSE);

	return tb;
}

static GtkWidget *
sp_select_toolbox_new ()
{
	GtkWidget *tb;
	GtkTooltips *tt;

	tt = gtk_tooltips_new ();
	tb = gtk_hbox_new (FALSE, 0);

	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_GROUP, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_UNGROUP, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_TO_FRONT, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_TO_BACK, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_RAISE, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_LOWER, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_OBJECT_ROTATE_90, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_OBJECT_FLIP_HORIZONTAL, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_OBJECT_FLIP_VERTICAL, tt);

	return tb;
}

static GtkWidget *
sp_node_toolbox_new ()
{
	GtkWidget *tb;
	GtkTooltips *tt;

	tt = gtk_tooltips_new ();
	tb = gtk_hbox_new (FALSE, 0);

	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_insert", GTK_SIGNAL_FUNC (sp_node_path_edit_add), tt, _("Insert new nodes into selected segments"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_delete", GTK_SIGNAL_FUNC (sp_node_path_edit_delete), tt, _("Delete selected nodes"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_join", GTK_SIGNAL_FUNC (sp_node_path_edit_join), tt, _("Join lines at selected nodes"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_join_segment", GTK_SIGNAL_FUNC (sp_node_path_edit_join_segment), tt, _("Join lines at selected nodes with new segment"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_break", GTK_SIGNAL_FUNC (sp_node_path_edit_break), tt, _("Break line at selected nodes"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_cusp", GTK_SIGNAL_FUNC (sp_node_path_edit_cusp), tt, _("Make selected nodes corner"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_smooth", GTK_SIGNAL_FUNC (sp_node_path_edit_smooth), tt, _("Make selected nodes smooth"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_line", GTK_SIGNAL_FUNC (sp_node_path_edit_toline), tt, _("Make selected segments lines"));
	sp_toolbox_button_new (tb, COMMAND_BUTTON_SIZE, "node_curve", GTK_SIGNAL_FUNC (sp_node_path_edit_tocurve), tt, _("Make selected segments curves"));

	return tb;
}

static GtkWidget *
sp_zoom_toolbox_new ()
{
	GtkWidget *tb;
	GtkTooltips *tt;

	tt = gtk_tooltips_new ();
	tb = gtk_hbox_new (FALSE, 0);

	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_IN, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_OUT, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_SELECTION, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_DRAWING, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_PAGE, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_1_1, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_1_2, tt);
	sp_toolbox_button_new_from_verb(tb, COMMAND_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_2_1, tt);

	return tb;
}

void
sp_tool_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop)
{
	toolbox_set_desktop (toolbox, desktop, update_tool_toolbox);
}

void
sp_aux_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop)
{
	toolbox_set_desktop (toolbox, desktop, update_aux_toolbox);
}

static void
toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, void (*update_func)(SPDesktop * desktop, SPEventContext * eventcontext, GtkWidget *toolbox))
{
	SPDesktop *old_desktop;

	old_desktop = SP_DESKTOP (g_object_get_data (G_OBJECT (toolbox), "desktop"));

	if (old_desktop) {
		g_signal_handlers_disconnect_by_func (G_OBJECT (old_desktop), (void*)G_CALLBACK (update_func), (gpointer)toolbox);
	}

	g_object_set_data (G_OBJECT (toolbox), "desktop", (gpointer)desktop);

	if (desktop) {
		gtk_widget_set_sensitive (toolbox, TRUE);
		update_func (desktop, SP_DESKTOP_EVENT_CONTEXT(desktop), toolbox);
		g_signal_connect (G_OBJECT (desktop), "event_context_changed", G_CALLBACK (update_func), (gpointer)toolbox);
	} else {
		update_func (NULL, NULL, toolbox);
		gtk_widget_set_sensitive (toolbox, FALSE);
	}
}

static void 
update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox)
{
	const gchar *tname;
	int i;

	if (eventcontext != NULL) {
		tname = gtk_type_name (GTK_OBJECT_TYPE (eventcontext));
	} else {
		tname = NULL;
	}

	for ( i = 0 ; tools[i].type_name ; i++ ) {
		SPButton *button=SP_BUTTON (g_object_get_data (G_OBJECT (toolbox),
		                                               tools[i].data_name));
		sp_button_toggle_set_down (button,
		                           tname && !strcmp(tname, tools[i].type_name),
		                           FALSE);
	}
}

static void
update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox) {
	const gchar *tname;
	int i;

	if (eventcontext != NULL) {
		tname = gtk_type_name (GTK_OBJECT_TYPE (eventcontext));
	} else {
		tname = NULL;
	}

	for ( i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
		GtkWidget *sub_toolbox = GTK_WIDGET (g_object_get_data (G_OBJECT (toolbox), aux_toolboxes[i].data_name));
		if (tname && !strcmp(tname, aux_toolboxes[i].type_name)) {
			gtk_widget_show (sub_toolbox);
		} else {
			gtk_widget_hide (sub_toolbox);
		}
	}
}

