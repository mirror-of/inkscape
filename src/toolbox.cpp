#define __SP_MAINTOOLBOX_C__

/*
 * Main toolbox
*
* Authors:
*   MenTaLguY <mental@rydia.net>
*   Lauris Kaplinski <lauris@kaplinski.com>
*   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
*
* Copyright (C) 2003 MenTaLguY
* Copyright (C) 1999-2002 authors
* Copyright (C) 2001-2002 Ximian, Inc.
*
* Released under GNU GPL, read the file 'COPYING' for more information
*/

#include <config.h>

#include <sigc++/sigc++.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkmenu.h>
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
#include "widgets/spw-utilities.h"
#include "widgets/sp-widget.h"
#include "widgets/spinbutton-events.h"

#include "prefs-utils.h"
#include "inkscape-stock.h"
#include "verbs.h"
#include "file.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "inkscape-private.h"
#include "document.h"
#include "inkscape.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "interface.h"
#include "nodepath.h"
#include "xml/repr-private.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"
#include "helper/unit-menu.h"
#include "helper/units.h"

#include "dialogs/object-properties.h"
#include "dialogs/transformation.h"
#include "dialogs/text-edit.h"
#include "dialogs/align.h"
#include "dialogs/export.h"
#include "dialogs/dialog-events.h"

#include "select-toolbar.h"
#include "star-context.h"
#include "spiral-context.h"
#include "sp-rect.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-pattern.h"
#include "sp-ellipse.h"
#include "selection.h"
#include "document-private.h"

#include "mod360.h"

#include "toolbox.h"

typedef void (*SetupFunction)(GtkWidget *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static GtkWidget * sp_node_toolbox_new (  SPDesktop *desktop);
static GtkWidget * sp_zoom_toolbox_new (  SPDesktop *desktop);
static GtkWidget * sp_star_toolbox_new (  SPDesktop *desktop);
static GtkWidget * sp_arc_toolbox_new (  SPDesktop *desktop);
static GtkWidget * sp_rect_toolbox_new (  SPDesktop *desktop);
static GtkWidget * sp_spiral_toolbox_new (SPDesktop *desktop);
static GtkWidget * sp_calligraphy_toolbox_new (SPDesktop *desktop);
static GtkWidget * sp_empty_toolbox_new (SPDesktop *desktop);

static const struct {
    const gchar *type_name;
    const gchar *data_name;
    sp_verb_t verb;
} tools[] = {
    { "SPSelectContext",   "select_tool",    SP_VERB_CONTEXT_SELECT },
    { "SPNodeContext",     "node_tool",      SP_VERB_CONTEXT_NODE },
    { "SPZoomContext",     "zoom_tool",      SP_VERB_CONTEXT_ZOOM },
    { "SPRectContext",     "rect_tool",      SP_VERB_CONTEXT_RECT },
    { "SPArcContext",      "arc_tool",       SP_VERB_CONTEXT_ARC },
    { "SPStarContext",     "star_tool",      SP_VERB_CONTEXT_STAR },
    { "SPSpiralContext",   "spiral_tool",    SP_VERB_CONTEXT_SPIRAL },
    { "SPPencilContext",   "pencil_tool",    SP_VERB_CONTEXT_PENCIL },
    { "SPPenContext",      "pen_tool",       SP_VERB_CONTEXT_PEN },
    { "SPDynaDrawContext", "dyna_draw_tool", SP_VERB_CONTEXT_CALLIGRAPHIC },
    { "SPTextContext",     "text_tool",      SP_VERB_CONTEXT_TEXT },
    { "SPDropperContext",  "dropper_tool",   SP_VERB_CONTEXT_DROPPER },
    { NULL, NULL, 0 }
};

static const struct {
    const gchar *type_name;
    const gchar *data_name;
    GtkWidget *(*create_func)(SPDesktop *desktop);
} aux_toolboxes[] = {
    { "SPSelectContext", "select_toolbox", sp_select_toolbox_new },
    { "SPNodeContext",   "node_toolbox",   sp_node_toolbox_new },
    { "SPZoomContext",   "zoom_toolbox",   sp_zoom_toolbox_new },
    { "SPStarContext",   "star_toolbox",   sp_star_toolbox_new },
    { "SPRectContext",   "rect_toolbox",   sp_rect_toolbox_new },
    { "SPArcContext",    "arc_toolbox",    sp_arc_toolbox_new },
    { "SPSpiralContext", "spiral_toolbox", sp_spiral_toolbox_new },
    { "SPPencilContext", "pencil_toolbox", NULL },
    { "SPPenContext", "pen_toolbox", NULL },
    { "SPDynaDrawContext", "calligraphy_toolbox", sp_calligraphy_toolbox_new },
    { "SPTextContext", "text_toolbox", NULL },
    { NULL, NULL, NULL }
};

static void toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func);

static void setup_tool_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);
static void setup_aux_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);
static void setup_commands_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_commands_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static void delete_connection(GObject *obj, sigc::connection *connection) {
	connection->disconnect();
	delete connection;
}

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
	if ( requisition->height < AUX_BUTTON_SIZE + 6 + 2 * AUX_SPACING) {
		requisition->height = AUX_BUTTON_SIZE + 6 + 2 * AUX_SPACING;
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
    gtk_box_set_spacing (GTK_BOX (tb), AUX_SPACING);
    gtk_box_pack_start (GTK_BOX (tb), GTK_WIDGET (tb_s), FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (tb), GTK_WIDGET (tb_e), FALSE, FALSE, 0);

    g_object_set_data (G_OBJECT (tb), "desktop", NULL);
    g_object_set_data (G_OBJECT (tb), "top_spacer", tb_s);

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
//# Commands Bar
//####################################

GtkWidget *
sp_commands_toolbox_new ()
{
    GtkWidget *tb = gtk_vbox_new (FALSE, 0);

    GtkWidget *tb_s = gtk_vbox_new (FALSE, 0);
    GtkWidget *tb_e = gtk_vbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (tb), AUX_SPACING);
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
    sp_node_selected_set_line_type (NR_LINETO);
}

void
sp_node_path_edit_tocurve (void)
{
    sp_node_selected_set_line_type (NR_CURVETO);
}

void
sp_node_path_edit_cusp (void)
{
    sp_node_selected_set_type (Path::NODE_CUSP);
}

void
sp_node_path_edit_smooth (void)
{
    sp_node_selected_set_type (Path::NODE_SMOOTH);
}

