#ifndef __INKSCAPE_H__
#define __INKSCAPE_H__

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
 * inkscape.h
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

#define INKSCAPE inkscape

#ifndef __INKSCAPE_C__
	extern Inkscape * inkscape;
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

Inkscape * inkscape_application_new (void);

/* Preference management */
void inkscape_load_preferences (Inkscape * inkscape);
void inkscape_save_preferences (Inkscape * inkscape);
SPRepr *inkscape_get_repr (Inkscape *inkscape, const gchar *key);

/* Extension management */
void inkscape_load_extensions (Inkscape *inkscape);

#define SP_ACTIVE_EVENTCONTEXT inkscape_active_event_context ()
SPEventContext * inkscape_active_event_context (void);

#define SP_ACTIVE_DOCUMENT inkscape_active_document ()
SPDocument * inkscape_active_document (void);

#define SP_ACTIVE_DESKTOP inkscape_active_desktop ()
SPDesktop * inkscape_active_desktop (void);

#ifdef __cplusplus
};
#endif // __cplusplus

/*
 * fixme: This has to be rethought
 */

void inkscape_refresh_display (Inkscape *inkscape);

/*
 * fixme: This also
 */

void inkscape_exit (Inkscape *inkscape);

#endif
