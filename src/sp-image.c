#define __SP_IMAGE_C__

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

#include <config.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <libnr/nr-matrix.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "display/nr-arena-image.h"
#include "svg/svg.h"
#include "attributes.h"
#include "print.h"
#include "style.h"
#include "brokenimage.xpm"
#include "document.h"
#include "dialogs/object-attributes.h"
#include "sp-image.h"
#include "helper/sp-intl.h"

/*
 * SPImage
 */

static void sp_image_class_init (SPImageClass * class);
static void sp_image_init (SPImage * image);

static void sp_image_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_image_release (SPObject * object);
static void sp_image_set (SPObject *object, unsigned int key, const unsigned char *value);
static void sp_image_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static SPRepr *sp_image_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_image_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags);
static void sp_image_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_image_description (SPItem * item);
static int sp_image_snappoints (SPItem *item, NRPointF *p, int size);
static NRArenaItem *sp_image_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_image_write_transform (SPItem *item, SPRepr *repr, NRMatrixF *transform);

#ifdef ENABLE_AUTOTRACE
static void sp_image_autotrace (GtkMenuItem *menuitem, SPAnchor *anchor);
static void autotrace_dialog(SPImage * img);
#endif /* Def: ENABLE_AUTOTRACE */

GdkPixbuf * sp_image_repr_read_image (SPRepr * repr);
static GdkPixbuf *sp_image_pixbuf_force_rgba (GdkPixbuf * pixbuf);
static void sp_image_update_canvas_image (SPImage *image);
static GdkPixbuf * sp_image_repr_read_dataURI (const gchar * uri_data);
static GdkPixbuf * sp_image_repr_read_b64 (const gchar * uri_data);

static SPItemClass *parent_class;

GType
sp_image_get_type (void)
{
	static GType image_type = 0;
	if (!image_type) {
		GTypeInfo image_info = {
			sizeof (SPImageClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_image_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPImage),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_image_init,
		};
		image_type = g_type_register_static (sp_item_get_type (), "SPImage", &image_info, 0);
	}
	return image_type;
}

static void
sp_image_class_init (SPImageClass * klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = g_type_class_ref (sp_item_get_type ());

	sp_object_class->build = sp_image_build;
	sp_object_class->release = sp_image_release;
	sp_object_class->set = sp_image_set;
	sp_object_class->update = sp_image_update;
	sp_object_class->write = sp_image_write;

	item_class->bbox = sp_image_bbox;
	item_class->print = sp_image_print;
	item_class->description = sp_image_description;
	item_class->show = sp_image_show;
	item_class->snappoints = sp_image_snappoints;
	item_class->write_transform = sp_image_write_transform;
}

static void
sp_image_init (SPImage *image)
{
	sp_svg_length_unset (&image->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&image->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&image->width, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&image->height, SP_SVG_UNIT_NONE, 0.0, 0.0);
}

static void
sp_image_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "xlink:href");
	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "width");
	sp_object_read_attr (object, "height");

	/* Register */
	sp_document_add_resource (document, "image", object);
}

