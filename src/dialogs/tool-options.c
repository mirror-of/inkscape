#define __TOOL_OPTIONS_C__

/*
 * Event context configuration
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
#include <gtk/gtksignal.h>
#include <gtk/gtklabel.h>
#include "macros.h"
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "forward.h"
#include "sodipodi.h"
#include "desktop-handles.h"
#include "event-context.h"

#include "tool-options.h"

static void sp_tool_options_dialog_destroy (GtkObject *object, gpointer data);
static void sp_tool_options_dialog_set_eventcontext (Sodipodi *sodipodi, SPEventContext *ec, gpointer data);

static void sp_tool_options_dialog_setup (SPEventContext *ec);

static GtkWidget *dlg = NULL;
static GtkWidget *tbl = NULL;

void
sp_tool_options_dialog (void)
{
	SPEventContext *ec;

	ec = SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP);

	if (!dlg) {
		dlg = sp_window_new (_("Tool options"), TRUE);
		g_signal_connect (G_OBJECT (dlg), "destroy", G_CALLBACK (sp_tool_options_dialog_destroy), NULL);

		sp_tool_options_dialog_setup (ec);

		g_signal_connect (G_OBJECT (SODIPODI), "set_eventcontext", G_CALLBACK (sp_tool_options_dialog_set_eventcontext), dlg);

		gtk_widget_show (dlg);
	}
}

static void
sp_tool_options_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (SODIPODI, dlg);

	dlg = NULL;
	tbl = NULL;
}

static void
sp_tool_options_dialog_set_eventcontext (Sodipodi *sodipodi, SPEventContext *ec, gpointer data)
{
	g_assert (dlg != NULL);
	g_assert (tbl != NULL);

	sp_tool_options_dialog_setup (ec);
}

static void
sp_tool_options_dialog_setup (SPEventContext *ec)
{
	g_assert (dlg != NULL);

	if (tbl) {
		gtk_object_destroy (GTK_OBJECT (tbl));
	}

	if (ec) {
		tbl = sp_event_context_config_widget (ec);
		if (!tbl) {
			tbl = gtk_label_new (_("Tool has no options"));
		}
	} else {
		tbl = gtk_label_new (_("No active tool"));
	}

	gtk_widget_show (tbl);

	gtk_container_add (GTK_CONTAINER (dlg), tbl);
}

