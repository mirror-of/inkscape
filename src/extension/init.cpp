/*
 * This is what gets executed to initialize all of the modules.  For
 * the internal modules this invovles executing their initialization
 * functions, for external ones it involves reading their .spmodule
 * files and bringing them into Sodipodi.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
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
#include "db.h"
#include "implementation/script.h"
#include "internal/svg.h"
#include "internal/ps.h"
#ifdef WITH_GNOME_PRINT
#include "internal/gnome.h"
#endif
#ifdef WIN32
#include "internal/win32.h"
#endif
#include "internal/ps-out.h"
#include "internal/eps-out.h"

namespace Inkscape {
namespace Extension {

/** This is the extention that all files are that are pulled from
    the extension directory and parsed */
#define SP_MODULE_EXTENSION  "inkmod"

static void build_module_from_dir (const gchar * dirname);
static void check_extensions (void);

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
init (void)
{
	/* TODO: Change to Internal */
	Internal::Svg::init();
	Internal::PsOutput::init();
	Internal::EpsOutput::init();
	Internal::PrintPS::init();
#ifdef WITH_GNOME_PRINT
	Internal::PrintGNOME::init();
#endif
#ifdef WIN32
	Internal::PrintWin32::init();
#endif
	build_module_from_dir(INKSCAPE_MODULESDIR);

	/* now we need to check and make sure everyone is happy */
	check_extensions();

	return;
}

/**
	\return    none
	\brief     This function parses a directory for files of SP_MODULE_EXTENSION
	           type and loads them.
	\param     dirname  The directory that should be searched for modules

	Here is just a basic function that moves through a directory.  It
	looks at every entry, and compares its filename with SP_MODULE_EXTENSION.
	Of those that pass, build_from_file is called
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
		build_from_file (filename, NULL);
		g_free(filename);
	}

	closedir(directory);

	return;
}

static void
check_extensions_internal (Extension * in_plug, gpointer in_data)
{
	int * count = (int *)in_data;

	if (!in_plug->check()) {
		printf("Deleting Extension: %s\n", in_plug->get_name());
		delete in_plug;
		(*count)++;
	}

	return;
}

static void
check_extensions (void)
{
	int count = 1;

	while (count != 0) {
		count = 0;
		db.foreach(check_extensions_internal, (gpointer)&count);
	}

	return;
}

}; }; /* namespace Inkscape::Extension */
