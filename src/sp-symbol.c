#define __SP_SYMBOL_C__

/*
 * SVG <symbol> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include "svg/svg.h"
#include "display/nr-arena-group.h"
#include "attributes.h"
#include "print.h"
#include "document.h"
#include "desktop.h"
#include "sp-defs.h"
#include "sp-namedview.h"
#include "sp-symbol.h"

static void sp_symbol_class_init (SPSymbolClass *klass);
static void sp_symbol_init (SPSymbol *symbol);

static void sp_symbol_build (SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_symbol_release (SPObject *object);
static void sp_symbol_set (SPObject *object, unsigned int key, const unsigned char *value);
static void sp_symbol_child_added (SPObject *object, SPRepr *child, SPRepr *ref);
static void sp_symbol_remove_child (SPObject *object, SPRepr *child);
static void sp_symbol_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_symbol_modified (SPObject *object, guint flags);
static SPRepr *sp_symbol_write (SPObject *object, SPRepr *repr, guint flags);

static NRArenaItem *sp_symbol_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_symbol_hide (SPItem *item, unsigned int key);
static void sp_symbol_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags);
static void sp_symbol_print (SPItem *item, SPPrintContext *ctx);

static SPGroupClass *parent_class;

GType
sp_symbol_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPSymbolClass),
			NULL, NULL,
			(GClassInitFunc) sp_symbol_class_init,
			NULL, NULL,
			sizeof (SPSymbol),
			16,
			(GInstanceInitFunc) sp_symbol_init,
		};
		type = g_type_register_static (SP_TYPE_GROUP, "SPSymbol", &info, 0);
	}
	return type;
}

static void
sp_symbol_class_init (SPSymbolClass *klass)
{
	GObjectClass *object_class;
	SPObjectClass *sp_object_class;
	SPItemClass *sp_item_class;

	object_class = G_OBJECT_CLASS (klass);
	sp_object_class = (SPObjectClass *) klass;
	sp_item_class = (SPItemClass *) klass;

	parent_class = g_type_class_ref (SP_TYPE_GROUP);

	sp_object_class->build = sp_symbol_build;
	sp_object_class->release = sp_symbol_release;
	sp_object_class->set = sp_symbol_set;
	sp_object_class->child_added = sp_symbol_child_added;
	sp_object_class->remove_child = sp_symbol_remove_child;
	sp_object_class->update = sp_symbol_update;
	sp_object_class->modified = sp_symbol_modified;
	sp_object_class->write = sp_symbol_write;

	sp_item_class->show = sp_symbol_show;
	sp_item_class->hide = sp_symbol_hide;
	sp_item_class->bbox = sp_symbol_bbox;
	sp_item_class->print = sp_symbol_print;
}

static void
sp_symbol_init (SPSymbol *symbol)
{
	symbol->viewBox_set = FALSE;

	nr_matrix_d_set_identity (&symbol->c2p);
}

static void
sp_symbol_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	SPGroup *group;
	SPSymbol *symbol;

	group = (SPGroup *) object;
	symbol = (SPSymbol *) object;

	sp_object_read_attr (object, "viewBox");
	sp_object_read_attr (object, "preserveAspectRatio");

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);
}

static void
sp_symbol_release (SPObject *object)
{
	SPSymbol * symbol;

	symbol = (SPSymbol *) object;

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

static void
sp_symbol_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPItem *item;
	SPSymbol *symbol;

	item = SP_ITEM (object);
	symbol = SP_SYMBOL (object);

	switch (key) {
	case SP_ATTR_VIEWBOX:
		if (value) {
			double x, y, width, height;
			char *eptr;
			/* fixme: We have to take original item affine into account */
			/* fixme: Think (Lauris) */
			eptr = (gchar *) value;
			x = strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			y = strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			width = strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			height = strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			if ((width > 0) && (height > 0)) {
				/* Set viewbox */
				symbol->viewBox.x0 = x;
				symbol->viewBox.y0 = y;
				symbol->viewBox.x1 = x + width;
				symbol->viewBox.y1 = y + height;
				symbol->viewBox_set = TRUE;
			} else {
				symbol->viewBox_set = FALSE;
			}
		} else {
			symbol->viewBox_set = FALSE;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PRESERVEASPECTRATIO:
		/* Do setup before, so we can use break to escape */
		symbol->aspect_set = FALSE;
		symbol->aspect_align = SP_ASPECT_NONE;
		symbol->aspect_clip = SP_ASPECT_MEET;
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		if (value) {
			int len;
			unsigned char c[256];
			const unsigned char *p, *e;
			unsigned int align, clip;
			p = value;
			while (*p && *p == 32) p += 1;
			if (!*p) break;
			e = p;
			while (*e && *e != 32) e += 1;
			len = e - p;
			if (len > 8) break;
			memcpy (c, value, len);
			c[len] = 0;
			/* Now the actual part */
			if (!strcmp (c, "none")) {
				align = SP_ASPECT_NONE;
			} else if (!strcmp (c, "xMinYMin")) {
				align = SP_ASPECT_XMIN_YMIN;
			} else if (!strcmp (c, "xMidYMin")) {
				align = SP_ASPECT_XMID_YMIN;
			} else if (!strcmp (c, "xMaxYMin")) {
				align = SP_ASPECT_XMAX_YMIN;
			} else if (!strcmp (c, "xMinYMid")) {
				align = SP_ASPECT_XMIN_YMID;
			} else if (!strcmp (c, "xMidYMid")) {
				align = SP_ASPECT_XMID_YMID;
			} else if (!strcmp (c, "xMaxYMin")) {
				align = SP_ASPECT_XMAX_YMID;
			} else if (!strcmp (c, "xMinYMax")) {
				align = SP_ASPECT_XMIN_YMAX;
			} else if (!strcmp (c, "xMidYMax")) {
				align = SP_ASPECT_XMID_YMAX;
			} else if (!strcmp (c, "xMaxYMax")) {
				align = SP_ASPECT_XMAX_YMAX;
			} else {
				break;
			}
			clip = SP_ASPECT_MEET;
			while (*e && *e == 32) e += 1;
			if (e) {
				if (!strcmp (e, "meet")) {
					clip = SP_ASPECT_MEET;
				} else if (!strcmp (e, "slice")) {
					clip = SP_ASPECT_SLICE;
				} else {
					break;
				}
			}
			symbol->aspect_set = TRUE;
			symbol->aspect_align = align;
			symbol->aspect_clip = clip;
		}
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_symbol_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPSymbol *symbol;
	SPGroup *group;

	symbol = (SPSymbol *) object;
	group = (SPGroup *) object;

	if (((SPObjectClass *) (parent_class))->child_added)
		((SPObjectClass *) (parent_class))->child_added (object, child, ref);
}

