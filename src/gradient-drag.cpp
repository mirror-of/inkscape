#define __GRADIENT_DRAG_C__

/*
 * Helper object for showing selected items
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/sigc++.h>
#include "desktop-handles.h"
#include "selection.h"
#include "desktop.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-util.h"
#include "display/sodipodi-ctrl.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrlrect.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-ops.h"
#include "prefs-utils.h"
#include "sp-item.h"
#include "style.h"
#include "knot.h"
#include "sp-gradient.h"
#include "sp-linear-gradient.h"
#include "gradient-chemistry.h"
#include "gradient-drag.h"

static void 
gr_drag_sel_changed(SPSelection *selection, gpointer data)
{
	GrDrag *drag = (GrDrag *) data;
	drag->updateDraggers ();
}

static void
gr_drag_sel_modified (SPSelection *selection, guint flags, gpointer data)
{
	GrDrag *drag = (GrDrag *) data;
	drag->updateDraggers ();
}


GrDrag::GrDrag(SPDesktop *desktop) {

	this->desktop = desktop;

	this->selection = SP_DT_SELECTION(desktop);

        this->draggers = NULL;
        this->lines = NULL;

	this->sel_changed_connection = this->selection->connectChanged(
            sigc::bind (
                sigc::ptr_fun(&gr_drag_sel_changed), 
                (gpointer)this )

	);
	this->sel_modified_connection = this->selection->connectModified(
            sigc::bind(
                sigc::ptr_fun(&gr_drag_sel_modified),
                (gpointer)this )
	);

	this->updateDraggers ();
}

GrDrag::~GrDrag() {
	this->sel_changed_connection.disconnect();
	this->sel_modified_connection.disconnect();

	for (GSList *l = this->draggers; l != NULL; l = l->next) {
          delete ((GrDragger *) l->data);
         //gtk_object_destroy( GTK_OBJECT (l->data));
	}
	g_slist_free (this->draggers);
	this->draggers = NULL;

	for (GSList *l = this->lines; l != NULL; l = l->next) {
         gtk_object_destroy( GTK_OBJECT (l->data));
	}
	g_slist_free (this->lines);
	this->lines = NULL;

}

// debugging, show a dragger as a simple canvas control
void
drag_mark (GrDrag *d, NR::Point p) 
{
    SPCanvasItem *box = sp_canvas_item_new (SP_DT_CONTROLS (d->desktop),
                                            SP_TYPE_CTRL,
                                            "mode", SP_CTRL_MODE_XOR,
                                            "shape", SP_CTRL_SHAPE_DIAMOND,
                                            "size", 5.0,
                                            "filled", TRUE,
                                            "fill_color", 0x000000ff,
                                            "stroked", FALSE,
                                            "stroke_color", 0x000000ff,
                                            NULL);
    sp_canvas_item_show (box);
    SP_CTRL(box)->moveto (p);
    sp_canvas_item_move_to_z (box, 0); // just low enough to not get in the way of other draggable knots
    d->draggers = g_slist_append (d->draggers, box);
}

GrDraggable::GrDraggable (SPItem *item, guint point_num, bool fill_or_stroke)
{
    this->item = item;
    this->point_num = point_num;
    this->fill_or_stroke = fill_or_stroke;

	g_object_ref (G_OBJECT (this->item));
}

GrDraggable::~GrDraggable ()
{
	g_object_unref (G_OBJECT (this->item));
}

GrDragger::GrDragger (SPDesktop *desktop, NR::Point p, gchar const *tip, GrDraggable *draggable) 
{
    this->draggables = NULL;

	this->knot = sp_knot_new (desktop, tip);
	g_object_set (G_OBJECT (this->knot->item), "shape", SP_KNOT_SHAPE_SQUARE, NULL);
	g_object_set (G_OBJECT (this->knot->item), "mode", SP_KNOT_MODE_XOR, NULL);

	/* Move to current point. */
	sp_knot_set_position (this->knot, &p, SP_KNOT_STATE_NORMAL);
	sp_knot_show (this->knot);

  this->draggables = g_slist_prepend (this->draggables, draggable);
}

GrDragger::~GrDragger ()
{
			/* unref should call destroy */
			g_object_unref (G_OBJECT (this->knot));

    for (GSList const* l = this->draggables; l != NULL; l = l->next) {
        delete ((GrDraggable *) l->data);
    }
    g_slist_free (this->draggables);
    this->draggables = NULL;
}

void
GrDrag::addLine (NR::Point p1, NR::Point p2) 
{
    SPCanvasItem *line = sp_canvas_item_new(SP_DT_CONTROLS(this->desktop),
                                                            SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_coords(SP_CTRLLINE(line), p1, p2);
    sp_canvas_item_show (line);
    sp_canvas_item_move_to_z (line, 0); // just low enough to not get in the way of other draggable knots
    this->lines = g_slist_append (this->lines, line);
}

void
GrDrag::updateDraggers ()
{
    for (GSList const* l = this->draggers; l != NULL; l = l->next) {
        delete ((GrDragger *) l->data);
        //gtk_object_destroy( GTK_OBJECT (l->data));
    }
    g_slist_free (this->draggers);
    this->draggers = NULL;

	for (GSList *l = this->lines; l != NULL; l = l->next) {
         gtk_object_destroy( GTK_OBJECT (l->data));
	}
	g_slist_free (this->lines);
	this->lines = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* l = this->selection->itemList(); l != NULL; l = l->next) {

        SPItem *item = SP_ITEM(l->data);

        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                SPLinearGradient *lg = SP_LINEARGRADIENT (server);
                NR::Point p1 (lg->x1.computed, lg->y1.computed);
                NR::Point p2 (lg->x2.computed, lg->y2.computed);
                p1 *= NR::Matrix (lg->gradientTransform) * sp_item_i2d_affine (item);
                p2 *= NR::Matrix (lg->gradientTransform) * sp_item_i2d_affine (item);

                this->draggers = g_slist_prepend (this->draggers, new GrDragger(this->desktop, p1, "drag1", new GrDraggable (item, POINT_LG_P1, true)));
                this->draggers = g_slist_prepend (this->draggers, new GrDragger(this->desktop, p2, "drag2", new GrDraggable (item, POINT_LG_P2, true)));
                this->addLine (p1, p2);
            }
        }
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
