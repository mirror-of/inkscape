/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "eps-out.h"
#include <print.h>
#include <extension/system.h>
#include <extension/db.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

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
	Inkscape::Extension::Extension * ext;

	ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS);
	if (ext == NULL)
		return;

	ext->get_param("pageBoundingBox", (bool *)&old_val);
	ext->set_param("pageBoundingBox", (bool)FALSE);

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
    sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>Encapsulated Postscript Output</name>\n"
			"<id>module.output.eps</id>\n"
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
