#define __SP_MAINTOOLBOX_C__

/*
 * Main toolbox
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 2003 MenTaLguY
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
#include <gtk/gtkhandlebox.h>

#include "macros.h"
#include "helper/window.h"
#include "widgets/icon.h"
#include "widgets/button.h"
#include "widgets/widget-sizes.h"

#include "prefs-utils.h"
#include "inkscape-stock.h"
#include "verbs.h"
#include "file.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "inkscape-private.h"
#include "document.h"
#include "inkscape.h"
#include "sp-item-transform.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "interface.h"
#include "nodepath.h"
#include "xml/repr-private.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"

#include "dialogs/object-properties.h"
#include "dialogs/transformation.h"
#include "dialogs/text-edit.h"
#include "dialogs/align.h"
#include "dialogs/export.h"
#include "dialogs/dialog-events.h"

#include "select-toolbar.h"

#include "toolbox.h"

typedef void (*SetupFunction)(GtkWidget *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static GtkWidget * sp_node_toolbox_new (SPDesktop *desktop);
static GtkWidget * sp_zoom_toolbox_new (SPDesktop *desktop);

static const struct {
	const gchar *type_name;
	const gchar *data_name;
	sp_verb_t verb;
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
	{ NULL, NULL, 0 }
};

static const struct {
	const gchar *type_name;
	const gchar *data_name;
	GtkWidget *(*create_func)(SPDesktop *desktop);
} aux_toolboxes[] = {
	{ "SPSelectContext", "select_toolbox", sp_select_toolbox_new },
	{ "SPNodeContext", "node_toolbox", sp_node_toolbox_new },
	{ "SPZoomContext", "zoom_toolbox", sp_zoom_toolbox_new },
	{ NULL, NULL, NULL }
};

static void toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func);

static void setup_tool_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);
static void setup_aux_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static GtkWidget *
sp_toolbox_button_new (GtkWidget *t, unsigned int size, const gchar *pxname, GtkSignalFunc handler, GtkTooltips *tt, const gchar *tip)
{
	GtkWidget *b = sp_button_new_from_data (size, SP_BUTTON_TYPE_NORMAL, NULL, pxname, tip, tt);
	gtk_widget_show (b);
	if (handler) gtk_signal_connect (GTK_OBJECT (b), "clicked", handler, NULL);
	gtk_box_pack_start (GTK_BOX (t), b, FALSE, FALSE, 0);

	return b;
}

GtkWidget *
sp_toolbox_button_new_from_verb (GtkWidget *t, unsigned int size, SPButtonType type, sp_verb_t verb, SPView *view, GtkTooltips *tt)
{
	SPAction *action = sp_verb_get_action (verb, view);
	if (!action) return NULL;
	/* fixme: Handle sensitive/unsensitive */
	/* fixme: Implement sp_button_new_from_action */
	GtkWidget *b = sp_button_new (size, type, action, tt);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (t), b, FALSE, FALSE, 0);
	return b;
}

GtkWidget * sp_toolbox_button_normal_new_from_verb (GtkWidget *t, unsigned int size, sp_verb_t verb, SPView *view, GtkTooltips *tt)
{
	return sp_toolbox_button_new_from_verb (t, size, SP_BUTTON_TYPE_NORMAL, verb, view, tt);
}

GtkWidget *
sp_tool_toolbox_new ()
{
	GtkTooltips *tt = gtk_tooltips_new ();
	GtkWidget *tb = gtk_vbox_new (FALSE, 0);

	g_object_set_data (G_OBJECT (tb), "desktop", NULL);
	g_object_set_data (G_OBJECT (tb), "tooltips", tt);

	gtk_widget_set_sensitive (tb, FALSE);

	GtkWidget *hb = gtk_handle_box_new ();
	gtk_handle_box_set_handle_position (GTK_HANDLE_BOX (hb), GTK_POS_TOP);
	gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX (hb), GTK_SHADOW_OUT);
	gtk_handle_box_set_snap_edge (GTK_HANDLE_BOX (hb), GTK_POS_LEFT);

	gtk_container_add (GTK_CONTAINER (hb), tb);
	gtk_widget_show (GTK_WIDGET (tb));

	return hb;
}

static void
aux_toolbox_size_request (GtkWidget *widget,
                          GtkRequisition *requisition,
                          gpointer user_data)
{
	if ( requisition->height < AUX_BUTTON_SIZE + 6 + 2 * AUX_BETWEEN_BUTTON_GROUPS) {
		requisition->height = AUX_BUTTON_SIZE + 6 + 2 * AUX_BETWEEN_BUTTON_GROUPS;
	}
	if (!g_object_get_data(G_OBJECT(widget), "is_detached")) {
		requisition->width = 0; // allow aux toolbar to be cut
	}
}

