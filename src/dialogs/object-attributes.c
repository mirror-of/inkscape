#define __SP_OBJECT_ATTRIBUTES_C__

/*
 * Generic properties editor
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "macros.h"
#include "sp-anchor.h"
#include "sp-attribute-widget.h"
#include "object-attributes.h"

typedef struct {
	guchar *label;
	guchar *attribute;
} SPAttrDesc;

static const SPAttrDesc anchor_desc[] = {
	{ N_("Href:"), "xlink:href"},
	{ N_("Target:"), "target"},
	{ N_("Type:"), "xlink:type"},
	{ N_("Role:"), "xlink:role"},
	{ N_("Arcrole:"), "xlink:arcrole"},
	{ N_("Title:"), "xlink:title"},
	{ N_("Show:"), "xlink:show"},
	{ N_("Actuate:"), "xlink:actuate"},
	{ NULL, NULL}
};

static const SPAttrDesc star_desc[] = {
	{ N_("Sides:"), "sodipodi:sides"},
	{ N_("Center X:"), "sodipodi:cx"},
	{ N_("Center Y:"), "sodipodi:cy"},
	{ N_("R1:"), "sodipodi:r1"},
	{ N_("R2:"), "sodipodi:r2"},
	{ N_("ARG1:"), "sodipodi:arg1"},
	{ N_("ARG2:"), "sodipodi:arg2"},
	{ NULL, NULL}
};

static const SPAttrDesc spiral_desc[] = {
	{ N_("Center X:"), "sodipodi:cx"},
	{ N_("Center Y:"), "sodipodi:cy"},
	{ N_("Expansion:"), "sodipodi:expansion"},
	{ N_("Revolution:"), "sodipodi:revolution"},
	{ N_("Radius:"), "sodipodi:radius"},
	{ N_("Argument:"), "sodipodi:argument"},
	{ N_("T0:"), "sodipodi:t0"},
	{ NULL, NULL}
};

static const SPAttrDesc image_desc[] = {
	{ N_("URL:"), "xlink:href"},
	{ N_("X:"), "x"},
	{ N_("Y:"), "y"},
	{ N_("Width:"), "width"},
	{ N_("Height:"), "height"},
	{ NULL, NULL}
};

static const SPAttrDesc rect_desc[] = {
	{ N_("X:"), "x"},
	{ N_("Y:"), "y"},
	{ N_("Width:"), "width"},
	{ N_("Height:"), "height"},
	{ N_("RX:"), "rx"},
	{ N_("RY:"), "ry"},
	{ NULL, NULL}
};

static void
object_released (GtkObject *object, GtkWidget *widget)
{
	gtk_widget_destroy (widget);
}

static void
window_destroyed (GtkObject *window, GtkObject *object)
{
	sp_signal_disconnect_by_data (object, window);
}

static void
sp_object_attr_show_dialog (SPObject *object, const SPAttrDesc *desc, const guchar *tag)
{
	const guchar **labels, **attrs;
	gint len, i;
	gchar *title;
	GtkWidget *w, *t;

	len = 0;
	while (desc[len].label) len += 1;

	labels = alloca (len * sizeof (char *));
	attrs = alloca (len * sizeof (char *));

	for (i = 0; i < len; i++) {
		labels[i] = desc[i].label;
		attrs[i] = desc[i].attribute;
	}

	title = g_strdup_printf (_("%s attributes"), tag);
	w = sp_window_new (title, TRUE);
	g_free (title);

	t = sp_attribute_table_new (object, len, labels, attrs);
	gtk_widget_show (t);
	gtk_container_add (GTK_CONTAINER (w), t);

	g_signal_connect (G_OBJECT (w), "destroy", G_CALLBACK (window_destroyed), object);
	g_signal_connect (G_OBJECT (object), "release", G_CALLBACK (object_released), w);

	gtk_widget_show (w);
}

void
sp_object_attributes_dialog (SPObject *object, const guchar *tag)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_OBJECT (object));
	g_return_if_fail (tag != NULL);

	if (!strcmp (tag, "SPAnchor")) {
		sp_object_attr_show_dialog (object, anchor_desc, tag);
	} else if (!strcmp (tag, "SPStar")) {
		sp_object_attr_show_dialog (object, star_desc, tag);
	} else if (!strcmp (tag, "SPSpiral")) {
		sp_object_attr_show_dialog (object, spiral_desc, tag);
	} else if (!strcmp (tag, "SPImage")) {
		sp_object_attr_show_dialog (object, image_desc, tag);
	} else if (!strcmp (tag, "SPRect")) {
		sp_object_attr_show_dialog (object, rect_desc, tag);
	}

}

