#ifndef __SP_TEXT_H__
#define __SP_TEXT_H__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

#include <sigc++/sigc++.h>

#include "forward.h"
#include "sp-item.h"
#include "sp-string.h"
#include "display/display-forward.h"
#include "libnr/nr-point.h"
#include "libnrtype/FlowSrc.h"
#include "svg/svg-types.h"


#define SP_TYPE_TEXT (sp_text_get_type ())
#define SP_TEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TEXT, SPText))
#define SP_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TEXT, SPTextClass))
#define SP_IS_TEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TEXT))
#define SP_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TEXT))

/* Text specific flags */
#define SP_TEXT_CONTENT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A
#define SP_TEXT_LAYOUT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A


class flow_src;
class flow_res;

/* SPText */

struct SPText : public SPItem {
	div_flow_src   contents;
	SPSVGLength		 x,y;
	SPSVGLength    linespacing;
	
	guint relayout : 1;
	
	flow_src*      f_src;
	flow_res*      f_res;

	void           ClearFlow(NRArenaGroup* in_arena);
	void           BuildFlow(NRArenaGroup* in_arena, NRRect *paintbox);
	void					 UpdateFlowSource(void);
	void					 ComputeFlowRes(void);
};

struct SPTextClass {
	SPItemClass parent_class;
};

#define SP_TEXT_CHILD_STRING(c) (SP_IS_TSPAN (c) ? SP_TSPAN_STRING (c) : SP_STRING (c))

GType sp_text_get_type ();

int sp_text_is_empty (SPText *text);
gchar *sp_text_get_string_multiline (SPText *text);
void sp_text_set_repr_text_multiline (SPText *text, const gchar *str);

SPCurve *sp_text_normalized_bpath (SPText *text);

/* fixme: Think about these (Lauris) */

SPTSpan *sp_text_append_line (SPText *text);
SPTSpan *sp_text_insert_line (SPText *text, gint pos);

/* This gives us SUM (strlen (STRING)) + (LINES - 1) */
gint sp_text_get_length (SPText *text);
gint sp_text_append (SPText *text, const gchar *utf8);
/* Returns position after inserted */
gint sp_text_insert (SPText *text, gint pos, const gchar *utf8);
/* Returns start position */
gint sp_text_delete (SPText *text, gint start, gint end);

gint sp_text_up (SPText *text, gint pos);
gint sp_text_down (SPText *text, gint pos);
gint sp_text_start_of_line (SPText *text, gint pos);
gint sp_text_end_of_line (SPText *text, gint pos);

void sp_text_get_cursor_coords (SPText *text, gint position, NR::Point &p0, NR::Point &p1);
guint sp_text_get_position_by_coords (SPText *text, NR::Point &p);

void sp_adjust_kerning_screen (SPText *text, gint pos, SPDesktop *desktop, NR::Point by);
void sp_adjust_tspan_letterspacing_screen (SPText *text, gint pos, SPDesktop *desktop, gdouble by);
void sp_adjust_linespacing_screen (SPText *text, SPDesktop *desktop, gdouble by);

#endif