static void
aux_toolbox_attached(GtkHandleBox *toolbox, GtkWidget *child)
{
	g_object_set_data(G_OBJECT(child), "is_detached", GINT_TO_POINTER(FALSE));
	gtk_widget_queue_resize(child);
}

static void
aux_toolbox_detached(GtkHandleBox *toolbox, GtkWidget *child)
{
	g_object_set_data(G_OBJECT(child), "is_detached", GINT_TO_POINTER(TRUE));
	gtk_widget_queue_resize(child);
}

GtkWidget *
sp_aux_toolbox_new ()
{
	GtkWidget *tb = gtk_vbox_new (FALSE, 0);

	GtkWidget *tb_s = gtk_vbox_new (FALSE, 0);
	GtkWidget *tb_e = gtk_vbox_new (FALSE, 0);
	gtk_box_set_spacing (GTK_BOX (tb), AUX_BETWEEN_BUTTON_GROUPS);
	gtk_box_pack_start (GTK_BOX (tb), GTK_WIDGET (tb_s), FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (tb), GTK_WIDGET (tb_e), FALSE, FALSE, 0);

	g_object_set_data (G_OBJECT (tb), "desktop", NULL);

	gtk_widget_set_sensitive (tb, FALSE);

	g_signal_connect_after(G_OBJECT(tb), "size_request", G_CALLBACK(aux_toolbox_size_request), NULL);

	GtkWidget *hb = gtk_handle_box_new ();
	gtk_handle_box_set_handle_position (GTK_HANDLE_BOX (hb), GTK_POS_LEFT);
	gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX (hb), GTK_SHADOW_OUT);
	gtk_handle_box_set_snap_edge (GTK_HANDLE_BOX (hb), GTK_POS_LEFT);

	g_signal_connect(G_OBJECT(hb), "child_attached", G_CALLBACK(aux_toolbox_attached), (gpointer)tb);
	g_signal_connect(G_OBJECT(hb), "child_detached", G_CALLBACK(aux_toolbox_detached), (gpointer)tb);

	gtk_container_add (GTK_CONTAINER (hb), tb);
	gtk_widget_show (GTK_WIDGET (tb));

	return hb;
}


//####################################
//# node editing callbacks
//####################################

void
sp_node_path_edit_add (void)
{
	sp_node_selected_add_node ();
}

void
sp_node_path_edit_delete (void)
{
	sp_node_selected_delete ();
}

void
sp_node_path_edit_delete_segment (void)
{
	sp_node_selected_delete_segment ();
}

void
sp_node_path_edit_break (void)
{
	sp_node_selected_break ();
}

void
sp_node_path_edit_join (void)
{
	sp_node_selected_join ();
}

void
sp_node_path_edit_join_segment (void)
{
	sp_node_selected_join_segment ();
}

void
sp_node_path_edit_toline (void)
{
	sp_node_selected_set_line_type (ART_LINETO);
}

void
sp_node_path_edit_tocurve (void)
{
	sp_node_selected_set_line_type (ART_CURVETO);
}

void
sp_node_path_edit_cusp (void)
{
	sp_node_selected_set_type (SP_PATHNODE_CUSP);
}

void
sp_node_path_edit_smooth (void)
{
	sp_node_selected_set_type (SP_PATHNODE_SMOOTH);
}

void
sp_node_path_edit_symmetrical (void)
{
	sp_node_selected_set_type (SP_PATHNODE_SYMM);
}



//####################################
//# node editing toolbox
//####################################

static GtkWidget *
sp_node_toolbox_new (SPDesktop *desktop)
{
	SPView *view=SP_VIEW (desktop);

	GtkTooltips *tt = gtk_tooltips_new ();
	GtkWidget *tb = gtk_hbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_insert",
		GTK_SIGNAL_FUNC (sp_node_path_edit_add), tt, _("Insert new nodes into selected segments"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_delete",
		GTK_SIGNAL_FUNC (sp_node_path_edit_delete), tt, _("Delete selected nodes"));

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_join",
		 GTK_SIGNAL_FUNC(sp_node_path_edit_join), tt, _("Join paths at selected nodes"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_join_segment",
		GTK_SIGNAL_FUNC (sp_node_path_edit_join_segment), tt, _("Join paths at selected nodes with new segment"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_delete_segment",
		GTK_SIGNAL_FUNC (sp_node_path_edit_delete_segment), tt, _("Split path between two or more nodes"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_break",
		GTK_SIGNAL_FUNC (sp_node_path_edit_break), tt, _("Break path at selected nodes"));

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_cusp",
		GTK_SIGNAL_FUNC (sp_node_path_edit_cusp), tt, _("Make selected nodes corner"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_smooth",
		GTK_SIGNAL_FUNC (sp_node_path_edit_smooth), tt, _("Make selected nodes smooth"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_symetric",
		GTK_SIGNAL_FUNC (sp_node_path_edit_symmetrical), tt, _("Make selected nodes symmetric"));

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_line",
		GTK_SIGNAL_FUNC (sp_node_path_edit_toline), tt, _("Make selected segments lines"));
	sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_curve",
		GTK_SIGNAL_FUNC (sp_node_path_edit_tocurve), tt, _("Make selected segments curves"));
	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_OBJECT_TO_CURVE, view, tt);

	gtk_widget_show_all (tb);

	return tb;
}

