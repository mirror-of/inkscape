#define __SP_ATTRIBUTE_WIDGET_C__

/*
 * SPAttributeWidget
 *
 * Widget, that listens and modifies repr attributes
 *
 * Authors:
 *  Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Licensed under GNU GPL
 */

#include <string.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktable.h>
#include <gtk/gtklabel.h>
#include "xml/repr.h"
#include "macros.h"
#include "document.h"
#include "sp-object.h"

#include "sp-attribute-widget.h"

static void sp_attribute_widget_class_init (SPAttributeWidgetClass *klass);
static void sp_attribute_widget_init (SPAttributeWidget *widget);
static void sp_attribute_widget_destroy (GtkObject *object);

static void sp_attribute_widget_changed (GtkEditable *editable);

static void sp_attribute_widget_object_modified (SPObject *object, guint flags, SPAttributeWidget *spaw);
static void sp_attribute_widget_object_release (SPObject *object, SPAttributeWidget *spaw);

static GtkEntryClass *parent_class;

GtkType
sp_attribute_widget_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		static const GtkTypeInfo info = {
			"SPAttributeWidget",
			sizeof (SPAttributeWidget),
			sizeof (SPAttributeWidgetClass),
			(GtkClassInitFunc) sp_attribute_widget_class_init,
			(GtkObjectInitFunc) sp_attribute_widget_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_ENTRY, &info);
	}
	return type;
}

static void
sp_attribute_widget_class_init (SPAttributeWidgetClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkEditableClass *editable_class;

	object_class = GTK_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);
	editable_class = GTK_EDITABLE_CLASS (klass);

	parent_class = gtk_type_class (GTK_TYPE_ENTRY);

	object_class->destroy = sp_attribute_widget_destroy;

	editable_class->changed = sp_attribute_widget_changed;
}

static void
sp_attribute_widget_init (SPAttributeWidget *spaw)
{
	spaw->blocked = FALSE;
	spaw->hasobj = FALSE;

	spaw->src.object = NULL;

	spaw->attribute = NULL;
}

static void
sp_attribute_widget_destroy (GtkObject *object)
{
	SPAttributeWidget *spaw;

	spaw = SP_ATTRIBUTE_WIDGET (object);

	if (spaw->attribute) {
		g_free (spaw->attribute);
		spaw->attribute = NULL;
	}

	if (spaw->hasobj) {
		if (spaw->src.object) {
			sp_signal_disconnect_by_data (spaw->src.object, spaw);
			spaw->src.object = NULL;
		}
	} else {
		if (spaw->src.repr) {
			spaw->src.repr = sp_repr_unref (spaw->src.repr);
		}
	}

	((GtkObjectClass *) parent_class)->destroy (object);
}

static void
sp_attribute_widget_changed (GtkEditable *editable)
{
	SPAttributeWidget *spaw;

	spaw = SP_ATTRIBUTE_WIDGET (editable);

	if (!spaw->blocked) {
		const gchar *text;
		spaw->blocked = TRUE;
		text = gtk_entry_get_text (GTK_ENTRY (spaw));
		if (!*text) text = NULL;
		if (spaw->hasobj && spaw->src.object) {
			if (!sp_repr_set_attr (SP_OBJECT_REPR (spaw->src.object), spaw->attribute, text)) {
				/* Cannot set attribute */
				text = sp_repr_attr (SP_OBJECT_REPR (spaw->src.object), spaw->attribute);
				gtk_entry_set_text (GTK_ENTRY (spaw), text ? text : "");
			}
			sp_document_done (SP_OBJECT_DOCUMENT (spaw->src.object));
		} else if (spaw->src.repr) {
			if (!sp_repr_set_attr (spaw->src.repr, spaw->attribute, text)) {
				/* Cannot set attribute */
				text = sp_repr_attr (spaw->src.repr, spaw->attribute);
				gtk_entry_set_text (GTK_ENTRY (spaw), text ? text : "");
			}
			/* fixme: Warning! Undo will not be flushed in given case */
		}
		spaw->blocked = FALSE;
	}
}

