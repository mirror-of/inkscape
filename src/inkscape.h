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
 * "desktop_deactivate"
 * "new_document"
 * "destroy_document"
 * "document_activate"
 * "document_deactivate"
 * "color_set"
 *
 */

#include "xml/repr.h"
#include "forward.h"

#define INKSCAPE inkscape_get_instance()

void inkscape_application_init (const gchar *argv0);

/* Preference management */
void inkscape_load_preferences (Inkscape::Application * inkscape);
void inkscape_save_preferences (Inkscape::Application * inkscape);
SPRepr *inkscape_get_repr (Inkscape::Application *inkscape, const gchar *key);

Inkscape::Application *inkscape_get_instance();

#define SP_ACTIVE_EVENTCONTEXT inkscape_active_event_context ()
SPEventContext * inkscape_active_event_context (void);

#define SP_ACTIVE_DOCUMENT inkscape_active_document ()
SPDocument * inkscape_active_document (void);

#define SP_ACTIVE_DESKTOP inkscape_active_desktop ()
SPDesktop * inkscape_active_desktop (void);

void inkscape_switch_desktops_next ();
void inkscape_switch_desktops_prev ();

void inkscape_dialogs_hide ();
void inkscape_dialogs_unhide ();
void inkscape_dialogs_toggle ();

/*
 * fixme: This has to be rethought
 */

void inkscape_refresh_display (Inkscape::Application *inkscape);

/*
 * fixme: This also
 */

void inkscape_exit (Inkscape::Application *inkscape);

#endif