static void
sp_image_release (SPObject *object)
{
	SPImage *image;

	image = SP_IMAGE (object);

	if (SP_OBJECT_DOCUMENT (object)) {
		/* Unregister ourselves */
		sp_document_remove_resource (SP_OBJECT_DOCUMENT (object), "image", SP_OBJECT (object));
	}

	if (image->href) {
		g_free (image->href);
		image->href = NULL;
	}

	if (image->pixbuf) {
		gdk_pixbuf_unref (image->pixbuf);
		image->pixbuf = NULL;
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_image_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPImage *image;
	gulong unit;

	image = SP_IMAGE (object);

	switch (key) {
	case SP_ATTR_XLINK_HREF:
		if (image->href) g_free (image->href);
		image->href = (value) ? g_strdup (value) : NULL;
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG | SP_IMAGE_HREF_MODIFIED_FLAG);
		break;
	case SP_ATTR_X:
		if (sp_svg_length_read_lff (value, &unit, &image->x.value, &image->x.computed) &&
		    /* fixme: These are probably valid, but require special treatment (Lauris) */
		    (unit != SP_SVG_UNIT_EM) &&
		    (unit != SP_SVG_UNIT_EX) &&
		    (unit != SP_SVG_UNIT_PERCENT)) {
			image->x.set = TRUE;
			image->x.unit = unit;
		} else {
			sp_svg_length_unset (&image->x, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_Y:
		if (sp_svg_length_read_lff (value, &unit, &image->y.value, &image->y.computed) &&
		    /* fixme: These are probably valid, but require special treatment (Lauris) */
		    (unit != SP_SVG_UNIT_EM) &&
		    (unit != SP_SVG_UNIT_EX) &&
		    (unit != SP_SVG_UNIT_PERCENT)) {
			image->y.set = TRUE;
			image->y.unit = unit;
		} else {
			sp_svg_length_unset (&image->y, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_WIDTH:
		if (sp_svg_length_read_lff (value, &unit, &image->width.value, &image->width.computed) &&
		    /* fixme: These are probably valid, but require special treatment (Lauris) */
		    (unit != SP_SVG_UNIT_EM) &&
		    (unit != SP_SVG_UNIT_EX) &&
		    (unit != SP_SVG_UNIT_PERCENT)) {
			image->width.set = TRUE;
			image->width.unit = unit;
		} else {
			sp_svg_length_unset (&image->width, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_HEIGHT:
		if (sp_svg_length_read_lff (value, &unit, &image->height.value, &image->height.computed) &&
		    /* fixme: These are probably valid, but require special treatment (Lauris) */
		    (unit != SP_SVG_UNIT_EM) &&
		    (unit != SP_SVG_UNIT_EX) &&
		    (unit != SP_SVG_UNIT_PERCENT)) {
			image->height.set = TRUE;
			image->height.unit = unit;
		} else {
			sp_svg_length_unset (&image->height, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) (parent_class))->set)
			((SPObjectClass *) (parent_class))->set (object, key, value);
		break;
	}
}

static void
sp_image_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPImage *image;

	image = (SPImage *) object;

	if (((SPObjectClass *) (parent_class))->update)
		((SPObjectClass *) (parent_class))->update (object, ctx, flags);

	if (flags & SP_IMAGE_HREF_MODIFIED_FLAG) {
		if (image->pixbuf) {
			gdk_pixbuf_unref (image->pixbuf);
			image->pixbuf = NULL;
		}
		if (image->href) {
			GdkPixbuf *pixbuf;
			pixbuf = sp_image_repr_read_image (object->repr);
			if (pixbuf) {
				pixbuf = sp_image_pixbuf_force_rgba (pixbuf);
				image->pixbuf = pixbuf;
			}
		}
	}

	sp_image_update_canvas_image ((SPImage *) object);
}

static SPRepr *
sp_image_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPImage *image;

	image = SP_IMAGE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("image");
	}

	sp_repr_set_attr (repr, "xlink:href", image->href);
	/* fixme: Reset attribute if needed (Lauris) */
	if (image->x.set) sp_repr_set_double (repr, "x", image->x.computed);
	if (image->y.set) sp_repr_set_double (repr, "y", image->y.computed);
	if (image->width.set) sp_repr_set_double (repr, "width", image->width.computed);
	if (image->height.set) sp_repr_set_double (repr, "height", image->height.computed);

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_image_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags)
{
	SPImage *image;

	image = SP_IMAGE (item);

	if ((image->width.computed > 0.0) && (image->height.computed > 0.0)) {
		float x0, y0, x1, y1;
		float x, y;

		x0 = image->x.computed;
		y0 = image->y.computed;
		x1 = x0 + image->width.computed;
		y1 = y0 + image->height.computed;

		x = NR_MATRIX_DF_TRANSFORM_X (transform, x0, y0);
		y = NR_MATRIX_DF_TRANSFORM_Y (transform, x0, y0);
		bbox->x0 = MIN (bbox->x0, x);
		bbox->y0 = MIN (bbox->y0, y);
		bbox->x1 = MAX (bbox->x1, x);
		bbox->y1 = MAX (bbox->y1, y);
		x = NR_MATRIX_DF_TRANSFORM_X (transform, x1, y0);
		y = NR_MATRIX_DF_TRANSFORM_Y (transform, x1, y0);
		bbox->x0 = MIN (bbox->x0, x);
		bbox->y0 = MIN (bbox->y0, y);
		bbox->x1 = MAX (bbox->x1, x);
		bbox->y1 = MAX (bbox->y1, y);
		x = NR_MATRIX_DF_TRANSFORM_X (transform, x1, y1);
		y = NR_MATRIX_DF_TRANSFORM_Y (transform, x1, y1);
		bbox->x0 = MIN (bbox->x0, x);
		bbox->y0 = MIN (bbox->y0, y);
		bbox->x1 = MAX (bbox->x1, x);
		bbox->y1 = MAX (bbox->y1, y);
		x = NR_MATRIX_DF_TRANSFORM_X (transform, x0, y1);
		y = NR_MATRIX_DF_TRANSFORM_Y (transform, x0, y1);
		bbox->x0 = MIN (bbox->x0, x);
		bbox->y0 = MIN (bbox->y0, y);
		bbox->x1 = MAX (bbox->x1, x);
		bbox->y1 = MAX (bbox->y1, y);
	}
}

static void
sp_image_print (SPItem *item, SPPrintContext *ctx)
{
	SPImage *image;
	NRMatrixF tp, ti, s, t;
	unsigned char *px;
	int w, h, rs;

	image = SP_IMAGE (item);

	if (!image->pixbuf) return;
	if ((image->width.computed <= 0.0) || (image->height.computed <= 0.0)) return;

	px = gdk_pixbuf_get_pixels (image->pixbuf);
	w = gdk_pixbuf_get_width (image->pixbuf);
	h = gdk_pixbuf_get_height (image->pixbuf);
	rs = gdk_pixbuf_get_rowstride (image->pixbuf);

	/* fixme: (Lauris) */
	nr_matrix_f_set_translate (&tp, image->x.computed, image->y.computed);
	nr_matrix_f_set_scale (&s, image->width.computed, -image->height.computed);
	nr_matrix_f_set_translate (&ti, 0.0, -1.0);

	nr_matrix_multiply_fff (&t, &s, &tp);
	nr_matrix_multiply_fff (&t, &ti, &t);

	sp_print_image_R8G8B8A8_N (ctx, px, w, h, rs, &t, SP_OBJECT_STYLE (item));
}

static gchar *
sp_image_description (SPItem * item)
{
	SPImage * image;

	image = SP_IMAGE (item);

	if (image->pixbuf == NULL) {
		return g_strdup_printf (_("Image with bad reference: %s"), image->href);
	} else {
		return g_strdup_printf (_("Color image %d x %d: %s"),
					  gdk_pixbuf_get_width (image->pixbuf),
					  gdk_pixbuf_get_height (image->pixbuf),
					  image->href);
	}
}

static NRArenaItem *
sp_image_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPImage * image;
	NRArenaItem *ai;

	image = (SPImage *) item;

	ai = nr_arena_item_new (arena, NR_TYPE_ARENA_IMAGE);

	if (image->pixbuf) {
		nr_arena_image_set_pixels (NR_ARENA_IMAGE (ai),
					   gdk_pixbuf_get_pixels (image->pixbuf),
					   gdk_pixbuf_get_width (image->pixbuf),
					   gdk_pixbuf_get_height (image->pixbuf),
					   gdk_pixbuf_get_rowstride (image->pixbuf));
	} else {
		nr_arena_image_set_pixels (NR_ARENA_IMAGE (ai), NULL, 0, 0, 0);
	}
	nr_arena_image_set_geometry (NR_ARENA_IMAGE (ai), image->x.computed, image->y.computed, image->width.computed, image->height.computed);

	return ai;
}

/*
 * utility function to try loading image from href
 *
 * docbase/relative_src
 * absolute_src
 *
 */

GdkPixbuf *
sp_image_repr_read_image (SPRepr * repr)
{
	const gchar * filename, * docbase;
	gchar * fullname;
	GdkPixbuf * pixbuf;

	filename = sp_repr_attr (repr, "xlink:href");
	if (filename == NULL) filename = sp_repr_attr (repr, "href"); /* FIXME */
	if (filename != NULL) {
		if (strncmp (filename,"file:",5) == 0) {
			fullname = g_filename_from_uri(filename, NULL, NULL);
			if (fullname) {
				pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
				g_free (fullname);
				if (pixbuf != NULL) return pixbuf;
			}
		} else if (strncmp (filename,"data:",5) == 0) {
			/* data URI - embedded image */
			filename += 5;
			pixbuf = sp_image_repr_read_dataURI (filename);
			if (pixbuf != NULL) return pixbuf;
		} else if (!g_path_is_absolute (filename)) {
			/* try to load from relative pos */
			docbase = sp_repr_attr (sp_repr_document_root (sp_repr_document (repr)), "sodipodi:docbase");
			if (!docbase) docbase = "./";
			fullname = g_strconcat (docbase, filename, NULL);
			pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
			g_free (fullname);
			if (pixbuf != NULL) return pixbuf;
		} else {
			/* try absolute filename */
			pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
			if (pixbuf != NULL) return pixbuf;
		}
	}
	/* at last try to load from sp absolute path name */
	filename = sp_repr_attr (repr, "sodipodi:absref");
	if (filename != NULL) {
		pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
		if (pixbuf != NULL) return pixbuf;
	}
	/* Nope: We do not find any valid pixmap file :-( */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const gchar **) brokenimage_xpm);

	/* It should be included xpm, so if it still does not does load, */
	/* our libraries are broken */
	g_assert (pixbuf != NULL);

	return pixbuf;
}

