/*
 * Code for handling extensions (i.e., scripts)
 *
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define __INKSCAPE_EXTENSION_IMPLEMENTATION_SCRIPT_C__

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <libart_lgpl/art_affine.h>
#include <svg/svg.h>
#include <xml/repr-private.h>
#include <interface.h>
#include <document.h>
/* #include <inkscape.h> */
#include <desktop.h>
#include <desktop-handles.h>
#include <selection.h>
#include <file.h>
#include <sp-object.h>
#include <sp-namedview.h>

#include "../system.h"

#include <extension/extension.h>
#include <extension/db.h>
#include "implementation.h"
#include "script.h"

/** This is the command buffer that gets allocated from the stack */
#define BUFSIZE (255)

/* Namespaces */
namespace Inkscape {
namespace Extension {
namespace Implementation {

/* Real functions */
/**
	\return    A script object
	\breif     This function creates a script object and sets up the
	           variables.

   This function just sets the command to NULL.  It should get built
   officially in the load function.  This allows for less allocation
   of memory in the unloaded state.
*/
Script::Script (void) : Implementation::Implementation()
{
	command = NULL;
	return;
}

/**
	\return    A string with the complete string with the relative directory expanded
	\brief     This function takes in a Repr that contains a reldir entry
	           and returns that data with the relative directory expanded.
			   Mostly it is here so that relative directories all get used
			   the same way.
	\param     reprin   The SPRepr with the reldir in it.

	Basically this function looks at an attribute of the Repr, and makes
	a decision based on that.  Currently, it is only working with the
	'exetensions' relative directory, but there will be more of them.
	One thing to notice is that this function always returns an allocated
	string.  This means that the caller of this function can always
	free what they are given (and should do it too!).
*/
gchar *
Script::solve_reldir (SPRepr * reprin) {
	const gchar * reldir;

	reldir = sp_repr_attr(reprin, "reldir");

	if (reldir == NULL) {
		return g_strdup(sp_repr_content(sp_repr_children(reprin)));
	}

	if (!strcmp(reldir, "extensions")) {
		return g_strdup_printf("%s/%s", INKSCAPE_EXTENSIONDIR, sp_repr_content(sp_repr_children(reprin)));
	} else {
		return g_strdup(sp_repr_content(sp_repr_children(reprin)));
	}

	return NULL;
}

/**
	\return   none
	\brief    This function 'loads' an extention, basically it determines
	          the full command for the extention and stores that.
	\param    module  The extention to be loaded.

	The most difficult part about this function is finding the actual
	command through all of the Reprs.  Basically it is hidden down a
	couple of layers, and so the code has to move down too.  When
	the command is actually found, it has its relative directory
	solved.

	At that point all of the loops are exited, and there is an
	if statement to make sure they didn't exit because of not finding
	the command.  If that's the case, the extention doesn't get loaded
	and should error out at a higher level.
*/

bool
Script::load (Inkscape::Extension::Extension * module)
{
	SPRepr * child_repr;
	gchar * command_text = NULL;

	if (module->loaded()) {
		return TRUE;
	}

	helper_extension = NULL;

	/* This should probably check to find the executable... */
	child_repr = sp_repr_children(module->get_repr());
	while (child_repr != NULL) {
		if (!strcmp(sp_repr_name(child_repr), "extension")) {
			child_repr = sp_repr_children(child_repr);
			while (child_repr != NULL) {
				if (!strcmp(sp_repr_name(child_repr), "command")) {
					command_text = solve_reldir(child_repr);
				}
				if (!strcmp(sp_repr_name(child_repr), "helper_extension")) {
					helper_extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
				}
				child_repr = sp_repr_next(child_repr);
			}

			break;
		}
		child_repr = sp_repr_next(child_repr);
	}

	g_return_val_if_fail(command_text != NULL, FALSE);

	g_free(command);
	command = command_text;

	if (helper_extension != NULL) {
		g_free(helper_extension);
		helper_extension = NULL;
	}

	return TRUE;
}

/**
	\return   None.
	\brief    Unload this puppy!
	\param    module  Extension to be unloaded.

	This function just sets the module to unloaded.  It free's the
	command if it has been allocated.
*/
void
Script::unload (Inkscape::Extension::Extension * module)
{
	g_free(command);
	g_free(helper_extension);
	return;
}

/**
	\return   A dialog for preferences
	\brief    A stub funtion right now
	\param    module    Module who's preferences need getting
	\param    filename  Hey, the file your getting might be important

	This function should really do something, right now it doesn't.
*/
GtkDialog *
Script::prefs (Inkscape::Extension::Input * module, const gchar * filename)
{
	/* Sad, this should really do something... */
	return NULL;
}

/**
	\return   A dialog for preferences
	\brief    A stub funtion right now
	\param    module    Module who's preferences need getting

	This function should really do something, right now it doesn't.
*/
GtkDialog *
Script::prefs (Inkscape::Extension::Output * module)
{
	/* Sad, this should really do something... */
	return NULL;
}

/**
	\return   A dialog for preferences
	\brief    A stub funtion right now
	\param    module    Module who's preferences need getting

	This function should really do something, right now it doesn't.
*/
GtkDialog *
Script::prefs (Inkscape::Extension::Effect * module)
{
	/* Sad, this should really do something... */
	return NULL;
}

/**
	\return  A new document that has been opened
	\brief   This function uses a filename that is put in, and calls
	         the extension's command to create an SVG file which is
			 returned.
	\param   module   Extension to use.
	\param   filename File to open.

	First things first, this function needs a temporary file name.  To
	create on of those the function g_mkstemp is used, with a filename with
	the header of ink_ext_.

	The extension is then executed using the 'execute' function
	with the filname coming in, and the temporary filename.  After
	That executing, the SVG should be in the temporary file.

	Finally, the temporary file is opened using the SVG input module and
	a document is returned.  That document has its filename set to
	the incoming filename (so that it's not the temporary filename).
	That document is then returned from this function.
*/
SPDocument *
Script::open (Inkscape::Extension::Input * module, const gchar * filename)
{
	char tempfilename_out_x[] = "/tmp/ink_ext_XXXXXX";
	gchar * tempfilename_out;
	SPDocument * mydoc;

	tempfilename_out = (gchar *)tempfilename_out_x;

	if (g_mkstemp((char *)tempfilename_out) == -1) {
		/* Error, couldn't create temporary filename */
		if (errno == EINVAL) {
			/* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
			perror("Extension::Script:  template for filenames is misconfigured.\n");
			exit(-1);	    
		} else if (errno == EEXIST) {
			/* Now the  contents of template are undefined. */
			perror("Extension::Script:  Could not create a unique temporary filename\n");
			return NULL;
		} else {
			perror("Extension::Script:  Unknown error creating temporary filename\n");
			exit(-1);
		}
	}

	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* local_filename = g_filename_from_utf8 ( filename,
                                 -1,  &bytesRead,  &bytesWritten, &error);

	execute(command, local_filename, (gchar *)tempfilename_out);

	g_free(local_filename);

	if (helper_extension == NULL) {
		mydoc = sp_module_system_open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), tempfilename_out);
	} else {
		mydoc = sp_module_system_open(Inkscape::Extension::db.get(helper_extension), tempfilename_out);
	}

