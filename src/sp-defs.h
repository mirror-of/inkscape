#ifndef __SP_DEFS_H__
#define __SP_DEFS_H__

/*
 * SVG <defs> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_DEFS            (sp_defs_get_type ())
#define SP_DEFS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_DEFS, SPDefs))
#define SP_DEFS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_DEFS, SPDefsClass))
#define SP_IS_DEFS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_DEFS))
#define SP_IS_DEFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_DEFS))

struct _SPDefs {
	SPObject object;
	SPObject *children;
};

struct _SPDefsClass {
	SPObjectClass parent_class;
};

GType sp_defs_get_type (void);

#endif
