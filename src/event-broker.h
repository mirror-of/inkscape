#ifndef __SP_EVENT_BROKER_H__
#define __SP_EVENT_BROKER_H__

/*
 * Event context stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

void sp_event_context_set_select (void * data);
void sp_event_context_set_node_edit (void * data);
void sp_event_context_set_rect (void * data);
void sp_event_context_set_arc (void * data);
void sp_event_context_set_star (void * data);
void sp_event_context_set_spiral (void * data);
void sp_event_context_set_freehand (void * data);
void sp_event_context_set_pen (void * data);
void sp_event_context_set_dynahand (void * data);
void sp_event_context_set_text (void * data);
void sp_event_context_set_zoom (void * data);
void sp_event_context_set_dropper (void * object, void * data);

#endif
