#define __SP_XMLVIEW_TREE_C__

/*
 * XML View
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkarrow.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "macros.h"
#include "../sodipodi.h"
#include "../document.h"
#include "../desktop-handles.h"
#include "../selection.h"
#include "../sp-item.h"

#include "../xml/repr-private.h"

#include "../widgets/sp-xmlview-tree.h"
#include "../widgets/sp-xmlview-content.h"
#include "../widgets/sp-xmlview-attr-list.h"

typedef struct _EditableDest {
	GtkEditable * editable;
	gchar * text;
} EditableDest;

static GtkWidget * dialog = NULL;
static GtkTooltips * tooltips = NULL;
static GtkEditable * attr_name = NULL;
static GtkTextView *attr_value = NULL;
static SPXMLViewTree * tree = NULL;
static SPXMLViewAttrList * attributes = NULL;
static SPXMLViewContent * content = NULL;

static gint blocked = 0;
static SPDesktop * current_desktop = NULL;
static SPDocument * current_document = NULL;
static gint selected_attr = 0;
static SPRepr * selected_repr = NULL;

static void set_tree_desktop (SPDesktop * desktop);
static void set_tree_document (SPDocument * document);
static void set_tree_repr (SPRepr * repr);

static void set_tree_select (SPRepr * repr);
static void propagate_tree_select (SPRepr * repr);

static SPRepr * get_dt_select (void);
static void set_dt_select (SPRepr * repr);

static void on_tree_select_row (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_unselect_row (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void after_tree_move (GtkCTree * tree, GtkCTreeNode * node, GtkCTreeNode * new_parent, GtkCTreeNode * new_sibling, gpointer data);
static void on_destroy (GtkObject * object, gpointer data);

static void on_tree_select_row_enable_if_element (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_enable_if_non_root (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_show_if_element (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_show_if_text (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_enable_if_indentable (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_enable_if_not_first_child (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_enable_if_not_last_child (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_select_row_enable_if_has_grandparent (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);

static void on_tree_unselect_row_clear_text (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_unselect_row_disable (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);
static void on_tree_unselect_row_hide (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data);

static void on_attr_select_row (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_unselect_row (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);

static void on_attr_select_row_enable (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_unselect_row_disable (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);

static void on_attr_select_row_set_name_content (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_select_row_set_value_content (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_unselect_row_clear_text (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);

static void on_editable_changed_enable_if_valid_xml_name (GtkEditable * editable, gpointer data);

static void on_desktop_selection_changed (SPSelection * selection);
static gboolean on_desktop_shutdown (SPDesktop * desktop, gpointer data);
static void on_document_uri_set (SPDocument * document, const guchar * uri, gpointer data);

static void on_clicked_get_editable_text (GtkWidget * widget, gpointer data);

static void cmd_new_element_node (GtkObject * object, gpointer data);
static void cmd_new_text_node (GtkObject * object, gpointer data);
static void cmd_duplicate_node (GtkObject * object, gpointer data);
static void cmd_delete_node (GtkObject * object, gpointer data);

static void cmd_raise_node (GtkObject * object, gpointer data);
static void cmd_lower_node (GtkObject * object, gpointer data);
static void cmd_indent_node (GtkObject * object, gpointer data);
static void cmd_unindent_node (GtkObject * object, gpointer data);

static void cmd_delete_attr (GtkObject * object, gpointer data);
static void cmd_set_attr (GtkObject * object, gpointer data);

void
sp_xml_tree_dialog (void)
{
	SPDesktop * desktop;

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return;
	g_assert (SP_IS_DESKTOP (desktop));

	if (dialog == NULL) {
		GtkWidget *box, *sw, *paned, *toolbar, *button;
		GtkWidget *text_container, *attr_container, *attr_subpaned_container, *box2;
		GtkWidget *set_attr;

		tooltips = gtk_tooltips_new ();
		gtk_tooltips_enable (tooltips);

		dialog = sp_window_new ("", TRUE);
		gtk_container_set_border_width (GTK_CONTAINER (dialog), 0);
		gtk_window_set_default_size (GTK_WINDOW (dialog), 640, 384);
		g_signal_connect (G_OBJECT (dialog), "destroy", G_CALLBACK (on_destroy), NULL);

		paned = gtk_hpaned_new ();
		gtk_paned_set_position (GTK_PANED (paned), 256);
		gtk_container_add (GTK_CONTAINER (dialog), paned);

		/* tree view */

		box = gtk_vbox_new (FALSE, 0);
		gtk_paned_pack1 (GTK_PANED (paned), box, FALSE, FALSE);

		tree = SP_XMLVIEW_TREE (sp_xmlview_tree_new (NULL, NULL, NULL));
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET (tree), _("Drag to reorder nodes"), NULL);
		g_signal_connect (G_OBJECT (tree), "tree_select_row", G_CALLBACK (on_tree_select_row), NULL);
		g_signal_connect (G_OBJECT (tree), "tree_unselect_row", G_CALLBACK (on_tree_unselect_row), NULL);
		g_signal_connect_after (G_OBJECT (tree), "tree_move", G_CALLBACK (after_tree_move), NULL);

		/* fixme: replace gtk_signal_connect_while_alive() with something else... */

		toolbar = gtk_toolbar_new ();
		gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
		gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
						  NULL,
						  _("New element node"),
						  NULL,
						  gtk_image_new_from_file (SODIPODI_PIXMAPDIR "/add_xml_element_node.xpm"),
						  G_CALLBACK (cmd_new_element_node),
						  NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree),
						"tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_element),
						button,
						GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree),
						"tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button,
						GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
						  NULL, _("New text node"), NULL,
						  gtk_image_new_from_file (SODIPODI_PIXMAPDIR "/add_xml_text_node.xpm"),
						  G_CALLBACK (cmd_new_text_node),
						  NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree),
						"tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_element),
						button,
						GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree),
						"tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button,
						GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
						  NULL, _("Duplicate node"), NULL,
						  gtk_image_new_from_file (SODIPODI_PIXMAPDIR "/duplicate_xml_node.xpm"),
						  G_CALLBACK (cmd_duplicate_node),
						  NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree),
						"tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_non_root),
						button,
						GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Delete node"), NULL,
						  gtk_image_new_from_file (SODIPODI_PIXMAPDIR "/delete_xml_node.xpm"),
						  G_CALLBACK (cmd_delete_node), NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_non_root),
						button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "<", _("Unindent node"), NULL,
						  gtk_arrow_new (GTK_ARROW_LEFT, GTK_SHADOW_IN),
						  G_CALLBACK (cmd_unindent_node), NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_has_grandparent),
						button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), ">", _("Indent node"), NULL,
						  gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_IN),
						  G_CALLBACK (cmd_indent_node), NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_indentable),
						button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						(GCallback) on_tree_unselect_row_disable, button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "^", _("Raise node"), NULL,
						  gtk_arrow_new (GTK_ARROW_UP, GTK_SHADOW_IN),
						  G_CALLBACK (cmd_raise_node), NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_not_first_child),
						button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "v", _("Lower node"), NULL,
						  gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN),
						  G_CALLBACK (cmd_lower_node), NULL);
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						G_CALLBACK (on_tree_select_row_enable_if_not_last_child),
						button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						G_CALLBACK (on_tree_unselect_row_disable),
						button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		gtk_box_pack_start (GTK_BOX (box), toolbar, FALSE, TRUE, 0); 

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (box), sw, TRUE, TRUE, 0);

		gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (tree));

		/* node view */

		box = gtk_vbox_new (FALSE, 0);
		gtk_paned_pack2 (GTK_PANED (paned), box, TRUE, TRUE);

		/* attributes */

		attr_container = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (attr_container), TRUE, TRUE, 0);

		attributes = SP_XMLVIEW_ATTR_LIST (sp_xmlview_attr_list_new (NULL));
		g_signal_connect (G_OBJECT (attributes), "select_row", G_CALLBACK (on_attr_select_row), NULL);
		g_signal_connect (G_OBJECT (attributes), "unselect_row", G_CALLBACK (on_attr_unselect_row), NULL);

		toolbar = gtk_toolbar_new ();
		gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
		gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);

		button = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Delete attribute"), NULL,
						  gtk_image_new_from_file (SODIPODI_PIXMAPDIR "/delete_xml_attribute.xpm"),
						  (GCallback) cmd_delete_attr, NULL);

		gtk_signal_connect_while_alive (GTK_OBJECT (attributes), "select_row",
						(GCallback) on_attr_select_row_enable, button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (attributes), "unselect_row",
						(GCallback) on_attr_unselect_row_disable, button, GTK_OBJECT (button));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						(GCallback) on_tree_unselect_row_disable, button, GTK_OBJECT (button));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

		gtk_box_pack_start (GTK_BOX (attr_container), GTK_WIDGET (toolbar), FALSE, TRUE, 0);

		attr_subpaned_container = gtk_vpaned_new();
		gtk_box_pack_start (GTK_BOX (attr_container), GTK_WIDGET (attr_subpaned_container), TRUE, TRUE, 0);
		gtk_widget_show(attr_subpaned_container);
		
		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_paned_pack1 (GTK_PANED(attr_subpaned_container), GTK_WIDGET (sw), TRUE, TRUE);
		gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (attributes));

		toolbar = gtk_vbox_new (FALSE, 4);
		gtk_container_set_border_width (GTK_CONTAINER (toolbar), 4);

		box2 = gtk_hbox_new (FALSE, 4);
		gtk_box_pack_start (GTK_BOX (toolbar), GTK_WIDGET (box2), FALSE, TRUE, 0);

		attr_name = GTK_EDITABLE (gtk_entry_new ());
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET (attr_name), _("Attribute name"), NULL);
		gtk_signal_connect (GTK_OBJECT (attributes), "select_row", (GCallback) on_attr_select_row_set_name_content, attr_name);
		gtk_signal_connect (GTK_OBJECT (attributes), "unselect_row", (GCallback) on_attr_unselect_row_clear_text, attr_name);
		gtk_signal_connect (GTK_OBJECT (tree), "tree_unselect_row", (GCallback) on_tree_unselect_row_clear_text, attr_name);
		gtk_box_pack_start (GTK_BOX (box2), GTK_WIDGET (attr_name), TRUE, TRUE, 0);

		set_attr = gtk_button_new ();
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET (set_attr), _("Set attribute"), NULL);
		gtk_container_add (GTK_CONTAINER (set_attr), gtk_image_new_from_file (SODIPODI_PIXMAPDIR "/set.xpm"));
		gtk_signal_connect (GTK_OBJECT (set_attr), "clicked", (GCallback) cmd_set_attr, NULL);
		gtk_signal_connect (GTK_OBJECT (attr_name), "changed", (GCallback) on_editable_changed_enable_if_valid_xml_name, set_attr);
		gtk_widget_set_sensitive (GTK_WIDGET (set_attr), FALSE);

		gtk_box_pack_start (GTK_BOX (box2), set_attr, FALSE, FALSE, 0);

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (toolbar), sw, TRUE, TRUE, 0);

		attr_value = (GtkTextView *) gtk_text_view_new ();
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET (attr_value), _("Attribute value"), NULL);
		gtk_signal_connect (GTK_OBJECT (attributes), "select_row", (GCallback) on_attr_select_row_set_value_content, attr_value);
		gtk_signal_connect (GTK_OBJECT (attributes), "unselect_row", (GCallback) on_attr_unselect_row_clear_text, attr_value);
		gtk_signal_connect (GTK_OBJECT (tree), "tree_unselect_row", (GCallback) on_tree_unselect_row_clear_text, attr_value);
		gtk_text_view_set_editable (attr_value, TRUE);
		gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (attr_value));

		gtk_paned_pack2 (GTK_PANED (attr_subpaned_container), GTK_WIDGET (toolbar), FALSE, TRUE);

		/* text */

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (sw), TRUE, TRUE, 0);

		content = SP_XMLVIEW_CONTENT (sp_xmlview_content_new (NULL));
		gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (content));

		text_container = sw;

		/* initial show/hide */

		gtk_widget_show_all (GTK_WIDGET (dialog));

		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						(GCallback) on_tree_select_row_show_if_element, attr_container, GTK_OBJECT (attr_container));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						(GCallback) on_tree_unselect_row_hide, attr_container, GTK_OBJECT (attr_container));
		gtk_widget_hide (attr_container);

		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_select_row",
						(GCallback) on_tree_select_row_show_if_text, text_container, GTK_OBJECT (text_container));
		gtk_signal_connect_while_alive (GTK_OBJECT (tree), "tree_unselect_row",
						(GCallback) on_tree_unselect_row_hide, text_container, GTK_OBJECT (text_container));
		gtk_widget_hide (text_container);
	} else {
		gdk_window_raise (GTK_WIDGET (dialog)->window);
	}

	g_assert (desktop != NULL);
	set_tree_desktop (desktop);
}