	sp_document_set_uri(mydoc, (const gchar *)filename);

	unlink((char *)tempfilename_out);

	return mydoc;
}

/**
	\return   none
	\brief    This function uses an extention to save a document.  It first
	          creates an SVG file of the document, and then runs it through
			  the script.
	\param    module    Extention to be used
	\param    doc       Document to be saved
	\param    filename  The name to save the final file as

	Well, at some point people need to save - it is really what makes
	the entire application useful.  And, it is possible that someone
	would want to use an extetion for this, so we need a function to
	do that eh?

	First things first, the document is saved to a temporary file that
	is an SVG file.  To get the temporary filename g_mkstemp is used with
	sp_ext_ as a prefix.  Don't worry, this file gets deleted at the
	end of the function.

	After we have the SVG file, then extention_execute is called with
	the temporary file name and the final output filename.  This should
	put the output of the script into the final output file.  We then
	delete the temporary file.
*/
void
Script::save (Inkscape::Extension::Output * module, SPDocument * doc, const gchar * filename)
{
	gchar tempfilename_in_x[] = "/tmp/ink_ext_XXXXXX";
	gchar * tempfilename_in;

	tempfilename_in = tempfilename_in_x;

	if (g_mkstemp(tempfilename_in) == -1) {
		/* Error, couldn't create temporary filename */
		if (errno == EINVAL) {
			/* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
			perror("Extension::Script:  template for filenames is misconfigured.\n");
			exit(-1);	    
		} else if (errno == EEXIST) {
			/* Now the  contents of template are undefined. */
			perror("Extension::Script:  Could not create a unique temporary filename\n");
			return;
		} else {
			perror("Extension::Script:  Unknown error creating temporary filename\n");
			exit(-1);
		}
	}

	if (helper_extension == NULL) {
		sp_module_system_save(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE), doc, tempfilename_in, FALSE, FALSE, FALSE);
	} else {
		sp_module_system_save(Inkscape::Extension::db.get(helper_extension), doc, tempfilename_in, FALSE, FALSE, FALSE);
	}

	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* local_filename = g_filename_from_utf8 ( filename,
                                 -1,  &bytesRead,  &bytesWritten, &error);

