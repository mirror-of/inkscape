#define __SP_EXTENSION_C__

/*
 * Code for handling extensions (i.e., scripts)
 *
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *
 * Copyright (C) 2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <monostd.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <libart_lgpl/art_affine.h>
#include "svg/svg.h"
#include "xml/repr-private.h"
#include "interface.h"
#include "document.h"
#include "sp-namedview.h"
#include "sodipodi.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "sp-module-exec-ext.h"
#include "file.h"

#define BUFSIZE (255)

static SPModuleExecClass * parent_class;

static void sp_module_exec_ext_class_init (SPModuleExecExtClass * klass);
static void sp_module_exec_ext_init       (SPModuleExecExt *      object);
static void sp_module_exec_ext_finalize    (GObject *            object);
static void sp_extension                  (SPModule *             in_plug,
                                           SPModuleDoc *          in_doc);


GType
sp_module_exec_ext_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleExecExtClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_exec_ext_class_init,
			NULL, NULL,
			sizeof (SPModuleExecExt),
			16,
			(GInstanceInitFunc) sp_module_exec_ext_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE_EXEC, "SPModuleExecExt", &info, 0);
	}
	return type;
}

static void sp_module_exec_ext_class_init (SPModuleExecExtClass * klass)
{
	GObjectClass * g_object_class;
	SPModuleExecClass * module;

	g_object_class = (GObjectClass *)klass;
	module = (SPModuleExecClass *)klass;

	parent_class = g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_exec_ext_finalize;

	module->exec = sp_extension;
}

static void
sp_module_exec_ext_init (SPModuleExecExt * object)
{
	/* NOP */
}

SPModuleExecExt * sp_module_exec_ext_new (void) {
	SPModuleExecExt * retval;

	retval = (SPModuleExecExt *) g_object_new (SP_TYPE_MODULE_EXEC_EXT, NULL);
	retval->command = NULL;
	
	return retval;
}

