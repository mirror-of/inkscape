#ifndef __SP_PATH_H__
#define __SP_PATH_H__

/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"

#define SP_TYPE_PATH (sp_path_get_type ())
#define SP_PATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_PATH, SPPath))
#define SP_IS_PATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_PATH))

struct _SPPath {
	SPShape shape;
};

struct _SPPathClass {
	SPShapeClass shape_class;
};

GType sp_path_get_type (void);

#endif
