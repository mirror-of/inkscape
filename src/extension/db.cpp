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
#include "db.h"

/* Types */

/** Holds the callback and user-supplied data used by sp_module_db_foreach_internal()
 * */
struct ModuleDBForeachClosure {
	void (* in_func)(Inkscape::Extension::Extension * in_plug, gpointer in_data);
	gpointer in_data;
};

/* Globals */

/** This is the actual database.  It has all of the modules in it,
    indexed by their ids.  It's a hash table for faster lookups */
static GHashTable *moduledict = NULL;

/* Prototypes */
static void sp_module_db_foreach_internal (gpointer in_key, gpointer in_value, gpointer in_data);

/**
	\brief     Add a module to the module database
	\param     module  The module to be registered.
*/
void
sp_module_db_register (Inkscape::Extension::Extension *module)
{
	g_return_if_fail(module->get_id() != NULL);

	g_hash_table_insert (moduledict, module->get_id(), module);
}

/**
	\brief     This function removes a module from the database
	\param     module  The module to be removed.
*/
void
sp_module_db_unregister (Inkscape::Extension::Extension * module)
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
	Inkscape::Extension::Extension *
sp_module_db_get (const gchar *key)
{
	Inkscape::Extension::Extension *mod;

	if (moduledict == NULL) {
		moduledict = g_hash_table_new (g_str_hash, g_str_equal);
	}

	mod = (Inkscape::Extension::Extension *)g_hash_table_lookup (moduledict, key);

	return mod;
}

const gchar *
sp_module_db_get_unique_id (gchar *c, int len, const gchar *val)
{
	static int mnumber = 0;
	if (moduledict == NULL) {
		moduledict = g_hash_table_new (g_str_hash, g_str_equal);
	}
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
sp_module_db_foreach (void (*in_func)(Inkscape::Extension::Extension * in_plug, gpointer in_data), gpointer in_data)
{
	g_return_if_fail(moduledict != NULL);

	ModuleDBForeachClosure closure = { in_func, in_data };

	g_hash_table_foreach(moduledict, (GHFunc)sp_module_db_foreach_internal, &closure);
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
static void
sp_module_db_foreach_internal (gpointer in_key, gpointer in_value, gpointer in_data)
{
	ModuleDBForeachClosure *closure=reinterpret_cast<ModuleDBForeachClosure *>(in_data);
	closure->in_func(reinterpret_cast<Inkscape::Extension::Extension *>(in_value), closure->in_data);
}

