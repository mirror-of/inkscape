#define __SP_XMLVIEW_CONTENT_C__

/*
 * Specialization of GtkTextView for the XML tree view
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
#include <gtk/gtkadjustment.h>

#include "xml/repr.h"
#include "xml/repr-private.h"
#include "sp-xmlview-content.h"
#include "macros.h"
#include "desktop-handles.h"
#include "document-private.h"
#include "inkscape.h"

static void sp_xmlview_content_class_init (SPXMLViewContentClass * klass);
static void sp_xmlview_content_init (SPXMLViewContent * text);
static void sp_xmlview_content_destroy (GtkObject * object);

void sp_xmlview_content_changed (GtkTextBuffer *tb, SPXMLViewContent *text);

static void event_content_changed (SPRepr * repr, const gchar * old_content, const gchar * new_content, gpointer data);

static GtkTextViewClass * parent_class = NULL;

static SPReprEventVector repr_events = {
	NULL, /* destroy */
	NULL, /* add_child */
	NULL, /* child_added */
	NULL, /* remove_child */
	NULL, /* child_removed */
	NULL, /* change_attr */
	NULL, /* attr_changed */
	NULL, /* change_text */
	event_content_changed,
	NULL, /* change_order */
	NULL  /* order_changed */
};

GtkWidget *
sp_xmlview_content_new (SPRepr * repr)
{
	GtkTextBuffer *tb;
	SPXMLViewContent *text;

	tb = gtk_text_buffer_new (NULL);
	text = (SPXMLViewContent*)gtk_type_new (SP_TYPE_XMLVIEW_CONTENT);
	gtk_text_view_set_buffer (GTK_TEXT_VIEW (text), tb);

	g_signal_connect (G_OBJECT (tb), "changed", G_CALLBACK (sp_xmlview_content_changed), text);

	/* should we alter the scrolling adjustments here? */

	sp_xmlview_content_set_repr (text, repr);

	return (GtkWidget *) text;
}

void
sp_xmlview_content_set_repr (SPXMLViewContent * text, SPRepr * repr)
{
	if ( repr == text->repr ) return;
	if (text->repr) {
		sp_repr_remove_listener_by_data (text->repr, text);
		sp_repr_unref (text->repr);
	}
	text->repr = repr;
	if (repr) {
		sp_repr_ref (repr);
		sp_repr_add_listener (repr, &repr_events, text);
		sp_repr_synthesize_events (repr, &repr_events, text);
	} else {
		gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text)), "", 0);
		gtk_text_view_set_editable (GTK_TEXT_VIEW (text), FALSE);
	}
}

GtkType
sp_xmlview_content_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		static const GtkTypeInfo info = {
			"SPXMLViewContent",
			sizeof (SPXMLViewContent),
			sizeof (SPXMLViewContentClass),
			(GtkClassInitFunc) sp_xmlview_content_class_init,
			(GtkObjectInitFunc) sp_xmlview_content_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_TEXT_VIEW, &info);
	}

	return type;
}

void
sp_xmlview_content_class_init (SPXMLViewContentClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = (GtkTextViewClass*)gtk_type_class (GTK_TYPE_TEXT_VIEW);

	object_class->destroy = sp_xmlview_content_destroy;
}

void
sp_xmlview_content_init (SPXMLViewContent *text)
{
	text->repr = NULL;
	text->blocked = FALSE;
}

void
sp_xmlview_content_destroy (GtkObject * object)
{
	SPXMLViewContent * text;

	text = SP_XMLVIEW_CONTENT (object);

	sp_xmlview_content_set_repr (text, NULL);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

void
event_content_changed (SPRepr * repr, const gchar * old_content, const gchar * new_content, gpointer data)
{
	SPXMLViewContent * text;
	text = SP_XMLVIEW_CONTENT (data);

	if (text->blocked) return;

	text->blocked = TRUE;

	if (new_content) {
		gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text)), new_content, strlen (new_content));
	} else {
		gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text)), "", 0);
	}
	gtk_text_view_set_editable (GTK_TEXT_VIEW (text), new_content != NULL);

	text->blocked = FALSE;
}

void
sp_xmlview_content_changed (GtkTextBuffer *tb, SPXMLViewContent *text)
{
	if (text->blocked) return;

	if (text->repr) {
		GtkTextIter start, end;
		gchar *data;
		text->blocked = TRUE;
		gtk_text_buffer_get_bounds (tb, &start, &end);
		data = gtk_text_buffer_get_text (tb, &start, &end, TRUE);
		sp_repr_set_content (text->repr, data);
		g_free (data);
		text->blocked = FALSE;
		sp_document_done (SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP));
	}
}