void
sp_node_path_edit_symmetrical (void)
{
    sp_node_selected_set_type (Path::NODE_SYMM);
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
        GTK_SIGNAL_FUNC (sp_node_path_edit_delete_segment), tt, _("Split path between two non-endpoint nodes"));

    sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_break",
        GTK_SIGNAL_FUNC (sp_node_path_edit_break), tt, _("Break path at selected nodes"));

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_cusp",
        GTK_SIGNAL_FUNC (sp_node_path_edit_cusp), tt, _("Make selected nodes corner"));

    sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_smooth",
        GTK_SIGNAL_FUNC (sp_node_path_edit_smooth), tt, _("Make selected nodes smooth"));

    sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_symmetric",
        GTK_SIGNAL_FUNC (sp_node_path_edit_symmetrical), tt, _("Make selected nodes symmetric"));

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_line",
        GTK_SIGNAL_FUNC (sp_node_path_edit_toline), tt, _("Make selected segments lines"));

    sp_toolbox_button_new (tb, AUX_BUTTON_SIZE, "node_curve",
        GTK_SIGNAL_FUNC (sp_node_path_edit_tocurve), tt, _("Make selected segments curves"));

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_OBJECT_TO_CURVE, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_SELECTION_OUTLINE, view, tt);

    gtk_widget_show_all (tb);

    return tb;

} // end of sp_node_toolbox_new()



static GtkWidget *
sp_zoom_toolbox_new (SPDesktop *desktop)
{
    SPView *view=SP_VIEW (desktop);

    GtkTooltips *tt = gtk_tooltips_new ();
    GtkWidget *tb = gtk_hbox_new (FALSE, 0);

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0),
                        FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_IN, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_OUT, view, tt);

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_SELECTION, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_DRAWING, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_PAGE, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_PAGE_WIDTH, view, tt);

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_PREV, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_NEXT, view, tt);

    gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_1_1, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_1_2, view, tt);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_2_1, view, tt);

    gtk_widget_show_all (tb);

    return tb;

} // end of sp_zoom_toolbox_new()


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

void
sp_commands_toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop)
{
    toolbox_set_desktop (gtk_bin_get_child (GTK_BIN (toolbox)), desktop, setup_commands_toolbox, update_commands_toolbox);
}


static void
toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func)
{
    gpointer ptr = g_object_get_data (G_OBJECT (toolbox), "desktop");
    SPDesktop *old_desktop = SP_IS_DESKTOP (ptr) ? SP_DESKTOP (ptr) : NULL;

    if (old_desktop) {
        GList *children, *iter;
        g_signal_handlers_disconnect_by_func ( G_OBJECT (old_desktop), (void*)G_CALLBACK (update_func), (gpointer)toolbox );

        children = gtk_container_get_children (GTK_CONTAINER (toolbox));
        for ( iter = children ; iter ; iter = iter->next ) {
            gtk_container_remove ( GTK_CONTAINER (toolbox), GTK_WIDGET (iter->data) );
        }
        g_list_free (children);
    }

    g_object_set_data (G_OBJECT (toolbox), "desktop", (gpointer)desktop);

    if (desktop) {
        gtk_widget_set_sensitive (toolbox, TRUE);
        setup_func (toolbox, desktop);
        update_func (desktop, SP_DESKTOP_EVENT_CONTEXT(desktop), toolbox);
        g_signal_connect ( G_OBJECT (desktop), "event_context_changed", G_CALLBACK (update_func), (gpointer)toolbox);
    } else {
        gtk_widget_set_sensitive (toolbox, FALSE);
    }

} // end of toolbox_set_desktop()


static void
setup_tool_toolbox (GtkWidget *toolbox, SPDesktop *desktop)
{
    GtkTooltips *tooltips=GTK_TOOLTIPS (g_object_get_data (G_OBJECT (toolbox), "tooltips"));

    for (int i = 0 ; tools[i].type_name ; i++ ) {
        GtkWidget *button =
            sp_toolbox_button_new_from_verb ( toolbox, TOOL_BUTTON_SIZE,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              tools[i].verb, SP_VIEW (desktop),
                                              tooltips );

        g_object_set_data ( G_OBJECT (toolbox), tools[i].data_name,
                            (gpointer)button );
    }

}


static void
update_tool_toolbox ( SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox )
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
		GtkWidget *sub_toolbox;
		if (aux_toolboxes[i].create_func == NULL)
			sub_toolbox=sp_empty_toolbox_new (desktop);
		else
			sub_toolbox=aux_toolboxes[i].create_func (desktop);
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
            gtk_widget_show_all (sub_toolbox);
            g_object_set_data (G_OBJECT(toolbox), "shows", sub_toolbox);
        } else {
            gtk_widget_hide (sub_toolbox);
        }
    }
}

static void
setup_commands_toolbox (GtkWidget *toolbox, SPDesktop *desktop)
{
    SPView *view=SP_VIEW (desktop);

    GtkTooltips *tt = gtk_tooltips_new ();
    GtkWidget *tb = gtk_hbox_new (FALSE, 0);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_NEW, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_OPEN, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_SAVE, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_PRINT, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_QUIT, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_IMPORT, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_FILE_EXPORT, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_UNDO, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_REDO, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_COPY, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_CUT, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_PASTE, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_DUPLICATE, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_GROUP, view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_UNGROUP, view, tt);


// disabled until we have icons for them:

//     find

//     gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

//     sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_CLONE, view, tt);
//     sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_UNLINK_CLONE, view, tt);

//     gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

//     sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_TILE, view, tt);
//     sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_EDIT_UNTILE, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_SELECTION, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_DRAWING, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_ZOOM_PAGE, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_DIALOG_FILL_STROKE, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_DIALOG_TEXT, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_DIALOG_XML_EDITOR, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_DIALOG_ALIGN_DISTRIBUTE, view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_DIALOG_DISPLAY, view, tt);
    sp_toolbox_button_new_from_verb(tb, AUX_BUTTON_SIZE, SP_BUTTON_TYPE_NORMAL, SP_VERB_DIALOG_NAMEDVIEW, view, tt);

    gtk_widget_show_all (tb);

    gtk_container_add (GTK_CONTAINER (toolbox), tb);
}

static void
update_commands_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox)
{
}

