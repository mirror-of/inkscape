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

struct GrDraggable {
	GrDraggable(SPItem *item, guint point_num, bool fill_or_stroke);
	~GrDraggable();

	SPItem *item;
	guint point_num;
	bool fill_or_stroke;
};

struct GrDrag;

struct GrDragger {
	GrDragger (GrDrag *parent, NR::Point p, GrDraggable *draggable, SPKnotShapeType shape);
	~GrDragger();

    SPKnot *knot;

	NR::Point point;

    /** Connection to \a knot's "moved" signal. */
    guint   handler_id;

	GSList *draggables;

	GrDrag *parent;

	void addDraggable(GrDraggable *draggable);

	void updateTip();

    /**
     * Called solely from knot_moved_handler.
     *
     * \param p Requested position of the knot, in item coordinates
     * \param origin Position where the knot started being dragged
     * \param state GTK event state (for keyboard modifiers)
     */
    void (* knot_set) (SPItem *item, NR::Point const &p, NR::Point const &origin, guint state);

    /**
     * Returns the position of the knot representation, in item coordinates.
     */
    NR::Point (* knot_get) (SPItem *item);

    void (* knot_click) (SPItem *item, guint state);
};

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

	bool local_change;

	SPDesktop *desktop;
	SPSelection *selection;
	sigc::connection sel_changed_connection;
	sigc::connection sel_modified_connection;
	GSList *draggers;
	GSList *lines;
};

#endif
