#define SP_CANVAS_TRANSLATOR_C

/*
 * SPCanvasTranslator
 *
 * This is very-very dirty object, translating xlib canvas draw requests
 *   to aa canvas render requests.
 *
 * Copyright (C) Lauris Kaplinski, 2000
 *
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "canvas-translator.h"

static void sp_canvas_translator_class_init (SPCanvasTranslatorClass * klass);
static void sp_canvas_translator_init (SPCanvasTranslator * group);
static void sp_canvas_translator_destroy (GtkObject * object);

static void sp_canvas_translator_realize (GnomeCanvasItem * item);
static void sp_canvas_translator_unrealize (GnomeCanvasItem * item);
static void sp_canvas_translator_map (GnomeCanvasItem * item);
static void sp_canvas_translator_unmap (GnomeCanvasItem * item);
static void sp_canvas_translator_draw (GnomeCanvasItem * item, GdkDrawable * drawable, int x, int y, int width, int height);

static GnomeCanvasGroupClass *parent_class;

GtkType
sp_canvas_translator_get_type (void)
{
	static GtkType group_type = 0;
	if (!group_type) {
		GtkTypeInfo group_info = {
			"SPCanvasTranslator",
			sizeof (SPCanvasTranslator),
			sizeof (SPCanvasTranslatorClass),
			(GtkClassInitFunc) sp_canvas_translator_class_init,
			(GtkObjectInitFunc) sp_canvas_translator_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};
		group_type = gtk_type_unique (gnome_canvas_group_get_type (), &group_info);
	}
	return group_type;
}

static void
sp_canvas_translator_class_init (SPCanvasTranslatorClass *klass)
{
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) klass;
	item_class = (GnomeCanvasItemClass *) klass;

	parent_class = gtk_type_class (gnome_canvas_group_get_type ());

	object_class->destroy = sp_canvas_translator_destroy;

	item_class->realize = sp_canvas_translator_realize;
	item_class->unrealize = sp_canvas_translator_unrealize;
	item_class->map = sp_canvas_translator_map;
	item_class->unmap = sp_canvas_translator_unmap;
	item_class->draw = sp_canvas_translator_draw;
}

static void
sp_canvas_translator_init (SPCanvasTranslator * translator)
{
}

static void
sp_canvas_translator_destroy (GtkObject *object)
{

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_canvas_translator_realize (GnomeCanvasItem * item)
{
	GTK_OBJECT_SET_FLAGS (item, GNOME_CANVAS_ITEM_REALIZED);

	gnome_canvas_item_request_update (item);
}

static void
sp_canvas_translator_unrealize (GnomeCanvasItem * item)
{
	GTK_OBJECT_UNSET_FLAGS (item, GNOME_CANVAS_ITEM_REALIZED);
}

static void
sp_canvas_translator_map (GnomeCanvasItem * item)
{
	GTK_OBJECT_SET_FLAGS (item, GNOME_CANVAS_ITEM_MAPPED);
}

static void
sp_canvas_translator_unmap (GnomeCanvasItem * item)
{
	GTK_OBJECT_UNSET_FLAGS (item, GNOME_CANVAS_ITEM_MAPPED);
}

static void
sp_canvas_translator_draw (GnomeCanvasItem * item, GdkDrawable * drawable, int x, int y, int width, int height)
{
	GnomeCanvasGroup * group;
	GnomeCanvasItem * child;
	GList * l;
	GnomeCanvasBuf buf;
	GdkPixbuf * pixbuf;
	gboolean rendered;

	group = GNOME_CANVAS_GROUP (item);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
	buf.buf = gdk_pixbuf_get_pixels (pixbuf);
	buf.buf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	buf.rect.x0 = x;
	buf.rect.y0 = y;
	buf.rect.x1 = x + width;
	buf.rect.y1 = y + height;

	gdk_pixbuf_get_from_drawable (pixbuf,
		drawable, gtk_widget_get_colormap (GTK_WIDGET (item->canvas)),
		0, 0,
		0, 0,
		width, height);

	buf.is_bg = FALSE;
	buf.is_buf = TRUE;

	rendered = FALSE;

	for (l = group->item_list; l != NULL; l = l->next) {
		child = GNOME_CANVAS_ITEM (l->data);

		if ((child->object.flags & GNOME_CANVAS_ITEM_VISIBLE) &&
			((child->x1 < (x + width)) &&
			(child->y1 < (y + height)) &&
			(child->x2 > x) &&
			(child->y2 > y))) {

			if (GNOME_CANVAS_ITEM_CLASS (child->object.klass)->render) {
				(* GNOME_CANVAS_ITEM_CLASS (child->object.klass)->render) (item, &buf);

				rendered = TRUE;
			}
		}
	}

	if (rendered) {
		gdk_pixbuf_render_to_drawable (pixbuf,
			drawable, item->canvas->pixmap_gc,
			0, 0,
			0, 0,
			width, height,
			GDK_RGB_DITHER_NONE,
			0,0);
	}

	gdk_pixbuf_unref (pixbuf);
}

