#define __SP_XMLVIEW_ATTR_LIST_C__

/*
 * Specialization of GtkCList for the XML tree view
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <string.h>
#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtklist.h>
#include <gtk/gtkadjustment.h>
#include "helper/sp-intl.h"
#include "../xml/repr.h"
#include "../xml/repr-private.h"
#include "sp-xmlview-attr-list.h"

static void sp_xmlview_attr_list_class_init (SPXMLViewAttrListClass * klass);
static void sp_xmlview_attr_list_init (SPXMLViewAttrList * list);
static void sp_xmlview_attr_list_destroy (GtkObject * object);

static void event_attr_changed (SPRepr * repr, const guchar * name, const guchar * old_value, const guchar * new_value, gpointer data);

static GtkCListClass * parent_class = NULL;

static SPReprEventVector repr_events = {
	NULL, /* destroy */
	NULL, /* add_child */
	NULL, /* child_added */
	NULL, /* remove_child */
	NULL, /* child_removed */
	NULL, /* change_attr */
	event_attr_changed,
	NULL, /* change_list */
	NULL, /* content_changed */
	NULL, /* change_order */
	NULL  /* order_changed */
};

GtkWidget *
sp_xmlview_attr_list_new (SPRepr * repr)
{
	SPXMLViewAttrList * list;

	list = g_object_new (SP_TYPE_XMLVIEW_ATTR_LIST, "n_columns", 2, NULL);

	gtk_clist_set_column_title (GTK_CLIST (list), 0, _("Attribute"));
	gtk_clist_set_column_title (GTK_CLIST (list), 1, _("Value"));
	gtk_clist_column_titles_show (GTK_CLIST (list));

	gtk_clist_column_titles_passive (GTK_CLIST (list));
	gtk_clist_set_column_auto_resize (GTK_CLIST (list), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (list), 1, TRUE);
	gtk_clist_set_sort_column (GTK_CLIST (list), 0);
	gtk_clist_set_auto_sort (GTK_CLIST (list), TRUE);

	sp_xmlview_attr_list_set_repr (list, repr);

	return (GtkWidget *) list;
}

void
sp_xmlview_attr_list_set_repr (SPXMLViewAttrList * list, SPRepr * repr)
{
	if ( repr == list->repr ) return;
	gtk_clist_freeze (GTK_CLIST (list));
	if (list->repr) {
		gtk_clist_clear (GTK_CLIST (list));
		sp_repr_remove_listener_by_data (list->repr, list);
		sp_repr_unref (list->repr);
	}
	list->repr = repr;
	if (repr) {
		sp_repr_ref (repr);
		sp_repr_add_listener (repr, &repr_events, list);
		sp_repr_synthesize_events (repr, &repr_events, list);
	}
	gtk_clist_thaw (GTK_CLIST (list));
}

GtkType
sp_xmlview_attr_list_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		static const GtkTypeInfo info = {
			"SPXMLViewAttrList",
			sizeof (SPXMLViewAttrList),
			sizeof (SPXMLViewAttrListClass),
			(GtkClassInitFunc) sp_xmlview_attr_list_class_init,
			(GtkObjectInitFunc) sp_xmlview_attr_list_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_CLIST, &info);
	}

	return type;
}

void
sp_xmlview_attr_list_class_init (SPXMLViewAttrListClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;
	object_class->destroy = sp_xmlview_attr_list_destroy;

	parent_class = gtk_type_class (GTK_TYPE_CLIST);
}

void
sp_xmlview_attr_list_init (SPXMLViewAttrList * list)
{
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

void
event_attr_changed (SPRepr * repr, const guchar * name, const guchar * old_value, const guchar * new_value, gpointer data)
{
	gint row;
	SPXMLViewAttrList * list;
	guchar new_text[64 + 4];
	gchar *gtktext;

	list = SP_XMLVIEW_ATTR_LIST (data);

	gtk_clist_freeze (GTK_CLIST (list));

	if (new_value) {
		strncpy (new_text, new_value, 64);
		if (strlen (new_value) >= 64) {
			strcpy (new_text + 64, "...");
		}
		gtktext = new_text;
	} else {
		gtktext = NULL;
	}

	if (old_value) {
		row = gtk_clist_find_row_from_data (GTK_CLIST (list), GINT_TO_POINTER (g_quark_from_string (name)));
		g_assert (row != -1);

		if (new_value) {
			gtk_clist_set_text (GTK_CLIST (list), row, 1, gtktext);
		} else {
			gtk_clist_remove (GTK_CLIST (list), row);
		}
	} else {
		const gchar * text[2];

		g_assert (new_value != NULL);

		text[0] = name;
		text[1] = gtktext;

		row = gtk_clist_append (GTK_CLIST (list), (gchar **)text);
		gtk_clist_set_row_data (GTK_CLIST (list), row, GINT_TO_POINTER (g_quark_from_string (name)));
	}

	gtk_clist_thaw (GTK_CLIST (list));
}

