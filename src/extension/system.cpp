/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include <interface.h>
#include <helper/sp-intl.h>
#include <xml/repr.h>
#include <document.h>
#include <inkscape.h>
#include <desktop-handles.h>

#include <extension.h>

#include "system.h"
#include "db.h"
#include "implementation/script.h"

static void open_internal (Inkscape::Extension::Extension * in_plug, gpointer in_data);
static void save_internal (Inkscape::Extension::Extension * in_plug, gpointer in_data);
static Inkscape::Extension::Extension * build_from_reprdoc (SPReprDoc * doc, Inkscape::Extension::Implementation::Implementation * in_imp);

/**
	\return   A new document created from the filename passed in
	\brief    This is a generic function to use the open function of
	          a module (including Autodetect)
	\param    key       Identifier of which module to use
	\param    filename  The file that should be opened

	First things first, are we looking at an autodetection?  Well if
	that's the case then the module needs to be found, and that is done
	with a database lookup through the module DB.  The foreach function
	is called, with the parameter being a gpointer array.  It contains
	both the filename (to find its extension) and where to write the
	module when it is found.

	If there is no autodetection, then the module database is queried with
	the key given.

	If everything is cool at this point, the module is loaded, and
	there is possibility for preferences.  If there is a function, then
	it is executed to get the dialog to be displayed.  After it is finished
	the function continues.

	Lastly, the open function is called in the module itself.
*/
SPDocument *
sp_module_system_open (Inkscape::Extension::Extension * key, const gchar * filename)
{
	gpointer parray[2];
	Inkscape::Extension::Input * imod = NULL;
	GtkDialog * prefs = NULL;
	SPDocument * doc;
	SPRepr * repr;
	bool last_chance_svg = FALSE;

	if (key == NULL) {
		parray[0] = (gpointer)filename;
		parray[1] = (gpointer)&imod;
		Inkscape::Extension::db.foreach(open_internal, (gpointer)&parray);
	} else {
		imod = dynamic_cast<Inkscape::Extension::Input *>(key);
	}

	if (key == NULL && imod == NULL) {
		last_chance_svg = TRUE;
		imod = dynamic_cast<Inkscape::Extension::Input *>(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG));
	}

	if (imod == NULL) {
		throw Inkscape::Extension::Input::no_extension_found();
	}

	imod->set_state(Inkscape::Extension::Extension::STATE_LOADED);

	if (!imod->loaded()) {
		throw Inkscape::Extension::Input::open_failed();
	}

	prefs = imod->prefs(filename);
	if (prefs != NULL) {
		gtk_dialog_run(prefs);
	}

	doc = imod->open(filename);

	if(!doc)
		return NULL;

	if (last_chance_svg == TRUE) {
		sp_ui_error_dialog(_("Format autodetect failed. The file is being opened as SVG."));
	}
	
	/* This kinda overkill as most of these are already set, but I want
	   to make sure for this release -- TJG */
	repr = sp_document_repr_root (doc);
	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_attr (repr, "sodipodi:modified", NULL);
	sp_document_set_undo_sensitive (doc, TRUE);

	sp_document_set_uri (doc, filename);

	return doc;
}

/**
	\return   none
	\brief    This is the function that searches each module to see
	          if it matches the filename for autodetection.
	\param    in_plug  The module to be tested
	\param    in_data  An array of pointers containing the filename, and
	                   the place to put a sucessfully found module.

	Basically this function only looks at input modules as it is part
	of the open function.  If the module is an input module, it then
	starts to take it appart, and the data that is passed in.  Because
	the data being passed in is in such a weird format, there are a few
	casts to make it easier to use.  While it looks like alot of local
	variables, they'll all get removed by the compiler.

	First thing that is checked is if the filename is shorter than the
	extension itself.  There is no way for a match in that case.  If
	it's long enough then there is a string compare of the end of the
	filename (for the length of the extension), and the extension itself.
	If this passes then the pointer passed in is set to the current
	module.
*/
static void
open_internal (Inkscape::Extension::Extension * in_plug, gpointer in_data)
{
	if (dynamic_cast<Inkscape::Extension::Input *>(in_plug)) {
		const gchar * ext;
		gpointer * parray;
		const gchar * filename;
		Inkscape::Extension::Input ** pimod;
		gchar * filenamelower, * extensionlower;

		parray = (gpointer *)in_data;
		filename = (const gchar *)parray[0];
		pimod = (Inkscape::Extension::Input **)parray[1];

		ext = dynamic_cast<Inkscape::Extension::Input *>(in_plug)->get_extension();

		filenamelower = g_utf8_strdown (filename, -1);
		extensionlower = g_utf8_strdown (ext, -1);

		if (g_str_has_suffix(filenamelower, extensionlower)) {
			*pimod = dynamic_cast<Inkscape::Extension::Input *>(in_plug);
		}

		g_free(filenamelower);
		g_free(extensionlower);
	}

	return;
}