void
set_tree_desktop (SPDesktop * desktop)
{
	if ( desktop == current_desktop ) return;
	if (current_desktop) {
		if (SP_DT_SELECTION (current_desktop)) {
			sp_signal_disconnect_by_data (SP_DT_SELECTION (current_desktop), dialog);
		}
		sp_signal_disconnect_by_data (current_desktop, dialog);
	}
	current_desktop = desktop;
	if (desktop) {
		g_signal_connect (G_OBJECT (desktop), "shutdown", G_CALLBACK (on_desktop_shutdown), dialog);
		g_signal_connect (G_OBJECT (SP_DT_SELECTION (desktop)), "changed", G_CALLBACK (on_desktop_selection_changed), dialog);
		set_tree_document (SP_DT_DOCUMENT (desktop));
	} else {
		set_tree_document (NULL);
	}
}

void
set_tree_document (SPDocument * document)
{
	if ( document == current_document ) return;
	if (current_document) {
		sp_signal_disconnect_by_data (current_document, dialog);
	}
	current_document = document;
	if (current_document) {
		g_signal_connect (G_OBJECT (current_document), "uri_set", G_CALLBACK (on_document_uri_set), dialog);
		on_document_uri_set (current_document, SP_DOCUMENT_URI (current_document), dialog);
		set_tree_repr (sp_document_repr_root (current_document));
	} else {
		set_tree_repr (NULL);
	}
}

