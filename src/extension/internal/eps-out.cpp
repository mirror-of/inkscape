/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "eps-out.h"
#include <gtk/gtk.h>
#include <print.h>
#include <helper/sp-intl.h>
#include <extension/system.h>
#include <extension/db.h>
#include <dialogs/dialog-events.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

EpsOutput::EpsOutput (void)
{
	dialog = NULL;
	return;
}

EpsOutput::~EpsOutput (void)
{
	if (dialog != NULL)
		gtk_widget_destroy(GTK_WIDGET(dialog));
	return;
}

void
EpsOutput::pageBoxToggle (GtkWidget * widget, Inkscape::Extension::Output * omod)
{
	omod->set_param("pageBoundingBox", (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	return;
}

GtkDialog *
EpsOutput::prefs (Inkscape::Extension::Output * module)
{
	GtkWidget * checkbox;
	bool pageBox;

	if (dialog != NULL)
		return dialog;

	dialog = GTK_DIALOG(
		     gtk_dialog_new_with_buttons (_("EPS Output Settings"),
			                              NULL,
										  (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										  GTK_STOCK_CANCEL,
										  GTK_RESPONSE_CANCEL,
										  GTK_STOCK_OK,
										  GTK_RESPONSE_OK,
										  NULL));
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	sp_transientize(GTK_WIDGET(dialog));

	checkbox = gtk_check_button_new_with_label(_("Make bounding box around full page"));
	module->get_param("pageBoundingBox", &pageBox);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), pageBox);
	g_signal_connect(G_OBJECT(checkbox), "toggled", G_CALLBACK(pageBoxToggle), (gpointer)module);
	gtk_widget_show(checkbox);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), checkbox, FALSE, FALSE, 5);

	return dialog;
}

/**
    \brief  This function calls the print system with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .ps)

	The most interesting thing that this function does is just attach
	an '>' on the front of the filename.  This is the syntax used to
	tell the printing system to save to file.
*/
void
EpsOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
	gchar * final_name;
	bool old_val;
	bool new_val;
	Inkscape::Extension::Extension * ext;

	ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS);
	if (ext == NULL)
		return;

	ext->get_param("pageBoundingBox", (bool *)&old_val);
	mod->get_param("pageBoundingBox", (bool *)&new_val);
	ext->set_param("pageBoundingBox", (bool)new_val);

	final_name = g_strdup_printf("> %s", uri);
	sp_print_document_to_file(doc, final_name);
	g_free(final_name);

	ext->set_param("pageBoundingBox", (bool)old_val);

	return;
}

/**
	\brief   A function allocate a copy of this function.

	This is the definition of postscript out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
EpsOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<spmodule>\n"
			"<name>Encapsulated Postscript Output</name>\n"
			"<id>module.output.eps</id>\n"
			"<param name=\"pageBoundingBox\" type=\"boolean\">FALSE</param>\n"
			"<output>\n"
				"<extension>.eps</extension>\n"
				"<mimetype>image/x-e-postscript</mimetype>\n"
				"<filetypename>Encapsulated Postscript (*.eps)</filetypename>\n"
				"<filetypetooltip>Encapsulated Postscript File</filetypetooltip>\n"
			"</output>\n"
		"</spmodule>", new EpsOutput());

	return;
}

};};}; /* namespace Inkscape, Extension, Implementation */