static void
sp_symbol_remove_child (SPObject * object, SPRepr * child)
{
	SPSymbol *symbol;

	symbol = (SPSymbol *) object;

	if (((SPObjectClass *) (parent_class))->remove_child)
		((SPObjectClass *) (parent_class))->remove_child (object, child);
}

static void
sp_symbol_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPItem *item;
	SPSymbol *symbol;
	SPItemCtx *ictx, rctx;
	SPItemView *v;

	item = SP_ITEM (object);
	symbol = SP_SYMBOL (object);
	ictx = (SPItemCtx *) ctx;

	if (SP_OBJECT_IS_CLONED (object)) {
		/* Cloned <symbol> is actually renderable */

		/* fixme: We have to set up clip here too */

		/* Create copy of item context */
		rctx = *ictx;

		/* Calculate child to parent transformation */
		/* Apply parent <use> translation (set up as vewport) */
		nr_matrix_d_set_translate (&symbol->c2p, rctx.vp.x0, rctx.vp.y0);

		if (symbol->viewBox_set) {
			double x, y, width, height;
			NRMatrixD q;
			/* Determine actual viewbox in viewport coordinates */
			if (symbol->aspect_align == SP_ASPECT_NONE) {
				x = 0.0;
				y = 0.0;
				width = rctx.vp.x1 - rctx.vp.x0;
				height = rctx.vp.y1 - rctx.vp.y0;
			} else {
				double scalex, scaley, scale;
				/* Things are getting interesting */
				scalex = (rctx.vp.x1 - rctx.vp.x0) / (symbol->viewBox.x1 - symbol->viewBox.x0);
				scaley = (rctx.vp.y1 - rctx.vp.y0) / (symbol->viewBox.y1 - symbol->viewBox.y0);
				scale = (symbol->aspect_clip == SP_ASPECT_MEET) ? MIN (scalex, scaley) : MAX (scalex, scaley);
				width = (symbol->viewBox.x1 - symbol->viewBox.x0) * scale;
				height = (symbol->viewBox.y1 - symbol->viewBox.y0) * scale;
				/* Now place viewbox to requested position */
				switch (symbol->aspect_align) {
				case SP_ASPECT_XMIN_YMIN:
					x = 0.0;
					y = 0.0;
					break;
				case SP_ASPECT_XMID_YMIN:
					x = 0.5 * ((rctx.vp.x1 - rctx.vp.x0) - width);
					y = 0.0;
					break;
				case SP_ASPECT_XMAX_YMIN:
					x = 1.0 * ((rctx.vp.x1 - rctx.vp.x0) - width);
					y = 0.0;
					break;
				case SP_ASPECT_XMIN_YMID:
					x = 0.0;
					y = 0.5 * ((rctx.vp.y1 - rctx.vp.y0) - height);
					break;
				case SP_ASPECT_XMID_YMID:
					x = 0.5 * ((rctx.vp.x1 - rctx.vp.x0) - width);
					y = 0.5 * ((rctx.vp.y1 - rctx.vp.y0) - height);
					break;
				case SP_ASPECT_XMAX_YMID:
					x = 1.0 * ((rctx.vp.x1 - rctx.vp.x0) - width);
					y = 0.5 * ((rctx.vp.y1 - rctx.vp.y0) - height);
					break;
				case SP_ASPECT_XMIN_YMAX:
					x = 0.0;
					y = 1.0 * ((rctx.vp.y1 - rctx.vp.y0) - height);
					break;
				case SP_ASPECT_XMID_YMAX:
					x = 0.5 * ((rctx.vp.x1 - rctx.vp.x0) - width);
					y = 1.0 * ((rctx.vp.y1 - rctx.vp.y0) - height);
					break;
				case SP_ASPECT_XMAX_YMAX:
					x = 1.0 * ((rctx.vp.x1 - rctx.vp.x0) - width);
					y = 1.0 * ((rctx.vp.y1 - rctx.vp.y0) - height);
					break;
				default:
					x = 0.0;
					y = 0.0;
					break;
				}
			}
			/* Compose additional transformation from scale and position */
			q.c[0] = width / (symbol->viewBox.x1 - symbol->viewBox.x0);
			q.c[1] = 0.0;
			q.c[2] = 0.0;
			q.c[3] = height / (symbol->viewBox.y1 - symbol->viewBox.y0);
			q.c[4] = -symbol->viewBox.x0 * q.c[0] + x;
			q.c[5] = -symbol->viewBox.y0 * q.c[3] + y;
			/* Append viewbox transformation */
			nr_matrix_multiply_ddd (&symbol->c2p, &q, &symbol->c2p);
		}

		nr_matrix_multiply_ddd (&rctx.i2doc, &symbol->c2p, &rctx.i2doc);

		/* If viewBox is set initialize child viewport */
		/* Otherwise <use> has set it up already */
		if (symbol->viewBox_set) {
			rctx.vp.x0 = symbol->viewBox.x0;
			rctx.vp.y0 = symbol->viewBox.y0;
			rctx.vp.x1 = symbol->viewBox.x1;
			rctx.vp.y1 = symbol->viewBox.y1;
			nr_matrix_d_set_identity (&rctx.i2vp);
		}

		/* And invoke parent method */
		if (((SPObjectClass *) (parent_class))->update)
			((SPObjectClass *) (parent_class))->update (object, (SPCtx *) &rctx, flags);

		/* As last step set additional transform of arena group */
		for (v = item->display; v != NULL; v = v->next) {
			NRMatrixF vbf;
			nr_matrix_f_from_d (&vbf, &symbol->c2p);
			nr_arena_group_set_child_transform (NR_ARENA_GROUP (v->arenaitem), &vbf);
		}
	} else {
		/* No-op */
		if (((SPObjectClass *) (parent_class))->update)
			((SPObjectClass *) (parent_class))->update (object, ctx, flags);
	}
}