static GdkPixbuf *
sp_image_pixbuf_force_rgba (GdkPixbuf * pixbuf)
{
	GdkPixbuf * newbuf;

	if (gdk_pixbuf_get_has_alpha (pixbuf)) return pixbuf;

	newbuf = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
	gdk_pixbuf_unref (pixbuf);

	return newbuf;
}

/* We assert that realpixbuf is either NULL or identical size to pixbuf */

static void
sp_image_update_canvas_image (SPImage *image)
{
	SPItem *item;
	SPItemView *v;

	item = SP_ITEM (image);

	if (image->pixbuf) {
		/* fixme: We are slightly violating spec here (Lauris) */
		if (!image->width.set) {
			image->width.computed = gdk_pixbuf_get_width (image->pixbuf);
		}
		if (!image->height.set) {
			image->height.computed = gdk_pixbuf_get_height (image->pixbuf);
		}
	}

	for (v = item->display; v != NULL; v = v->next) {
		nr_arena_image_set_pixels (NR_ARENA_IMAGE (v->arenaitem),
					   gdk_pixbuf_get_pixels (image->pixbuf),
					   gdk_pixbuf_get_width (image->pixbuf),
					   gdk_pixbuf_get_height (image->pixbuf),
					   gdk_pixbuf_get_rowstride (image->pixbuf));
		nr_arena_image_set_geometry (NR_ARENA_IMAGE (v->arenaitem),
					     image->x.computed, image->y.computed,
					     image->width.computed, image->height.computed);
	}
}

