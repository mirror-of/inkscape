#define __TOOL_ATTRIBUTES_C__

/*
 * Untyped event context configuration
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <string.h>
#include <glib/gtypes.h>
#include <gtk/gtksignal.h>
#include <gtk/gtklabel.h>
#include "macros.h"
#include "helper/sp-intl.h"
#include "helper/window.h"

#include "forward.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "sp-attribute-widget.h"

#include "tool-attributes.h"

static void sp_tool_attributes_dialog_destroy (GtkObject *object, gpointer data);
static void sp_tool_attributes_dialog_set_eventcontext (Inkscape::Application *inkscape, SPEventContext *ec, gpointer data);

static void sp_tool_attributes_dialog_setup (SPEventContext *ec);

static GtkWidget *dlg = NULL;
static GtkWidget *tbl = NULL;

void
sp_tool_attributes_dialog (void)
{
	SPEventContext *ec;

	ec = SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP);

	if (!dlg) {
		dlg = sp_window_new (_("Tool attributes"), TRUE);
		g_signal_connect (G_OBJECT (dlg), "destroy", G_CALLBACK (sp_tool_attributes_dialog_destroy), NULL);

		sp_tool_attributes_dialog_setup (ec);

		g_signal_connect (G_OBJECT (INKSCAPE), "set_eventcontext", G_CALLBACK (sp_tool_attributes_dialog_set_eventcontext), dlg);

		gtk_widget_show (dlg);
	}
}

static void
sp_tool_attributes_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (INKSCAPE, dlg);

	dlg = NULL;
	tbl = NULL;
}

static void
sp_tool_attributes_dialog_set_eventcontext (Inkscape::Application *inkscape, SPEventContext *ec, gpointer data)
{
	g_assert (dlg != NULL);
	g_assert (tbl != NULL);

	sp_tool_attributes_dialog_setup (ec);
}

static void
sp_tool_attributes_dialog_setup (SPEventContext *ec)
{
	g_assert (dlg != NULL);

	if (tbl) {
		gtk_object_destroy (GTK_OBJECT (tbl));
	}

	if (ec) {
		const gchar *type_name;
		type_name = gtk_type_name (GTK_OBJECT_TYPE (ec));
		if (!strcmp (type_name, "SPSpiralContext")) {
			SPRepr *repr;
			repr = inkscape_get_repr (INKSCAPE, "tools.shapes.spiral");
			if (repr) {
				gchar *labels[] = {N_("Revolution:"), N_("Expansion"), N_("Inner radius")};
				gchar *attrs[] = {"revolution", "expansion", "t0"};
				tbl = sp_attribute_table_new_repr (repr, 3, (const gchar **) labels, (const gchar **) attrs);
			} else {
				tbl = gtk_label_new (_("Missing tool preferences"));
			}
		} else if (!strcmp (type_name, "SPStarContext")) {
			SPRepr *repr;
			repr = inkscape_get_repr (INKSCAPE, "tools.shapes.star");
			if (repr) {
				gchar *labels[] = {N_("Corners:"), N_("Proportion")};
				gchar *attrs[] = {"magnitude", "proportion"};
				tbl = sp_attribute_table_new_repr (repr, 2, (const gchar **) labels, (const gchar **) attrs);
			} else {
				tbl = gtk_label_new (_("Missing tool preferences"));
			}
		} else if (!strcmp (type_name, "SPSelectContext")) {
			SPRepr *repr;
			repr = inkscape_get_repr (INKSCAPE, "tools.select");
			if (repr) {
				gchar *labels[] = {N_("Show:"), N_("Transform:")};
				gchar *attrs[] = {"show", "transform"};
				tbl = sp_attribute_table_new_repr (repr, 2, (const gchar **) labels, (const gchar **) attrs);
			} else {
				tbl = gtk_label_new (_("Missing tool preferences"));
			}
		} else if (!strcmp (type_name, "SPDynaDrawContext")) {
			SPRepr *repr;
			repr = inkscape_get_repr (INKSCAPE, "tools.calligraphic");
			if (repr) {
				gchar *labels[] = {N_("Mass:"), N_("Drag:"), N_("Angle"), N_("Width:")};
				gchar *attrs[] = {"mass", "drag", "angle", "width"};
				tbl = sp_attribute_table_new_repr (repr, 4, (const gchar **) labels, (const gchar **) attrs);
			} else {
				tbl = gtk_label_new (_("Missing tool preferences"));
			}
		} else {
			tbl = gtk_label_new (_("Tool has no attributes"));
		}
	} else {
		tbl = gtk_label_new (_("No active tool"));
	}

	gtk_widget_show (tbl);

	gtk_container_add (GTK_CONTAINER (dlg), tbl);
}