void
set_tree_repr (SPRepr * repr)
{
	if ( repr == selected_repr ) return;

	gtk_clist_freeze (GTK_CLIST (tree));

	sp_xmlview_tree_set_repr (tree, repr);

	if (repr) {
		set_tree_select (get_dt_select ());
	} else {
		set_tree_select (NULL);
	}

	gtk_clist_thaw (GTK_CLIST (tree));

	propagate_tree_select (selected_repr);
}

void
set_tree_select (SPRepr * repr)
{
	if (selected_repr) {
		sp_repr_unref (selected_repr);
	}
	selected_repr = repr;
	if (repr) {
		GtkCTreeNode * node;

		sp_repr_ref (selected_repr);

		node = sp_xmlview_tree_get_repr_node (SP_XMLVIEW_TREE (tree), repr);
		if (node) {
			GtkCTreeNode * parent;

			gtk_ctree_select (GTK_CTREE (tree), node);

			parent = GTK_CTREE_ROW (node)->parent;
			while (parent) {
				gtk_ctree_expand (GTK_CTREE (tree), parent);
				parent = GTK_CTREE_ROW (parent)->parent;
			}
		}
	} else {
		gtk_clist_unselect_all (GTK_CLIST (tree));
	}
	propagate_tree_select (repr);
}

