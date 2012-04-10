/*
 * Specialization of GtkTreeView for the XML tree view
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cstring>
#include <glibmm/i18n.h>

#include "helper/sp-marshal.h"
#include "../xml/node-event-vector.h"
#include "sp-xmlview-attr-list.h"

static void sp_xmlview_attr_list_class_init (SPXMLViewAttrListClass * klass);
static void sp_xmlview_attr_list_init (SPXMLViewAttrList * list);
static void sp_xmlview_attr_list_destroy (GtkObject * object);

static void event_attr_changed (Inkscape::XML::Node * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data);

static GtkTreeViewClass * parent_class = NULL;

static Inkscape::XML::NodeEventVector repr_events = {
	NULL, /* child_added */
	NULL, /* child_removed */
	event_attr_changed,
	NULL, /* content_changed */
	NULL  /* order_changed */
};

enum {COL_NAME=0, COL_VALUE, COL_ATTR};

GtkWidget *
sp_xmlview_attr_list_new (Inkscape::XML::Node * repr)
{
    SPXMLViewAttrList * attr_list;

    attr_list = (SPXMLViewAttrList*)g_object_new (SP_TYPE_XMLVIEW_ATTR_LIST, NULL);

    attr_list->store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    gtk_tree_view_set_model (GTK_TREE_VIEW(attr_list), GTK_TREE_MODEL(attr_list->store));

    GtkCellRenderer *cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(attr_list), COL_NAME, _("Attribute"), cell, "text", 0, NULL);
    GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW(attr_list), COL_NAME);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_sort_column_id (column, COL_NAME);
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(attr_list->store), COL_NAME, GTK_SORT_ASCENDING);
    gtk_cell_renderer_set_padding (cell, 2, 0);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(attr_list), COL_VALUE, _("Value"), cell, "text", COL_VALUE, NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW(attr_list), COL_VALUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_cell_renderer_set_padding (cell, 2, 0);

    sp_xmlview_attr_list_set_repr (attr_list, repr);

    return (GtkWidget *) attr_list;
}

void
sp_xmlview_attr_list_set_repr (SPXMLViewAttrList * list, Inkscape::XML::Node * repr)
{
	if ( repr == list->repr ) return;
	if (list->repr) {
		gtk_list_store_clear(list->store);
		sp_repr_remove_listener_by_data (list->repr, list);
		Inkscape::GC::release(list->repr);
	}
	list->repr = repr;
	if (repr) {
		Inkscape::GC::anchor(repr);
		sp_repr_add_listener (repr, &repr_events, list);
		sp_repr_synthesize_events (repr, &repr_events, list);
	}
}

GType sp_xmlview_attr_list_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPXMLViewAttrListClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_xmlview_attr_list_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPXMLViewAttrList),
            0, // n_preallocs
            (GInstanceInitFunc)sp_xmlview_attr_list_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_TREE_VIEW, "SPXMLViewAttrList", &info, static_cast<GTypeFlags>(0));
    }

    return type;
}

void
sp_xmlview_attr_list_class_init (SPXMLViewAttrListClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;
	object_class->destroy = sp_xmlview_attr_list_destroy;

	parent_class = (GtkTreeViewClass*)g_type_class_peek_parent (klass);

    g_signal_new (  "row-value-changed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (SPXMLViewAttrListClass, row_changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE, 1,
        G_TYPE_STRING);
}

void
sp_xmlview_attr_list_init (SPXMLViewAttrList * list)
{
    list->store = NULL;
	list->repr = NULL;
}

void
sp_xmlview_attr_list_destroy (GtkObject * object)
{
	SPXMLViewAttrList * list;

	list = SP_XMLVIEW_ATTR_LIST (object);

	sp_xmlview_attr_list_set_repr (list, NULL);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

void sp_xmlview_attr_list_select_row_by_key(SPXMLViewAttrList * list, gchar *name)
{
    GtkTreeIter iter;
    const gchar *n;
    gboolean match = false;
    gboolean valid = gtk_tree_model_get_iter_first( GTK_TREE_MODEL(list->store), &iter );
    while ( valid ) {
        gtk_tree_model_get (GTK_TREE_MODEL(list->store), &iter, COL_NAME, &n, -1);
        if (!strcmp(n, name)) {
            match = true;
            break;
        }
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(list->store), &iter);
    }

    if (match) {
        GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(list));
        gtk_tree_selection_select_iter(selection, &iter);
    }
}

void
event_attr_changed (Inkscape::XML::Node * /*repr*/,
                    const gchar * name,
                    const gchar * /*old_value*/,
                    const gchar * new_value,
                    bool /*is_interactive*/,
                    gpointer data)
{
	gint row = -1;
	SPXMLViewAttrList * list;

	list = SP_XMLVIEW_ATTR_LIST (data);

    GtkTreeIter iter;
    const gchar *n;
    gboolean valid = gtk_tree_model_get_iter_first( GTK_TREE_MODEL(list->store), &iter );
    gboolean match = false;
    while ( valid ) {
        gtk_tree_model_get (GTK_TREE_MODEL(list->store), &iter, COL_NAME, &n, -1);
        if (!strcmp(n, name)) {
            match = true;
            break;
        }
        row++;
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(list->store), &iter);
    }

	if (match) {
		if (new_value) {
			gtk_list_store_set (list->store, &iter, COL_NAME, name, COL_VALUE, new_value, COL_ATTR, GINT_TO_POINTER (g_quark_from_string (name)), -1);
		} else {
			gtk_list_store_remove  (list->store, &iter);
		}
	} else if (new_value != NULL) {
	    gtk_list_store_append (list->store, &iter);
        gtk_list_store_set (list->store, &iter, COL_NAME, name, COL_VALUE, new_value, COL_ATTR, GINT_TO_POINTER (g_quark_from_string (name)), -1);

	}

	// send a "changed" signal so widget owners will know I've updated
	g_signal_emit_by_name(G_OBJECT (list), "row-value-changed", name );
}

