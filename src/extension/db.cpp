/*
 * Functions to keep a listing of all modules in the system.  Has its
 * own file mostly for abstraction reasons, but is pretty simple
 * otherwise.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <helper/sp-intl.h>
#include "db.h"

/* Globals */

/* Namespaces */

namespace Inkscape {
namespace Extension {

/** This is the actual database object.  There is only one of these */
DB db;

/* Types */

/** Holds the callback and user-supplied data used by sp_module_db_foreach_internal()
 * */
struct ModuleDBForeachClosure {
	void (* in_func)(Inkscape::Extension::Extension * in_plug, gpointer in_data);
	gpointer in_data;
};

DB::DB (void) {
	moduledict = g_hash_table_new (g_str_hash, g_str_equal);
}

/**
	\brief     Add a module to the module database
	\param     module  The module to be registered.
*/
void
DB::register_ext (Extension *module)
{
	g_return_if_fail(module->get_id() != NULL);

	/* printf("Registering: %s\n", module->get_name()); */
	g_hash_table_insert (moduledict, module->get_id(), module);
}

/**
	\brief     This function removes a module from the database
	\param     module  The module to be removed.
*/
void
DB::unregister_ext (Extension * module)
{
	g_return_if_fail(module->get_id() != NULL);

	g_hash_table_remove (moduledict, module->get_id());
}

/**
	\return    A reference to the Inkscape::Extension::Extension specified by the input key.
	\brief     This function looks up a Inkscape::Extension::Extension by using its unique
	           id.  It then returns a reference to that module.
	\param     key   The unique ID of the module

	Retrieves a module by name; if non-NULL, it refs the returned
  	module; the caller is responsible for releasing that reference
	when it is no longer needed.
*/
Extension *
DB::get (const gchar *key)
{
	Extension *mod;

	if (key == NULL) return NULL;

	mod = (Inkscape::Extension::Extension *)g_hash_table_lookup (moduledict, key);

	return mod;
}

const gchar *
DB::get_unique_id (gchar *c, int len, const gchar *val)
{
	static int mnumber = 0;

	while (!val || g_hash_table_lookup (moduledict, val)) {
		g_snprintf (c, len, "Module_%d", ++mnumber);
		val = c;
	}
	return val;
}

/**
	\return    none
	\brief     A function to execute another function with every entry
	           in the database as a parameter.
	\param     in_func  The function to execute for every module
	\param     in_data  A data pointer that is also passed to in_func

 	Enumerates the modules currently in the database, calling a given
	callback for each one.
*/
void
DB::foreach (void (*in_func)(Extension * in_plug, gpointer in_data), gpointer in_data)
{
	g_return_if_fail(moduledict != NULL);

	ModuleDBForeachClosure closure = { in_func, in_data };

	g_hash_table_foreach(moduledict, (GHFunc)DB::foreach_internal, &closure);
	return;
}

/**
	\brief     Adaptor used by sp_module_db_foreach()
	\param     in_key    The key of the object being pulled out of the
	                     hash table.
	\param     in_value  The actual module (a pointer to it) that is
	                     located in the hash table.
	\param     in_data   A pointer to a ModuleDBForeachClosure struct

	This function serves as an adaptor to use sp_module_db_foreach()'s
	callbacks with g_hash_table_foreach().
*/
void
DB::foreach_internal (gpointer in_key, gpointer in_value, gpointer in_data)
{
	ModuleDBForeachClosure *closure=reinterpret_cast<ModuleDBForeachClosure *>(in_data);
	closure->in_func(reinterpret_cast<Inkscape::Extension::Extension *>(in_value), closure->in_data);
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an input module, then add it to the open menu.
	\param     in_plug  Module to be examined
	\param     data     The list to be attached to

	The first thing that is checked is if this module is an input
	module.  If it is, then it is turned into a...
*/
void
DB::input_internal (Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Input *>(in_plug)) {
		const gchar * name              = NULL;
		const gchar * tooltip           = NULL;
		const gchar * extension         = NULL;
		const gchar * mimetype          = NULL;
		Input * imod                    = NULL;
		IOExtensionDescription * desc   = NULL;

		imod = dynamic_cast<Input *>(in_plug);

		name = imod->get_filetypename();
		if (name == NULL) {
			name = in_plug->get_name();
		}

		tooltip = imod->get_filetypetooltip();

		desc = new IOExtensionDescription(name, extension, mimetype, in_plug);
		g_slist_append((GSList *)data, (gpointer)desc);
	}

	return;
}

void
DB::output_internal (Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Output *>(in_plug)) {
		const gchar * name              = NULL;
		const gchar * tooltip           = NULL;
		const gchar * extension         = NULL;
		const gchar * mimetype          = NULL;
		Output * omod                   = NULL;
		IOExtensionDescription * desc   = NULL;

		omod = dynamic_cast<Output *>(in_plug);

		name = omod->get_filetypename();
		if (name == NULL) {
			name = in_plug->get_name();
		}

		tooltip = omod->get_filetypetooltip();

		desc = new IOExtensionDescription(name, extension, mimetype, in_plug);
		g_slist_append((GSList *)data, (gpointer)desc);
	}

	return;
}

GSList *
DB::get_input_list (void)
{
	GSList * retlist = NULL;
	IOExtensionDescription * desc;

	desc = new IOExtensionDescription(_("Autodetect"), NULL, NULL, NULL);
	retlist = g_slist_append(retlist, (gpointer)desc);
	foreach(input_internal, (gpointer)retlist);

	return retlist;
}

GSList *
DB::get_output_list (void)
{
	GSList * retlist = NULL;
	IOExtensionDescription * desc;

	desc = new IOExtensionDescription(_("Autodetect"), NULL, NULL, NULL);
	retlist = g_slist_append(retlist, (gpointer)desc);
	foreach(output_internal, (gpointer)retlist);

	return retlist;
}

void
DB::free_list (GSList * in_list)
{
	return;
}

DB::IOExtensionDescription::IOExtensionDescription(const gchar * in_name, const gchar * in_file_extension, const gchar * in_mime, Extension * in_extension)
{
	name = in_name;
	file_extension = in_file_extension;
	mimetype = in_mime;
	extension = in_extension;
	return;
}

DB::IOExtensionDescription::~IOExtensionDescription(void)
{
	return;
}

}; }; /* namespace Extension, Inkscape */
