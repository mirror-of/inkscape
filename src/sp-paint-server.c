#define __SP_PAINT_SERVER_C__

/*
 * Base class for gradients and patterns
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-pixblock-pattern.h>
#include "sp-paint-server.h"

static void sp_paint_server_class_init (SPPaintServerClass *klass);
static void sp_paint_server_init (SPPaintServer *ps);

static void sp_paint_server_release (SPObject *object);

static void sp_painter_stale_fill (SPPainter *painter, NRPixBlock *pb);

static SPObjectClass *parent_class;
static GSList *stale_painters = NULL;

GType
sp_paint_server_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPPaintServerClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_paint_server_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPPaintServer),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_paint_server_init,
		};
		type = g_type_register_static (SP_TYPE_OBJECT, "SPPaintServer", &info, 0);
	}
	return type;
}

static void
sp_paint_server_class_init (SPPaintServerClass *klass)
{
	SPObjectClass *sp_object_class;

	sp_object_class = (SPObjectClass *) klass;

	parent_class = g_type_class_ref (SP_TYPE_OBJECT);

	sp_object_class->release = sp_paint_server_release;
}

static void
sp_paint_server_init (SPPaintServer *ps)
{
	ps->painters = NULL;
}

static void
sp_paint_server_release (SPObject *object)
{
	SPPaintServer *ps;

	ps = SP_PAINT_SERVER (object);

	while (ps->painters) {
		SPPainter *painter;
		painter = ps->painters;
		ps->painters = painter->next;
		stale_painters = g_slist_prepend (stale_painters, painter);
		painter->next = NULL;
		painter->server = NULL;
		painter->fill = sp_painter_stale_fill;
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

SPPainter *
sp_paint_server_painter_new (SPPaintServer *ps, const gdouble *affine, const NRRectF *bbox)
{
	SPPainter *painter;
	SPPaintServerClass *psc;

	g_return_val_if_fail (ps != NULL, NULL);
	g_return_val_if_fail (SP_IS_PAINT_SERVER (ps), NULL);
	g_return_val_if_fail (affine != NULL, NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	painter = NULL;
	psc = (SPPaintServerClass *) G_OBJECT_GET_CLASS (ps);
	if (((SPPaintServerClass *) G_OBJECT_GET_CLASS(ps))->painter_new)
		painter = (* ((SPPaintServerClass *) G_OBJECT_GET_CLASS(ps))->painter_new) (ps, affine, bbox);

	if (painter) {
		painter->next = ps->painters;
		painter->server = ps;
		painter->type = G_OBJECT_TYPE (ps);
		ps->painters = painter;
	}

	return painter;
}

static void
sp_paint_server_painter_free (SPPaintServer *ps, SPPainter *painter)
{
	SPPainter *p, *r;

	g_return_if_fail (ps != NULL);
	g_return_if_fail (SP_IS_PAINT_SERVER (ps));
	g_return_if_fail (painter != NULL);

	r = NULL;
	for (p = ps->painters; p != NULL; p = p->next) {
		if (p == painter) {
			if (r) {
				r->next = p->next;
			} else {
				ps->painters = p->next;
			}
			p->next = NULL;
			if (((SPPaintServerClass *) G_OBJECT_GET_CLASS(ps))->painter_free)
				(* ((SPPaintServerClass *) G_OBJECT_GET_CLASS(ps))->painter_free) (ps, painter);
			return;
		}
		r = p;
	}
	g_assert_not_reached ();
}

SPPainter *
sp_painter_free (SPPainter *painter)
{
	g_return_val_if_fail (painter != NULL, NULL);

	if (painter->server) {
		sp_paint_server_painter_free (painter->server, painter);
	} else {
		if (((SPPaintServerClass *) g_type_class_ref (painter->type))->painter_free)
			(* ((SPPaintServerClass *) g_type_class_ref (painter->type))->painter_free) (NULL, painter);
		stale_painters = g_slist_remove (stale_painters, painter);
	}

	return NULL;
}

static void
sp_painter_stale_fill (SPPainter *painter, NRPixBlock *pb)
{
	nr_pixblock_render_gray_noise (pb, NULL);
}