static void sp_module_exec_ext_finalize (GObject * object)
{
	SPModuleExecExt *mexe;

	mexe = (SPModuleExecExt *) object;

	if (mexe->command != NULL) {
		g_free (mexe->command);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

gchar * sp_module_exec_ext_set_command (SPModuleExecExt * object, gchar * command) {
	g_return_val_if_fail(SP_IS_MODULE_EXEC_EXT(object), NULL);
	g_return_val_if_fail(command != NULL, NULL);

	if (object->command != NULL) {
		g_free(object->command);
	}
	object->command = g_strdup(command);
	return object->command;
}

/* Todo:  Consider splitting into some sub-functions */
static void
sp_extension(SPModule * in_plug, SPModuleDoc * in_doc)
{
#ifndef WIN32
	SPDocument * document;
        SPSelection * selection;
        SPDesktop * desktop;
        SPRepr * repr;
        GSList * rl;
/*        GSList * l; */

	FILE * ppipe;
	FILE * pfile;
	char buf[BUFSIZE];
	char command[BUFSIZE];
	char tempfilename_in_x[] = "/tmp/sp_ext_XXXXXX";
	char tempfilename_out_x[] = "/tmp/sp_ext_XXXXXX";
	char * tempfilename_in;
	char * tempfilename_out;
	char * extension_command;
	SPModuleExecExt * module_exec;

	tempfilename_in = (char *)tempfilename_in_x;
	tempfilename_out = (char *)tempfilename_out_x;
	/* Get the name of the extension to be run  */

	g_return_if_fail(SP_IS_MODULE(in_plug));
	g_return_if_fail(SP_IS_MODULE_EXEC_EXT(in_plug->exec));
	module_exec = SP_MODULE_EXEC_EXT(in_plug->exec);

	extension_command = module_exec->command;
	if (strlen(extension_command) == 0) {
	  perror("extension.c:  No command registered for this button\n");
	  return;
	}

	/* Can I get things out of the sodipodi object, globally? */
	/* Answer:  Yes.  This shows getting an item from .sodipodi/preferences */
	if (!SP_IS_MODULE_INPUT(in_plug)) {
	repr = sodipodi_get_repr (SODIPODI, "toolboxes.node");
	if (repr) {
	  gint state;
	  state = sp_repr_get_int_attribute (repr, "state", 0);
	  printf("State:  %d\n", state);
	}

        desktop = SP_ACTIVE_DESKTOP;
        if (desktop == NULL) return;
        document = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
        selection = SP_DT_SELECTION (SP_ACTIVE_DESKTOP);

        if (sp_selection_is_empty (selection)) return;

        rl = g_slist_copy ((GSList *) sp_selection_repr_list (selection));

	/* Store SVG text to a temporary file */
	if (mkstemp(tempfilename_in) == -1) {
	  /* Error, couldn't create temporary filename */
	  if (errno == EINVAL) {
	    /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
	    perror("extension.c:  template for filenames is misconfigured.\n");
	    exit(-1);	    
	  } else if (errno == EEXIST) {
	    /* Now the  contents of template are undefined. */
	    perror("extension.c:  Could not create a unique temporary filename\n");
	    return;
	  } else {
	    perror("extension.c:  Unknown error creating temporary filename\n");
	    exit(-1);
	  }
	}

	sp_repr_save_file(sp_document_repr_doc (document), tempfilename_in);

	g_slist_free (rl);

	/* Add data to document */

	sp_document_done (document);

	} else { /* if it is an input module - we're going to do the following */
		tempfilename_in = sp_module_doc_get_filename(in_doc);
	}

	/* Todo:  Replace sp_repr_save_file call with the following...
	   convert GSList * rl to a repr somehow...
	sp_repr_write_stream (SPRepr * repr, FILE * file, 0);

	*/

	/* Get the commandline to be run */
	/* Todo:  Obtain command via a lookup in registered extensions table */
	/* Todo:  Perhaps replace with a sprintf? */
	strncpy(command, extension_command, BUFSIZE);
	strncat(command, " ", BUFSIZE-strlen(command));
	strncat(command, tempfilename_in, BUFSIZE-strlen(command));

	/* Run script */
	ppipe = popen(command, "r");

	if (ppipe == NULL) {
	  /* Error - could not open pipe - check errno */
	  if (errno == EINVAL) {
	    perror("extension.c:  Invalid mode argument in popen\n");
	  } else if (errno == ECHILD) {
	    perror("extension.c:  Cannot obtain child extension status in popen\n");
	  }
	  return;
	}

	/* Store SVG text to a temporary file */
	if (mkstemp(tempfilename_out) == -1) {
	  /* Error, couldn't create temporary filename */
	  if (errno == EINVAL) {
	    /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
	    perror("extension.c:  template for filenames is misconfigured.\n");
	    exit(-1);	    
	  } else if (errno == EEXIST) {
	    /* Now the  contents of template are undefined. */
	    perror("extension.c:  Could not create a unique temporary filename\n");
	    return;
	  } else {
	    perror("extension.c:  Unknown error creating temporary filename\n");
	    exit(-1);
	  }
	}

	pfile = fopen(tempfilename_out, "w");

	if (pfile == NULL) {
	  /* Error - could not open file */
	  if (errno == EINVAL) {
	    perror("extension.c:  The mode provided to fopen was invalid\n");
	  } else {
	    perror("extension.c:  Unknown error attempting to open temporary file\n");
	  }
	  return;
	}

	/* Copy pipe output to a temporary file */
	while (fgets(buf, BUFSIZE, ppipe) != NULL) {
	  (void) fprintf(pfile, "%s", buf);
	}

	/* Close file */
	if (fclose(pfile) == EOF) {
	  if (errno == EBADF) {
	    perror("extension.c:  The filedescriptor for the temporary file is invalid\n");
	    return;
	  } else {
	    perror("extension.c:  Unknown error closing temporary file\n");
	  }
	}

	/* Close pipe */
	if (pclose(ppipe) == -1) {
	  if (errno == EINVAL) {
	    perror("extension.c:  Invalid mode set for pclose\n");
	  } else if (errno == ECHILD) {
	    perror("extension.c:  Could not obtain child status for pclose\n");
	  } else {
	    perror("extension.c:  Unknown error for pclose\n");
	  }
	}

	/* TODO:  Make a routine like sp_file_open, that can load from popen's output stream */	

	if (1 /* New document */) {
	  /* Create new document */
		SPDocument *doc;
		SPViewWidget *dtw;

		doc = sp_document_new (tempfilename_out, TRUE, TRUE);
		g_return_if_fail (doc != NULL);

		dtw = sp_desktop_widget_new (sp_document_namedview (doc, NULL));
		sp_document_unref (doc);
		g_return_if_fail (dtw != NULL);

		sp_create_window (dtw, TRUE);

		if (SP_IS_MODULE_INPUT(in_plug)) {
			sp_document_set_uri(doc, sp_module_doc_get_filename(in_doc));
		}
	}

	if (!SP_IS_MODULE_INPUT(in_plug)) {
		unlink(tempfilename_in);
	}
	unlink(tempfilename_out);

	if (1 /* Replace selection */) {
	  /* Remove selection */
	}

	return;
#endif
}

/*
<lauris> btw,
<lauris> I'd like more the idea sodipodi writing file, then executing
<lauris> my-script -f tmpfile_name
<lauris> and reading script stdout
<bryce> yup
<bryce> from the script point of view, that's the better approach too
<lauris> because this is more close, what people do executing script by hand
* bryce nods
<bryce> so what command do I run to save the current document to a file?
<bryce> I see sp_file_save_document() ?
<bryce> hmm, I probably want lower level than that
<bryce> sp_repr_save_file?
<lauris> this needs full document
<lauris> The one you have to use, is repr_write
<lauris> rename it to
<lauris> sp_repr_write_stream
<lauris> and make public
<bryce> okay
<lauris> level is just convenience identation level (i.e. 0 in given case)
<bryce> trying full document approach first...
<bryce> oh wow, it worked :-)
<bryce> oka326, next...  given a stream can I create a new document?
<lauris> nope
<lauris> look, what sp_repr_read_file does
<lauris> no
<lauris> sp_repr_read_mem
<lauris> because you do noth have filename to give to xml parser
<bryce> ok
<bryce> hmm
<lauris> hmmm...
<bryce> well for tonight maybe I can just make it write output to file and load that
<lauris> I think you should read full stream into mem
<lauris> and then use sp_repr_read_mem
<bryce> ok
<lauris> You cannot do incremental load anyways with libxml DOM tree
<bryce> ah, pity
<bryce> probably not a problem tho
*/

