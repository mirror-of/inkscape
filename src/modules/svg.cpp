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
#include <module.h>
#include "svg.h"
#include "system.h"

/* Prototypes */
static void svg_save (SPModule *mod, SPDocument *doc, const gchar *uri);
static SPDocument * svg_open (SPModule *mod, const gchar *uri);

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
svg_init(void)
{
	SPModuleInput * imod;
	SPModuleOutput * omod;

	/* SVG in */
    imod = SP_MODULE_INPUT(sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>SVG Input</name>\n"
			"<id>" SP_MODULE_KEY_INPUT_SVG "</id>\n"
			"<input>\n"
				"<extension>.svg</extension>\n"
				"<mimetype>image/x-svg</mimetype>\n"
				"<filetypename>Scalable Vector Graphic (SVG)</filetypename>\n"
				"<filetypetooltip>Inkscape native file format and W3C standard</filetypetooltip>\n"
			"</input>\n"
		"</spmodule>"));
	g_return_if_fail(imod != NULL);
	imod->open = svg_open;

	/* SVG out Inkscape*/
    omod = SP_MODULE_OUTPUT(sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>SVG Output Inkscape</name>\n"
			"<id>" SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "</id>\n"
			"<output>\n"
				"<extension>.svg</extension>\n"
				"<mimetype>image/x-svg</mimetype>\n"
				"<filetypename>SVG with extension namespaces</filetypename>\n"
				"<filetypetooltip>Scalable Vector Graphics format with Inkscape extensions</filetypetooltip>\n"
			"</output>\n"
		"</spmodule>"));
	g_return_if_fail(omod != NULL);
	omod->save = svg_save;

	/* SVG out */
    omod = SP_MODULE_OUTPUT(sp_module_system_build_from_mem(
		"<spmodule>\n"
			"<name>SVG Output</name>\n"
			"<id>" SP_MODULE_KEY_OUTPUT_SVG "</id>\n"
			"<output>\n"
				"<extension>.w3c.svg</extension>\n"
				"<mimetype>image/x-svg</mimetype>\n"
				"<filetypename>Plain SVG</filetypename>\n"
				"<filetypetooltip>Scalable Vector Graphics format</filetypetooltip>\n"
			"</output>\n"
		"</spmodule>"));
	g_return_if_fail(omod != NULL);
	omod->save = svg_save;

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
static SPDocument *
svg_open (SPModule *mod, const gchar *uri)
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

	This function first checks its parameters, and makes sure that
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
static void
svg_save (SPModule *mod, SPDocument *doc, const gchar *uri)
{
	SPRepr *repr;
	gboolean spns;
	const GSList *images, *l;
	SPReprDoc *rdoc;
	const gchar *save_path;

	g_return_if_fail(doc != NULL);
	g_return_if_fail(uri != NULL);

	save_path = g_dirname (uri);

	spns = (!SP_MODULE_ID (mod) || !strcmp (SP_MODULE_ID (mod), SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE));
	if (spns) {
		rdoc = NULL;
		repr = sp_document_repr_root (doc);
		sp_repr_set_attr (repr, "sodipodi:docbase", save_path);
		sp_repr_set_attr (repr, "sodipodi:docname", uri);
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

	/* TODO: */
	sp_repr_save_file (sp_repr_document (repr), uri);
	sp_document_set_uri (doc, uri);

	if (!spns) sp_repr_document_unref (rdoc);
}

