#ifndef __SP_FILE_H__
#define __SP_FILE_H__

/*
 * File/Print operations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Chema Celorio <chema@celorio.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include <extension/extension.h>

/*######################
## N E W
######################*/

/**
 * Creates a new Inkscape document and window
 */
void sp_file_new (const gchar *templ);
void sp_file_new_default (void);

/*######################
## D E L E T E
######################*/

/**
 * Close the document/view
 */
void sp_file_exit (void);

/*######################
## O P E N
######################*/

/**
 * Opens a new file and window from the given URI
 */
bool sp_file_open (const gchar *uri, Inkscape::Extension::Extension *key);

/**
 * Displays a file open dialog. Calls sp_file_open on
 * an OK.
 */
void sp_file_open_dialog (gpointer object, gpointer data);

/**
 * Reverts file to disk-copy on "YES"
 */
void sp_file_revert_dialog ();

/*######################
## S A V E
######################*/

/**
 *
 */
bool sp_file_save (gpointer object, gpointer data);

/**
 *  Saves the given document.  Displays a file select dialog
 *  to choose the new name.
 */
bool sp_file_save_as (gpointer object, gpointer data);

/**
 *  Saves the given document.  Displays a file select dialog
 *  if needed.
 */
gboolean sp_file_save_document (SPDocument *document);

/* Do the saveas dialog with a document as the parameter */
gboolean sp_file_save_dialog (SPDocument *doc);


/*######################
## I M P O R T
######################*/

/**
 * Displays a file selector dialog, to allow the
 * user to import data into the current document.
 */
void sp_file_import (GtkWidget * widget);


/*######################
## E X P O R T
######################*/

/**
 * Displays a "Save as" dialog for the user, with an
 * additional type selection, to allow the user to export
 * the a document as a given type.
 */
void sp_file_export_dialog (void *widget);

/**
 * Export the given document as a Portable Network Graphics (PNG)
 * file.
 */
void sp_export_png_file (SPDocument *doc, const gchar *filename,
			 double x0, double y0, double x1, double y1,
			 unsigned int width, unsigned int height,
			 unsigned long bgcolor,
			 unsigned int (*status) (float, void *), void *data, bool force_overwrite = false);


/*######################
## P R I N T
######################*/

/* These functions are redundant now, but
would be useful as instance methods
*/

/**
 *
 */
void sp_file_print (void);

/**
 *
 */
void sp_file_print_direct (void);

/**
 *
 */
void sp_file_print_preview (gpointer object, gpointer data);

/*#####################
## U T I L I T Y
#####################*/

/**
 * clean unused defs out of file
 */
void sp_file_vacuum ();


#endif