void
propagate_tree_select (SPRepr * repr)
{
	if (repr && SP_REPR_TYPE (repr) == SP_XML_ELEMENT_NODE) {
		sp_xmlview_attr_list_set_repr (attributes, repr);
	} else {
		sp_xmlview_attr_list_set_repr (attributes, NULL);
	}
	if (repr && SP_REPR_TYPE (repr) == SP_XML_TEXT_NODE) {
		sp_xmlview_content_set_repr (content, repr);
	} else {
		sp_xmlview_content_set_repr (content, NULL);
	}
}

SPRepr *
get_dt_select (void)
{
	if (!current_desktop) return NULL;

	return sp_selection_repr (SP_DT_SELECTION (current_desktop));
}

void
set_dt_select (SPRepr *repr)
{
	SPSelection *selection;
	SPObject *object;
	const gchar *id;
	
	if (!current_desktop) return;
	selection = SP_DT_SELECTION (current_desktop);

	if (repr) {
		while ( ( SP_REPR_TYPE(repr) != SP_XML_ELEMENT_NODE ) && sp_repr_parent(repr) ) repr = sp_repr_parent(repr);

		id = sp_repr_attr(repr, "id");
		object = (id) ? sp_document_lookup_id (SP_DT_DOCUMENT (current_desktop), id) : NULL;
	} else {
		object = NULL;
	}

	blocked++;
	if (object && SP_IS_ITEM (object)) {
		sp_selection_set_item (selection, SP_ITEM (object));
	} else {
		sp_selection_empty (selection);
	}
	blocked--;
}
	
