#ifndef __SP_DESKTOP_HANDLES_H__
#define __SP_DESKTOP_HANDLES_H__

/*
 * Frontends
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "helper/helper-forward.h"
#include "forward.h"

#define SP_DT_IS_EDITABLE(d) (TRUE)

#define SP_DT_EVENTCONTEXT(d) sp_desktop_event_context (d)
#define SP_DT_SELECTION(d) sp_desktop_selection (d)
#define SP_DT_DOCUMENT(d) sp_desktop_document (d)
#define SP_DT_CANVAS(d) sp_desktop_canvas (d)
#define SP_DT_ACETATE(d) sp_desktop_acetate (d)
#define SP_DT_MAIN(d) sp_desktop_main (d)
#define SP_DT_GRID(d) sp_desktop_grid (d)
#define SP_DT_GUIDES(d) sp_desktop_guides (d)
#define SP_DT_DRAWING(d) sp_desktop_drawing (d)
#define SP_DT_SKETCH(d) sp_desktop_sketch (d)
#define SP_DT_CONTROLS(d) sp_desktop_controls (d)

SPEventContext * sp_desktop_event_context (SPDesktop * desktop);
SPSelection * sp_desktop_selection (SPDesktop * desktop);
SPDocument * sp_desktop_document (SPDesktop * desktop);
SPCanvas * sp_desktop_canvas (SPDesktop * desktop);
SPCanvasItem * sp_desktop_acetate (SPDesktop * desktop);
SPCanvasGroup * sp_desktop_main (SPDesktop * desktop);
SPCanvasGroup * sp_desktop_grid (SPDesktop * desktop);
SPCanvasGroup * sp_desktop_guides (SPDesktop * desktop);
SPCanvasItem *sp_desktop_drawing (SPDesktop *desktop);
SPCanvasGroup * sp_desktop_sketch (SPDesktop * desktop);
SPCanvasGroup * sp_desktop_controls (SPDesktop * desktop);

#endif