/**
	\return   None
	\brief    This is a generic function to use the save function of
	          a module (including Autodetect)
	\param    key       Identifier of which module to use
	\param    doc       The document to be saved
	\param    filename  The file that the document should be saved to
      \param official  (optional) whether to set :output_module and :modified in the 
                        document; is true for normal save, false for temporary saves

	First things first, are we looking at an autodetection?  Well if
	that's the case then the module needs to be found, and that is done
	with a database lookup through the module DB.  The foreach function
	is called, with the parameter being a gpointer array.  It contains
	both the filename (to find its extension) and where to write the
	module when it is found.

	If there is no autodetection the module database is queried with
	the key given.

	If everything is cool at this point, the module is loaded, and
	there is possibility for preferences.  If there is a function, then
	it is executed to get the dialog to be displayed.  After it is finished
	the function continues.

	Lastly, the save function is called in the module itself.
*/
void
sp_module_system_save (Inkscape::Extension::Extension * key, SPDocument * doc, const gchar * filename, bool setextension, bool check_overwrite, bool official)
{
	Inkscape::Extension::Output * omod;
	gpointer parray[2];
	GtkDialog * prefs;
	gchar * fileName = NULL;

	if (key == NULL) {
		parray[0] = (gpointer)filename;
		parray[1] = (gpointer)&omod;
		omod = NULL;
		Inkscape::Extension::db.foreach(save_internal, (gpointer)&parray);

		/* This is a nasty hack, but it is required to ensure that
		   autodetect will always save with the Inkscape extensions
		   if they are available. */
		if (omod != NULL && !strcmp(omod->get_id(), SP_MODULE_KEY_OUTPUT_SVG)) {
			omod = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE));
		}
		/* If autodetect fails, save as Inkscape SVG */
		if (omod == NULL) {
			omod = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE));
		}
	} else {
		omod = dynamic_cast<Inkscape::Extension::Output *>(key);
	}

	if (!dynamic_cast<Inkscape::Extension::Output *>(omod)) {
		g_warning ("Unable to find output module to handle file: %s\n", filename);
		throw Inkscape::Extension::Output::no_extension_found();
		return;
	}

	omod->set_state(Inkscape::Extension::Extension::STATE_LOADED);
	if (!omod->loaded()) {
		throw Inkscape::Extension::Output::save_failed();
	}

	prefs = omod->prefs();
	if (prefs != NULL) {
		gtk_dialog_run(prefs);
	}

	if (setextension) {
		gchar * lowerfile = g_utf8_strdown (filename, -1);
		gchar * lowerext = g_utf8_strdown (omod->get_extension(), -1);

		if (!g_str_has_suffix(lowerfile, lowerext)) {
			fileName = g_strdup_printf("%s%s", filename, omod->get_extension());
		}

		g_free(lowerfile);
		g_free(lowerext);
	} 

	if (fileName == NULL) {
		fileName = g_strdup(filename);
	}

	if (check_overwrite && !sp_ui_overwrite_file(fileName)) {
		g_free(fileName);
		throw Inkscape::Extension::Output::no_overwrite();
	}

	if (official)
		sp_document_set_uri (doc, fileName);

	omod->save(doc, fileName);

	g_free(fileName);
	return;
}

