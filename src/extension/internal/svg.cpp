/*
 * This is the code that moves all of the SVG loading and saving into
 * the module format.  Really Inkscape is built to handle these formats
 * internally, so this is just calling those internal functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <helper/sp-intl.h>
#include <xml/repr.h>
#include <sp-object.h>
#include <document.h>
#include <dir-util.h>
#include "../implementation/implementation.h"
#include "svg.h"
#include <extension/system.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
	\return   None
	\brief    What would an SVG editor be without loading/saving SVG
	          files.  This function sets that up.

	For each module there is a call to sp_module_system_build_from_mem
	with a rather large XML file passed in.  This is a constant string
	that describes the module.  At the end of this call a module is
	returned that is basically filled out.  The one thing that it doesn't
	have is the key function for the operation.  And that is linked at
	the end of each call.
*/
void
Svg::init(void)
{
	Inkscape::Extension::Extension * ext;
	
	/* SVG in */
    ext = sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>SVG Input</name>\n"
			"<id>" SP_MODULE_KEY_INPUT_SVG "</id>\n"
			"<input>\n"
				"<extension>.svg</extension>\n"
				"<mimetype>image/x-svg</mimetype>\n"
				"<filetypename>Scalable Vector Graphic (SVG)</filetypename>\n"
				"<filetypetooltip>Inkscape native file format and W3C standard</filetypetooltip>\n"
			"</input>\n"
		"</spmodule>", new Svg());

	/* SVG out Inkscape*/
    ext = sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>SVG Output Inkscape</name>\n"
			"<id>" SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "</id>\n"
			"<output>\n"
				"<extension>.svg</extension>\n"
				"<mimetype>image/x-svg</mimetype>\n"
				"<filetypename>Inkscape SVG (*.svg)</filetypename>\n"
				"<filetypetooltip>SVG format with Inkscape extensions</filetypetooltip>\n"
			"</output>\n"
		"</spmodule>", new Svg());

	/* SVG out */
    ext = sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>SVG Output</name>\n"
			"<id>" SP_MODULE_KEY_OUTPUT_SVG "</id>\n"
			"<output>\n"
				"<extension>.svg</extension>\n"
				"<mimetype>image/x-svg</mimetype>\n"
				"<filetypename>Plain SVG (*.svg)</filetypename>\n"
				"<filetypetooltip>Scalable Vector Graphics format as defined by the W3C</filetypetooltip>\n"
			"</output>\n"
		"</spmodule>", new Svg());

	return;
}

/**
	\return    A new document just for you!
	\brief     This function takes in a filename of a SVG document and
	           turns it into a SPDocument.
	\param     mod   Module to use
	\param     uri   The path to the file

	This function is really simple, it just calles sp_document_new...
*/
SPDocument *
Svg::open (Inkscape::Extension::Input *mod, const gchar *uri)
{
	return sp_document_new (uri, TRUE, TRUE);
}

/**
	\return    None
	\brief     This is the function that does all of the SVG saves in
	           Inkscape.  It detects whether it should do a Inkscape
			   namespace save internally.
	\param     mod   Extension to use.
	\param     doc   Document to save.
	\param     uri   The filename to save the file to.

	This function first checks it's parameters, and makes sure that
	we're getting good data.  It also checks the module ID of the
	incoming module to figure out if this is save should include
	the Inkscape namespace stuff or not.  The result of that comparison
	is stored in the spns variable.

	If there is not to be Inkscape name spaces a new document is created
	without.  (I think, I'm not sure on this code)

	All of the internally referenced imageins are also set to relative
	paths in the file.  And the file is saved.

	This really needs to be fleshed out more, but I don't quite understand
	all of this code.  I just stole it.
*/
void
Svg::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
	SPRepr *repr;
	gboolean spns;
	const GSList *images, *l;
	SPReprDoc *rdoc;
	const gchar *save_path;

	g_return_if_fail(doc != NULL);
	g_return_if_fail(uri != NULL);

	save_path = g_dirname (uri);

	spns = (!mod->get_id() || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE));
	if (spns) {
		rdoc = NULL;
		repr = sp_document_repr_root (doc);
	} else {
		rdoc = sp_repr_document_new ("svg");
		repr = sp_repr_document_root (rdoc);
		repr = sp_object_invoke_write (sp_document_root (doc), repr, SP_OBJECT_WRITE_BUILD);
	}

	images = sp_document_get_resource_list (doc, "image");
	for (l = images; l != NULL; l = l->next) {
		SPRepr *ir;
		const gchar *href, *relname;
		ir = SP_OBJECT_REPR (l->data);
		href = sp_repr_attr (ir, "xlink:href");
		if (spns && !g_path_is_absolute (href)) {
			href = sp_repr_attr (ir, "sodipodi:absref");
		}
		if (href && g_path_is_absolute (href)) {
			relname = sp_relative_path_from_path (href, save_path);
			sp_repr_set_attr (ir, "xlink:href", relname);
		}
	}

	sp_repr_save_file (sp_repr_document (repr), uri);

	if (!spns) sp_repr_document_unref (rdoc);

	return;
}

};};}; /* namespace inkscape, module, implementation */
