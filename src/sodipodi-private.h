#ifndef __SODIPODI_PRIVATE_H__
#define __SODIPODI_PRIVATE_H__

/*
 * Some forward declarations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_SODIPODI (sodipodi_get_type ())
#define SP_SODIPODI(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_SODIPODI, Sodipodi))
#define SP_SODIPODI_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_SODIPODI, SodipodiClass))
#define SP_IS_SODIPODI(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_SODIPODI))
#define SP_IS_SODIPODI_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_SODIPODI))

#include "sodipodi.h"

GType sodipodi_get_type (void);

Sodipodi *sodipodi_new ();

void sodipodi_ref (void);
void sodipodi_unref (void);

/*
 * These are meant solely for desktop, document etc. implementations
 */

void sodipodi_selection_modified (SPSelection *selection, guint flags);
void sodipodi_selection_changed (SPSelection * selection);
void sodipodi_selection_set (SPSelection * selection);
void sodipodi_eventcontext_set (SPEventContext * eventcontext);
void sodipodi_add_desktop (SPDesktop * desktop);
void sodipodi_remove_desktop (SPDesktop * desktop);
void sodipodi_activate_desktop (SPDesktop * desktop);
void sodipodi_add_document (SPDocument *document);
void sodipodi_remove_document (SPDocument *document);

void sodipodi_set_color (SPColor *color, float opacity);

#endif


