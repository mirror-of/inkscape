#define __SP_EVENT_BROKER_C__

/*
 * Event context stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "select-context.h"
#include "node-context.h"
#include "rect-context.h"
#include "arc-context.h"
#include "star-context.h"
#include "spiral-context.h"
#include "draw-context.h"
#include "dyna-draw-context.h"
#include "text-context.h"
#include "zoom-context.h"
#include "dropper-context.h"
#include "sodipodi-private.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "event-broker.h"

void
sp_event_context_set_select (void * data)
{
	if (SP_ACTIVE_DESKTOP) {
		sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_SELECT_CONTEXT, "tools.select");
		/* fixme: This is really ugly hack. We should bind bind and unbind class methods */
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, TRUE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
	}
}

void
sp_event_context_set_node_edit (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_NODE_CONTEXT, "tools.nodes");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, TRUE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_rect (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_RECT_CONTEXT, "tools.shapes.rect");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_arc (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_ARC_CONTEXT, "tools.shapes.arc");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_star (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_STAR_CONTEXT, "tools.shapes.star");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_spiral (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_SPIRAL_CONTEXT, "tools.shapes.spiral");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_freehand (void * data)
{
	if (SP_ACTIVE_DESKTOP) {
		sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_PENCIL_CONTEXT, "tools.freehand.pencil");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
	}
}

void
sp_event_context_set_pen (void * data)
{
	if (SP_ACTIVE_DESKTOP) {
		sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_PEN_CONTEXT, "tools.freehand.pen");
		/* fixme: */
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
	}
}

void
sp_event_context_set_dynahand (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_DYNA_DRAW_CONTEXT, "tools.calligraphic");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_text (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_TEXT_CONTEXT, "tools.text");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_zoom (void * data)
{
  if (SP_ACTIVE_DESKTOP) {
    sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_ZOOM_CONTEXT, "tools.zoom");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
    sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
  }
}

void
sp_event_context_set_dropper (void *object, void *data)
{
	if (SP_ACTIVE_DESKTOP) {
		sp_desktop_set_event_context (SP_ACTIVE_DESKTOP, SP_TYPE_DROPPER_CONTEXT, "tools.dropper");
		sp_desktop_activate_guides (SP_ACTIVE_DESKTOP, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP));
	}
}