static void
sp_symbol_modified (SPObject *object, guint flags)
{
	SPSymbol *symbol;

	symbol = SP_SYMBOL (object);

	if (((SPObjectClass *) (parent_class))->modified)
		(* ((SPObjectClass *) (parent_class))->modified) (object, flags);
}

static SPRepr *
sp_symbol_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPSymbol *symbol;

	symbol = SP_SYMBOL (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("symbol");
	}

	sp_repr_set_attr (repr, "viewBox", sp_repr_attr (object->repr, "viewBox"));
	sp_repr_set_attr (repr, "preserveAspectRatio", sp_repr_attr (object->repr, "preserveAspectRatio"));

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static NRArenaItem *
sp_symbol_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
	SPSymbol *symbol;
	NRArenaItem *ai;

	symbol = SP_SYMBOL (item);

	if (SP_OBJECT_IS_CLONED (symbol)) {
		/* Cloned <symbol> is actually renderable */
		if (((SPItemClass *) (parent_class))->show) {
			ai = ((SPItemClass *) (parent_class))->show (item, arena, key, flags);
			if (ai) {
				NRMatrixF vbf;
				nr_matrix_f_from_d (&vbf, &symbol->c2p);
				nr_arena_group_set_child_transform (NR_ARENA_GROUP (ai), &vbf);
			}
		} else {
			ai = NULL;
		}
	} else {
		ai = NULL;
	}

	return ai;
}

