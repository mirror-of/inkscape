#ifndef __SODIPODI_H__
#define __SODIPODI_H__

/*
 * Interface to main application
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * sodipodi.h
 *
 * Signals:
 * "selection_changed"
 * "selection_set"
 * "eventcontext_set"
 * "new_desktop"
 * "destroy_desktop"
 * "desktop_activate"
 * "desktop_desactivate"
 * "new_document"
 * "destroy_document"
 * "document_activate"
 * "document_desactivate"
 * "color_set"
 *
 */

#include "xml/repr.h"
#include "forward.h"

#define SODIPODI sodipodi

#ifndef __SODIPODI_C__
	extern Sodipodi * sodipodi;
#endif

Sodipodi * sodipodi_application_new (void);

/* Preference management */
void sodipodi_load_preferences (Sodipodi * sodipodi);
void sodipodi_save_preferences (Sodipodi * sodipodi);
SPRepr *sodipodi_get_repr (Sodipodi *sodipodi, const unsigned char *key);

/* Extension management */
void sodipodi_load_extensions (Sodipodi *sodipodi);

#define SP_ACTIVE_EVENTCONTEXT sodipodi_active_event_context ()
SPEventContext * sodipodi_active_event_context (void);

#define SP_ACTIVE_DOCUMENT sodipodi_active_document ()
SPDocument * sodipodi_active_document (void);

#define SP_ACTIVE_DESKTOP sodipodi_active_desktop ()
SPDesktop * sodipodi_active_desktop (void);

/*
 * fixme: This has to be rethought
 */

void sodipodi_refresh_display (Sodipodi *sodipodi);

/*
 * fixme: This also
 */

void sodipodi_exit (Sodipodi *sodipodi);

#endif