void
on_tree_select_row (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;

	if (blocked) return;

	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	g_assert ( repr != NULL );

	if ( selected_repr == repr ) return;

	if (selected_repr) {
		sp_repr_unref (selected_repr);
		selected_repr = NULL;
	}
	selected_repr = repr;
	sp_repr_ref (selected_repr);

	propagate_tree_select (selected_repr);

	set_dt_select (selected_repr);
}

void
on_tree_unselect_row (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;

	if (blocked) return;

	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	propagate_tree_select (NULL);
	set_dt_select (NULL);
	if ( selected_repr && ( selected_repr == repr ) ) {
		sp_repr_unref (selected_repr);
		selected_repr = NULL;
		selected_attr = 0;
	}
}

void
after_tree_move (GtkCTree * tree, GtkCTreeNode * node, GtkCTreeNode * new_parent, GtkCTreeNode * new_sibling, gpointer data)
{
	if (GTK_CTREE_ROW (node)->parent == new_parent && GTK_CTREE_ROW (node)->sibling == new_sibling) {
		sp_document_done (current_document);
	} else {
		sp_document_cancel (current_document);
	}
}

void
on_destroy (GtkObject * object, gpointer data)
{
	set_tree_desktop (NULL);
	gtk_object_destroy (GTK_OBJECT (tooltips));
	dialog = NULL;
}

void
on_tree_select_row_enable (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
}

void
on_tree_select_row_enable_if_element (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;
	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	if (SP_REPR_TYPE (repr) == SP_XML_ELEMENT_NODE) {
		gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
	}
}

void
on_tree_select_row_show_if_element (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;
	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	if (SP_REPR_TYPE (repr) == SP_XML_ELEMENT_NODE) {
		gtk_widget_show (GTK_WIDGET (data));
	} else {
		gtk_widget_hide (GTK_WIDGET (data));
	}
}

void
on_tree_select_row_show_if_text (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;
	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	if (SP_REPR_TYPE (repr) == SP_XML_TEXT_NODE) {
		gtk_widget_show (GTK_WIDGET (data));
	} else {
		gtk_widget_hide (GTK_WIDGET (data));
	}
}