	execute(command, (gchar *)tempfilename_in, local_filename);

	g_free(local_filename);

	unlink(tempfilename_in);

	return;
}

/**
	\return    none
	\brief     This function uses an extention as a effect on a document.
	\param     module   Extention to effect with.
	\param     doc      Document to run through the effect.

	This function is a little bit trickier than the previous two.  It
	needs two temporary files to get it's work done.  Both of these
	files have random names created for them using the g_mkstemp function
	with the sp_ext_ prefix in the temporary directory.  Like the other
	functions, the temporary files are deleted at the end.

	To save/load the two temporary documents (both are SVG) the internal
	modules for SVG load and save are used.  They are both used through
	the module system function by passing their keys into the functions.

	The command itself is built a little bit differently than in other
	functions because the effect support selections.  So on the command
	line a list of all the ids that are selected is included.  Currently,
	this only works for a single selected object, but there will be more.
	The command string is filled with the data, and then after the execution
	it is freed.

	The execute function is used at the core of this function
	to execute the Script on the two SVG documents (actually only one
	exists at the time, the other is created by that script).  At that
	point both should be full, and the second one is loaded.
*/
void
Script::effect (Inkscape::Extension::Effect * module, SPDocument * doc)
{
	char tempfilename_in_x[] = "/tmp/ink_ext_XXXXXX";
	gchar * tempfilename_in;
	char tempfilename_out_x[] = "/tmp/ink_ext_XXXXXX";
	gchar * tempfilename_out;
	char * command = NULL;
	SPItem * selected;
	SPDocument * mydoc;

	tempfilename_in = (char *)tempfilename_in_x;

	if (g_mkstemp(tempfilename_in) == -1) {
		/* Error, couldn't create temporary filename */
		if (errno == EINVAL) {
			/* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
			perror("Extension::Script:  template for filenames is misconfigured.\n");
			exit(-1);	    
		} else if (errno == EEXIST) {
			/* Now the  contents of template are undefined. */
			perror("Extension::Script:  Could not create a unique temporary filename\n");
			return;
		} else {
			perror("Extension::Script:  Unknown error creating temporary filename\n");
			exit(-1);
		}
	}

	tempfilename_out = (char *)tempfilename_out_x;

	if (g_mkstemp(tempfilename_out) == -1) {
		/* Error, couldn't create temporary filename */
		if (errno == EINVAL) {
			/* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
			perror("Extension::Script:  template for filenames is misconfigured.\n");
			exit(-1);	    
		} else if (errno == EEXIST) {
			/* Now the  contents of template are undefined. */
			perror("Extension::Script:  Could not create a unique temporary filename\n");
			return;
		} else {
			perror("Extension::Script:  Unknown error creating temporary filename\n");
			exit(-1);
		}
	}

	sp_module_system_save(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE), doc, tempfilename_in, FALSE, FALSE, FALSE);

	/* TODO: I don't think this is the best way to do this, plus,
	         it needs to handle all of the cases where there is more
			 than one object selected.  This is a start though. */
	/* selected = sp_selection_item (SP_DT_SELECTION (SP_ACTIVE_DESKTOP)); */
	/* TODO: fix this */
	selected = NULL;
	if (selected != NULL && !SP_OBJECT_IS_CLONED(selected)) {
		command = g_strdup_printf("%s --id=%s",
		                          command,
		                          SP_OBJECT_ID(selected));
	} else {
		command = g_strdup_printf("%s", command);
	}

	execute(command, tempfilename_in, tempfilename_out);
	g_free(command);

	mydoc = sp_module_system_open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), tempfilename_out);

	unlink(tempfilename_in);
	unlink(tempfilename_out);

	/* Do something with mydoc.... */
	/* TODO: This creates a new window, which really isn't
	 * ideal...  there needs to be a better way to do this. */
	if (1) {
		SPViewWidget *dtw;

		g_return_if_fail (mydoc != NULL);

		dtw = sp_desktop_widget_new (sp_document_namedview (mydoc, NULL));
		sp_document_unref (mydoc);
		g_return_if_fail (dtw != NULL);

		sp_create_window (dtw, TRUE);
	}

	return;
}