void show_aux_toolbox (GtkWidget *toolbox_toplevel)
{
    gtk_widget_show (toolbox_toplevel);
    GtkWidget *toolbox = gtk_bin_get_child (GTK_BIN (toolbox_toplevel));

    GtkWidget *shown_toolbox = GTK_WIDGET (g_object_get_data(G_OBJECT(toolbox), "shows"));
    if (!shown_toolbox) {
        return;
    }
    gtk_widget_show (toolbox);

     // need to show the spacer, or the padding will be off
    GtkWidget *spacer = GTK_WIDGET (g_object_get_data(G_OBJECT(toolbox), "top_spacer"));
    gtk_widget_show (spacer);

    gtk_widget_show_all (shown_toolbox);
}

void
aux_toolbox_space (GtkWidget *tb, gint space)
{
	gtk_box_pack_start (GTK_BOX (tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, space);
}

static GtkWidget *
sp_empty_toolbox_new (SPDesktop *desktop)
{
    GtkWidget *tbl;

    tbl = gtk_hbox_new (FALSE, 0);
    gtk_object_set_data (GTK_OBJECT (tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data (GTK_OBJECT (tbl), "desktop", desktop);

    gtk_widget_show_all (tbl);
    sp_set_font_size (tbl, AUX_FONT_SIZE);

    return tbl;
}

// helper UI functions

GtkWidget *
sp_tb_spinbutton (
    gchar *label, const gchar *tooltip, 
    const gchar *path, const gchar *data, gdouble def,
    GtkWidget *us, 
    SPWidget *tbl, 
    gboolean altx, const gchar *altx_mark,
    gdouble lower, gdouble upper, gdouble step, gdouble page,
    void (*callback) (GtkAdjustment *, SPWidget *),
    gdouble climb = 0.1, guint digits = 2)
{
    GtkTooltips *tt = gtk_tooltips_new ();

    GtkWidget *hb = gtk_hbox_new (FALSE, 1);

    GtkWidget *l = gtk_label_new (label);
    gtk_widget_show (l);
    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
    gtk_container_add (GTK_CONTAINER (hb), l);

    GtkObject *a = gtk_adjustment_new (prefs_get_double_attribute (path, data, def), 
                                       lower, upper, step, page, page);
    gtk_object_set_data (GTK_OBJECT (tbl), data, a);
    if (us)
        sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), GTK_ADJUSTMENT (a));

    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), climb, digits);
    gtk_tooltips_set_tip (tt, sb, tooltip, NULL);
    if (altx)
        gtk_object_set_data (GTK_OBJECT (sb), altx_mark, sb);
    gtk_widget_set_size_request (sb, AUX_SPINBUTTON_WIDTH, AUX_SPINBUTTON_HEIGHT);
    gtk_widget_show (sb);
    gtk_signal_connect (GTK_OBJECT (sb), "focus-in-event", GTK_SIGNAL_FUNC (spinbutton_focus_in), tbl);
    gtk_signal_connect (GTK_OBJECT (sb), "key-press-event", GTK_SIGNAL_FUNC (spinbutton_keypress), tbl);
    gtk_container_add (GTK_CONTAINER (hb), sb);
    gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (callback), tbl);

    return hb;
}

//########################
//##       Star         ##
//########################

static void
sp_stb_magnitude_value_changed (GtkAdjustment *adj, SPWidget *tbl) 
{
    prefs_set_int_attribute ("tools.shapes.star", "magnitude", (gint)adj->value);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    bool modmade = FALSE;

    SPSelection *selection = SP_DT_SELECTION (desktop);
    const GSList *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR ((SPItem *) items->data))	{
            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_int(repr,"sodipodi:sides",(gint)adj->value);
            sp_repr_set_double(repr,"sodipodi:arg2",sp_repr_get_double_attribute (repr, "sodipodi:arg1", 0.5) + M_PI / (gint)adj->value);
            SP_OBJECT ((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }
    if (modmade)  sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_stb_proportion_value_changed (GtkAdjustment *adj, SPWidget *tbl)
{
    prefs_set_double_attribute ("tools.shapes.star", "proportion", adj->value);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");

    bool modmade = FALSE;
    SPSelection *selection = SP_DT_SELECTION(desktop);
    const GSList *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR ((SPItem *) items->data)) {
            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);

            gdouble r1 = sp_repr_get_double_attribute (repr, "sodipodi:r1", 1.0);
            gdouble r2 = sp_repr_get_double_attribute (repr, "sodipodi:r2", 1.0);
            if (r2 < r1) {
                sp_repr_set_double(repr, "sodipodi:r2", r1*adj->value);
            } else {
                sp_repr_set_double(repr, "sodipodi:r1", r2*adj->value);
            }

            SP_OBJECT ((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }

    if (modmade) sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_stb_sides_flat_state_changed (GtkWidget *widget, GtkObject *tbl)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        prefs_set_string_attribute ("tools.shapes.star", "isflatsided", "true");
    } else {
        prefs_set_string_attribute ("tools.shapes.star", "isflatsided", "false");
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");

    SPSelection *selection = SP_DT_SELECTION(desktop);
    const GSList *items = selection->itemList();
    GtkWidget *prop_widget = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "prop_widget");
    bool modmade = FALSE;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        gtk_widget_set_sensitive (GTK_WIDGET (prop_widget), FALSE);
        for (; items != NULL; items = items->next) {
            if (SP_IS_STAR ((SPItem *) items->data))
            { SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_attr(repr, "inkscape:flatsided", "true");
            SP_OBJECT ((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
            }
        }
    } else {
        gtk_widget_set_sensitive (GTK_WIDGET (prop_widget), TRUE);
        for (; items != NULL; items = items->next) {
            if (SP_IS_STAR ((SPItem *) items->data))	{
                SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
                sp_repr_set_attr(repr, "inkscape:flatsided", "false");
                SP_OBJECT (items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                modmade = true;
            }
        }
    }
    if (modmade) sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_stb_rounded_value_changed (GtkAdjustment *adj, SPWidget *tbl) 
{
    prefs_set_double_attribute ("tools.shapes.star", "rounded", (gdouble) adj->value);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    bool modmade = FALSE;

    SPSelection *selection = SP_DT_SELECTION (desktop);
    const GSList *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR ((SPItem *) items->data)) {
            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_double (repr, "inkscape:rounded", (gdouble) adj->value);
            SP_OBJECT (items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }
    if (modmade)  sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    spinbutton_defocus (GTK_OBJECT (tbl));
}


static void star_tb_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive,  gpointer data) 
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    GtkAdjustment *adj;

    adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "magnitude");
    gtk_adjustment_set_value (adj, sp_repr_get_int_attribute (repr, "sodipodi:sides", 0));

    adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "proportion");
    gdouble r1 = sp_repr_get_double_attribute (repr, "sodipodi:r1", 1.0);
    gdouble r2 = sp_repr_get_double_attribute (repr, "sodipodi:r2", 1.0);
    if (r2 < r1) {
        gtk_adjustment_set_value (adj, r2/r1);
    } else {
        gtk_adjustment_set_value (adj, r1/r2);
    }

    adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "rounded");
    gtk_adjustment_set_value (adj, sp_repr_get_double_attribute (repr, "inkscape:rounded", 0.0));

    GtkWidget *fscb = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "flat_checkbox");
    GtkWidget *prop_widget = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "prop_widget");
    const char *flatsides = sp_repr_attr(repr,"inkscape:flatsided");

    if (flatsides && !strcmp (flatsides,"false" )) {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (prop_widget), TRUE);
    } else {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (prop_widget), FALSE);
    }

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
}


