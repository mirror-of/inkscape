#ifndef __SP_KNOT_H__
#define __SP_KNOT_H__

/*
 * Desktop-bound visual control object
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include <gtk/gtkenums.h>
#include "display/display-forward.h"
#include "forward.h"
#include <libnr/nr-point.h>
#include "knot-enums.h"

class SPKnot;
class SPKnotClass;

#define SP_TYPE_KNOT            (sp_knot_get_type ())
#define SP_KNOT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_KNOT, SPKnot))
#define SP_KNOT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_KNOT, SPKnotClass))
#define SP_IS_KNOT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_KNOT))
#define SP_IS_KNOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_KNOT))

struct SPKnot {
	GObject object;
	SPDesktop * desktop;		/* Desktop we are on */
	SPCanvasItem * item;		/* Our CanvasItem */
	guint flags;

	guint size;			/* Always square */
	NR::Point pos;			/* Our desktop coordinates */
	NR::Point grabbed_rel_pos;	/* Grabbed relative position */
	NR::Point drag_origin;	/* Origin of drag */
	GtkAnchorType anchor;		/* Anchor */

	SPKnotShapeType shape;		/* Shape type */
	SPKnotModeType mode;

	guint32 fill[SP_KNOT_VISIBLE_STATES];
	guint32 stroke[SP_KNOT_VISIBLE_STATES];
	guchar *image[SP_KNOT_VISIBLE_STATES];

	GdkCursor *cursor[SP_KNOT_VISIBLE_STATES];

	GdkCursor *saved_cursor;
        gpointer pixbuf;

	gchar *tip;
};

struct SPKnotClass {
	GObjectClass parent_class;

	gint (* event) (SPKnot * knot, GdkEvent * event);

	/*
	 * These are unconditional
	 */

	void (* clicked) (SPKnot *knot, guint state);
	void (* grabbed) (SPKnot *knot, guint state);
	void (* ungrabbed) (SPKnot *knot, guint state);
	void (* moved) (SPKnot *knot, NR::Point *position, guint state);
	void (* stamped)  (SPKnot *know, guint state);
	
	/* Request knot to move to absolute position */
	gboolean (* request) (SPKnot *knot, NR::Point *pos, guint state);

	/* Find complex distance from knot to point */
	gdouble (* distance) (SPKnot *knot, NR::Point *pos, guint state);
};

GType sp_knot_get_type (void);

SPKnot * sp_knot_new (SPDesktop * desktop, const gchar *tip = NULL);

#define SP_KNOT_IS_VISIBLE(k) ((k->flags & SP_KNOT_VISIBLE) != 0)
#define SP_KNOT_IS_MOSEOVER(k) ((k->flags & SP_KNOT_MOUSEOVER) != 0)
#define SP_KNOT_IS_DRAGGING(k) ((k->flags & SP_KNOT_DRAGGING) != 0)
#define SP_KNOT_IS_GRABBED(k) ((k->flags & SP_KNOT_GRABBED) != 0)

void sp_knot_show (SPKnot *knot);
void sp_knot_hide (SPKnot *knot);

void sp_knot_request_position (SPKnot * knot, NR::Point *pos, guint state);
gdouble sp_knot_distance (SPKnot * knot, NR::Point *p, guint state);

/* Unconditional */

void sp_knot_set_position(SPKnot *knot, NR::Point *p, guint state);

NR::Point sp_knot_position (SPKnot const *knot);

#endif