GtkWidget *
sp_attribute_widget_new (SPObject *object, const guchar *attribute)
{
	SPAttributeWidget *spaw;

	g_return_val_if_fail (!object || SP_IS_OBJECT (object), NULL);
	g_return_val_if_fail (!object || attribute, NULL);

	spaw = gtk_type_new (SP_TYPE_ATTRIBUTE_WIDGET);

	sp_attribute_widget_set_object (spaw, object, attribute);

	return GTK_WIDGET (spaw);
}

GtkWidget *
sp_attribute_widget_new_repr (SPRepr *repr, const guchar *attribute)
{
	SPAttributeWidget *spaw;

	spaw = gtk_type_new (SP_TYPE_ATTRIBUTE_WIDGET);

	sp_attribute_widget_set_repr (spaw, repr, attribute);

	return GTK_WIDGET (spaw);
}

void
sp_attribute_widget_set_object (SPAttributeWidget *spaw, SPObject *object, const guchar *attribute)
{
	g_return_if_fail (spaw != NULL);
	g_return_if_fail (SP_IS_ATTRIBUTE_WIDGET (spaw));
	g_return_if_fail (!object || SP_IS_OBJECT (object));
	g_return_if_fail (!object || attribute);
	g_return_if_fail (attribute != NULL);

	if (spaw->attribute) {
		g_free (spaw->attribute);
		spaw->attribute = NULL;
	}

	if (spaw->hasobj) {
		if (spaw->src.object) {
			sp_signal_disconnect_by_data (spaw->src.object, spaw);
			spaw->src.object = NULL;
		}
	} else {
		if (spaw->src.repr) {
			spaw->src.repr = sp_repr_unref (spaw->src.repr);
		}
	}

	spaw->hasobj = TRUE;

	if (object) {
		const guchar *val;

		spaw->blocked = TRUE;
		spaw->src.object = object;
		g_signal_connect (G_OBJECT (object), "modified", G_CALLBACK (sp_attribute_widget_object_modified), spaw);
		g_signal_connect (G_OBJECT (object), "release", G_CALLBACK (sp_attribute_widget_object_release), spaw);

		spaw->attribute = g_strdup (attribute);

		val = sp_repr_attr (SP_OBJECT_REPR (object), attribute);
		gtk_entry_set_text (GTK_ENTRY (spaw), val ? val : (const guchar *) "");
		spaw->blocked = FALSE;
	}

	gtk_widget_set_sensitive (GTK_WIDGET (spaw), (spaw->src.object != NULL));
}

void
sp_attribute_widget_set_repr (SPAttributeWidget *spaw, SPRepr *repr, const guchar *attribute)
{
	g_return_if_fail (spaw != NULL);
	g_return_if_fail (SP_IS_ATTRIBUTE_WIDGET (spaw));
	g_return_if_fail (attribute != NULL);

	if (spaw->attribute) {
		g_free (spaw->attribute);
		spaw->attribute = NULL;
	}

	if (spaw->hasobj) {
		if (spaw->src.object) {
			sp_signal_disconnect_by_data (spaw->src.object, spaw);
			spaw->src.object = NULL;
		}
	} else {
		if (spaw->src.repr) {
			spaw->src.repr = sp_repr_unref (spaw->src.repr);
		}
	}

	spaw->hasobj = FALSE;

	if (repr) {
		const guchar *val;

		spaw->blocked = TRUE;
		spaw->src.repr = sp_repr_ref (repr);
		spaw->attribute = g_strdup (attribute);

		val = sp_repr_attr (repr, attribute);
		gtk_entry_set_text (GTK_ENTRY (spaw), val ? val : (const guchar *) "");
		spaw->blocked = FALSE;
	}

	gtk_widget_set_sensitive (GTK_WIDGET (spaw), (spaw->src.repr != NULL));
}

static void
sp_attribute_widget_object_modified (SPObject *object, guint flags, SPAttributeWidget *spaw)
{
	if (flags && SP_OBJECT_MODIFIED_FLAG) {
		const guchar *val, *text;
		val = sp_repr_attr (SP_OBJECT_REPR (spaw->src.object), spaw->attribute);
		text = gtk_entry_get_text (GTK_ENTRY (spaw));
		if (val || text) {
			if (!val || !text || strcmp (val, text)) {
				/* We are different */
				spaw->blocked = TRUE;
				gtk_entry_set_text (GTK_ENTRY (spaw), val ? val : (const guchar *) "");
				spaw->blocked = FALSE;
			}
		}
	}
}

