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

struct SPItem;
struct SPKnot;
namespace NR {
class Point;
}

struct GrDraggable {
	GrDraggable();
	~GrDraggable();

	SPItem *item;
	guint fill_or_stroke;
	guint point_num;
};

struct GrDragger {
	GrDragger();
	~GrDragger();

    SPKnot *knot;

    /** Connection to \a knot's "moved" signal. */
    guint   handler_id;

	GSList *draggables;

	void addDraggable(GrDraggable *draggable);

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

	void updateDraggers ();

	SPDesktop *desktop;
	SPSelection *selection;
	sigc::connection sel_changed_connection;
	sigc::connection sel_modified_connection;
	GSList *draggers;
};

#endif
