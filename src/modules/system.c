/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include <xml/repr.h>
#include <document.h>
#include <inkscape.h>
#include <desktop-handles.h>

#include <module.h>

#include "system.h"
#include "db.h"
#include "extension.h"

static void open_internal (SPModule * in_plug, gpointer in_data);
static void save_internal (SPModule * in_plug, gpointer in_data);
static SPModule * build_from_reprdoc (SPReprDoc * doc);
static void load_module (SPModule * in_mod);

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
sp_module_system_open (const gchar * key, const gchar * filename)
{
	gpointer parray[2];
	SPModuleInput * imod = NULL;
	GtkDialog * prefs = NULL;

	if (!strcmp(key, SP_MODULE_KEY_AUTODETECT)) {
		parray[0] = (gpointer)filename;
		parray[1] = (gpointer)&imod;
		sp_module_db_foreach(open_internal, (gpointer)&parray);
	} else {
		imod = SP_MODULE_INPUT(sp_module_db_get(key));
	}

	g_return_val_if_fail(SP_IS_MODULE_INPUT(imod), NULL);
	load_module(SP_MODULE(imod));
	g_return_val_if_fail(SP_MODULE(imod)->state != SP_MODULE_UNLOADED, NULL);

	if (imod->prefs != NULL) {
		prefs = imod->prefs(SP_MODULE(imod), filename);
		if (prefs != NULL) {
			gtk_dialog_run(prefs);
		}
	}

	return imod->open(SP_MODULE(imod), filename);
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
open_internal (SPModule * in_plug, gpointer in_data)
{
	if (SP_IS_MODULE_INPUT(in_plug)) {
		const gchar * ext;
		gpointer * parray;
		const gchar * filename;
		size_t filename_len, ext_len;
		SPModuleInput ** pimod;

		parray = (gpointer *)in_data;
		filename = (const gchar *)parray[0];
		pimod = (SPModuleInput **)parray[1];

		ext = SP_MODULE_INPUT(in_plug)->extension;

		filename_len = strlen (filename);
		ext_len = strlen (ext);
		if (filename_len < ext_len) {
			return;
		}

		if (memcmp (ext, filename + filename_len - ext_len, ext_len)) {
			return;
		}

		*pimod = SP_MODULE_INPUT(in_plug);
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
sp_module_system_save (const gchar * key, SPDocument * doc, const gchar * filename)
{
	SPModuleOutput * omod;
	gpointer parray[2];
	GtkDialog * prefs;

	if (!strcmp(key, SP_MODULE_KEY_AUTODETECT)) {
		parray[0] = (gpointer)filename;
		parray[1] = (gpointer)&omod;
		omod = NULL;
		sp_module_db_foreach(save_internal, (gpointer)&parray);
	} else {
		omod = SP_MODULE_OUTPUT(sp_module_db_get(key));
	}

	g_return_if_fail(SP_IS_MODULE_OUTPUT(omod));
	load_module(SP_MODULE(omod));
	g_return_if_fail(SP_MODULE(omod)->state != SP_MODULE_UNLOADED);

	if (omod->prefs != NULL) {
		prefs = omod->prefs(SP_MODULE(omod));
		if (prefs != NULL) {
			gtk_dialog_run(prefs);
		}
	}

	return omod->save(SP_MODULE(omod), doc, filename);
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
save_internal (SPModule * in_plug, gpointer in_data)
{
	if (SP_IS_MODULE_OUTPUT(in_plug)) {
		const gchar * ext;
		gpointer * parray;
		const gchar * filename;
		SPModuleOutput ** pomod;
		size_t filename_len, ext_len;

		parray = (gpointer *)in_data;
		filename = (const gchar *)parray[0];
		pomod = (SPModuleOutput **)parray[1];

		ext = SP_MODULE_OUTPUT(in_plug)->extension;

		ext_len = strlen (ext);
		filename_len = strlen (filename);
		if (filename_len < ext_len) {
			return;
		}

		if (memcmp (ext, filename + filename_len - ext_len, ext_len)) {
			return;
		}

		*pomod = SP_MODULE_OUTPUT(in_plug);
	}

	return;
}

/**
	\return   None
	\brief    Loads a module
	\param    in_mod  The module that is loaded

	Basically this is a function to load a module, and it calls
	the load function in that module to do so.  Nothing more interesting
	than that.
*/
static void
load_module (SPModule * in_mod)
{
	if (in_mod->state == SP_MODULE_UNLOADED) {
		in_mod->load(in_mod);
	}

	return;
}

/**
	\return   None
	\brief    A function that can be attached to a menu item to
	          execute a filter.
	\param    object   unused (for prototype matching)
	\param    key      Key of filter to be used

	This function just looks up the filter from the module database
	and then makes sure it is loaded.  After that, it calls the filter
	function for the module.  Really, it is more of a generic wrapper
	function.
*/
void
sp_module_system_filter (GtkObject * object, const gchar * key)
{
	SPModuleFilter * fmod;
	SPDocument * doc;

	g_return_if_fail(key != NULL);

	fmod = SP_MODULE_FILTER(sp_module_db_get(key));
	g_return_if_fail(SP_IS_MODULE_FILTER(fmod));

	load_module(SP_MODULE(fmod));
	g_return_if_fail(SP_MODULE(fmod)->state != SP_MODULE_UNLOADED);

	doc = SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP);
	g_return_if_fail(doc != NULL);

	return fmod->filter(SP_MODULE(fmod), doc);
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
static SPModule *
build_from_reprdoc (SPReprDoc * doc)
{
	SPRepr * repr;
	SPModule * module = NULL;
	enum {
		MODULE_EXTENSION,
		MODULE_UNKNOWN_IMP
	} module_implementation_type = MODULE_UNKNOWN_IMP;
	enum {
		MODULE_INPUT,
		MODULE_OUTPUT,
		MODULE_FILTER,
		MODULE_UNKNOWN_FUNC
	} module_functional_type = MODULE_UNKNOWN_FUNC;
	SPRepr * old_repr;
	SPRepr * child_repr;

	g_return_val_if_fail(doc != NULL, NULL);

	repr = sp_repr_document_root(doc);

	/* sp_repr_print(repr); */

	if (strcmp(sp_repr_name(repr), "spmodule")) {
		printf("How come I don't have a spmodule?\n");
		goto while_end;
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
		if (!strcmp(sp_repr_name(child_repr), "filter")) {
			module_functional_type = MODULE_FILTER;
		}
		if (!strcmp(sp_repr_name(child_repr), "extension")) {
			module_implementation_type = MODULE_EXTENSION;
		}

		old_repr = child_repr;
		child_repr = sp_repr_next(child_repr);
		/* sp_repr_unref(old_repr); */
	}
	
	switch (module_functional_type)
	{
		case MODULE_INPUT:
			{
				SPModuleInput * imod;

				imod = sp_module_input_new(repr);
				module = SP_MODULE(imod);
				break;
			}
		case MODULE_OUTPUT:
			{
				SPModuleOutput * omod;

				omod = sp_module_output_new(repr);
				module = SP_MODULE(omod);
				break;
			}
		case MODULE_FILTER:
			{
				SPModuleFilter * fmod;

				fmod = sp_module_filter_new(repr);
				module = SP_MODULE(fmod);
				break;
			}
		default:
			goto while_end;
			break;
	}

	switch (module_implementation_type) {
		case MODULE_EXTENSION:
			module->load   = extension_load;
			module->unload = extension_unload;
			break;
	default:
		;
	}

while_end:
	sp_repr_document_unref(doc);

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
SPModule *
sp_module_system_build_from_file (const gchar * filename)
{
	/* TODO: Need to define namespace here, need to write the
	         DTD in general for this stuff */
	return build_from_reprdoc (sp_repr_read_file(filename, NULL));
}

/**
	\return   The module created
	\brief    This function creates a module from a buffer holding an 
	          XML description.
	\param    buffer  The buffer holding the XML description of the module.

	This function calls build_from_reprdoc with using sp_repr_read_mem
	to create the reprdoc.  It finds the length of the buffer using strlen.
*/
SPModule *
sp_module_system_build_from_mem (const gchar * buffer)
{
	return build_from_reprdoc (sp_repr_read_mem(buffer, strlen(buffer), NULL));
}