/**
	\return   none
	\brief    This is the function that searches each module to see
	          if it matches the filename for autodetection.
	\param    in_plug  The module to be tested
	\param    in_data  An array of pointers containing the filename, and
	                   the place to put a sucessfully found module.

	Basically this function only looks at output modules as it is part
	of the open function.  If the module is an output module, it then
	starts to take it appart, and the data that is passed in.  Because
	the data being passed in is in such a weird format, there are a few
	casts to make it easier to use.  While it looks like alot of local
	variables, they'll all get removed by the compiler.

	First thing that is checked is if the filename is shorter than the
	extension itself.  There is no way for a match in that case.  If
	it's long enough then there is a string compare of the end of the
	filename (for the length of the extension), and the extension itself.
	If this passes then the pointer passed in is set to the current
	module.
*/
static void
save_internal (Inkscape::Extension::Extension * in_plug, gpointer in_data)
{
	if (dynamic_cast<Inkscape::Extension::Output *>(in_plug)) {
		const gchar * ext;
		gpointer * parray;
		const gchar * filename;
		Inkscape::Extension::Output ** pomod;
		gchar * filenamelower, * extensionlower;

		parray = (gpointer *)in_data;
		filename = (const gchar *)parray[0];
		pomod = (Inkscape::Extension::Output **)parray[1];

		ext = dynamic_cast<Inkscape::Extension::Output *>(in_plug)->get_extension();

		filenamelower = g_utf8_strdown (filename, -1);
		extensionlower = g_utf8_strdown (ext, -1);

		if (g_str_has_suffix(filenamelower, extensionlower)) {
			*pomod = dynamic_cast<Inkscape::Extension::Output *>(in_plug);
		}

		g_free(filenamelower);
		g_free(extensionlower);
	}

	return;
}

/**
	\return   None
	\brief    A function that can be attached to a menu item to
	          execute a effect.
	\param    object   unused (for prototype matching)
	\param    key      Key of effect to be used

	This function just looks up the effect from the module database
	and then makes sure it is loaded.  After that, it calls the effect
	function for the module.  Really, it is more of a generic wrapper
	function.
*/
void
sp_module_system_filter (GtkObject * object, const gchar * key)
{
	Inkscape::Extension::Effect * fmod;
	SPDocument * doc;

	g_return_if_fail(key != NULL);

	fmod = dynamic_cast<Inkscape::Extension::Effect *>(Inkscape::Extension::db.get(key));
	g_return_if_fail(fmod != NULL);

	fmod->set_state(Inkscape::Extension::Extension::STATE_LOADED);
	g_return_if_fail(fmod->loaded());

	doc = SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP);
	g_return_if_fail(doc != NULL);

	return fmod->effect(doc);
}

