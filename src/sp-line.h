#ifndef __SP_LINE_H__
#define __SP_LINE_H__

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

#include "svg/svg-types.h"
#include "sp-shape.h"

G_BEGIN_DECLS

#define SP_TYPE_LINE            (sp_line_get_type ())
#define SP_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_LINE, SPLine))
#define SP_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_LINE, SPLineClass))
#define SP_IS_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_LINE))
#define SP_IS_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_LINE))

typedef struct _SPLine SPLine;
typedef struct _SPLineClass SPLineClass;

struct _SPLine {
	SPShape shape;

	SPSVGLength x1;
	SPSVGLength y1;
	SPSVGLength x2;
	SPSVGLength y2;
};

struct _SPLineClass {
	SPShapeClass parent_class;
};

GType sp_line_get_type (void);

G_END_DECLS

#endif