static SPReprEventVector star_tb_repr_events =
{
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    star_tb_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};


/**
 *  \param selection Should not be NULL.
 */
static void
sp_star_toolbox_selection_changed (SPSelection * selection, GtkObject *tbl)
{
    g_assert(selection != NULL);
    
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    int no_stars_selected = 0;
    SPRepr *repr = NULL;
    SPRepr *oldrepr = NULL;
    
    for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_STAR ((SPItem *) items->data)) {
            no_stars_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }
    
    if (no_stars_selected == 1) {

        oldrepr = (SPRepr *) gtk_object_get_data (GTK_OBJECT (tbl), "repr");
        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data (oldrepr, tbl);
            sp_repr_unref (oldrepr);
            oldrepr = 0;
        }
        
        if (repr) {
            g_object_set_data (G_OBJECT (tbl), "repr", repr);
            sp_repr_ref (repr);
            sp_repr_add_listener (repr, &star_tb_repr_events, tbl);
            sp_repr_synthesize_events (repr, &star_tb_repr_events, tbl);
        }
    }
}


static void
sp_stb_defaults (GtkWidget *widget,  SPWidget *tbl)
{
	g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

	GtkAdjustment *adj;

	// fixme: make settable
	gint mag = 5;
	gdouble prop = 0.5;
	gboolean flat = FALSE;

	GtkWidget *fscb = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "flat_checkbox");
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  flat);
	GtkWidget *sb2 = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "prop_widget");
	gtk_widget_set_sensitive (GTK_WIDGET (sb2), !flat);

	adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "magnitude");
	gtk_adjustment_set_value (adj, mag);
	adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "proportion");
	gtk_adjustment_set_value (adj, prop);
	adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "rounded");
	gtk_adjustment_set_value (adj, 0.0);

	bool modmade=FALSE;
	SPDesktop *desktop = SP_DESKTOP (g_object_get_data (G_OBJECT (tbl), "desktop"));
	SPSelection *selection = SP_DT_SELECTION(desktop);
	const GSList *items = selection->itemList();
	for (; items != NULL; items = items->next) {
		if (SP_IS_STAR ((SPItem *) items->data)) {
			SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
			if (repr) {
				sp_repr_set_attr(repr, "inkscape:flatsided", flat? "true" : "false");
				sp_repr_set_int(repr,"sodipodi:sides", mag);
				// we assume arg1 and r1 are set and do not change them
				sp_repr_set_double(repr,"sodipodi:arg2", sp_repr_get_double_attribute (repr, "sodipodi:arg1", 0) + M_PI / mag);
				sp_repr_set_double(repr,"sodipodi:r2", sp_repr_get_double_attribute (repr, "sodipodi:r1", 1.0)*prop);
				sp_repr_set_double(repr,"inkscape:rounded", 0.0);
				SP_OBJECT (items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
			}
			modmade = true;
		}
	}

	if (modmade) sp_document_done (SP_DT_DOCUMENT (desktop));

	g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

	spinbutton_defocus (GTK_OBJECT (tbl));
}

static GtkWidget *
sp_star_toolbox_new (SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new (FALSE, 0);

    gtk_object_set_data (GTK_OBJECT (tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data (GTK_OBJECT (tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new ();

    /* Magnitude */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Corners:"), _("Number of corners of a polygon or star"), 
                                          "tools.shapes.star", "magnitude", 3,
                                          NULL, (SPWidget *) tbl, TRUE, "altx-star",
                                          3, 32, 1, 1, 
                                          sp_stb_magnitude_value_changed, 1, 0);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    const gchar *flatsidedstr = NULL;

    /* Flatsided checkbox */
    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 1);
        GtkWidget *fscb = gtk_check_button_new_with_label (_("Polygon"));
        gtk_widget_set_sensitive (GTK_WIDGET (fscb), TRUE);
        flatsidedstr = prefs_get_string_attribute ("tools.shapes.star", "isflatsided");
        if (!flatsidedstr || (flatsidedstr && !strcmp (flatsidedstr, "false")))
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  FALSE);
        else
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  TRUE);
        gtk_tooltips_set_tip (tt, fscb, _("Regular polygon (with one handle) instead of a star"), NULL);
        gtk_widget_show (fscb);
        gtk_object_set_data (GTK_OBJECT (tbl), "flat_checkbox", fscb);
        gtk_container_add (GTK_CONTAINER (hb), fscb);
        g_signal_connect (G_OBJECT(fscb), "toggled", GTK_SIGNAL_FUNC (sp_stb_sides_flat_state_changed ), tbl);
        gtk_box_pack_start (GTK_BOX (tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Spoke ratio */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Spoke ratio:"),
        // TRANSLATORS: Tip radius of a star is the distance from the center to the farthest handle.
        // Base radius is the same for the closest handle.
                                          _("Base radius to tip radius ratio"),
                                          "tools.shapes.star", "proportion", 0.5,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          0.01, 1.0, 0.01, 0.1,
                                          sp_stb_proportion_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
        g_object_set_data (G_OBJECT (tbl), "prop_widget", hb);
        if (!flatsidedstr || (flatsidedstr && !strcmp (flatsidedstr, "false")))
            gtk_widget_set_sensitive (GTK_WIDGET (hb), TRUE);
        else
            gtk_widget_set_sensitive (GTK_WIDGET (hb), FALSE);
    }

    /* Roundedness */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Roundedness:"), _("How much rounded are the corners (0 for sharp)"), 
                                          "tools.shapes.star", "rounded", 0.0,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          -100.0, 100.0, 0.01, 0.1,
                                          sp_stb_rounded_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }



    /* Reset */
    {
    GtkWidget *hb = gtk_hbox_new (FALSE, 1);
    GtkWidget *b = gtk_button_new_with_label (_("Defaults"));
    gtk_widget_show (b);
    gtk_container_add (GTK_CONTAINER (hb), b);
    gtk_tooltips_set_tip (tt, b, _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"), NULL);
    gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_stb_defaults), tbl);
    gtk_box_pack_start (GTK_BOX (tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    gtk_widget_show_all (tbl);
    sp_set_font_size (tbl, AUX_FONT_SIZE);

    sigc::connection *connection = new sigc::connection(
        SP_DT_SELECTION(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_star_toolbox_selection_changed), (GtkObject *)tbl))
    );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    return tbl;
}