/**
	\return   none
	\brief    This is the core of the extension file as it actually does
	          the execution of the extension.
	\param    in_command  The command to be executed
	\param    filein      Filename coming in
	\param    fileout     Filename of the out file

	The first thing that this function does is build the command to be
	executed.  This consists of the first string (in_command) and then
	the filename for input (filein).  This file is put on the command
	line.

	The next thing is that this function does is open a pipe to the
	command and get the file handle in the ppipe variable.  It then
	opens the output file with the output file handle.  Both of these
	operations are checked extensively for errors.

	After both are opened, then the data is coppied from the output
	of the pipe into the file out using fread and fwrite.  These two
	functions are used because of their primitive nature they make
	no assumptions about the data.  A buffer is used in the transfer,
	but the output of fread is stored so the exact number of bytes
	is handled gracefully.

	At the very end (after the data has been coppied) both of the files
	are closed, and we return to what we were doing.
*/
void
Script::execute (const gchar * in_command, const gchar * filein, const gchar * fileout)
{
	FILE * ppipe;
	FILE * pfile;
	char buf[BUFSIZE];
	char  * command;
	int num_read;

	g_return_if_fail(in_command != NULL);

	/* Get the commandline to be run */
	/* TODO:  Perhaps replace with a sprintf? */
	command = g_strdup_printf("%s \"%s\"", in_command, filein);

	/* Run script */
	ppipe = popen(command, "r");
	g_free(command);

	if (ppipe == NULL) {
	  /* Error - could not open pipe - check errno */
	  if (errno == EINVAL) {
	    perror("Extension::Script:  Invalid mode argument in popen\n");
	  } else if (errno == ECHILD) {
	    perror("Extension::Script:  Cannot obtain child extension status in popen\n");
	  }
	  return;
	}

	pfile = fopen(fileout, "w");

	if (pfile == NULL) {
	  /* Error - could not open file */
	  if (errno == EINVAL) {
	    perror("Extension::Script:  The mode provided to fopen was invalid\n");
	  } else {
	    perror("Extension::Script:  Unknown error attempting to open temporary file\n");
	  }
	  return;
	}

	/* Copy pipe output to a temporary file */
	while ((num_read = fread(buf, 1, BUFSIZE, ppipe)) != 0) {
		fwrite(buf, 1, num_read, pfile);
	}

	/* Close file */
	if (fclose(pfile) == EOF) {
	  if (errno == EBADF) {
	    perror("Extension::Script:  The filedescriptor for the temporary file is invalid\n");
	    return;
	  } else {
	    perror("Extension::Script:  Unknown error closing temporary file\n");
	  }
	}

	/* Close pipe */
	if (pclose(ppipe) == -1) {
	  if (errno == EINVAL) {
	    perror("Extension::Script:  Invalid mode set for pclose\n");
	  } else if (errno == ECHILD) {
	    perror("Extension::Script:  Could not obtain child status for pclose\n");
	  } else {
	    perror("Extension::Script:  Unknown error for pclose\n");
	  }
	}

	return;
}

}; /* Inkscape  */
}; /* module  */
}; /* Implementation  */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