Inkscape::Extension::Print *
sp_module_system_get_print (const gchar * key)
{
	return dynamic_cast<Inkscape::Extension::Print *>(Inkscape::Extension::db.get(key));
}
/**
	\return   The built module
	\brief    Creates a module from a SPReprDoc describing the module
	\param    doc  The XML description of the module

	This function basically has two segments.  The first is that it goes
	through the Repr tree provided, and determines what kind of of
	module this is, and what kind of implementation to use.  All of
	these are then stored in two enums that are defined in this function.
	This makes it easier to add additional types (which will happen in
	the future, I'm sure).

	Second, there is case statements for these enums.  The first one
	is the type of module.  This is the one where the module is
	actually created.  After that, then the implmentation is applied
	to get the load and unload functions.  If there is no implmentation
	these are not set.  This case could apply to modules that are
	built in (like the SVG load/save functions).
*/
static Inkscape::Extension::Extension *
build_from_reprdoc (SPReprDoc * doc, Inkscape::Extension::Implementation::Implementation * in_imp)
{
	SPRepr * repr;
	Inkscape::Extension::Extension * module = NULL;
	Inkscape::Extension::Implementation::Implementation * imp;
	enum {
		MODULE_EXTENSION,
		MODULE_UNKNOWN_IMP
	} module_implementation_type = MODULE_UNKNOWN_IMP;
	enum {
		MODULE_INPUT,
		MODULE_OUTPUT,
		MODULE_FILTER,
		MODULE_PRINT,
		MODULE_UNKNOWN_FUNC
	} module_functional_type = MODULE_UNKNOWN_FUNC;
	SPRepr * old_repr;
	SPRepr * child_repr;

	g_return_val_if_fail(doc != NULL, NULL);

	repr = sp_repr_document_root(doc);

	/* sp_repr_print(repr); */

	if (strcmp(sp_repr_name(repr), "spmodule")) {
		printf("How come I don't have a spmodule?\n");
		return NULL;
	}

	child_repr = sp_repr_children(repr);
	while (child_repr != NULL) {
		/* printf("Child: %s\n", sp_repr_name(child_repr)); */
		if (!strcmp(sp_repr_name(child_repr), "input")) {
			module_functional_type = MODULE_INPUT;
		}
		if (!strcmp(sp_repr_name(child_repr), "output")) {
			module_functional_type = MODULE_OUTPUT;
		}
		if (!strcmp(sp_repr_name(child_repr), "effect")) {
			module_functional_type = MODULE_FILTER;
		}
		if (!strcmp(sp_repr_name(child_repr), "print")) {
			module_functional_type = MODULE_PRINT;
		}
		if (!strcmp(sp_repr_name(child_repr), "extension")) {
			module_implementation_type = MODULE_EXTENSION;
		}

		old_repr = child_repr;
		child_repr = sp_repr_next(child_repr);
		/* sp_repr_unref(old_repr); */
	}

	if (in_imp == NULL) {
		switch (module_implementation_type) {
			case MODULE_EXTENSION:
				Inkscape::Extension::Implementation::Script * script;

				script = new Inkscape::Extension::Implementation::Script();
				imp = dynamic_cast<Inkscape::Extension::Implementation::Implementation *>(script);
				break;
			default:
				imp = NULL;
				break;
		}
	} else {
		imp = in_imp;
	}
	
	switch (module_functional_type)
	{
		case MODULE_INPUT:
			{
				module = new Inkscape::Extension::Input(repr, imp);
				break;
			}
		case MODULE_OUTPUT:
			{
				module = new Inkscape::Extension::Output(repr, imp);
				break;
			}
		case MODULE_FILTER:
			{
				module = new Inkscape::Extension::Effect(repr, imp);
				break;
			}
		case MODULE_PRINT:
			{
				module = new Inkscape::Extension::Print(repr, imp);
				break;
			}
		default:
			break;
	}


	return module;
}

/**
	\return   The module created
	\brief    This function creates a module from a filename of an
	          XML description.
	\param    filename  The file holding the XML description of the module.

	This function calls build_from_reprdoc with using sp_repr_read_file
	to create the reprdoc.
*/
Inkscape::Extension::Extension *
sp_module_system_build_from_file (const gchar * filename, Inkscape::Extension::Implementation::Implementation * in_imp)
{
	SPReprDoc * doc;
	Inkscape::Extension::Extension * ext;

	/* TODO: Need to define namespace here, need to write the
	         DTD in general for this stuff */
	doc = sp_repr_read_file(filename, NULL);
	ext = build_from_reprdoc (doc, in_imp);
	sp_repr_document_unref(doc);
	return ext;
}

/**
	\return   The module created
	\brief    This function creates a module from a buffer holding an 
	          XML description.
	\param    buffer  The buffer holding the XML description of the module.

	This function calls build_from_reprdoc with using sp_repr_read_mem
	to create the reprdoc.  It finds the length of the buffer using strlen.
*/
Inkscape::Extension::Extension *
sp_module_system_build_from_mem (const gchar * buffer, Inkscape::Extension::Implementation::Implementation * in_imp)
{
	SPReprDoc * doc;
	Inkscape::Extension::Extension * ext;

	doc = sp_repr_read_mem(buffer, strlen(buffer), NULL);
	ext = build_from_reprdoc (doc, in_imp);
	sp_repr_document_unref(doc);
	return ext;
}
