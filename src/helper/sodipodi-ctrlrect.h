#ifndef __SODIPODI_CTRLRECT_H__
#define __SODIPODI_CTRLRECT_H__

/*
 * Simple non-transformed rectangle, usable for rubberband
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include <glib.h>
#include <libart_lgpl/art_rect.h>
#include "sp-canvas.h"

G_BEGIN_DECLS

#define SP_TYPE_CTRLRECT (sp_ctrlrect_get_type ())
#define SP_CTRLRECT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CTRLRECT, SPCtrlRect))
#define SP_CTRLRECT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CTRLRECT, SPCtrlRectClass))
#define SP_IS_CTRLRECT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CTRLRECT))
#define SP_IS_CTRLRECT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CTRLRECT))

typedef struct _SPCtrlRect SPCtrlRect;
typedef struct _SPCtrlRectClass SPCtrlRectClass;

struct _SPCtrlRect {
	SPCanvasItem item;

	guint has_fill : 1;

	ArtDRect rect;
	gint shadow;

	ArtIRect area;
	gint shadow_size;
	guint32 border_color;
	guint32 fill_color;
	guint32 shadow_color;
};

struct _SPCtrlRectClass {
	SPCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType sp_ctrlrect_get_type (void);

void sp_ctrlrect_set_area (SPCtrlRect *rect, double x0, double y0, double x1, double y1);
void sp_ctrlrect_set_color (SPCtrlRect *rect, guint32 border_color, gboolean has_fill, guint32 fill_color);
void sp_ctrlrect_set_shadow (SPCtrlRect *rect, gint shadow_size, guint32 shadow_color);

/* Deprecated */
void sp_ctrlrect_set_rect (SPCtrlRect * rect, ArtDRect * box);

G_END_DECLS

#endif