static void
sp_attribute_widget_object_release (SPObject *object, SPAttributeWidget *spaw)
{
	sp_attribute_widget_set_object (spaw, NULL, NULL);
}

/* SPAttributeTable */

static void sp_attribute_table_class_init (SPAttributeTableClass *klass);
static void sp_attribute_table_init (SPAttributeTable *widget);
static void sp_attribute_table_destroy (GtkObject *object);

static void sp_attribute_table_object_modified (SPObject *object, guint flags, SPAttributeTable *spaw);
static void sp_attribute_table_object_release (SPObject *object, SPAttributeTable *spaw);
static void sp_attribute_table_entry_changed (GtkEditable *editable, SPAttributeTable *spat);

static GtkVBoxClass *table_parent_class;

GtkType
sp_attribute_table_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		static const GtkTypeInfo info = {
			"SPAttributeTable",
			sizeof (SPAttributeTable),
			sizeof (SPAttributeTableClass),
			(GtkClassInitFunc) sp_attribute_table_class_init,
			(GtkObjectInitFunc) sp_attribute_table_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_VBOX, &info);
	}
	return type;
}

static void
sp_attribute_table_class_init (SPAttributeTableClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = GTK_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	table_parent_class = gtk_type_class (GTK_TYPE_VBOX);

	object_class->destroy = sp_attribute_table_destroy;
}

static void
sp_attribute_table_init (SPAttributeTable *spat)
{
	spat->blocked = FALSE;
	spat->hasobj = FALSE;
	spat->table = NULL;
	spat->src.object = NULL;
	spat->num_attr = 0;
	spat->attributes = NULL;
	spat->entries = NULL;
}

static void
sp_attribute_table_destroy (GtkObject *object)
{
	SPAttributeTable *spat;

	spat = SP_ATTRIBUTE_TABLE (object);

	if (spat->attributes) {
		gint i;
		for (i = 0; i < spat->num_attr; i++) {
			g_free (spat->attributes[i]);
		}
		g_free (spat->attributes);
		spat->attributes = NULL;
	}

	if (spat->hasobj) {
		if (spat->src.object) {
			sp_signal_disconnect_by_data (spat->src.object, spat);
			spat->src.object = NULL;
		}
	} else {
		if (spat->src.repr) {
			spat->src.repr = sp_repr_unref (spat->src.repr);
		}
	}

	if (spat->entries) {
		g_free (spat->entries);
		spat->entries = NULL;
	}

	spat->table = NULL;

	if (((GtkObjectClass *) table_parent_class)->destroy)
		(* ((GtkObjectClass *) table_parent_class)->destroy) (object);
}

GtkWidget *
sp_attribute_table_new (SPObject *object, gint num_attr, const guchar **labels, const guchar **attributes)
{
	SPAttributeTable *spat;

	g_return_val_if_fail (!object || SP_IS_OBJECT (object), NULL);
	g_return_val_if_fail (!object || (num_attr > 0), NULL);
	g_return_val_if_fail (!num_attr || (labels && attributes), NULL);

	spat = gtk_type_new (SP_TYPE_ATTRIBUTE_TABLE);

	sp_attribute_table_set_object (spat, object, num_attr, labels, attributes);

	return GTK_WIDGET (spat);
}

GtkWidget *
sp_attribute_table_new_repr (SPRepr *repr, gint num_attr, const guchar **labels, const guchar **attributes)
{
	SPAttributeTable *spat;

	g_return_val_if_fail (!num_attr || (labels && attributes), NULL);

	spat = gtk_type_new (SP_TYPE_ATTRIBUTE_TABLE);

	sp_attribute_table_set_repr (spat, repr, num_attr, labels, attributes);

	return GTK_WIDGET (spat);
}

#define XPAD 4
#define YPAD 0