static int
sp_image_snappoints (SPItem *item, NRPointF *p, int size)
{
	SPImage *image;
	NRMatrixF i2d;
	float x0, y0, x1, y1;
	int i;

	image = SP_IMAGE (item);

	sp_item_i2d_affine (item, &i2d);

	/* we use corners of image only */
	x0 = image->x.computed;
	y0 = image->y.computed;
	x1 = x0 + image->width.computed;
	y1 = y0 + image->height.computed;

	i = 0;
	if (i < size) {
		p[i].x = NR_MATRIX_DF_TRANSFORM_X (&i2d, x0, y0);
		p[i].y = NR_MATRIX_DF_TRANSFORM_Y (&i2d, x0, y0);
		i += 1;
	}
	if (i < size) {
		p[i].x = NR_MATRIX_DF_TRANSFORM_X (&i2d, x1, y0);
		p[i].y = NR_MATRIX_DF_TRANSFORM_Y (&i2d, x1, y0);
		i += 1;
	}
	if (i < size) {
		p[i].x = NR_MATRIX_DF_TRANSFORM_X (&i2d, x1, y1);
		p[i].y = NR_MATRIX_DF_TRANSFORM_Y (&i2d, x1, y1);
		i += 1;
	}
	if (i < size) {
		p[i].x = NR_MATRIX_DF_TRANSFORM_X (&i2d, x0, y1);
		p[i].y = NR_MATRIX_DF_TRANSFORM_Y (&i2d, x0, y1);
		i += 1;
	}

	return i;
}

/*
 * Initially we'll do:
 * Transform x, y, set x, y, clear translation
 */