//########################
//##       Rect         ##
//########################


static void
sp_rtb_rx_value_changed (GtkAdjustment *adj, SPWidget *tbl)
{
    prefs_set_double_attribute ("tools.shapes.rect", "rx", adj->value);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");

    GtkWidget *us = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (tbl), "units");
    const SPUnit *unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));

    bool modmade = false;
    SPSelection *selection = SP_DT_SELECTION(desktop);
    const GSList *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_RECT ((SPItem *) items->data)) {
            sp_rect_set_visible_rx (SP_RECT (items->data), sp_units_get_points (adj->value, unit));
            modmade = true;
        }
    }

    if (modmade) 
        sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_rtb_ry_value_changed (GtkAdjustment *adj, SPWidget *tbl)
{
    prefs_set_double_attribute ("tools.shapes.rect", "ry", adj->value);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");

    GtkWidget *us = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (tbl), "units");
    const SPUnit *unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));

    bool modmade = FALSE;
    SPSelection *selection = SP_DT_SELECTION(desktop);
    const GSList *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_RECT ((SPItem *) items->data)) {
            sp_rect_set_visible_ry (SP_RECT (items->data), sp_units_get_points (adj->value, unit));
            modmade = true;
        }
    }

    if (modmade)  
        sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_rtb_defaults ( GtkWidget *widget, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    GtkAdjustment *adj;
    adj = (GtkAdjustment*)gtk_object_get_data (obj, "rx");
    gtk_adjustment_set_value (adj, 0.0);
    adj = (GtkAdjustment*)gtk_object_get_data (obj, "ry");
    gtk_adjustment_set_value (adj, 0.0);

    bool modmade=FALSE;
    SPDesktop *desktop = SP_DESKTOP (g_object_get_data (G_OBJECT (tbl), "desktop"));
    for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_RECT ((SPItem *) items->data)) {
            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
            if (repr) {
                sp_repr_set_attr (repr, "rx", NULL);
                sp_repr_set_attr (repr, "ry", NULL);
            }
            modmade = true;
        }
    }

    if (modmade) sp_document_done (SP_DT_DOCUMENT (desktop));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void rect_tb_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive,  gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPItem *item = SP_ITEM(g_object_get_data (G_OBJECT (tbl), "item"));
    if (SP_IS_RECT (item)) {
        {
            GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "rx");
            gdouble rx = sp_rect_get_visible_rx (SP_RECT (item));
            gtk_adjustment_set_value (adj, rx);
        }

        {
            GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "ry");
            gdouble ry = sp_rect_get_visible_ry (SP_RECT (item));
            gtk_adjustment_set_value (adj, ry);
        }
    }

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
}


static SPReprEventVector rect_tb_repr_events = {
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    rect_tb_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};

/**
 *  \param selection should not be NULL.
 */
static void
sp_rect_toolbox_selection_changed (SPSelection * selection, GtkObject *tbl)
{
    g_assert(selection != NULL);
  
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    int rects_selected = 0;
    SPRepr *repr = NULL;
    SPItem *item = NULL;
    SPRepr *oldrepr = NULL;
  
    for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_RECT ((SPItem *) items->data)) {
            rects_selected++;
            item = (SPItem *) items->data;
            repr = SP_OBJECT_REPR(item);
        }
    }
  
    if (rects_selected == 1) {
        oldrepr = (SPRepr *) gtk_object_get_data (GTK_OBJECT (tbl), "repr");
        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data (oldrepr, tbl);
            sp_repr_unref (oldrepr);
            oldrepr = 0;
        }
        if (repr) {
            g_object_set_data (G_OBJECT (tbl), "repr", repr);
            g_object_set_data (G_OBJECT (tbl), "item", item);
            sp_repr_ref (repr);
            sp_repr_add_listener (repr, &rect_tb_repr_events, tbl);
            sp_repr_synthesize_events (repr, &rect_tb_repr_events, tbl);
        }
    }
}


static GtkWidget *
sp_rect_toolbox_new (SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new (FALSE, 0);

    gtk_object_set_data (GTK_OBJECT (tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data (GTK_OBJECT (tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new ();

    // rx/ry units menu: create
    GtkWidget *us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
    sp_unit_selector_setsize (us, AUX_OPTION_MENU_WIDTH, AUX_OPTION_MENU_HEIGHT);
    // fixme: add %

    /* rx */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Rx:"), _("Horizontal radius of rounded corner"), 
                                          "tools.shapes.rect", "rx", 0,
                                          us, (SPWidget *) tbl, TRUE, "altx-rect",
                                          0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                          sp_rtb_rx_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* ry */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Ry:"), _("Vertical radius of rounded corner"), 
                                          "tools.shapes.rect", "ry", 0,
                                          us, (SPWidget *) tbl, FALSE, NULL,
                                          0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                          sp_rtb_ry_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    // add the units menu
    gtk_widget_show (us);
    gtk_box_pack_start (GTK_BOX (tbl), us, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    gtk_object_set_data (GTK_OBJECT (tbl), "units", us);

    /* Reset */
    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label (_("Not rounded"));
        gtk_tooltips_set_tip (tt, b, _("Make corners sharp"), NULL);
        gtk_widget_show (b);
        gtk_container_add (GTK_CONTAINER (hb), b);
        gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_rtb_defaults), tbl);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    gtk_widget_show_all (tbl);
    sp_set_font_size (tbl, AUX_FONT_SIZE);

    sigc::connection *connection = new sigc::connection(
        SP_DT_SELECTION(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_rect_toolbox_selection_changed), (GtkObject *)tbl))
    );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    return tbl;
}

//########################
//##       Spiral       ##
//########################

static void
sp_spl_tb_value_changed (GtkAdjustment *adj, SPWidget *tbl, const gchar* value_name)
{
    prefs_set_double_attribute ("tools.shapes.spiral", value_name, adj->value);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);

    bool modmade = FALSE;
    for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL ((SPItem *) items->data)) {
            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_double( repr, namespaced_name, adj->value );
            SP_OBJECT ((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }

    g_free(namespaced_name);

    if (modmade) {
        sp_document_done (SP_DT_DOCUMENT (desktop));
    }

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
        
    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_spl_tb_revolution_value_changed (GtkAdjustment *adj, SPWidget *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "revolution");
}

static void
sp_spl_tb_expansion_value_changed (GtkAdjustment *adj, SPWidget *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "expansion");
}

