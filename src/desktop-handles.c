#define __SP_DESKTOP_HANDLES_C__

/*
 * Frontends
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "helper/sp-canvas.h"
#include "desktop.h"
#include "desktop-handles.h"

SPEventContext *
sp_desktop_event_context (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->event_context;
}

SPSelection *
sp_desktop_selection (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->selection;
}

SPDocument *
sp_desktop_document (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return SP_VIEW_DOCUMENT (desktop);
}

SPCanvas *
sp_desktop_canvas (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return ((SPCanvasItem *) desktop->main)->canvas;
}

SPCanvasItem *
sp_desktop_acetate (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->acetate;
}

SPCanvasGroup *
sp_desktop_main (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->main;
}

SPCanvasGroup *
sp_desktop_grid (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->grid;
}

SPCanvasGroup *
sp_desktop_guides (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->guides;
}

SPCanvasItem *
sp_desktop_drawing (SPDesktop *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->drawing;
}

SPCanvasGroup *
sp_desktop_sketch (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->sketch;
}

SPCanvasGroup *
sp_desktop_controls (SPDesktop * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);
	g_return_val_if_fail (SP_IS_DESKTOP (desktop), NULL);

	return desktop->controls;
}