void
on_tree_select_row_enable_if_non_root (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	if (GTK_CTREE_ROW (node)->parent) {
		gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
	}
}

void
on_tree_unselect_row_disable (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
}

void
on_tree_unselect_row_hide (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	gtk_widget_hide (GTK_WIDGET (data));
}

void
on_tree_unselect_row_clear_text (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	if (GTK_IS_EDITABLE (data)) {
		gtk_editable_delete_text (GTK_EDITABLE (data), 0, -1);
	} else if (GTK_IS_TEXT_VIEW (data)) {
		GtkTextBuffer *tb;
		tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
		gtk_text_buffer_set_text (tb, "", 0);
	}
}

void
on_attr_select_row (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	selected_attr = sp_xmlview_attr_list_get_row_key (list, row);
}

void
on_attr_unselect_row (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	selected_attr = 0;
}

void
on_attr_select_row_set_name_content (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	GtkEditable * editable;
	const guchar * name;
	gint pos;
	editable = GTK_EDITABLE (data);
	name = g_quark_to_string (sp_xmlview_attr_list_get_row_key (list, row));
	gtk_editable_delete_text (editable, 0, -1);
	pos = 0;
	gtk_editable_insert_text (editable, name, strlen(name), &pos);
	
}

void
on_attr_select_row_set_value_content (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	GtkTextBuffer *tb;
	const guchar *name, *value;
	tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
	name = g_quark_to_string (sp_xmlview_attr_list_get_row_key (list, row));
	value = sp_repr_attr (selected_repr, name);
	gtk_text_buffer_set_text (tb, value, strlen (value));
}

void
on_tree_select_row_enable_if_indentable (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr, * prev;
	gboolean indentable;
	indentable = FALSE;
	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	if ( repr->parent && repr != repr->parent->children ) {
		g_assert (repr->parent->children);
		for ( prev = repr->parent->children ; prev != repr ;
		      prev = prev->next );
		if (SP_REPR_TYPE (prev) == SP_XML_ELEMENT_NODE) {
			indentable = TRUE;
		}
	}
	gtk_widget_set_sensitive (GTK_WIDGET (data), indentable);
}

void
on_tree_select_row_enable_if_not_first_child (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;
	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	if ( repr->parent && repr != repr->parent->children ) {
		gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
	}
}

void
on_tree_select_row_enable_if_not_last_child (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	SPRepr * repr;
	repr = sp_xmlview_tree_node_get_repr (SP_XMLVIEW_TREE (tree), node);
	if ( repr->parent && repr->parent->parent && repr->next ) {
		gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
	}
}

void
on_tree_select_row_enable_if_has_grandparent (GtkCTree * tree, GtkCTreeNode * node, gint column, gpointer data)
{
	GtkCTreeNode * parent, * grandparent;
	parent = GTK_CTREE_ROW (node)->parent;
	if (parent) {
		grandparent = GTK_CTREE_ROW (parent)->parent;
		if (grandparent) {
			gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
		} else {
			gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
		}
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
	}
}

void
on_attr_select_row_enable (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
}

void
on_attr_unselect_row_disable (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
}

void
on_attr_unselect_row_clear_text (GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
	if (GTK_IS_EDITABLE (data)) {
		gtk_editable_delete_text (GTK_EDITABLE (data), 0, -1);
	} else if (GTK_IS_TEXT_VIEW (data)) {
		GtkTextBuffer *tb;
		tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
		gtk_text_buffer_set_text (tb, "", 0);
	}
}

void
on_editable_changed_enable_if_valid_xml_name (GtkEditable * editable, gpointer data)
{
	gchar * text;
	text = gtk_editable_get_chars (editable, 0, -1);
	/* fixme: need to do checking a little more rigorous than this */
	if (strlen(text)) {
		gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
	}
	g_free (text);
}

void
on_desktop_selection_changed (SPSelection * selection)
{
	if (!blocked++) {
		set_tree_select (get_dt_select ());
	}
	blocked--;
}

