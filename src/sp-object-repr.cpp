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
#include "sp-linear-gradient-fns.h"
#include "sp-path.h" 
#include "sp-radial-gradient-fns.h"
#include "sp-rect.h" 
#include "sp-ellipse.h"
#include "sp-star.h" 
#include "sp-stop-fns.h"
#include "sp-spiral.h" 
#include "sp-offset.h"
#include "sp-line.h"
#include "sp-polyline.h"
#include "sp-polygon.h"
#include "sp-text.h" 
#include "sp-tspan.h" 
#include "sp-pattern.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-anchor.h"
#include "sp-animation.h"
#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"
#include "sp-object-repr.h"
#include "xml/repr-private.h"

SPObject *
sp_object_repr_build_tree (SPDocument *document, SPRepr *repr)
{
	g_assert (document != NULL);
	g_assert (SP_IS_DOCUMENT (document));
	g_assert (repr != NULL);

	gchar const * const name = sp_repr_name(repr);
	g_assert (name != NULL);
	GType const type = sp_object_type_lookup(name);
	g_assert (g_type_is_a (type, SP_TYPE_ROOT));
        gpointer newobj = g_object_new(type, 0);
	g_assert(newobj != NULL);
	SPObject * const object = SP_OBJECT(newobj);
	g_assert(object != NULL);
	sp_object_invoke_build(object, document, repr, FALSE);

	return object;
}

GType
sp_repr_type_lookup (SPRepr *repr)
{
	if ( repr->type == SP_XML_TEXT_NODE ) {
		return SP_TYPE_STRING;
	} else if ( repr->type == SP_XML_ELEMENT_NODE ) {
		gchar const * const type_name = sp_repr_attr(repr, "sodipodi:type");
		gchar const * const name = ( type_name
					     ? type_name
					     : sp_repr_name(repr) );

		return sp_object_type_lookup(name);
	} else {
		return 0;
	}
}

static GHashTable *dtable = NULL;

GType
sp_object_type_lookup (const gchar * name)
{
	if (!dtable) {
		struct { char const *const name; GType const type_id; } const entries[] = {
			{ "a", SP_TYPE_ANCHOR },
			{ "animate", SP_TYPE_ANIMATE },
			{ "arc", SP_TYPE_ARC },
			{ "circle", SP_TYPE_CIRCLE },
			{ "clipPath", SP_TYPE_CLIPPATH },
			{ "defs", SP_TYPE_DEFS },
			{ "ellipse", SP_TYPE_ELLIPSE },
			{ "flowDiv", SP_TYPE_FLOWDIV },
			{ "flowLine", SP_TYPE_FLOWLINE },
			{ "flowPara", SP_TYPE_FLOWPARA },
			{ "flowRegion", SP_TYPE_FLOWREGION },
			{ "flowRegionBreak", SP_TYPE_FLOWREGIONBREAK },
			{ "flowRegionExclude", SP_TYPE_FLOWREGIONEXCLUDE },
			{ "flowRoot", SP_TYPE_FLOWTEXT },
			{ "flowSpan", SP_TYPE_FLOWTSPAN },
			{ "g", SP_TYPE_GROUP },
			{ "image", SP_TYPE_IMAGE },
			{ "inkscape:offset", SP_TYPE_OFFSET },
			{ "line", SP_TYPE_LINE },
			{ "linearGradient", SP_TYPE_LINEARGRADIENT },
			{ "marker", SP_TYPE_MARKER },
			{ "mask", SP_TYPE_MASK },
			{ "path", SP_TYPE_PATH },
			{ "pattern", SP_TYPE_PATTERN },
			{ "polygon", SP_TYPE_POLYGON },
			{ "polyline", SP_TYPE_POLYLINE },
			{ "radialGradient", SP_TYPE_RADIALGRADIENT },
			{ "rect", SP_TYPE_RECT },
			{ "spiral", SP_TYPE_SPIRAL },
			{ "star", SP_TYPE_STAR },
			{ "stop", SP_TYPE_STOP },
			{ "svg", SP_TYPE_ROOT },
			{ "switch", SP_TYPE_GROUP },
			{ "symbol", SP_TYPE_SYMBOL },
			{ "text", SP_TYPE_TEXT },
			{ "textPath", SP_TYPE_TEXTPATH },
			{ "tspan", SP_TYPE_TSPAN },
			{ "use", SP_TYPE_USE }
		};
		dtable = g_hash_table_new(g_str_hash, g_str_equal);
		for (unsigned i = 0; i < G_N_ELEMENTS(entries); ++i) {
			g_hash_table_insert(dtable,
					    (void *)entries[i].name,
					    GINT_TO_POINTER(entries[i].type_id));
		}
	}

	gpointer const data = g_hash_table_lookup(dtable, name);
	return ( ( data == NULL )
		 ? SP_TYPE_OBJECT
		 : GPOINTER_TO_INT(data) );
}

void
sp_object_type_register(gchar const *name, GType const gtype)
{
	GType const current = sp_object_type_lookup(name);
	if (current == SP_TYPE_OBJECT) {
		g_hash_table_insert(dtable, const_cast<gchar *>(name), GINT_TO_POINTER(gtype));
	} else {
		/* Already registered. */
		if (current != gtype) {
			g_warning("repr type `%s' already registered as type #%lu, ignoring attempt to re-register as #%lu.",
				  name, current, gtype);
		}
	}
}
