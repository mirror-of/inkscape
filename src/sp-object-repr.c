#define __SP_OBJECT_REPR_C__

/*
 * Object type dictionary and build frontend
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "document.h"
#include "sp-item.h"
#include "sp-defs.h"
#include "sp-symbol.h"
#include "sp-marker.h"
#include "sp-use.h"
#include "sp-root.h"
#include "sp-namedview.h"
#include "sp-guide.h"
#include "sp-image.h"
#include "sp-path.h" 
#include "sp-rect.h" 
#include "sp-ellipse.h"
#include "sp-star.h" 
#include "sp-spiral.h" 
#include "sp-line.h"
#include "sp-polyline.h"
#include "sp-polygon.h"
#include "sp-text.h" 
#include "sp-gradient.h"
#include "sp-pattern.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-anchor.h"
#include "sp-animation.h"
#include "sp-object-repr.h"

SPObject *
sp_object_repr_build_tree (SPDocument *document, SPRepr *repr)
{
	const gchar * name;
	GType type;
	SPObject * object;

	g_assert (document != NULL);
	g_assert (SP_IS_DOCUMENT (document));
	g_assert (repr != NULL);

	name = sp_repr_name (repr);
	g_assert (name != NULL);
	type = sp_object_type_lookup (name);
	g_assert (g_type_is_a (type, SP_TYPE_ROOT));
	object = g_object_new (type, 0);
	g_assert (object != NULL);
	sp_object_invoke_build (object, document, repr, FALSE);

	return object;
}

GType
sp_repr_type_lookup (SPRepr *repr)
{
	const guchar *name;

	name = sp_repr_attr (repr, "sodipodi:type");
	if (!name) name = sp_repr_name (repr);

	return sp_object_type_lookup (name);
}

static GHashTable *dtable = NULL;

GType
sp_object_type_lookup (const guchar * name)
{
	gpointer data;

	if (!dtable) {
		dtable = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (dtable, "a", GINT_TO_POINTER (SP_TYPE_ANCHOR));
		g_hash_table_insert (dtable, "animate", GINT_TO_POINTER (SP_TYPE_ANIMATE));
		g_hash_table_insert (dtable, "arc", GINT_TO_POINTER (SP_TYPE_ARC));
		g_hash_table_insert (dtable, "circle", GINT_TO_POINTER (SP_TYPE_CIRCLE));
		g_hash_table_insert (dtable, "clipPath", GINT_TO_POINTER (SP_TYPE_CLIPPATH));
		g_hash_table_insert (dtable, "defs", GINT_TO_POINTER (SP_TYPE_DEFS));
		g_hash_table_insert (dtable, "ellipse", GINT_TO_POINTER (SP_TYPE_ELLIPSE));
		g_hash_table_insert (dtable, "g", GINT_TO_POINTER (SP_TYPE_GROUP));
		g_hash_table_insert (dtable, "image", GINT_TO_POINTER (SP_TYPE_IMAGE));
		g_hash_table_insert (dtable, "line", GINT_TO_POINTER (SP_TYPE_LINE));
		g_hash_table_insert (dtable, "linearGradient", GINT_TO_POINTER (SP_TYPE_LINEARGRADIENT));
		g_hash_table_insert (dtable, "mask", GINT_TO_POINTER (SP_TYPE_MASK));
		g_hash_table_insert (dtable, "marker", GINT_TO_POINTER (SP_TYPE_MARKER));
		g_hash_table_insert (dtable, "path", GINT_TO_POINTER (SP_TYPE_PATH));
		g_hash_table_insert (dtable, "pattern", GINT_TO_POINTER (SP_TYPE_PATTERN));
		g_hash_table_insert (dtable, "polygon", GINT_TO_POINTER (SP_TYPE_POLYGON));
		g_hash_table_insert (dtable, "polyline", GINT_TO_POINTER (SP_TYPE_POLYLINE));
		g_hash_table_insert (dtable, "radialGradient", GINT_TO_POINTER (SP_TYPE_RADIALGRADIENT));
		g_hash_table_insert (dtable, "rect", GINT_TO_POINTER (SP_TYPE_RECT));
		g_hash_table_insert (dtable, "spiral", GINT_TO_POINTER (SP_TYPE_SPIRAL));
		g_hash_table_insert (dtable, "star", GINT_TO_POINTER (SP_TYPE_STAR));
		g_hash_table_insert (dtable, "stop", GINT_TO_POINTER (SP_TYPE_STOP));
		g_hash_table_insert (dtable, "svg", GINT_TO_POINTER (SP_TYPE_ROOT));
		g_hash_table_insert (dtable, "symbol", GINT_TO_POINTER (SP_TYPE_SYMBOL));
		g_hash_table_insert (dtable, "text", GINT_TO_POINTER (SP_TYPE_TEXT));
		g_hash_table_insert (dtable, "use", GINT_TO_POINTER (SP_TYPE_USE));
	}

	data = g_hash_table_lookup (dtable, name);

	if (data == NULL) return SP_TYPE_OBJECT;

	return  (GType)data;
}

/* Return TRUE on success */

GType
sp_object_type_register (const unsigned char *name, GType type)
{
	GType current;
	current = sp_object_type_lookup (name);
	if (current != SP_TYPE_OBJECT) return FALSE;
	g_hash_table_insert (dtable, (char *) name, GINT_TO_POINTER (type));
	return TRUE;
}
