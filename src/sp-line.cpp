#define __SP_LINE_C__

/*
 * SVG <line> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <math.h>
#include <string.h>
#include "svg/svg.h"
#include "attributes.h"
#include "style.h"
#include "sp-line.h"
#include "display/curve.h"
#include "helper/sp-intl.h"
#include <libnr/nr-point.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <xml/repr.h>

static void sp_line_class_init (SPLineClass *klass);
static void sp_line_init (SPLine *line);

static void sp_line_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_line_set (SPObject *object, unsigned int key, const gchar *value);
static SPRepr *sp_line_write (SPObject *object, SPRepr *repr, guint flags);

static gchar *sp_line_description (SPItem * item);
static NR::Matrix sp_line_set_transform(SPItem *item, NR::Matrix const &xform);

static void sp_line_set_shape (SPLine * line);

static SPShapeClass *parent_class;

GType
sp_line_get_type (void)
{
	static GType line_type = 0;

	if (!line_type) {
		GTypeInfo line_info = {
			sizeof (SPLineClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_line_class_init,
			NULL,	/* klass_finalize */
			NULL,	/* klass_data */
			sizeof (SPLine),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_line_init,
			NULL,	/* value_table */
		};
		line_type = g_type_register_static (SP_TYPE_SHAPE, "SPLine", &line_info, (GTypeFlags)0);
	}
	return line_type;
}

static void
sp_line_class_init (SPLineClass *klass)
{
	parent_class = (SPShapeClass *) g_type_class_ref (SP_TYPE_SHAPE);

	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	sp_object_class->build = sp_line_build;
	sp_object_class->set = sp_line_set;
	sp_object_class->write = sp_line_write;

	SPItemClass *item_class = (SPItemClass *) klass;
	item_class->description = sp_line_description;
	item_class->set_transform = sp_line_set_transform;
}

static void
sp_line_init (SPLine * line)
{
	sp_svg_length_unset (&line->x1, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&line->y1, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&line->x2, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&line->y2, SP_SVG_UNIT_NONE, 0.0, 0.0);
}


static void
sp_line_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
        if (((SPObjectClass *) parent_class)->build) {
		((SPObjectClass *) parent_class)->build (object, document, repr);
        }

	sp_object_read_attr (object, "x1");
	sp_object_read_attr (object, "y1");
	sp_object_read_attr (object, "x2");
	sp_object_read_attr (object, "y2");
}

static void
sp_line_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPLine * line = SP_LINE (object);

	/* fixme: we should really collect updates */

	switch (key) {
	case SP_ATTR_X1:
		if (!sp_svg_length_read (value, &line->x1)) {
			sp_svg_length_unset (&line->x1, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_line_set_shape (line);
		break;
	case SP_ATTR_Y1:
		if (!sp_svg_length_read (value, &line->y1)) {
			sp_svg_length_unset (&line->y1, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_line_set_shape (line);
		break;
	case SP_ATTR_X2:
		if (!sp_svg_length_read (value, &line->x2)) {
			sp_svg_length_unset (&line->x2, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_line_set_shape (line);
		break;
	case SP_ATTR_Y2:
		if (!sp_svg_length_read (value, &line->y2)) {
			sp_svg_length_unset (&line->y2, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_line_set_shape (line);
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static SPRepr *
sp_line_write (SPObject *object, SPRepr *repr, guint flags)
{
	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("line");
	}

	if (repr != SP_OBJECT_REPR (object)) {
		sp_repr_merge (repr, SP_OBJECT_REPR (object), "id");
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static gchar *
sp_line_description (SPItem * item)
{
	return g_strdup ("Line");
}

static NR::Matrix
sp_line_set_transform (SPItem *item, NR::Matrix const &xform)
{
	SPLine *line = SP_LINE (item);
	NR::Point points[2];

	points[0] = NR::Point(line->x1.computed, line->y1.computed);
	points[1] = NR::Point(line->x2.computed, line->y2.computed);

	points[0] = points[0] * xform;
	points[1] = points[1] * xform;

	line->x1 = points[0][NR::X];
	line->y1 = points[0][NR::Y];
	line->x2 = points[1][NR::X];
	line->y2 = points[1][NR::Y];

	sp_line_set_shape(line);

	/* Scalers */
	const double sw = sqrt (xform[0] * xform[0] + xform[1] * xform[1]);
	const double sh = sqrt (xform[2] * xform[2] + xform[3] * xform[3]);

	/* Scale changed, so we have to adjust stroke width */
	if ((fabs (sw - 1.0) > 1e-9) || (fabs (sh - 1.0) > 1e-9)) {
		SPStyle *style=SP_OBJECT_STYLE (item);
		style->stroke_width.computed *= sqrt (fabs (sw * sh));
	}

	item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

	return NR::identity();
}

static void
sp_line_set_shape (SPLine * line)
{
	SPCurve *c = sp_curve_new ();

	sp_curve_moveto (c, line->x1.computed, line->y1.computed);
	sp_curve_lineto (c, line->x2.computed, line->y2.computed);

	sp_shape_set_curve (SP_SHAPE (line), c, TRUE);
	sp_curve_unref (c);
}

