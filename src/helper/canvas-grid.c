#define SP_CANVAS_GRID_C

/*
 * SPCGrid
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <math.h>

#include <libart_lgpl/art_affine.h>

#include "sp-canvas.h"
#include "sp-canvas-util.h"
#include "canvas-grid.h"

enum {
	ARG_0,
	ARG_ORIGINX,
	ARG_ORIGINY,
	ARG_SPACINGX,
	ARG_SPACINGY,
	ARG_COLOR
};


static void sp_cgrid_class_init (SPCGridClass *klass);
static void sp_cgrid_init (SPCGrid *grid);
static void sp_cgrid_destroy (GtkObject *object);
static void sp_cgrid_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void sp_cgrid_update (SPCanvasItem *item, double *affine, unsigned int flags);
static void sp_cgrid_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass * parent_class;

GtkType
sp_cgrid_get_type (void)
{
	static GtkType cgrid_type = 0;

	if (!cgrid_type) {
		GtkTypeInfo cgrid_info = {
			"SPCGrid",
			sizeof (SPCGrid),
			sizeof (SPCGridClass),
			(GtkClassInitFunc) sp_cgrid_class_init,
			(GtkObjectInitFunc) sp_cgrid_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		cgrid_type = gtk_type_unique (sp_canvas_item_get_type (), &cgrid_info);
	}
	return cgrid_type;
}

static void
sp_cgrid_class_init (SPCGridClass *klass)
{
	GtkObjectClass *object_class;
	SPCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) klass;
	item_class = (SPCanvasItemClass *) klass;

	parent_class = gtk_type_class (sp_canvas_item_get_type ());

	gtk_object_add_arg_type ("SPCGrid::originx", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ORIGINX);
	gtk_object_add_arg_type ("SPCGrid::originy", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ORIGINY);
	gtk_object_add_arg_type ("SPCGrid::spacingx", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_SPACINGX);
	gtk_object_add_arg_type ("SPCGrid::spacingy", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_SPACINGY);
	gtk_object_add_arg_type ("SPCGrid::color", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_COLOR);

	object_class->destroy = sp_cgrid_destroy;
	object_class->set_arg = sp_cgrid_set_arg;

	item_class->update = sp_cgrid_update;
	item_class->render = sp_cgrid_render;
}

static void
sp_cgrid_init (SPCGrid *grid)
{
	grid->origin.x = grid->origin.y = 0.0;
	grid->spacing.x = grid->spacing.y = 8.0;
	grid->color = 0x0000ff7f;
}

static void
sp_cgrid_destroy (GtkObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (SP_IS_CGRID (object));

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_cgrid_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	SPCanvasItem *item;
	SPCGrid *grid;

	item = SP_CANVAS_ITEM (object);
	grid = SP_CGRID (object);

	switch (arg_id) {
	case ARG_ORIGINX:
		grid->origin.x = GTK_VALUE_DOUBLE (* arg);
		sp_canvas_item_request_update (item);
		break;
	case ARG_ORIGINY:
		grid->origin.y = GTK_VALUE_DOUBLE (* arg);
		sp_canvas_item_request_update (item);
		break;
	case ARG_SPACINGX:
		grid->spacing.x = GTK_VALUE_DOUBLE (* arg);
		if (grid->spacing.x < 0.01) grid->spacing.x = 0.01;
		sp_canvas_item_request_update (item);
		break;
	case ARG_SPACINGY:
		grid->spacing.y = GTK_VALUE_DOUBLE (* arg);
		if (grid->spacing.y < 0.01) grid->spacing.y = 0.01;
		sp_canvas_item_request_update (item);
		break;
	case ARG_COLOR:
		grid->color = GTK_VALUE_INT (* arg);
		sp_canvas_item_request_update (item);
		break;
	default:
		break;
	}
}

#define RGBA_R(v) ((v) >> 24)
#define RGBA_G(v) (((v) >> 16) & 0xff)
#define RGBA_B(v) (((v) >> 8) & 0xff)
#define RGBA_A(v) ((v) & 0xff)
#define COMPOSE(b,f,a) (((255 - (a)) * b + (f * a) + 127) / 255)

static void
sp_grid_hline (SPCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba)
{
	if ((y >= buf->rect.y0) && (y < buf->rect.y1)) {
		guint r, g, b, a;
		gint x0, x1, x;
		guchar *p;
		r = RGBA_R (rgba);
		g = RGBA_G (rgba);
		b = RGBA_B (rgba);
		a = RGBA_A (rgba);
		x0 = MAX (buf->rect.x0, xs);
		x1 = MIN (buf->rect.x1, xe + 1);
		p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x0 - buf->rect.x0) * 3;
		for (x = x0; x < x1; x++) {
			p[0] = COMPOSE (p[0], r, a);
			p[1] = COMPOSE (p[1], g, a);
			p[2] = COMPOSE (p[2], b, a);
			p += 3;
		}
	}
}

static void
sp_grid_vline (SPCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba)
{
	if ((x >= buf->rect.x0) && (x < buf->rect.x1)) {
		guint r, g, b, a;
		gint y0, y1, y;
		guchar *p;
		r = RGBA_R (rgba);
		g = RGBA_G (rgba);
		b = RGBA_B (rgba);
		a = RGBA_A (rgba);
		y0 = MAX (buf->rect.y0, ys);
		y1 = MIN (buf->rect.y1, ye + 1);
		p = buf->buf + (y0 - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 3;
		for (y = y0; y < y1; y++) {
			p[0] = COMPOSE (p[0], r, a);
			p[1] = COMPOSE (p[1], g, a);
			p[2] = COMPOSE (p[2], b, a);
			p += buf->buf_rowstride;
		}
	}
}

static void
sp_cgrid_render (SPCanvasItem * item, SPCanvasBuf * buf)
{
	SPCGrid * grid;
	gdouble sxg, syg, x, y;

	grid = SP_CGRID (item);

	sp_canvas_buf_ensure_buf (buf);
	buf->is_bg = FALSE;

	sxg = floor ((buf->rect.x0 - grid->ow.x) / grid->sw.x) * grid->sw.x + grid->ow.x;
	syg = floor ((buf->rect.y0 - grid->ow.y) / grid->sw.y) * grid->sw.y + grid->ow.y;

	for (y = syg; y < buf->rect.y1; y += grid->sw.y) {
		gint y0, y1;
		y0 = (gint) floor (y + 0.5);
		y1 = (gint) floor (y + grid->sw.y + 0.5);
		sp_grid_hline (buf, y0, buf->rect.x0, buf->rect.x1 - 1, grid->color);

		for (x = sxg; x < buf->rect.x1; x += grid->sw.x) {
			gint ix;
			ix = (gint) floor (x + 0.5);
			sp_grid_vline (buf, ix, y0 + 1, y1 - 1, grid->color);
		}
	}
}

static void
sp_cgrid_update (SPCanvasItem *item, double * affine, unsigned int flags)
{
	SPCGrid * grid;
	GtkWidget * w;

	grid = SP_CGRID (item);
	w = GTK_WIDGET (item->canvas);

	if (parent_class->update)
		(* parent_class->update) (item, affine, flags);

	art_affine_point (&grid->ow, &grid->origin, affine);
	art_affine_point (&grid->sw, &grid->spacing, affine);
	grid->sw.x -= affine[4];
	grid->sw.y -= affine[5];
	grid->sw.x = fabs (grid->sw.x);
	grid->sw.y = fabs (grid->sw.y);
	while (grid->sw.x < 8.0) {
		grid->sw.x *= 5.0;
		if (grid->sw.x < 8.0) grid->sw.x *= 2.0;
	}
	while (grid->sw.y < 8.0) {
		grid->sw.y *= 5.0;
		if (grid->sw.y < 8.0) grid->sw.y *= 2.0;
	}

	sp_canvas_request_redraw (item->canvas,
				     -1000000, -1000000,
				     1000000, 1000000);
				     
	item->x1 = item->y1 = -1000000;
	item->x2 = item->y2 = 1000000;
}