static void
sp_symbol_hide (SPItem *item, unsigned int key)
{
	SPSymbol *symbol;

	symbol = SP_SYMBOL (item);

	if (SP_OBJECT_IS_CLONED (symbol)) {
		/* Cloned <symbol> is actually renderable */
		if (((SPItemClass *) (parent_class))->hide)
			((SPItemClass *) (parent_class))->hide (item, key);
	}
}

static void
sp_symbol_bbox (SPItem *item, NRRectF *bbox, const NRMatrixD *transform, unsigned int flags)
{
	SPSymbol *symbol;
	NRMatrixD a[6];

	symbol = SP_SYMBOL (item);

	if (SP_OBJECT_IS_CLONED (symbol)) {
		/* Cloned <symbol> is actually renderable */

		nr_matrix_multiply_ddd (a, &symbol->c2p, transform);

		if (((SPItemClass *) (parent_class))->bbox) {
			((SPItemClass *) (parent_class))->bbox (item, bbox, a, flags);
		}
	}
}

static void
sp_symbol_print (SPItem *item, SPPrintContext *ctx)
{
	SPSymbol *symbol;
	NRMatrixF t;

	symbol = SP_SYMBOL (item);

	if (SP_OBJECT_IS_CLONED (symbol)) {
		/* Cloned <symbol> is actually renderable */

		nr_matrix_f_from_d (&t, &symbol->c2p);
		sp_print_bind (ctx, &t, 1.0);

		if (((SPItemClass *) (parent_class))->print) {
			((SPItemClass *) (parent_class))->print (item, ctx);
		}

		sp_print_release (ctx);
	}
}