static void
sp_image_write_transform (SPItem *item, SPRepr *repr, NRMatrixF *t)
{
	SPImage *image;
	NRMatrixF rev;
	gdouble px, py, sw, sh;
	guchar c[80];

	image = SP_IMAGE (item);

	/* Calculate text start in parent coords */
	px = NR_MATRIX_DF_TRANSFORM_X (t, image->x.computed, image->y.computed);
	py = NR_MATRIX_DF_TRANSFORM_Y (t, image->x.computed, image->y.computed);

	/* Clear translation */
	t->c[4] = 0.0;
	t->c[5] = 0.0;

	/* Scalers */
	sw = sqrt (t->c[0] * t->c[0] + t->c[1] * t->c[1]);
	sh = sqrt (t->c[2] * t->c[2] + t->c[3] * t->c[3]);
	if (sw > 1e-9) {
		t->c[0] = t->c[0] / sw;
		t->c[1] = t->c[1] / sw;
	} else {
		t->c[0] = 1.0;
		t->c[1] = 0.0;
	}
	if (sh > 1e-9) {
		t->c[2] = t->c[2] / sh;
		t->c[3] = t->c[3] / sh;
	} else {
		t->c[2] = 0.0;
		t->c[3] = 1.0;
	}
	sp_repr_set_double_attribute (repr, "width", image->width.computed * sw);
	sp_repr_set_double_attribute (repr, "height", image->height.computed * sh);

	/* Find start in item coords */
	nr_matrix_f_invert (&rev, t);
	sp_repr_set_double_attribute (repr, "x", px * rev.c[0] + py * rev.c[2]);
	sp_repr_set_double_attribute (repr, "y", px * rev.c[1] + py * rev.c[3]);

	if (sp_svg_transform_write (c, 80, t)) {
		sp_repr_set_attr (SP_OBJECT_REPR (item), "transform", c);
	} else {
		sp_repr_set_attr (SP_OBJECT_REPR (item), "transform", NULL);
	}
}

#ifdef ENABLE_AUTOTRACE
static void
sp_image_autotrace (GtkMenuItem *menuitem, SPAnchor *anchor)
{
  autotrace_dialog (SP_IMAGE(anchor));
}
#endif /* Def: ENABLE_AUTOTRACE */

static GdkPixbuf *
sp_image_repr_read_dataURI (const gchar * uri_data)
{	GdkPixbuf * pixbuf = NULL;

	gint data_is_image = 0;
	gint data_is_base64 = 0;

	const gchar * data = uri_data;

	while (*data) {
		if (strncmp (data,"base64",6) == 0) {
			/* base64-encoding */
			data_is_base64 = 1;
			data += 6;
		}
		else if (strncmp (data,"image/png",9) == 0) {
			/* PNG image */
			data_is_image = 1;
			data += 9;
		}
		else if (strncmp (data,"image/jpg",9) == 0) {
			/* JPEG image */
			data_is_image = 1;
			data += 9;
		}
		else if (strncmp (data,"image/jpeg",10) == 0) {
			/* JPEG image */
			data_is_image = 1;
			data += 10;
		}
		else { /* unrecognized option; skip it */
			while (*data) {
				if (((*data) == ';') || ((*data) == ',')) break;
				data++;
			}
		}
		if ((*data) == ';') {
			data++;
			continue;
		}
		if ((*data) == ',') {
			data++;
			break;
		}
	}

	if ((*data) && data_is_image && data_is_base64) {
		pixbuf = sp_image_repr_read_b64 (data);
	}

	return pixbuf;
}

