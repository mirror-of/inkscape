#define __SP_POLYGON_C__

/*
 * SVG <polygon> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "attributes.h"
#include "sp-polygon.h"
#include "helper/sp-intl.h"
#include "svg/stringstream.h"

static void sp_polygon_class_init (SPPolygonClass *klass);
static void sp_polygon_init (SPPolygon *polygon);

static void sp_polygon_build (SPObject * object, SPDocument * document, SPRepr * repr);
static SPRepr *sp_polygon_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_polygon_set (SPObject *object, unsigned int key, const gchar *value);

static gchar *sp_polygon_description (SPItem *item);

static SPShapeClass *parent_class;

GType
sp_polygon_get_type (void)
{
	static GType polygon_type = 0;

	if (!polygon_type) {
		GTypeInfo polygon_info = {
			sizeof (SPPolygonClass),
			NULL, NULL,
			(GClassInitFunc) sp_polygon_class_init,
			NULL, NULL,
			sizeof (SPPolygon),
			16,
			(GInstanceInitFunc) sp_polygon_init,
			NULL,	/* value_table */
		};
		polygon_type = g_type_register_static (SP_TYPE_SHAPE, "SPPolygon", &polygon_info, (GTypeFlags)0);
	}
	return polygon_type;
}

static void
sp_polygon_class_init (SPPolygonClass *klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = (SPShapeClass *)g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_polygon_build;
	sp_object_class->write = sp_polygon_write;
	sp_object_class->set = sp_polygon_set;

	item_class->description = sp_polygon_description;
}

static void
sp_polygon_init (SPPolygon * polygon)
{
	/* Nothing here */
}

static void
sp_polygon_build (SPObject * object, SPDocument * document, SPRepr * repr)
{

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "points");
}

/*
 * sp_svg_write_polygon: Write points attribute for polygon tag.
 * @bpath: 
 *
 * Return value: points attribute string.
 */
static gchar *
sp_svg_write_polygon (const NArtBpath * bpath)
{
	Inkscape::SVGOStringStream os;
	int i;
	
	g_return_val_if_fail (bpath != NULL, NULL);

	for (i = 0; bpath[i].code != NR_END; i++){
		switch (bpath [i].code){
		case NR_LINETO:
		case NR_MOVETO:
		case NR_MOVETO_OPEN:
			os << bpath [i].x3 << "," << bpath [i].y3 << " ";
			break;

		case NR_CURVETO:
		default:
			g_assert_not_reached ();
		}
	}
	
	return g_strdup (os.str().c_str());
}

static SPRepr *
sp_polygon_write (SPObject *object, SPRepr *repr, guint flags)
{
        SPShape *shape;
        NArtBpath *abp;
        gchar *str;

	shape = SP_SHAPE (object);
	// Tolerable workaround: we need to update the object's curve before we set points=
	// because it's out of sync when e.g. some extension attrs of the polygon or star are changed in XML editor
	sp_shape_set_shape (shape);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("polygon");
	}

	/* We can safely write points here, because all subclasses require it too (Lauris) */
	abp = sp_curve_first_bpath (shape->curve);
	str = sp_svg_write_polygon (abp);
	sp_repr_set_attr (repr, "points", str);
	g_free (str);

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_polygon_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPPolygon *polygon;

	polygon = SP_POLYGON (object);

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

			x = g_ascii_strtod (cptr, &eptr);
			if (eptr == cptr) break;
			cptr = strchr (eptr, ',');
			if (!cptr) break;
			cptr++;
			y = g_ascii_strtod (cptr, &eptr);
			if (eptr == cptr) break;
			cptr = eptr;
			if (hascpt) {
				sp_curve_lineto (curve, x, y);
			} else {
				sp_curve_moveto (curve, x, y);
				hascpt = TRUE;
			}
		}
		
		sp_curve_closepath (curve);
		sp_shape_set_curve (SP_SHAPE (polygon), curve, TRUE);
		sp_curve_unref (curve);
		break;
	}
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static gchar *
sp_polygon_description (SPItem * item)
{
	return g_strdup ("Polygon");
}