gboolean
on_desktop_shutdown (SPDesktop * desktop, gpointer data)
{
	g_assert (dialog != NULL);
	gtk_widget_destroy (dialog);

	return FALSE;
}

void
on_document_uri_set (SPDocument * document, const guchar * uri, gpointer data)
{
	guchar * title;
	title = g_strdup_printf (_("Sodipodi: %s : XML View"), SP_DOCUMENT_NAME (document));
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	g_free (title);
}

void
on_clicked_get_editable_text (GtkWidget * widget, gpointer data)
{
	EditableDest * dest;
	dest = (EditableDest *) data;
	dest->text = gtk_editable_get_chars (dest->editable, 0, -1);
}

void
cmd_new_element_node (GtkObject * object, gpointer data)
{
	EditableDest name;
	GtkWidget * window, * create, * cancel, * vbox, * entry, * bbox, * sep;

	g_assert (selected_repr != NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 4);
	gtk_window_set_title (GTK_WINDOW (window), _("New element node..."));
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (dialog));
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", gtk_main_quit, NULL);

	vbox = gtk_vbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, TRUE, 0);

	sep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), sep, FALSE, TRUE, 0);

	bbox = gtk_hbutton_box_new ();
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_END);
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, TRUE, 0);

	cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_signal_connect_object (GTK_OBJECT (cancel), "clicked", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (window));
	gtk_container_add (GTK_CONTAINER (bbox), cancel);

	create = gtk_button_new_with_label (_("Create"));
	gtk_widget_set_sensitive (GTK_WIDGET (create), FALSE);
	gtk_signal_connect (GTK_OBJECT (entry), "changed", G_CALLBACK (on_editable_changed_enable_if_valid_xml_name), create);
	gtk_signal_connect (GTK_OBJECT (create), "clicked", G_CALLBACK (on_clicked_get_editable_text), &name);
	gtk_signal_connect_object (GTK_OBJECT (create), "clicked", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (window));
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (create), GTK_CAN_DEFAULT | GTK_RECEIVES_DEFAULT);
	gtk_container_add (GTK_CONTAINER (bbox), create);
	
	gtk_widget_show_all (GTK_WIDGET (window));
	gtk_window_set_default (GTK_WINDOW (window), GTK_WIDGET (create));
	gtk_window_set_focus (GTK_WINDOW (window), GTK_WIDGET (entry));

	name.editable = GTK_EDITABLE (entry);
	name.text = NULL;

	gtk_main();

	g_assert (selected_repr != NULL);

	if (name.text) {
		SPRepr * new_repr;
		new_repr = sp_repr_new (name.text);
		g_free (name.text);
		sp_repr_append_child (selected_repr, new_repr);
		set_tree_select (new_repr);
		set_dt_select (new_repr);
	}

}

void
cmd_new_text_node (GtkObject * object, gpointer data)
{
	SPRepr * text;

	g_assert (selected_repr != NULL);

	text = sp_repr_new_text ("");
	sp_repr_append_child (selected_repr, text);

	sp_document_done (current_document);
	
	set_tree_select (text);
	set_dt_select (text);
}

void
cmd_duplicate_node (GtkObject * object, gpointer data)
{
	SPRepr * parent;
	SPRepr * dup;
	GtkCTreeNode * node;

	g_assert (selected_repr != NULL);

	parent = sp_repr_parent (selected_repr);
	dup = sp_repr_duplicate (selected_repr);
	sp_repr_add_child (parent, dup, selected_repr);

	sp_document_done (current_document);

	node = sp_xmlview_tree_get_repr_node (SP_XMLVIEW_TREE (tree), dup);
	if (node) gtk_ctree_select (GTK_CTREE (tree), node);
}

void
cmd_delete_node (GtkObject * object, gpointer data)
{
	g_assert (selected_repr != NULL);
	sp_repr_unparent (selected_repr);

	sp_document_done (current_document);
}

