#ifndef __SP_USE_H__
#define __SP_USE_H__

/*
 * SVG <use> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg-types.h"
#include "sp-item.h"

G_BEGIN_DECLS

#define SP_TYPE_USE            (sp_use_get_type ())
#define SP_USE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_USE, SPUse))
#define SP_USE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_USE, SPUseClass))
#define SP_IS_USE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_USE))
#define SP_IS_USE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_USE))

typedef struct _SPUse SPUse;
typedef struct _SPUseClass SPUseClass;

struct _SPUse {
	SPItem item;
	SPObject *child;
	SPSVGLength x;
	SPSVGLength y;
	SPSVGLength width;
	SPSVGLength height;
	guchar *href;
};

struct _SPUseClass {
	SPItemClass parent_class;
};

GType sp_use_get_type (void);

G_END_DECLS

#endif