static void
sp_spl_tb_t0_value_changed (GtkAdjustment *adj, SPWidget *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "t0");
}

static void
sp_spl_tb_defaults (GtkWidget *widget, GtkObject *obj)
{
	GtkWidget *tbl = GTK_WIDGET(obj);

	g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

	GtkAdjustment *adj;

	// fixme: make settable
	gdouble rev = 5;
	gdouble exp = 1.0;
	gdouble t0 = 0.0;

	adj = (GtkAdjustment*)gtk_object_get_data (obj, "revolution");
	gtk_adjustment_set_value (adj, rev);
	adj = (GtkAdjustment*)gtk_object_get_data (obj, "expansion");
	gtk_adjustment_set_value (adj, exp);
	adj = (GtkAdjustment*)gtk_object_get_data (obj, "t0");
	gtk_adjustment_set_value (adj, t0);

	bool modmade=FALSE;
	SPDesktop *desktop = SP_DESKTOP (g_object_get_data (G_OBJECT (tbl), "desktop"));
	for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
		if (SP_IS_SPIRAL ((SPItem *) items->data)) {
			SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
			if (repr) {
				sp_repr_set_double (repr, "sodipodi:revolution", rev);
				sp_repr_set_double (repr, "sodipodi:expansion", exp);
				sp_repr_set_double (repr, "sodipodi:t0", t0);
				modmade = true;
			}
		}
	}

	if (modmade) {
            sp_document_done (SP_DT_DOCUMENT (desktop));
        }

	g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));

	spinbutton_defocus (GTK_OBJECT (tbl));
}


static void spiral_tb_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive,  gpointer data) 
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    GtkAdjustment *adj;
    adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "revolution");
    gtk_adjustment_set_value (adj, (sp_repr_get_double_attribute (repr, "sodipodi:revolution", 3.0)));
    adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "expansion");
    gtk_adjustment_set_value (adj, (sp_repr_get_double_attribute (repr, "sodipodi:expansion", 1.0)));
    adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "t0");
    gtk_adjustment_set_value (adj, (sp_repr_get_double_attribute (repr, "sodipodi:t0", 0.0)));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
}


static SPReprEventVector spiral_tb_repr_events = {
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    spiral_tb_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};

static void
sp_spiral_toolbox_selection_changed (SPSelection * selection, GtkObject *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    int no_spirals_selected = 0;
    SPRepr *repr = NULL;
    SPRepr *oldrepr = NULL;
  
    for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL ((SPItem *) items->data)) {
            no_spirals_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }
  
    if (no_spirals_selected == 1) {
        
        oldrepr = (SPRepr *) gtk_object_get_data (GTK_OBJECT (tbl), "repr");
        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data (oldrepr, tbl);
            sp_repr_unref (oldrepr);
            oldrepr = 0;
        }
        
        if (repr) {
            g_object_set_data (G_OBJECT (tbl), "repr", repr);
            sp_repr_ref (repr);
            sp_repr_add_listener (repr, &spiral_tb_repr_events, tbl);
            sp_repr_synthesize_events (repr, &spiral_tb_repr_events, tbl);
        }
    }
}


