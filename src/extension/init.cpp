/*
 * This is what gets executed to initialize all of the modules.  For
 * the internal modules this invovles executing their initialization
 * functions, for external ones it involves reading their .spmodule
 * files and bringing them into Sodipodi.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <dirent.h>
#include <glib.h>
#include <string.h>

#include <xml/repr.h>
#include <helper/sp-intl.h>

#include <extension.h>
#include "system.h"
#include "implementation/script.h"
#include "internal/svg.h"

/** This is the extention that all files are that are pulled from
    the extension directory and parsed */
#define SP_MODULE_EXTENSION  "inkmod"

static void build_module_from_dir (const gchar * dirname);

/**
	\return   none
	\brief    A function to hold all of the init routines for internal
	          modules.

	This should be a list of all the internal modules
	that need to initialized.  This is just a convinent
	place to put them.  Also, this function calls
	build_module_from_dir to parse the Sodipodi extensions
	directory.
*/
void
sp_modules_init (void)
{
	svg_init();
	build_module_from_dir(INKSCAPE_MODULESDIR);

	return;
}

/**
	\return    none
	\brief     This function parses a directory for files of SP_MODULE_EXTENSION
	           type and loads them.
	\param     dirname  The directory that should be searched for modules

	Here is just a basic function that moves through a directory.  It
	looks at every entry, and compares its filename with SP_MODULE_EXTENSION.
	Of those that pass, sp_module_system_build_from_file is called
	with their filenames.
*/
static void
build_module_from_dir (const gchar * dirname)
{
	DIR * directory;
	struct dirent * dentry;

	directory = opendir(dirname);
	if (directory == NULL) {
		g_warning(_("Modules directory (%s) is unavailable.  External modules in that directory will not be loaded."), dirname);
		return;
	}

	while ((dentry = readdir(directory)) != NULL) {
		gchar * filename;

		if (strlen(dentry->d_name) < strlen(SP_MODULE_EXTENSION)) {
			continue;
		}

		if (strcmp(SP_MODULE_EXTENSION, dentry->d_name + (strlen(dentry->d_name) - strlen(SP_MODULE_EXTENSION)))) {
			continue;
		}

		filename = g_strdup_printf("%s/%s", INKSCAPE_MODULESDIR, dentry->d_name);
		sp_module_system_build_from_file (filename);
		g_free(filename);
	}

	closedir(directory);

	return;
}


