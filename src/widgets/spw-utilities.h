#ifndef __SPW_UTILITIES_H__
#define __SPW_UTILITIES_H__

/*
 * Sodipodi Widget Utilities
 *
 * Author:
 *   Bryce W. Harrington <brycehar@bryceharrington.com>
 * 
 * Copyright (C) 2003 Bryce Harrington
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* The following are helper routines for making Sodipodi dialog widgets.
   All are prefixed with spw_, short for sodipodi_widget.  This is not to
   be confused with SPWidget, an existing datatype associated with SPRepr/
   SPObject, that reacts to modification.
*/

void
spw_checkbutton(GtkWidget * dialog, GtkWidget * t,
		const guchar * label, guchar * key, int col, int row,
		int sensitive, GCallback cb);

void
spw_dropdown(GtkWidget * dialog, GtkWidget * t,
	     const guchar * label, guchar * key, int row,
	     GtkWidget * selector
	     );

void
spw_unit_selector(GtkWidget * dialog, GtkWidget * t,
		  const guchar * label, guchar * key, int row,
		  GtkWidget * us, GCallback cb);

#endif
