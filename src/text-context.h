#ifndef __SP_TEXT_CONTEXT_H__
#define __SP_TEXT_CONTEXT_H__

/*
 * SPTextContext
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*  #include <gdk/gdkic.h> */
#include <sigc++/sigc++.h>
#include <gtk/gtkimcontext.h>

#include "event-context.h"
#include <display/display-forward.h>
#include <libnr/nr-point.h>

#define SP_TYPE_TEXT_CONTEXT (sp_text_context_get_type ())
#define SP_TEXT_CONTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_TEXT_CONTEXT, SPTextContext))
#define SP_TEXT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_TEXT_CONTEXT, SPTextContextClass))
#define SP_IS_TEXT_CONTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_TEXT_CONTEXT))
#define SP_IS_TEXT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_TEXT_CONTEXT))

class SPTextContext;
class SPTextContextClass;

struct SPTextContext {
	SPEventContext event_context;

	sigc::connection sel_changed_connection;
	sigc::connection sel_modified_connection;

	GtkIMContext *imc;

	SPItem *text; // the text we're editing, or NULL if none selected

	/* Text item position in root coordinates */
	NR::Point pdoc;
	/* Insertion point position */
	int ipos;

	gchar uni[5];
	gchar unimode;
	guint unipos;

	SPCanvasItem *cursor;
	SPCanvasItem *indicator;
	gint timeout;
	guint show : 1;
	guint phase : 1;
	guint nascent_object : 1; // true if we're clicked on canvas to put cursor, but no text typed yet so ->text is still NULL

	/* Preedit String */
	gchar* preedit_string;
};

struct SPTextContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */
GtkType sp_text_context_get_type (void);

bool sp_text_paste_inline(SPEventContext *ec);

#endif