void
sp_attribute_table_set_object (SPAttributeTable *spat, SPObject *object, gint num_attr, const guchar **labels, const guchar **attributes)
{
	g_return_if_fail (spat != NULL);
	g_return_if_fail (SP_IS_ATTRIBUTE_TABLE (spat));
	g_return_if_fail (!object || SP_IS_OBJECT (object));
	g_return_if_fail (!object || (num_attr > 0));
	g_return_if_fail (!num_attr || (labels && attributes));

	if (spat->table) {
		gtk_widget_destroy (spat->table);
		spat->table = NULL;
	}

	if (spat->attributes) {
		gint i;
		for (i = 0; i < spat->num_attr; i++) {
			g_free (spat->attributes[i]);
		}
		g_free (spat->attributes);
		spat->attributes = NULL;
	}

	if (spat->entries) {
		g_free (spat->entries);
		spat->entries = NULL;
	}

	if (spat->hasobj) {
		if (spat->src.object) {
			sp_signal_disconnect_by_data (spat->src.object, spat);
			spat->src.object = NULL;
		}
	} else {
		if (spat->src.repr) {
			spat->src.repr = sp_repr_unref (spat->src.repr);
		}
	}

	spat->hasobj = TRUE;

	if (object) {
		gint i;

		spat->blocked = TRUE;

		/* Set up object */
		spat->src.object = object;
		spat->num_attr = num_attr;
		g_signal_connect (G_OBJECT (object), "modified", G_CALLBACK (sp_attribute_table_object_modified), spat);
		g_signal_connect (G_OBJECT (object), "release", G_CALLBACK (sp_attribute_table_object_release), spat);
		/* Create table */
		spat->table = gtk_table_new (num_attr, 2, FALSE);
		gtk_container_add (GTK_CONTAINER (spat), spat->table);
		/* Arrays */
		spat->attributes = g_new0 (guchar *, num_attr);
		spat->entries = g_new0 (GtkWidget *, num_attr);
		/* Fill rows */
		for (i = 0; i < num_attr; i++) {
			GtkWidget *w;
			const guchar *val;

			spat->attributes[i] = g_strdup (attributes[i]);
			w = gtk_label_new (labels[i]);
			gtk_widget_show (w);
			gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
			gtk_table_attach (GTK_TABLE (spat->table), w, 0, 1, i, i + 1, GTK_FILL, GTK_EXPAND | GTK_FILL, XPAD, YPAD);
			w = gtk_entry_new ();
			gtk_widget_show (w);
			val = sp_repr_attr (SP_OBJECT_REPR (object), attributes[i]);
			gtk_entry_set_text (GTK_ENTRY (w), val ? val : (const guchar *) "");
			gtk_table_attach (GTK_TABLE (spat->table), w, 1, 2, i, i + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, XPAD, YPAD);
			spat->entries[i] = w;
			g_signal_connect (G_OBJECT (w), "changed", G_CALLBACK (sp_attribute_table_entry_changed), spat);
		}
		/* Show table */
		gtk_widget_show (spat->table);

		spat->blocked = FALSE;
	}

	gtk_widget_set_sensitive (GTK_WIDGET (spat), (spat->src.object != NULL));
}

