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

#include <math.h>
#include <string.h>
#include "svg/svg.h"
#include "attributes.h"
#include "style.h"
#include "sp-line.h"
#include "helper/sp-intl.h"

#define hypot(a,b) sqrt ((a) * (a) + (b) * (b))

static void sp_line_class_init (SPLineClass *class);
static void sp_line_init (SPLine *line);

static void sp_line_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_line_set (SPObject *object, unsigned int key, const unsigned char *value);
static SPRepr *sp_line_write (SPObject *object, SPRepr *repr, guint flags);

static gchar *sp_line_description (SPItem * item);
static void sp_line_write_transform (SPItem *item, SPRepr *repr, NRMatrixF *t);

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
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPLine),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_line_init,
		};
		line_type = g_type_register_static (SP_TYPE_SHAPE, "SPLine", &line_info, 0);
	}
	return line_type;
}

static void
sp_line_class_init (SPLineClass *class)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	gobject_class = (GObjectClass *) class;
	sp_object_class = (SPObjectClass *) class;
	item_class = (SPItemClass *) class;

	parent_class = g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_line_build;
	sp_object_class->set = sp_line_set;
	sp_object_class->write = sp_line_write;

	item_class->description = sp_line_description;
	item_class->write_transform = sp_line_write_transform;
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

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "x1");
	sp_object_read_attr (object, "y1");
	sp_object_read_attr (object, "x2");
	sp_object_read_attr (object, "y2");
}

static void
sp_line_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPLine * line;

	line = SP_LINE (object);

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
	SPLine *line;

	line = SP_LINE (object);

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

static void
sp_line_write_transform (SPItem *item, SPRepr *repr, NRMatrixF *t)
{
	double sw, sh;
	SPLine *line;

	line = SP_LINE (item);

	/* fixme: Would be nice to preserve units here */
	sp_repr_set_double (repr, "x1", t->c[0] * line->x1.computed + t->c[2] * line->y1.computed + t->c[4]);
	sp_repr_set_double (repr, "y1", t->c[1] * line->x1.computed + t->c[3] * line->y1.computed + t->c[5]);
	sp_repr_set_double (repr, "x2", t->c[0] * line->x2.computed + t->c[2] * line->y2.computed + t->c[4]);
	sp_repr_set_double (repr, "y2", t->c[1] * line->x2.computed + t->c[3] * line->y2.computed + t->c[5]);

	/* Scalers */
	sw = sqrt (t->c[0] * t->c[0] + t->c[1] * t->c[1]);
	sh = sqrt (t->c[2] * t->c[2] + t->c[3] * t->c[3]);

	/* And last but not least */
	if ((fabs (sw - 1.0) > 1e-9) || (fabs (sh - 1.0) > 1e-9)) {
		SPStyle *style;
		guchar *str;
		/* Scale changed, so we have to adjust stroke width */
		style = SP_OBJECT_STYLE (item);
		style->stroke_width.computed *= sqrt (fabs (sw * sh));
		str = sp_style_write_difference (style, SP_OBJECT_STYLE (SP_OBJECT_PARENT (item)));
		sp_repr_set_attr (SP_OBJECT_REPR (item), "style", str);
		g_free (str);
	}
}

static void
sp_line_set_shape (SPLine * line)
{
	SPCurve * c;
	
	if (hypot (line->x2.computed - line->x1.computed, line->y2.computed - line->y1.computed) < 1e-12) return;

	c = sp_curve_new ();

	sp_curve_moveto (c, line->x1.computed, line->y1.computed);
	sp_curve_lineto (c, line->x2.computed, line->y2.computed);

	sp_shape_set_curve (SP_SHAPE (line), c, TRUE);
	sp_curve_unref (c);
}

