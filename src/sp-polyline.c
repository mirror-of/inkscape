#define __SP_POLYLINE_C__

/*
 * SVG <polyline> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "attributes.h"
#include "sp-polyline.h"
#include "helper/sp-intl.h"

static void sp_polyline_class_init (SPPolyLineClass *class);
static void sp_polyline_init (SPPolyLine *polyline);

static void sp_polyline_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_polyline_set (SPObject *object, unsigned int key, const unsigned char *value);
static SPRepr *sp_polyline_write (SPObject *object, SPRepr *repr, guint flags);

static gchar * sp_polyline_description (SPItem * item);

static SPShapeClass *parent_class;

GType
sp_polyline_get_type (void)
{
	static GType polyline_type = 0;

	if (!polyline_type) {
		GTypeInfo polyline_info = {
			sizeof (SPPolyLineClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_polyline_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPPolyLine),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_polyline_init,
		};
		polyline_type = g_type_register_static (SP_TYPE_SHAPE, "SPPolyLine", &polyline_info, 0);
	}
	return polyline_type;
}

static void
sp_polyline_class_init (SPPolyLineClass *class)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	gobject_class = (GObjectClass *) class;
	sp_object_class = (SPObjectClass *) class;
	item_class = (SPItemClass *) class;

	parent_class = g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_polyline_build;
	sp_object_class->set = sp_polyline_set;
	sp_object_class->write = sp_polyline_write;

	item_class->description = sp_polyline_description;
}

static void
sp_polyline_init (SPPolyLine * polyline)
{
	/* Nothing here */
}

static void
sp_polyline_build (SPObject * object, SPDocument * document, SPRepr * repr)
{

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "points");
}

static void
sp_polyline_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPPolyLine *polyline;

	polyline = SP_POLYLINE (object);

	switch (key) {
	case SP_ATTR_POINTS: {
		SPCurve * curve;
		const gchar * cptr;
		char * eptr;
		gboolean hascpt;

		if (!value) break;
		curve = sp_curve_new ();
		hascpt = FALSE;

		cptr = value;
		eptr = NULL;

		while (TRUE) {
			gdouble x, y;

			x = strtod (cptr, &eptr);
			if (eptr == cptr) break;
			cptr = strchr (eptr, ',');
			if (!cptr) break;
			cptr++;
			y = strtod (cptr, &eptr);
			if (eptr == cptr) break;
			cptr = eptr;
			if (hascpt) {
				sp_curve_lineto (curve, x, y);
			} else {
				sp_curve_moveto (curve, x, y);
				hascpt = TRUE;
			}
		}
		
		sp_shape_set_curve (SP_SHAPE (polyline), curve, TRUE);
		sp_curve_unref (curve);
		break;
	}
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static SPRepr *
sp_polyline_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPPolyLine *polyline;

	polyline = SP_POLYLINE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("polyline");
	}

	if (repr != SP_OBJECT_REPR (object)) {
		sp_repr_merge (repr, SP_OBJECT_REPR (object), "id");
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static gchar *
sp_polyline_description (SPItem * item)
{
	return g_strdup ("PolyLine");
}

