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

		extension = imod->get_extension();
		mimetype  = imod->get_mimetype();
		tooltip   = imod->get_filetypetooltip();

		desc = new IOExtensionDescription(name, extension, mimetype, in_plug, !in_plug->deactivated());
		g_slist_append((GSList *)data, (gpointer)desc);
	}
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

		extension = omod->get_extension();
		mimetype  = omod->get_mimetype();
		tooltip   = omod->get_filetypetooltip();

		desc = new IOExtensionDescription(name, extension, mimetype, in_plug, !in_plug->deactivated());
		g_slist_append((GSList *)data, (gpointer)desc);
	}
}

GSList *
DB::get_input_list (void)
{
	GSList * retlist = NULL;
	IOExtensionDescription * desc;

	desc = new IOExtensionDescription(_("Autodetect"), NULL, NULL, NULL, TRUE);
	retlist = g_slist_append(retlist, (gpointer)desc);
	foreach(input_internal, (gpointer)retlist);

	return retlist;
}

GSList *
DB::get_output_list (void)
{
	GSList * retlist = NULL;
	IOExtensionDescription * desc;

	desc = new IOExtensionDescription(_("Autodetect"), NULL, NULL, NULL, TRUE);
	retlist = g_slist_append(retlist, (gpointer)desc);
	foreach(output_internal, (gpointer)retlist);

	return retlist;
}

void
DB::free_list (GSList * in_list)
{
}

DB::IOExtensionDescription::IOExtensionDescription(const gchar * in_name, const gchar * in_file_extension, const gchar * in_mime, Extension * in_extension, bool in_sensitive)
{
    name = in_name;
    file_extension = in_file_extension;
    mimetype = in_mime;
    extension = in_extension;
    sensitive = in_sensitive;
    pattern.clear();
    if ( in_extension )
    {
        Glib::ustring tmp(in_file_extension);
        for ( guint i = 0; i < tmp.length(); i++ )
        {
            Glib::ustring::value_type ch = tmp.at(i);
            if ( Glib::Unicode::isalpha(ch) )
            {
                pattern += '[';
                pattern += Glib::Unicode::toupper(ch);
                pattern += Glib::Unicode::tolower(ch);
                pattern += ']';
            }
            else
            {
                pattern += ch;
            }
        }
        //g_message(" processed glob from '%s' to be '%s'", tmp.c_str(), pattern.c_str() );
    }
}

DB::IOExtensionDescription::~IOExtensionDescription(void)
{
}

} } /* namespace Extension, Inkscape */
