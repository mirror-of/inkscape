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
#include "sp-metadata.h"
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

enum NameType { REPR_NAME, SODIPODI_TYPE };
static unsigned const N_NAME_TYPES = SODIPODI_TYPE + 1;

static GType name_to_gtype(NameType name_type, gchar const *name);

SPObject *
sp_object_repr_build_tree (SPDocument *document, SPRepr *repr)
{
	g_assert (document != NULL);
	g_assert (SP_IS_DOCUMENT (document));
	g_assert (repr != NULL);

	gchar const * const name = sp_repr_name(repr);
	g_assert (name != NULL);
	GType const type = name_to_gtype(REPR_NAME, name);
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
		return ( type_name
			 ? name_to_gtype(SODIPODI_TYPE, type_name)
			 : name_to_gtype(REPR_NAME, sp_repr_name(repr)) );
	} else {
		return 0;
	}
}

static GHashTable *t2dtable[N_NAME_TYPES] = {NULL};

static void
populate_dtables()
{
	struct NameTypeEntry { char const *const name; GType const type_id; };
	NameTypeEntry const repr_name_entries[] = {
		{ "a", SP_TYPE_ANCHOR },
		{ "animate", SP_TYPE_ANIMATE },
		{ "circle", SP_TYPE_CIRCLE },
		{ "clipPath", SP_TYPE_CLIPPATH },
		{ "defs", SP_TYPE_DEFS },
		{ "ellipse", SP_TYPE_ELLIPSE },
		/* Note: flow* elements are proposed additions for SVG 1.2, they aren't in
		   SVG 1.1. */
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
		{ "line", SP_TYPE_LINE },
		{ "linearGradient", SP_TYPE_LINEARGRADIENT },
		{ "marker", SP_TYPE_MARKER },
		{ "mask", SP_TYPE_MASK },
		{ "metadata", SP_TYPE_METADATA },
		{ "path", SP_TYPE_PATH },
		{ "pattern", SP_TYPE_PATTERN },
		{ "polygon", SP_TYPE_POLYGON },
		{ "polyline", SP_TYPE_POLYLINE },
		{ "radialGradient", SP_TYPE_RADIALGRADIENT },
		{ "rect", SP_TYPE_RECT },
		{ "stop", SP_TYPE_STOP },
		{ "svg", SP_TYPE_ROOT },
		{ "switch", SP_TYPE_GROUP },
		{ "symbol", SP_TYPE_SYMBOL },
		{ "text", SP_TYPE_TEXT },
		{ "textPath", SP_TYPE_TEXTPATH },
		{ "tspan", SP_TYPE_TSPAN },
		{ "use", SP_TYPE_USE }
	};
	NameTypeEntry const sodipodi_name_entries[] = {
		{ "arc", SP_TYPE_ARC },
		{ "inkscape:offset", SP_TYPE_OFFSET },
		{ "spiral", SP_TYPE_SPIRAL },
		{ "star", SP_TYPE_STAR }
	};

	NameTypeEntry const *const t2entries[] = {
		repr_name_entries,
		sodipodi_name_entries
	};
	unsigned const t2n_entries[] = {
		G_N_ELEMENTS(repr_name_entries),
		G_N_ELEMENTS(sodipodi_name_entries)
	};

	for (unsigned nt = 0; nt < N_NAME_TYPES; ++nt) {
		NameTypeEntry const *const entries = t2entries[nt];
		unsigned const n_entries = t2n_entries[nt];
		GHashTable *&dtable = t2dtable[nt];

		dtable = g_hash_table_new(g_str_hash, g_str_equal);
		for (unsigned i = 0; i < n_entries; ++i) {
			g_hash_table_insert(dtable,
					    (void *)entries[i].name,
					    GINT_TO_POINTER(entries[i].type_id));
		}
	}
}

static inline void
ensure_dtables_populated()
{
	if (!*t2dtable) {
		populate_dtables();
	}
}

static GType
name_to_gtype(NameType const name_type, gchar const *name)
{
	ensure_dtables_populated();

	gpointer const data = g_hash_table_lookup(t2dtable[name_type], name);
	return ( ( data == NULL )
		 ? SP_TYPE_OBJECT
		 : GPOINTER_TO_INT(data) );
}

void
sp_object_type_register(gchar const *name, GType const gtype)
{
	GType const current = name_to_gtype(REPR_NAME, name);
	if (current == SP_TYPE_OBJECT) {
		g_hash_table_insert(t2dtable[REPR_NAME],
				    const_cast<gchar *>(name),
				    GINT_TO_POINTER(gtype));
	} else {
		/* Already registered. */
		if (current != gtype) {
			g_warning("repr type `%s' already registered as type #%lu, ignoring attempt to re-register as #%lu.",
				  name, current, gtype);
		}
	}
}