static GdkPixbuf *
sp_image_repr_read_b64 (const gchar * uri_data)
{	GdkPixbuf * pixbuf = NULL;
	GdkPixbufLoader * loader = NULL;

	gint j;
	gint k;
	gint l;
	gint b;
	gint len;
	gint eos = 0;
	gint failed = 0;

	guint32 bits;

	static const gchar B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	const gchar* btr = uri_data;

	gchar ud[4];

	guchar bd[57];

	loader = gdk_pixbuf_loader_new ();

	if (loader == NULL) return NULL;

	while (eos == 0) {
		l = 0;
		for (j = 0; j < 19; j++) {
			len = 0;
			for (k = 0; k < 4; k++) {
				while (isspace ((int) (*btr))) {
					if ((*btr) == '\0') break;
					btr++;
				}
				if (eos) {
					ud[k] = 0;
					continue;
				}
				if (((*btr) == '\0') || ((*btr) == '=')) {
					eos = 1;
					ud[k] = 0;
					continue;
				}
				ud[k] = 64;
				for (b = 0; b < 64; b++) { /* There must a faster way to do this... ?? */
					if (B64[b] == (*btr)) {
						ud[k] = (gchar) b;
						break;
					}
				}
				if (ud[k] == 64) { /* data corruption ?? */
					eos = 1;
					ud[k] = 0;
					continue;
				}
				btr++;
				len++;
			}
			bits = (guint32) ud[0];
			bits = (bits << 6) | (guint32) ud[1];
			bits = (bits << 6) | (guint32) ud[2];
			bits = (bits << 6) | (guint32) ud[3];
			bd[l++] = (guchar) ((bits & 0xff0000) >> 16);
			if (len > 2) {
				bd[l++] = (guchar) ((bits & 0xff00) >>  8);
			}
			if (len > 3) {
				bd[l++] = (guchar)  (bits & 0xff);
			}
		}

		if (!gdk_pixbuf_loader_write (loader, (const guchar *) bd, (size_t) l, NULL)) {
			failed = 1;
			break;
		}
	}

	gdk_pixbuf_loader_close (loader, NULL);

	if (!failed) pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

	return pixbuf;
}

#ifdef ENABLE_AUTOTRACE
#define GDK_PIXBUF_TO_AT_BITMAP_DEBUG 0
#include <autotrace/autotrace.h>
#include <frontline/frontline.h>

#include "view.h"
#include "desktop.h"
#include "interface.h"

/* getpid and umask */
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

/* g_strdup_printf */
#include <glib.h>

static at_bitmap_type * gdk_pixbuf_to_at_bitmap (GdkPixbuf * pixbuf);
static void load_trace_result(FrontlineDialog * fl_dialog, gpointer user_data);
static void load_splines(at_splines_type * splines);
static void load_file(const guchar * filename);
static void handle_msg(at_string msg, at_msg_type msg_type, at_address client_data);
static GtkWidget *  build_header_area(SPRepr *repr);

static void
autotrace_dialog (SPImage * img)
{
	gchar *title = _("Trace");

	GtkWidget *trace_dialog;
	GtkWidget * header_area;
	GtkWidget * header_sep;
	at_bitmap_type * bitmap;

#ifdef FRONTLINE_INIT
	frontline_init();
#endif /* Def: FRONTLINE_INIT */

	trace_dialog = frontline_dialog_new();
	gtk_window_set_title (GTK_WINDOW (trace_dialog), title);

	gtk_signal_connect_object_while_alive (GTK_OBJECT (img), 
					       "release",
					       GTK_SIGNAL_FUNC (gtk_widget_destroy),
					       GTK_OBJECT (trace_dialog));
	gtk_signal_connect_object (GTK_OBJECT(FRONTLINE_DIALOG(trace_dialog)->close_button),
				   "clicked",
				   GTK_SIGNAL_FUNC(gtk_widget_destroy), 
				   GTK_OBJECT(trace_dialog));
	gtk_signal_connect(GTK_OBJECT(trace_dialog),
			   "trace_done",
			   GTK_SIGNAL_FUNC (load_trace_result), 
			   trace_dialog);
	
	header_area = build_header_area(SP_OBJECT_REPR(img));
	gtk_box_pack_start_defaults(GTK_BOX(FRONTLINE_DIALOG(trace_dialog)->header_area),
				    header_area);
	gtk_widget_show(header_area);
	
	header_sep = gtk_hseparator_new();
	gtk_box_pack_start_defaults(GTK_BOX(FRONTLINE_DIALOG(trace_dialog)->header_area), 
				    header_sep);
	gtk_widget_show(header_sep);

	
	bitmap = gdk_pixbuf_to_at_bitmap(img->pixbuf);
	frontline_dialog_set_bitmap(FRONTLINE_DIALOG(trace_dialog), bitmap);
	
	gtk_widget_show (trace_dialog);
}

