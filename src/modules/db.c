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

/* Globals */
/** This is the actual database.  It has all of the modules in it,
    indexed by their ids.  It's a hash table for faster lookups */
static GHashTable *moduledict = NULL;

/* Prototypes */
static void sp_module_db_foreach_internal (gpointer in_key, gpointer in_value, gpointer in_data);

/**
	\return    none
	\brief     Add a module to the module database
	\param     module  The module to be registered.

	This function is very simple in that it just checks to make sure
	the module exists, and that it has an id.  If all of those are true,
	then it adds it into the hash table.
*/
void
sp_module_db_register (SPModule *module)
{
	g_return_if_fail(module != NULL);
	g_return_if_fail(module->id != NULL);

	g_hash_table_insert (moduledict, module->id, module);
	return;
}

/**
	\return    none
	\brief     This function removes an entry from the database
	\param     module  The module to be removed.

	This function just makes sure the parameters are valid, and then
	removes the module from the hash table.
*/
void
sp_module_db_unregister (SPModule *module)
{
	g_return_if_fail(module != NULL);
	g_return_if_fail(module->id != NULL);

	g_hash_table_remove (moduledict, module->id);
	return;
}

/**
	\return    A reference to the SPModule specified by the input key.
	\brief     This function looks up a SPModule by using its unique
	           id.  It then returns a reference to that module.
	\param     key   The unique ID of the module

	This is a pretty simple function in that the GLib hash table
	does most of the work.  One of the things that is significant
	is that a reference to the module is returned.  It will need to
	be unreferenced after a call to this function.
*/
SPModule *
sp_module_db_get (const unsigned char *key)
{
	SPModule *mod;
	if (moduledict == NULL) {
		moduledict = g_hash_table_new (g_str_hash, g_str_equal);
	}
	mod = g_hash_table_lookup (moduledict, key);
	if (mod != NULL) {
		sp_module_ref (mod);
	}
	return mod;
}

const unsigned char *
sp_module_db_get_unique_id (unsigned char *c, int len, const unsigned char *val)
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

	Basically this function just steps through the database.  The only
	thing that is interesting about this function is how it mangles
	the data into a structure on the stack, and then uses the internal
	function to demangle this.  By putting this data in the stack we
	don't have to worry about how this was called.  Unfortunately we
	need two peices of data in each call, so just using the pointer
	provided by g_hash_table_foreach won't work out.
*/
void
sp_module_db_foreach (void (*in_func)(SPModule * in_plug, gpointer in_data), gpointer in_data)
{
	gpointer data[2];

	g_return_if_fail(moduledict != NULL);

	data[0] = (gpointer)in_func;
	data[1] = in_data;
	g_hash_table_foreach(moduledict, sp_module_db_foreach_internal, data);
	return;
}

/**
	\return    none
	\brief     The internal function for the 'foreach' command
	\param     in_key    The key of the object being pulled out of the
	                     hash table.
	\param     in_value  The actual module (a pointer to it) that is
	                     located in the hash table.
	\param     in_data   A pointer to an array of pointers that is
	                     earlier in the stack.  It contains pointers
						 to the function we need to call, and the data
						 for it.

	The key purpose of this function is to take the rather obtuse
	prototype that is provided by the Glib hash table functions and
	turn it into something that is more natural by users of the moulde
	database.  Basically this moves the in_value parameter to the first
	parameter of the call.  It also grabs the function call and the data
	to be passed to it from a structure of two gpointers that is passed
	through the in_data parameter.  This structure is built in
	sp_module_db_foreach, but is not formally defined.
*/
static void
sp_module_db_foreach_internal (gpointer in_key, gpointer in_value, gpointer in_data)
{
	gpointer * in_array = (gpointer *)in_data;
	void (* in_func)(SPModule * in_plug, gpointer in_data);
	in_func = in_array[0];
	return in_func(SP_MODULE(in_value), in_array[1]);
}