static GtkWidget *
sp_spiral_toolbox_new (SPDesktop *desktop)
{

    GtkWidget *tbl = gtk_hbox_new (FALSE, 0);

    gtk_object_set_data (GTK_OBJECT (tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data (GTK_OBJECT (tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new ();

    /* Revolution */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Turns:"), _("Number of revolutions"), 
                                          "tools.shapes.spiral", "revolution", 3.0,
                                          NULL, (SPWidget *) tbl, TRUE, "altx-spiral",
                                          0.01, 20.0, 0.1, 1.0, 
                                          sp_spl_tb_revolution_value_changed, 1, 2);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Expansion */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Divergence:"), _("How much denser/sparser are outer revolutions; 1 = uniform"), 
                                          "tools.shapes.spiral", "expansion", 1.0,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          0.0, 1000.0, 0.01, 1.0,
                                          sp_spl_tb_expansion_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* T0 */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Inner radius:"), _("Radius of the innermost revolution (relative to the spiral size)"), 
                                          "tools.shapes.spiral", "t0", 0.0,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          0.0, 0.999, 0.01, 1.0,
                                          sp_spl_tb_t0_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Reset */
    {
    GtkWidget *hb = gtk_hbox_new (FALSE, 1);
    GtkWidget *b = gtk_button_new_with_label (_("Defaults"));
    gtk_tooltips_set_tip (tt, b, _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"), NULL);
    gtk_widget_show (b);
    gtk_container_add (GTK_CONTAINER (hb), b);
    gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_spl_tb_defaults), tbl);
    gtk_box_pack_start (GTK_BOX (tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    gtk_widget_show_all (tbl);
    sp_set_font_size (tbl, AUX_FONT_SIZE);

    sigc::connection *connection = new sigc::connection(
        SP_DT_SELECTION(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_spiral_toolbox_selection_changed), (GtkObject *)tbl))
    );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    return tbl;
}


//########################
//##     Calligraphy    ##
//########################

static void
sp_ddc_mass_value_changed(GtkAdjustment *adj,  SPWidget *tbl)
{
	prefs_set_double_attribute ("tools.calligraphic", "mass", adj->value);
	spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_ddc_drag_value_changed(GtkAdjustment *adj, SPWidget *tbl)
{
	prefs_set_double_attribute ("tools.calligraphic", "drag", adj->value);
	spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_ddc_angle_value_changed(GtkAdjustment *adj, SPWidget *tbl)
{
	prefs_set_double_attribute ("tools.calligraphic", "angle", adj->value);
	spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_ddc_width_value_changed(GtkAdjustment *adj, SPWidget *tbl)
{
	prefs_set_double_attribute ("tools.calligraphic", "width", adj->value);
	spinbutton_defocus (GTK_OBJECT (tbl));
}

static void sp_ddc_defaults(GtkWidget *, SPWidget *tbl)
{
	// FIXME: make defaults settable via Inkscape Options
   struct KeyValue {
        char const *key;
        double value;
    } const key_values[] = {
        {"mass", 0.3},
        {"drag", 0.5},
        {"angle", 30.0},
        {"width", 0.2}
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(key_values); ++i) {
        KeyValue const &kv = key_values[i];
        GtkAdjustment &adj = *static_cast<GtkAdjustment *>(gtk_object_get_data(GTK_OBJECT(tbl), kv.key));
        gtk_adjustment_set_value(&adj, kv.value);
    }

    spinbutton_defocus (GTK_OBJECT (tbl));
}

static GtkWidget *
sp_calligraphy_toolbox_new (SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new (FALSE, 0);

    gtk_object_set_data (GTK_OBJECT (tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data (GTK_OBJECT (tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new ();

    /* Width */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Width:"), _("The width of the calligraphic pen"), 
                                          "tools.calligraphic", "width", 0.2,
                                          NULL, (SPWidget *) tbl, TRUE, "altx-calligraphy",
                                          0.01, 1.0, 0.01, 0.1, 
                                          sp_ddc_width_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Angle */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Angle:"), _("The angle of the calligraphic pen (in degrees)"), 
                                          "tools.calligraphic", "angle", 30,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          0.0, 360.0, 1.0, 10.0,
                                          sp_ddc_angle_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Mass */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Mass:"), _("How much inertia affects the movement of the pen"), 
                                          "tools.calligraphic", "mass", 0.3,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          0.0, 1.0, 0.01, 0.1,
                                          sp_ddc_mass_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Drag */
    {
        // TRANSLATORS: "drag" means "resistance" here
        GtkWidget *hb = sp_tb_spinbutton (_("Drag:"), _("How much resistance affects the movement of the pen"), 
                                          "tools.calligraphic", "drag", 0.5,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          0.0, 1.0, 0.01, 0.1,
                                          sp_ddc_drag_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }


    /* Reset */
    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Defaults"));
        gtk_tooltips_set_tip (tt, b, _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"), NULL);
        gtk_widget_show(b);
        gtk_container_add (GTK_CONTAINER (hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_ddc_defaults), tbl);
        gtk_box_pack_start (GTK_BOX (tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    gtk_widget_show_all (tbl);
    sp_set_font_size (tbl, AUX_FONT_SIZE);

    return tbl;
}


//########################
//##    Circle / Arc    ##
//########################

static void
sp_arctb_start_value_changed(GtkAdjustment *adj,  SPWidget *tbl)
{
    prefs_set_double_attribute ("tools.shapes.arc", "start", (adj->value * M_PI)/ 180);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");

    bool modmade = FALSE;
    for (const GSList *items = SP_DT_SELECTION (desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_ARC ((SPItem *) items->data)) {

            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);

            if (adj->value != 0)
                sp_repr_set_double(repr, "sodipodi:start", (adj->value * M_PI)/ 180);
            else
                sp_repr_set_attr (repr, "sodipodi:start", NULL);

            modmade = true;
        }
    }

    GtkAdjustment *end = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(tbl), "end");
    GtkWidget *ocb = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "open_checkbox");

    if (adj->value==0 && end->value==0) {
        gtk_widget_set_sensitive (GTK_WIDGET (ocb), FALSE);
    } else {
        gtk_widget_set_sensitive (GTK_WIDGET (ocb), TRUE);
    }

    if (modmade) {
        sp_document_maybe_done (SP_DT_DOCUMENT (desktop), "start_spin_changed");
    }

    // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
    spinbutton_defocus (GTK_OBJECT (tbl));

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
}

static void
sp_arctb_end_value_changed(GtkAdjustment *adj, SPWidget *tbl)
{
    prefs_set_double_attribute ("tools.shapes.arc", "end", (adj->value * M_PI)/ 180);

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    
    bool modmade = FALSE;
    for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_ARC ((SPItem *) items->data)) {

            SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);

            if (adj->value != 0)
                sp_repr_set_double(repr, "sodipodi:end", (adj->value * M_PI)/ 180 );
            else
                sp_repr_set_attr (repr, "sodipodi:end", NULL);
            
            modmade = true;
        }
    }

    GtkAdjustment *start = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(tbl), "start");
    GtkWidget *ocb = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "open_checkbox");
    
    if (adj->value == 0 && start->value == 0) {
        gtk_widget_set_sensitive (GTK_WIDGET (ocb), FALSE);
    } else {
        gtk_widget_set_sensitive (GTK_WIDGET (ocb), TRUE);
    }
    
    if (modmade) {
        sp_document_maybe_done (SP_DT_DOCUMENT (desktop), "end_spin_changed");
    }

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
    
    // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void
sp_arctb_open_state_changed (GtkWidget *widget, GtkObject *tbl)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        prefs_set_string_attribute ("tools.shapes.arc", "open", "true");
    } else {
        prefs_set_string_attribute ("tools.shapes.arc", "open", NULL);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
    bool modmade = FALSE;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
       for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
            items != NULL;
            items = items->next)
       {
           if (SP_IS_ARC ((SPItem *) items->data)) {
               SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
               sp_repr_set_attr(repr, "sodipodi:open", "true");
               SP_OBJECT ((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
               modmade = true;
           }
       }
    } else {
        for (const GSList *items = SP_DT_SELECTION(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC ((SPItem *) items->data))    {
                SPRepr *repr = SP_OBJECT_REPR((SPItem *) items->data);
                sp_repr_set_attr(repr, "sodipodi:open", NULL);
                SP_OBJECT ((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                modmade = true;
            }
        }
    }
    
    if (modmade) {
        sp_document_done (SP_DT_DOCUMENT (desktop));
    }

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
    
    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void sp_arctb_defaults(GtkWidget *, SPWidget *tbl)
{
    // FIXME: make defaults settable via Inkscape Options
   struct KeyValue {
        char const *key;
        double value;
    } const key_values[] = {
        {"start", 0.0},
        {"end", 0.0}
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(key_values); ++i) {
        KeyValue const &kv = key_values[i];
        GtkAdjustment &adj = *static_cast<GtkAdjustment *>(gtk_object_get_data(GTK_OBJECT(tbl), kv.key));
        gtk_adjustment_set_value(&adj, kv.value);
    }
    spinbutton_defocus (GTK_OBJECT (tbl));
}

static void arc_tb_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive,  gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data (G_OBJECT (tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (TRUE));

    gdouble start = sp_repr_get_double_attribute (repr, "sodipodi:start", 0.0);
    gdouble end = sp_repr_get_double_attribute (repr, "sodipodi:end", 0.0);

    GtkAdjustment *adj1,*adj2;
    adj1 = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "start");
    gtk_adjustment_set_value (adj1, mod360 ((start * 180)/M_PI));
    adj2 = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "end");
    gtk_adjustment_set_value (adj2, mod360 ((end * 180)/M_PI));

    GtkWidget *ocb = (GtkWidget*) g_object_get_data (G_OBJECT (tbl), "open_checkbox");
    const char *openstr = NULL;
    openstr = sp_repr_attr(repr,"sodipodi:open");

    if (adj1->value == 0 && adj2->value == 0) 
	gtk_widget_set_sensitive (GTK_WIDGET (ocb), FALSE);
    else 
	gtk_widget_set_sensitive (GTK_WIDGET (ocb), TRUE);

    if (openstr) {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (ocb),  TRUE);
    } else {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (ocb),  FALSE);
    }

    g_object_set_data (G_OBJECT (tbl), "freeze", GINT_TO_POINTER (FALSE));
}

static SPReprEventVector arc_tb_repr_events = {
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    NULL, /* child_removed */
    NULL, /* change_attr */
    arc_tb_event_attr_changed,
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};


static void
sp_arc_toolbox_selection_changed (SPSelection * selection, GtkObject *tbl)
{
  SPDesktop *desktop = (SPDesktop *) gtk_object_get_data (GTK_OBJECT (tbl), "desktop");
  int no_arcs_selected = 0;
  SPRepr *repr = NULL;
  SPRepr *oldrepr = NULL;
  
  for (const GSList *items = SP_DT_SELECTION (desktop)->itemList();
       items != NULL;
       items = items->next)
  {
      if (SP_IS_ARC ((SPItem *) items->data)) {
          no_arcs_selected++;
          repr = SP_OBJECT_REPR((SPItem *) items->data);
      }
  }
  
  if (no_arcs_selected == 1) {
     oldrepr = (SPRepr *) gtk_object_get_data (GTK_OBJECT (tbl), "repr");
     
     if (oldrepr) { // remove old listener
         sp_repr_remove_listener_by_data (oldrepr, tbl);
         sp_repr_unref (oldrepr);
         oldrepr = 0;
     }
     
     if (repr) {
         g_object_set_data (G_OBJECT (tbl), "repr", repr);
         sp_repr_ref (repr);
         sp_repr_add_listener (repr, &arc_tb_repr_events, tbl);
         sp_repr_synthesize_events (repr, &arc_tb_repr_events, tbl);
     }
  }
}


static GtkWidget *
sp_arc_toolbox_new (SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new (FALSE, 0);

    gtk_object_set_data (GTK_OBJECT (tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data (GTK_OBJECT (tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new ();

    /* Start */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("Start:"), _("The angle (in degrees) from the horizontal to the arc's start point"), 
                                          "tools.shapes.arc", "start", 0.0,
                                          NULL, (SPWidget *) tbl, TRUE, "altx-arc",
                                          -360.0, 360.0, 1.0, 10.0,
                                          sp_arctb_start_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* End */
    {
        GtkWidget *hb = sp_tb_spinbutton (_("End:"), _("The angle (in degrees) from the horizontal to the arc's end point"), 
                                          "tools.shapes.arc", "end", 0.0,
                                          NULL, (SPWidget *) tbl, FALSE, NULL,
                                          -360.0, 360.0, 1.0, 10.0,
                                          sp_arctb_end_value_changed);
        gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Segments / Pie checkbox */
     {
        GtkWidget *hb = gtk_hbox_new (FALSE, 1);
        GtkWidget *fscb = gtk_check_button_new_with_label (_("Open arc"));
        gtk_tooltips_set_tip (tt, fscb, _("Switch between arc (unclosed shape) and segment (closed shape with two radii)"), NULL);

        const gchar *openstr = NULL;
        openstr = prefs_get_string_attribute ("tools.shapes.arc", "open");
        if (!openstr || (openstr && !strcmp (openstr, "false")))
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  FALSE);
        else
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (fscb),  TRUE);

        GtkAdjustment *adj1 = GTK_ADJUSTMENT (gtk_object_get_data (GTK_OBJECT (tbl), "start"));
        GtkAdjustment *adj2 = GTK_ADJUSTMENT (gtk_object_get_data (GTK_OBJECT (tbl), "end"));

        if (adj1->value == 0 && adj2->value == 0) 
		gtk_widget_set_sensitive (GTK_WIDGET (fscb), FALSE);
        else 
		gtk_widget_set_sensitive (GTK_WIDGET (fscb), TRUE);

        gtk_widget_show (fscb);
        gtk_object_set_data (GTK_OBJECT (tbl), "open_checkbox", fscb);
        gtk_container_add (GTK_CONTAINER (hb), fscb);
        g_signal_connect (G_OBJECT(fscb), "toggled", GTK_SIGNAL_FUNC (sp_arctb_open_state_changed ), tbl);
        gtk_box_pack_start (GTK_BOX (tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Make Whole */
    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Make whole"));
        gtk_tooltips_set_tip (tt, b, _("Make the shape a whole ellipse, not arc or segment"), NULL);
        gtk_widget_show(b);
        gtk_container_add (GTK_CONTAINER (hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_arctb_defaults), tbl);
        gtk_box_pack_start (GTK_BOX (tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    sigc::connection *connection = new sigc::connection(
        SP_DT_SELECTION(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_arc_toolbox_selection_changed), (GtkObject *)tbl))
    );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    gtk_widget_show_all (tbl);
    sp_set_font_size (tbl, AUX_FONT_SIZE);

    return tbl;
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
