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

#include <glib.h>

#include <sigc++/sigc++.h>


#define SP_TYPE_TEXT (sp_text_get_type ())
#define SP_TEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TEXT, SPText))
#define SP_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TEXT, SPTextClass))
#define SP_IS_TEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TEXT))
#define SP_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TEXT))

#define SP_TYPE_TSPAN (sp_tspan_get_type ())
#define SP_TSPAN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TSPAN, SPTSpan))
#define SP_TSPAN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TSPAN, SPTSpanClass))
#define SP_IS_TSPAN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TSPAN))
#define SP_IS_TSPAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TSPAN))

#define SP_TYPE_TEXTPATH (sp_textpath_get_type ())
#define SP_TEXTPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_TEXTPATH, SPTextPath))
#define SP_TEXTPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_TEXTPATH, SPTextPathClass))
#define SP_IS_TEXTPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_TEXTPATH))
#define SP_IS_TEXTPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_TEXTPATH))

#define SP_TYPE_STRING (sp_string_get_type ())
#define SP_STRING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_STRING, SPString))
#define SP_STRING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_STRING, SPStringClass))
#define SP_IS_STRING(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_STRING))
#define SP_IS_STRING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_STRING))

/* Text specific flags */
#define SP_TEXT_CONTENT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A
#define SP_TEXT_LAYOUT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A

#define SP_TSPAN_STRING(t) ((SPString *) SP_TSPAN (t)->string)
#define SP_TEXTPATH_STRING(t) ((SPString *) SP_TEXTPATH (t)->string)

#include <libnr/nr-point.h>
#include "svg/svg-types.h"
#include "sp-chars.h"

class SPLayoutData;
class SPUseReference;

struct SPLayoutData {
	/* fixme: Vectors */
	SPSVGLength x;
	SPSVGLength y;
	GList *dx; // list of SPSVGLength
	GList *dy; // list of SPSVGLength
	unsigned int rotate_set : 1;
	float rotate;
	float linespacing;
};

/*
 * The ultimate source of current mess is, that we have to derive string <- chars
 * This will be changed as soon as we have NRArenaGlyphList class
 */

/* SPString */

struct SPString {
	SPChars chars;
	/* Link to parent layout */
	SPLayoutData *ly;
	/* Content */
	gchar *text;
	NR::Point *p;
	/* Bookkeeping */
	guint start;
	guint length;
	/* Using current direction and style */
	NRRect bbox;
	NR::Point advance;
};

struct SPStringClass {
	SPCharsClass parent_class;
};

#define SP_STRING_TEXT(s) (SP_STRING (s)->text)

GType sp_string_get_type ();

/* SPTSpan */

enum {
	SP_TSPAN_ROLE_UNSPECIFIED,
	SP_TSPAN_ROLE_PARAGRAPH,
	SP_TSPAN_ROLE_LINE
};

struct SPTSpan {
	SPItem item;

	guint role : 2;

	SPLayoutData ly;

	SPObject *string;
};

struct SPTSpanClass {
	SPItemClass parent_class;
};

GType sp_tspan_get_type ();

/* SPTextPath */

class Path;
struct SPTextPath {
	SPItem        item;
	SPLayoutData  ly;
	SPObject      *string;

  Path           *originalPath; // will be a livarot Path, just don't declare it here to please the gcc linker
  char           *original;     // SVG description of the source path

	bool           sourceDirty;
	bool           isUpdating;
	
	gchar					 *sourceHref;
	SPUseReference *sourceRef;
  SPRepr         *sourceRepr; // the repr associated with that id
	SPObject			 *sourceObject;
	
	gulong           _modified_connection;
	SigC::Connection _delete_connection;
	SigC::Connection _changed_connection;
};

struct SPTextPathClass {
	SPItemClass  parent_class;
};

GType sp_textpath_get_type();

/* SPText */

struct SPText {
	SPItem item;

	SPLayoutData ly;

	guint relayout : 1;
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