static at_bitmap_type *
gdk_pixbuf_to_at_bitmap (GdkPixbuf * pixbuf)
{
	guchar *    datum;
	at_bitmap_type * bitmap;
	unsigned short width, height;
	int i, j;

	if (GDK_PIXBUF_TO_AT_BITMAP_DEBUG)
		g_message("%d:channel %d:width %d:height %d:bits_per_sample %d:has_alpha %d:rowstride\n", 
			  gdk_pixbuf_get_n_channels(pixbuf),
			  gdk_pixbuf_get_width(pixbuf),
			  gdk_pixbuf_get_height(pixbuf),
			  gdk_pixbuf_get_bits_per_sample(pixbuf),
			  gdk_pixbuf_get_has_alpha(pixbuf),
			  gdk_pixbuf_get_rowstride(pixbuf));

	datum   = gdk_pixbuf_get_pixels(pixbuf);
	width   = gdk_pixbuf_get_width(pixbuf);
	height  = gdk_pixbuf_get_height(pixbuf);

	bitmap = at_bitmap_new(width, height, 3);

	if (gdk_pixbuf_get_has_alpha(pixbuf)) {
		j = 0;
		for(i = 0; i < width * height * 4; i++)
			if (3 != (i % 4))
				bitmap->bitmap[j++] = datum[i];
	}
	else
		memmove(bitmap->bitmap, datum, width * height * 3);
	return bitmap;
}

static void
load_trace_result(FrontlineDialog * fl_dialog, gpointer user_data)
{
	FrontlineDialog * trace_dialog;
	trace_dialog = FRONTLINE_DIALOG(user_data);
	
	if (!trace_dialog->splines) 
	  return;
	if (fl_ask(GTK_WINDOW(trace_dialog), trace_dialog->splines))
	  load_splines(trace_dialog->splines);
}


static void
load_splines(at_splines_type * splines)
{
  	static int serial_num = 0;
	FILE * tmp_fp;
	at_output_write_func writer;
	gchar * filename;

	mode_t old_mask;

  	filename = g_strdup_printf("/tmp/at-%s-%d-%d.svg", 
				   g_get_user_name(),
				   getpid(), 
				   serial_num++);
	
	/* Make the mode of temporary svg file 
	   "readable and writable by the user only". */
	old_mask = umask(066);
	tmp_fp = fopen(filename, "w");
	umask(old_mask);
  
	writer = at_output_get_handler_by_suffix ("svg");
	at_splines_write (writer, tmp_fp, filename, NULL, splines,
			  handle_msg, filename);
	fclose(tmp_fp);
  
	load_file (filename);

	unlink(filename);
	g_free(filename);
}


static void
load_file (const guchar *filename)
{
	SPDocument * doc;
	SPViewWidget *dtw;
  
	/* fixme: Either use file:: method, or amke this private (Lauris) */
	/* fixme: In latter case we may want to publish it on save (Lauris) */
	doc = sp_document_new (filename, TRUE);
	dtw = sp_desktop_widget_new (sp_document_namedview (doc, NULL));
	sp_document_unref (doc);
	sp_create_window (dtw, TRUE);
}

static void
handle_msg(at_string msg, at_msg_type msg_type, at_address client_data)
{
	GtkWidget *dialog;
	guchar * long_msg;
	guchar * target = client_data;
	
	if (msg_type == AT_MSG_FATAL) {
		long_msg = g_strdup_printf(_("Error to write %s: %s"), 
					   target, msg);	
		dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 long_msg,
						 NULL);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_free(long_msg);
	}
	else {
		long_msg = g_strdup_printf("%s: %s", msg, target);	
		g_warning("%s", long_msg);
		g_free(long_msg);
	}
}

static GtkWidget *
build_header_area(SPRepr *repr)
{
	const gchar * doc_name, * img_uri;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * entry;
	
	vbox = gtk_vbox_new(TRUE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	
	/* Doc name */
	doc_name = sp_repr_attr(sp_repr_document_root(sp_repr_document(repr)), 
				"sodipodi:docname");
	if (!doc_name)
		doc_name = _("Untitled");
	hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);
	
	label = gtk_label_new(_("Document Name:"));
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), doc_name);
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 4);
	gtk_widget_show(label);
	gtk_widget_show(entry);
	gtk_widget_set_sensitive(entry, FALSE);

	/* Image URI */
	img_uri = sp_repr_attr(repr, "xlink:href");
	if (!img_uri)
		img_uri = _("Unknown");

	hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);
	
	label = gtk_label_new(_("Image URI:"));
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), img_uri);
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 4);
	gtk_widget_show(label);
	gtk_widget_show(entry);
	gtk_widget_set_sensitive(entry, FALSE);
	
	return vbox;
}

#endif /* Def: ENABLE_AUTOTRACE */