void
sp_attribute_table_set_repr (SPAttributeTable *spat, SPRepr *repr, gint num_attr, const guchar **labels, const guchar **attributes)
{
	g_return_if_fail (spat != NULL);
	g_return_if_fail (SP_IS_ATTRIBUTE_TABLE (spat));
	g_return_if_fail (!num_attr || (labels && attributes));

	if (spat->table) {
		gtk_widget_destroy (spat->table);
		spat->table = NULL;
	}

	if (spat->attributes) {
		gint i;
		for (i = 0; i < spat->num_attr; i++) {
			g_free (spat->attributes[i]);
		}
		g_free (spat->attributes);
		spat->attributes = NULL;
	}

	if (spat->entries) {
		g_free (spat->entries);
		spat->entries = NULL;
	}

	if (spat->hasobj) {
		if (spat->src.object) {
			sp_signal_disconnect_by_data (spat->src.object, spat);
			spat->src.object = NULL;
		}
	} else {
		if (spat->src.repr) {
			spat->src.repr = sp_repr_unref (spat->src.repr);
		}
	}

	spat->hasobj = FALSE;

	if (repr) {
		gint i;

		spat->blocked = TRUE;

		/* Set up repr */
		spat->src.repr = sp_repr_ref (repr);
		spat->num_attr = num_attr;
		/* Create table */
		spat->table = gtk_table_new (num_attr, 2, FALSE);
		gtk_container_add (GTK_CONTAINER (spat), spat->table);
		/* Arrays */
		spat->attributes = g_new0 (guchar *, num_attr);
		spat->entries = g_new0 (GtkWidget *, num_attr);
		/* Fill rows */
		for (i = 0; i < num_attr; i++) {
			GtkWidget *w;
			const guchar *val;

			spat->attributes[i] = g_strdup (attributes[i]);
			w = gtk_label_new (labels[i]);
			gtk_widget_show (w);
			gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
			gtk_table_attach (GTK_TABLE (spat->table), w, 0, 1, i, i + 1, GTK_FILL, GTK_EXPAND | GTK_FILL, XPAD, YPAD);
			w = gtk_entry_new ();
			gtk_widget_show (w);
			val = sp_repr_attr (repr, attributes[i]);
			gtk_entry_set_text (GTK_ENTRY (w), val ? val : (const guchar *) "");
			gtk_table_attach (GTK_TABLE (spat->table), w, 1, 2, i, i + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, XPAD, YPAD);
			spat->entries[i] = w;
			g_signal_connect (G_OBJECT (w), "changed", G_CALLBACK (sp_attribute_table_entry_changed), spat);
		}
		/* Show table */
		gtk_widget_show (spat->table);

		spat->blocked = FALSE;
	}

	gtk_widget_set_sensitive (GTK_WIDGET (spat), (spat->src.repr != NULL));
}

static void
sp_attribute_table_object_modified (SPObject *object, guint flags, SPAttributeTable *spat)
{
	if (flags && SP_OBJECT_MODIFIED_FLAG) {
		gint i;
		for (i = 0; i < spat->num_attr; i++) {
			const guchar *val, *text;
			val = sp_repr_attr (SP_OBJECT_REPR (spat->src.object), spat->attributes[i]);
			text = gtk_entry_get_text (GTK_ENTRY (spat->entries[i]));
			if (val || text) {
				if (!val || !text || strcmp (val, text)) {
					/* We are different */
					spat->blocked = TRUE;
					gtk_entry_set_text (GTK_ENTRY (spat->entries[i]), val ? val : (const guchar *) "");
					spat->blocked = FALSE;
				}
			}
		}
	}
}

static void
sp_attribute_table_object_release (SPObject *object, SPAttributeTable *spat)
{
	sp_attribute_table_set_object (spat, NULL, 0, NULL, NULL);
}

static void
sp_attribute_table_entry_changed (GtkEditable *editable, SPAttributeTable *spat)
{
	if (!spat->blocked) {
		gint i;
		for (i = 0; i < spat->num_attr; i++) {
			if (GTK_WIDGET (editable) == spat->entries[i]) {
				const guchar *text;
				spat->blocked = TRUE;
				text = gtk_entry_get_text (GTK_ENTRY (spat->entries[i]));
				if (!*text) text = NULL;
				if (spat->hasobj && spat->src.object) {
					if (!sp_repr_set_attr (SP_OBJECT_REPR (spat->src.object), spat->attributes[i], text)) {
						/* Cannot set attribute */
						text = sp_repr_attr (SP_OBJECT_REPR (spat->src.object), spat->attributes[i]);
						gtk_entry_set_text (GTK_ENTRY (spat->entries[i]), text ? text : (const guchar *) "");
					}
					sp_document_done (SP_OBJECT_DOCUMENT (spat->src.object));
				} else if (spat->src.repr) {
					if (!sp_repr_set_attr (spat->src.repr, spat->attributes[i], text)) {
						/* Cannot set attribute */
						text = sp_repr_attr (spat->src.repr, spat->attributes[i]);
						gtk_entry_set_text (GTK_ENTRY (spat->entries[i]), text ? text : (const guchar *) "");
					}
					/* fixme: Warning! Undo will not be flushed in given case */
				}
				spat->blocked = FALSE;
				return;
			}
		}
		g_warning ("file %s: line %d: Entry signalled change, but there is no such entry", __FILE__, __LINE__);
	}
}