void
cmd_delete_attr (GtkObject * object, gpointer data)
{
	g_assert (selected_repr != NULL);
	g_assert (selected_attr != 0);
	sp_repr_set_attr (selected_repr, g_quark_to_string (selected_attr), NULL);

	sp_document_done (current_document);
}

void
cmd_set_attr (GtkObject * object, gpointer data)
{
	GtkTextIter start, end;
	gchar * name;
	gchar * value;
	gint row;

	g_assert (selected_repr != NULL);

	name = gtk_editable_get_chars (attr_name, 0, -1);
	gtk_text_buffer_get_bounds (gtk_text_view_get_buffer (attr_value), &start, &end);
	value = gtk_text_buffer_get_text (gtk_text_view_get_buffer (attr_value), &start, &end, TRUE);

	sp_repr_set_attr (selected_repr, name, value);

	g_free (name);
	g_free (value);

	sp_document_done (current_document);

	/* fixme: actually, the row won't have been created yet.  why? */
	row = sp_xmlview_attr_list_find_row_from_key (GTK_CLIST (attributes), g_quark_from_string (name));
	if (row != -1) {
		gtk_clist_select_row (GTK_CLIST (attributes), row, 0);
	}
}

void
cmd_raise_node (GtkObject * object, gpointer data)
{
	SPRepr * before, * parent, * ref;
	g_assert (selected_repr != NULL);

	parent = sp_repr_parent (selected_repr);
	g_return_if_fail (parent != NULL);
	g_return_if_fail (parent->children != selected_repr);

	ref = NULL;
	before = parent->children;
	while (before && before->next != selected_repr) {
		ref = before;
		before = before->next;
	}

	sp_repr_change_order (parent, selected_repr, ref);

	sp_document_done (current_document);

	set_tree_select (selected_repr);
	set_dt_select (selected_repr);
}

void
cmd_lower_node (GtkObject * object, gpointer data)
{
	SPRepr * parent;
	g_assert (selected_repr != NULL);
	g_return_if_fail (selected_repr->next != NULL);
	parent = sp_repr_parent (selected_repr);

	sp_repr_change_order (parent, selected_repr, selected_repr->next);

	sp_document_done (current_document);

	set_tree_select (selected_repr);
	set_dt_select (selected_repr);
}

void
cmd_indent_node (GtkObject * object, gpointer data)
{
	int ok;
	SPRepr * prev, * parent, * repr, * ref;

	repr = selected_repr;
	g_assert (repr != NULL);
	parent = sp_repr_parent (repr);
	g_return_if_fail (parent != NULL);
	g_return_if_fail (parent->children != repr);

	prev = parent->children;
	while (prev && prev->next != repr) {
	      prev = prev->next;
	}
	g_return_if_fail (prev != NULL);
	g_return_if_fail (SP_REPR_TYPE (prev) == SP_XML_ELEMENT_NODE);

	if (prev->children) {
		for ( ref = prev->children ; ref->next ; ref = ref->next );
	} else {
		ref = NULL;
	}

	ok = sp_repr_remove_child (parent, repr) &&
	     sp_repr_add_child (prev, repr, ref);

	if (ok) {
		sp_document_done (current_document);
		set_tree_select (repr);
		set_dt_select (repr);
	} else {
		sp_document_cancel (current_document);
	}
}

void
cmd_unindent_node (GtkObject * object, gpointer data)
{
	SPRepr * grandparent, * parent, * repr;
	int ok;

	repr = selected_repr;
	g_assert (repr != NULL);
	parent = sp_repr_parent (repr);
	g_return_if_fail (parent);
	grandparent = sp_repr_parent (parent);
	g_return_if_fail (grandparent);
	
	ok = sp_repr_remove_child (parent, repr) &&
	     sp_repr_add_child (grandparent, repr, parent);
	
	if (ok) {
		sp_document_done (current_document);
		set_tree_select (repr);
		set_dt_select (repr);
	} else {
		sp_document_cancel (current_document);
	}
}