static GtkWidget *
sp_zoom_toolbox_new (SPDesktop *desktop)
{
	SPView *view=SP_VIEW (desktop);

	GtkTooltips *tt = gtk_tooltips_new ();
	GtkWidget *tb = gtk_hbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_IN, view, tt);
	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_OUT, view, tt);

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_SELECTION, view, tt);
	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_DRAWING, view, tt);
	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_PAGE, view, tt);

	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_1_1, view, tt);
	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_1_2, view, tt);
	sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_2_1, view, tt);

	gtk_widget_show_all (tb);

	return tb;
}

void
sp_tool_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop)
{
	toolbox_set_desktop (gtk_bin_get_child (GTK_BIN (toolbox)), desktop, setup_tool_toolbox, update_tool_toolbox);
}

void
sp_aux_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop)
{
	toolbox_set_desktop (gtk_bin_get_child (GTK_BIN (toolbox)), desktop, setup_aux_toolbox, update_aux_toolbox);
}

static void
toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func)
{
	gpointer ptr = g_object_get_data (G_OBJECT (toolbox), "desktop");
	SPDesktop *old_desktop = SP_IS_DESKTOP (ptr) ? SP_DESKTOP (ptr) : NULL;

	if (old_desktop) {
		GList *children, *iter;
		g_signal_handlers_disconnect_by_func (G_OBJECT (old_desktop), (void*)G_CALLBACK (update_func), (gpointer)toolbox);
		children = gtk_container_get_children (GTK_CONTAINER (toolbox));
		for ( iter = children ; iter ; iter = iter->next ) {
			gtk_container_remove (GTK_CONTAINER (toolbox), GTK_WIDGET (iter->data));
		}
		g_list_free (children);
	}

	g_object_set_data (G_OBJECT (toolbox), "desktop", (gpointer)desktop);

	if (desktop) {
		gtk_widget_set_sensitive (toolbox, TRUE);
		setup_func (toolbox, desktop);
		update_func (desktop, SP_DESKTOP_EVENT_CONTEXT(desktop), toolbox);
		g_signal_connect (G_OBJECT (desktop), "event_context_changed", G_CALLBACK (update_func), (gpointer)toolbox);
	} else {
		gtk_widget_set_sensitive (toolbox, FALSE);
	}
}

static void 
setup_tool_toolbox (GtkWidget *toolbox, SPDesktop *desktop)
{
	GtkTooltips *tooltips=GTK_TOOLTIPS (g_object_get_data (G_OBJECT (toolbox), "tooltips"));

	for (int i = 0 ; tools[i].type_name ; i++ ) {
		GtkWidget *button = sp_toolbox_button_new_from_verb (toolbox, TOOL_BUTTON_SIZE, SP_BUTTON_TYPE_TOGGLE, tools[i].verb, SP_VIEW (desktop), tooltips);
		g_object_set_data (G_OBJECT (toolbox), tools[i].data_name, (gpointer)button);
	}

}

static void
update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox)
{
	const gchar *tname;

	if (eventcontext != NULL) {
		tname = gtk_type_name (GTK_OBJECT_TYPE (eventcontext));
	} else {
		tname = NULL;
	}

	for (int i = 0 ; tools[i].type_name ; i++ ) {
		SPButton *button=SP_BUTTON (g_object_get_data (G_OBJECT (toolbox), tools[i].data_name));
		sp_button_toggle_set_down (button, tname && !strcmp(tname, tools[i].type_name));
	}
}

static void
setup_aux_toolbox (GtkWidget *toolbox, SPDesktop *desktop)
{
	for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
		GtkWidget *sub_toolbox=aux_toolboxes[i].create_func (desktop);
		gtk_container_add (GTK_CONTAINER (toolbox), sub_toolbox);
		g_object_set_data (G_OBJECT (toolbox), aux_toolboxes[i].data_name, sub_toolbox);
	}
}

static void
update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox)
{
	const gchar *tname;

	if (eventcontext != NULL) {
		tname = gtk_type_name (GTK_OBJECT_TYPE (eventcontext));
	} else {
		tname = NULL;
	}

	for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
		GtkWidget *sub_toolbox = GTK_WIDGET (g_object_get_data (G_OBJECT (toolbox), aux_toolboxes[i].data_name));
		if (tname && !strcmp(tname, aux_toolboxes[i].type_name)) {
			gtk_widget_show (sub_toolbox);
		} else {
			gtk_widget_hide (sub_toolbox);
		}
	}
}

void
aux_toolbox_space (GtkWidget *tb, gint space)
{
	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, space);
}

