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
#include "document.h"
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

#define GR_KNOT_COLOR_NORMAL 0xffffff00
#define GR_KNOT_COLOR_SELECTED 0x0000ff00

// screen pixels between knots when they snap:
#define SNAP_DIST 4 

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

static void 
gr_drag_sel_changed(SPSelection *selection, gpointer data)
{
	GrDrag *drag = (GrDrag *) data;
	drag->updateDraggers ();
	drag->updateLines ();
}

static void
gr_drag_sel_modified (SPSelection *selection, guint flags, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;
    if (drag->local_change) {
        drag->local_change = false;
    } else {
        drag->updateDraggers ();
    }
    drag->updateLines ();
}


GrDrag::GrDrag(SPDesktop *desktop) {

    this->desktop = desktop;

    this->selection = SP_DT_SELECTION(desktop);

    this->draggers = NULL;
    this->lines = NULL;
    this->selected = NULL;

    this->local_change = false;

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
    this->updateLines ();
}

GrDrag::~GrDrag() 
{
	this->sel_changed_connection.disconnect();
	this->sel_modified_connection.disconnect();

	for (GSList *l = this->draggers; l != NULL; l = l->next) {
          delete ((GrDragger *) l->data);
	}
	g_slist_free (this->draggers);
	this->draggers = NULL;
	this->selected = NULL;

	for (GSList *l = this->lines; l != NULL; l = l->next) {
         gtk_object_destroy( GTK_OBJECT (l->data));
	}
	g_slist_free (this->lines);
	this->lines = NULL;

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

static void
gr_knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->point = *p;

    for (GSList const* l = dragger->draggables; l != NULL; l = l->next) {
        GrDraggable *draggable = (GrDraggable *) l->data;
        dragger->parent->local_change = true;
        sp_item_gradient_set_coords (draggable->item, draggable->point_num, *p, draggable->fill_or_stroke, false);
    }

    // See if we need to snap to another dragger
    double snap_dist = SNAP_DIST / SP_DESKTOP_ZOOM (dragger->parent->desktop);
    for (GSList *l = dragger->parent->draggers; l != NULL; l = l->next) {
        GrDragger *d_new = (GrDragger *) l->data;
        if (d_new == dragger)
            continue;
        if (NR::L2 (d_new->point - *p) < snap_dist) {
            for (GSList const* l = dragger->draggables; l != NULL; l = l->next) { // for all draggables of dragger
                GrDraggable *draggable = (GrDraggable *) l->data;
                // copy draggable to d_new:
                GrDraggable *da_new = new GrDraggable (draggable->item, draggable->point_num, draggable->fill_or_stroke);
                d_new->addDraggable (da_new); 
                // move to the exact position of d_new, writing to repr:
                sp_item_gradient_set_coords (da_new->item, da_new->point_num, d_new->point, da_new->fill_or_stroke, true);
            }
            dragger->parent->draggers = g_slist_remove (dragger->parent->draggers, dragger);
            delete dragger;
            d_new->parent->updateLines();
            sp_document_done (SP_DT_DOCUMENT (d_new->parent->desktop));
            return;
        }
    }

}

static void
gr_knot_ungrabbed_handler (SPKnot *knot, unsigned int state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    for (GSList const* l = dragger->draggables; l != NULL; l = l->next) {
        GrDraggable *draggable = (GrDraggable *) l->data;
        dragger->parent->local_change = true;
        sp_item_gradient_set_coords (draggable->item, draggable->point_num, knot->pos, draggable->fill_or_stroke, true);
    }

    dragger->parent->setSelected (dragger);

    sp_document_done (SP_DT_DOCUMENT (dragger->parent->desktop));
}

static void
gr_knot_clicked_handler(SPKnot *knot, guint state, gpointer data)
{
   GrDragger *dragger = (GrDragger *) data;

   dragger->parent->setSelected (dragger);
}

void
GrDragger::addDraggable (GrDraggable *draggable)
{
    this->draggables = g_slist_prepend (this->draggables, draggable);
}


GrDragger::GrDragger (GrDrag *parent, NR::Point p, gchar const *tip, GrDraggable *draggable) 
{
    this->draggables = NULL;

    this->parent = parent;

    this->point = p;

    this->knot = sp_knot_new (parent->desktop, tip);
    g_object_set (G_OBJECT (this->knot->item), "shape", SP_KNOT_SHAPE_SQUARE, NULL);
    g_object_set (G_OBJECT (this->knot->item), "mode", SP_KNOT_MODE_XOR, NULL);
    this->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;

    /* Move to current point. */
    sp_knot_set_position (this->knot, &p, SP_KNOT_STATE_NORMAL);
    sp_knot_show (this->knot);

    this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "clicked", G_CALLBACK (gr_knot_clicked_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "ungrabbed", G_CALLBACK (gr_knot_ungrabbed_handler), this);

    this->addDraggable (draggable);
}

GrDragger::~GrDragger ()
{
    /* unref should call destroy */
    g_object_unref (G_OBJECT (this->knot));

    if (this->parent->selected == this)
        this->parent->selected = NULL;

    for (GSList const* l = this->draggables; l != NULL; l = l->next) {
        delete ((GrDraggable *) l->data);
    }
    g_slist_free (this->draggables);
    this->draggables = NULL;
}

void
GrDrag::setSelected (GrDragger *dragger) 
{
    if (this->selected) {
       this->selected->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;
       g_object_set (G_OBJECT (this->selected->knot->item), "fill_color", GR_KNOT_COLOR_NORMAL, NULL);
    }
    if (dragger)
        dragger->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_SELECTED;
    this->selected = dragger;
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
GrDrag::addDragger (NR::Point p, const gchar *tip, GrDraggable *draggable)
{
    for (GSList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *dragger = (GrDragger *) l->data;
        if (NR::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addDraggable (draggable);
            return;
        }
    }

    this->draggers = g_slist_prepend (this->draggers, new GrDragger(this, p, tip, draggable));
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
    this->selected = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* l = this->selection->itemList(); l != NULL; l = l->next) {

        SPItem *item = SP_ITEM(l->data);

        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                SPLinearGradient *lg = SP_LINEARGRADIENT (server);

                addDragger (sp_lg_get_p1 (item, lg), "drag1", new GrDraggable (item, POINT_LG_P1, true));
                addDragger (sp_lg_get_p2 (item, lg), "drag2", new GrDraggable (item, POINT_LG_P2, true));
            }
        }

        if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                SPLinearGradient *lg = SP_LINEARGRADIENT (server);

                addDragger (sp_lg_get_p1 (item, lg), "drag1", new GrDraggable (item, POINT_LG_P1, false));
                addDragger (sp_lg_get_p2 (item, lg), "drag2", new GrDraggable (item, POINT_LG_P2, false));
            }
        }


    }
}

void
GrDrag::updateLines ()
{
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

                this->addLine (sp_lg_get_p1 (item, lg), sp_lg_get_p2 (item, lg));
            }
        }

        if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                SPLinearGradient *lg = SP_LINEARGRADIENT (server);

                this->addLine (sp_lg_get_p1 (item, lg), sp_lg_get_p2 (item, lg));
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
