#ifndef __GRADIENT_DRAG_H__
#define __GRADIENT_DRAG_H__

/*
 * On-canvas gradient dragging 
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gslist.h>
#include <sigc++/sigc++.h>

#include <forward.h>
#include <knot-enums.h>

struct SPItem;
struct SPKnot;
namespace NR {
class Point;
}

/**
This class represents a single draggable point of a gradient. It remembers the item
which has the gradient, whether it's fill or stroke, and the point number (from the
GrPoint enum).
*/
struct GrDraggable {
	GrDraggable(SPItem *item, guint point_num, bool fill_or_stroke);
	~GrDraggable();

	SPItem *item;
	guint point_num;
	bool fill_or_stroke;
};

struct GrDrag;

/**
This class holds together a visible on-canvas knot and a list of draggables that need to
be moved when the knot moves. Normally there's one draggable in the list, but there may
be more when draggers are snapped together.
*/
struct GrDragger {
	GrDragger (GrDrag *parent, NR::Point p, GrDraggable *draggable, SPKnotShapeType shape);
	~GrDragger();

	SPKnot *knot;

	NR::Point point;

	/** Connection to \a knot's "moved" signal, for blocking it (unused?). */
	guint   handler_id;

	GSList *draggables;

	GrDrag *parent;

	void addDraggable(GrDraggable *draggable);

	void updateTip();
};

/**
This is the root class of the gradient dragging machinery. It holds lists of GrDraggers
and of lines (simple canvas items). It also remembers one of the draggers as selected.
*/
struct GrDrag {
	GrDrag(SPDesktop *desktop);
	~GrDrag();

	void addLine (NR::Point p1, NR::Point p2);

	void updateDraggers ();
	void updateLines ();

	void addDragger (NR::Point p, GrDraggable *draggable, SPKnotShapeType shape);

	void GrDrag::addDraggersRadial (SPRadialGradient *rg, SPItem *item, bool fill_or_stroke);
	void GrDrag::addDraggersLinear (SPLinearGradient *lg, SPItem *item, bool fill_or_stroke);

	GrDragger *selected;
	void setSelected (GrDragger *dragger);
	void GrDrag::restoreSelected (GrDraggable *da1);

	bool local_change;

	SPDesktop *desktop;
	SPSelection *selection;
	sigc::connection sel_changed_connection;
	sigc::connection sel_modified_connection;
	GSList *draggers;
	GSList *lines;
};

#endif
