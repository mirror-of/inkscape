#ifndef __SP_IMAGE_H__
#define __SP_IMAGE_H__

/*
 * SVG <image> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_IMAGE (sp_image_get_type ())
#define SP_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_IMAGE, SPImage))
#define SP_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_IMAGE, SPImageClass))
#define SP_IS_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_IMAGE))
#define SP_IS_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_IMAGE))

typedef struct _SPImage SPImage;
typedef struct _SPImageClass SPImageClass;

/* SPImage */

#include <glib.h>

G_BEGIN_DECLS

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "svg/svg-types.h"
#include "sp-item.h"

#define SP_IMAGE_HREF_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A

struct _SPImage {
	SPItem item;

	SPSVGLength x;
	SPSVGLength y;
	SPSVGLength width;
	SPSVGLength height;

	guchar *href;

	GdkPixbuf *pixbuf;
};

struct _SPImageClass {
	SPItemClass parent_class;
};

GType sp_image_get_type (void);

G_END_DECLS

#endif
