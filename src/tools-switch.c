/*
 * Utility functions for switching tools (= contexts)
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

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

#include "inkscape-private.h"
#include "file.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "shortcuts.h"
#include "verbs.h"

#include "tools-switch.h"

static char *tool_names[] = {
	NULL,
  "tools.select",
  "tools.nodes",
  "tools.shapes.rect",
  "tools.shapes.arc",
  "tools.shapes.star",
  "tools.shapes.spiral",
  "tools.freehand.pencil",
  "tools.freehand.pen",
  "tools.calligraphic",
  "tools.text",
  "tools.zoom",
  "tools.dropper",
NULL
};

static char *tool_ids[] = {
	NULL,
  "select",
  "nodes",
  "rect",
  "arc",
  "star",
  "spiral",
  "pencil",
  "pen",
  "calligraphic",
  "text",
  "zoom",
  "dropper",
NULL
};

int 
tools_id2num (const char *id) 
{
  int i = 1;
  while (tool_ids[i])
    if (strcmp (tool_ids[i], id) == 0)
      return i; 
    else i++;
}

char* 
tools_num2name (int num) 
{
  return tool_names[num];
}

int
tools_isactive (SPDesktop *dt, int num)
{
	return (!strcmp (sp_repr_attr (dt->event_context->repr, "id"), tool_ids[num]));
}

int
tools_active (SPDesktop *dt)
{
	return (tools_id2num (sp_repr_attr (dt->event_context->repr, "id")));
}

void
tools_switch (SPDesktop *dt, int num)
{
	switch (num) {
	case TOOLS_SELECT:
		sp_desktop_set_event_context (dt, SP_TYPE_SELECT_CONTEXT, tool_names[num]);
		/* fixme: This is really ugly hack. We should bind and unbind class methods */
		sp_desktop_activate_guides (dt, TRUE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_NODES:
		sp_desktop_set_event_context (dt, SP_TYPE_NODE_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, TRUE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break; 
	case TOOLS_SHAPES_RECT:
		sp_desktop_set_event_context (dt, SP_TYPE_RECT_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_SHAPES_ARC:
		sp_desktop_set_event_context (dt, SP_TYPE_ARC_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_SHAPES_STAR:
		sp_desktop_set_event_context (dt, SP_TYPE_STAR_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_SHAPES_SPIRAL:
		sp_desktop_set_event_context (dt, SP_TYPE_SPIRAL_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_FREEHAND_PENCIL:
		sp_desktop_set_event_context (dt, SP_TYPE_PENCIL_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_FREEHAND_PEN:
		sp_desktop_set_event_context (dt, SP_TYPE_PEN_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_CALLIGRAPHIC:
		sp_desktop_set_event_context (dt, SP_TYPE_DYNA_DRAW_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_TEXT:
		sp_desktop_set_event_context (dt, SP_TYPE_TEXT_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_ZOOM:
		sp_desktop_set_event_context (dt, SP_TYPE_ZOOM_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case TOOLS_DROPPER:
		sp_desktop_set_event_context (dt, SP_TYPE_DROPPER_CONTEXT, tool_names[num]);
		sp_desktop_activate_guides (dt, FALSE);
		inkscape_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	}
}

void 
tools_switch_current (int num)
{
	SPDesktop *dt;
	dt = SP_ACTIVE_DESKTOP;
	tools_switch (dt, num);
}
