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
#include <errno.h>
#include "svg/svg.h"
#include "xml/repr-private.h"
#include "document.h"
#include "sodipodi.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "extension.h"
#include "file.h"

#define BUFSIZE (255)

/* Structure defining a single extension */
typedef struct _SPExtension SPExtension;

struct SPExtension {
  gchar * name;
  gchar * command;
  gchar * tooltip;
  gchar * pixmap;
};

/* Hashmap (dictionary) for the extension registry */
/*
int sp_extension_index (const guchar *str)
{
  static GHashTable *extension_registry = NULL;

  if (!extension_registry) {
    const SPExtension * ext;
    ext = g_hash_table_new (g_str_hash, g_str_equal);
    for (ext = exts; ext->code != SP_EXTENSION_INVALID; ext++) {
      g_hash_table_insert (extension_registry, ext->name, GINT_TO_POINTER (ext->code));
    }
  }

  return GPOINTER_TO_INT (g_hash_table_lookup (extension_registry, str));
}
*/

/* Todo:  Consider splitting into some sub-functions */
void sp_extension(GtkWidget * widget)
{
        SPDocument * document;
        SPSelection * selection;
        SPDesktop * desktop;
        SPRepr * repr;
        GSList * rl;
	unsigned char *path;
/*        GSList * l; */

	FILE * ppipe;
	FILE * pfile;
	char buf[BUFSIZE];
	char command[BUFSIZE];
	char tempfilename[] = "sp_ext_XXXXXX";
	char tempfilename2[] = "sp_ext_XXXXXX";

	/* Get the name of the extension to be run  */
	char extension_name[BUFSIZE];

	strncpy(extension_name, gtk_widget_get_name(widget), BUFSIZE);
	if (strlen(extension_name) == 0) {
	  perror("extension.c:  No command registered for this button\n");
	  return;
	}

	path = g_strdup_printf ("extensions.%s", extension_name);
	repr = sodipodi_get_repr (SODIPODI, path);
	g_free (path);
	if (! repr) {
	  printf("Error: invalid extension %s\n", extension_name);
	  /* Todo:  Popup error dialog box */
	  return;
	}

        desktop = SP_ACTIVE_DESKTOP;
        if (desktop == NULL) return;
        document = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
        selection = SP_DT_SELECTION (SP_ACTIVE_DESKTOP);

        if (sp_selection_is_empty (selection)) return;

        rl = g_slist_copy ((GSList *) sp_selection_repr_list (selection));

#ifndef WIN32
	/* Store SVG text to a temporary file */
	if (mkstemp(tempfilename) == -1) {
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
#else
	/* Store SVG text to a temporary file */
	if (_mktemp(tempfilename) == -1) {
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
#endif

	sp_repr_save_file(sp_document_repr_doc (document), tempfilename);

	/* Todo:  Replace sp_repr_save_file call with the following...
	   convert GSList * rl to a repr somehow...
	sp_repr_write_stream (SPRepr * repr, FILE * file, 0);

	*/

	/* Get the commandline to be run */
	strncpy(command, sp_repr_attr(repr, "executable_filename"), BUFSIZE);
	strncat(command, " ", BUFSIZE-strlen(command));
	strncat(command, tempfilename, BUFSIZE-strlen(command));

	/* Run script */
#ifndef WIN32
	ppipe = popen (command, "r");
#else
	ppipe = _popen (command, "r");
#endif

	if (ppipe == NULL) {
	  /* Error - could not open pipe - check errno */
	  if (errno == EINVAL) {
	    perror("extension.c:  Invalid mode argument in popen\n");
	  } else if (errno == ECHILD) {
	    perror("extension.c:  Cannot obtain child extension status in popen\n");
	  }
	  return;
	}

#ifndef WIN32
	/* Store SVG text to a temporary file */
	if (mkstemp(tempfilename2) == -1) {
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
#else
	/* Store SVG text to a temporary file */
	if (_mktemp(tempfilename2) == -1) {
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
#endif

	pfile = fopen(tempfilename2, "w");

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
#ifndef WIN32
	if (pclose(ppipe) == -1) {
	  if (errno == EINVAL) {
	    perror("extension.c:  Invalid mode set for pclose\n");
	  } else if (errno == ECHILD) {
	    perror("extension.c:  Could not obtain child status for pclose\n");
	  } else {
	    perror("extension.c:  Unknown error for pclose\n");
	  }
	}
#else
	if (_pclose (ppipe) == -1) {
	  if (errno == EINVAL) {
	    perror("extension.c:  Invalid mode set for pclose\n");
	  } else if (errno == ECHILD) {
	    perror("extension.c:  Could not obtain child status for pclose\n");
	  } else {
	    perror("extension.c:  Unknown error for pclose\n");
	  }
	}
#endif

	/* TODO:  Make a routine like sp_file_open, that can load from popen's output stream */	

	if (1 /* New document */) {
		/* Create new document */
		sp_file_open (tempfilename2, NULL);
	}

	if (1 /* Replace selection */) {
	  /* Remove selection */
	}

	/* Add data to document */

        g_slist_free (rl);

        sp_document_done (document);
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

