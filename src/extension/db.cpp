/*
 * Functions to keep a listing of all modules in the system.  Has its
 * own file mostly for abstraction reasons, but is pretty simple
 * otherwise.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <glib.h>
#include <helper/sp-intl.h>
#include "db.h"
#include "input.h"
#include "output.h"
#include "effect.h"

/* Globals */

/* Namespaces */

namespace Inkscape {
namespace Extension {

/** This is the actual database object.  There is only one of these */
DB db;

/* Types */

DB::DB (void) {
}

/**
	\brief     Add a module to the module database
	\param     module  The module to be registered.
*/
void
DB::register_ext (Extension *module)
{
	g_return_if_fail(module->get_id() != NULL);

	// printf("Registering: %s\n", module->get_id());
	moduledict[module->get_id()] = module;
}

/**
	\brief     This function removes a module from the database
	\param     module  The module to be removed.
*/
void
DB::unregister_ext (Extension * module)
{
	g_return_if_fail(module->get_id() != NULL);

	// printf("Extension DB: removing %s\n", module->get_id());
	moduledict.erase(moduledict.find(module->get_id()));
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

	mod = (*moduledict.find(key)).second;

	if ( !mod || mod->deactivated() )
		return NULL;

	return mod;
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
	std::map <const char *, Extension *>::iterator cur;

	for (cur = moduledict.begin(); cur != moduledict.end(); cur++) {
		in_func((*cur).second, in_data);
	}
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an input module, then add it to the list.
	\param     in_plug  Module to be examined
	\param     data     The list to be attached to

	The first thing that is checked is if this module is an input
	module.  If it is, then it is added to the list which is passed
	in through \c data.
*/
void
DB::input_internal (Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Input *>(in_plug)) {
		InputList * ilist;
		Input * imod;

		imod = dynamic_cast<Input *>(in_plug);
		ilist = reinterpret_cast<InputList *>(data);

		ilist->push_front(imod);
	}
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an output module, then add it to the list.
	\param     in_plug  Module to be examined
	\param     data     The list to be attached to

	The first thing that is checked is if this module is an output
	module.  If it is, then it is added to the list which is passed
	in through \c data.
*/
void
DB::output_internal (Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Output *>(in_plug)) {
		OutputList * olist;
		Output * omod;

		omod = dynamic_cast<Output *>(in_plug);
		olist = reinterpret_cast<OutputList *>(data);

		olist->push_front(omod);
	}

	return;
}

/**
	\return    none
	\brief     The function to look at each module and see if it is
	           an effect module, then add it to the list.
	\param     in_plug  Module to be examined
	\param     data     The list to be attached to

	The first thing that is checked is if this module is an effect
	module.  If it is, then it is added to the list which is passed
	in through \c data.
*/
void
DB::effect_internal (Extension * in_plug, gpointer data)
{
	if (dynamic_cast<Effect *>(in_plug)) {
		EffectList * elist;
		Effect * emod;

		emod = dynamic_cast<Effect *>(in_plug);
		elist = reinterpret_cast<EffectList *>(data);

		elist->push_front(emod);
	}

	return;
}

/**
	\brief  Creates a list of all the Input extensions
	\param  ou_list  The list that is used to put all the extensions in

	Calls the database \c foreach function with \c input_internal.
*/
DB::InputList &
DB::get_input_list (DB::InputList &ou_list)
{
	foreach(input_internal, (gpointer)&ou_list);
	return ou_list;
}

/**
	\brief  Creates a list of all the Output extensions
	\param  ou_list  The list that is used to put all the extensions in

	Calls the database \c foreach function with \c output_internal.
*/
DB::OutputList &
DB::get_output_list (DB::OutputList &ou_list)
{
	foreach(output_internal, (gpointer)&ou_list);
	return ou_list;
}

/**
	\brief  Creates a list of all the Effect extensions
	\param  ou_list  The list that is used to put all the extensions in

	Calls the database \c foreach function with \c effect_internal.
*/
DB::EffectList &
DB::get_effect_list (DB::EffectList &ou_list)
{
	foreach(effect_internal, (gpointer)&ou_list);
	return ou_list;
}

} } /* namespace Extension, Inkscape */
