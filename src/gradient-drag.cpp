#define __GRADIENT_DRAG_C__

/*
 * On-canvas gradient dragging
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/sigc++.h>
#include <glibmm/i18n.h>

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
#include "sp-radial-gradient.h"
#include "gradient-chemistry.h"
#include "gradient-drag.h"

#define GR_KNOT_COLOR_NORMAL 0xffffff00
#define GR_KNOT_COLOR_SELECTED 0x0000ff00

// screen pixels between knots when they snap:
#define SNAP_DIST 5

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

// knot shapes corresponding to GrPoint enum
SPKnotShapeType gr_knot_shapes [] = {
        SP_KNOT_SHAPE_SQUARE, //POINT_LG_P1
        SP_KNOT_SHAPE_SQUARE,
        SP_KNOT_SHAPE_DIAMOND,
        SP_KNOT_SHAPE_CIRCLE,
        SP_KNOT_SHAPE_CIRCLE,
        SP_KNOT_SHAPE_CROSS // POINT_RG_FOCUS
};

const gchar *gr_knot_descr [] = {
    N_("Linear gradient <b>start</b>"), //POINT_LG_P1
    N_("Linear gradient <b>end</b>"),
    N_("Radial gradient <b>center</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>focus</b>") // POINT_RG_FOCUS
};

static void 
gr_drag_sel_changed(SPSelection *selection, gpointer data)
{
	GrDrag *drag = (GrDrag *) data;
	drag->updateDraggers ();
	drag->updateLines ();
	drag->updateLevels ();
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
    drag->updateLevels ();
}


GrDrag::GrDrag(SPDesktop *desktop) {

    this->desktop = desktop;

    this->selection = SP_DT_SELECTION(desktop);

    this->draggers = NULL;
    this->lines = NULL;
    this->selected = NULL;

    this->hor_levels.clear();
    this->vert_levels.clear();

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
    this->updateLevels ();
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

NR::Point *
get_snap_vector (NR::Point p, NR::Point o, double snap, double initial)
{
    double r = NR::L2 (p - o);
    if (r < 1e-3) 
        return NULL;
    double angle = NR::atan2 (p - o);
    // snap angle to snaps increments, starting from initial:
    double a_snapped = initial + floor((angle - initial)/snap + 0.5) * snap;
    // calculate the new position and subtract p to get the vector:
    return new NR::Point (o + r * NR::Point(cos(a_snapped), sin(a_snapped)) - p);
}

static void
gr_knot_moved_handler(SPKnot *knot, NR::Point const *ppointer, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    NR::Point p = *ppointer;

    // FIXME: take from prefs
    double snap_dist = SNAP_DIST / SP_DESKTOP_ZOOM (dragger->parent->desktop);

    if (state & GDK_CONTROL_MASK) {
        unsigned snaps = abs(prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12));
        /* 0 means no snapping. */

        // This list will store snap vectors from all draggables of dragger
        GSList *snap_vectors = NULL;

        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) {
            GrDraggable *draggable = (GrDraggable *) i->data;

            NR::Point *dr_snap = NULL;

            if (draggable->point_num == POINT_LG_P1 || draggable->point_num == POINT_LG_P2) {
                for (GSList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item, 
                                    draggable->point_num == POINT_LG_P1? POINT_LG_P2 : POINT_LG_P1,
                                    draggable->fill_or_stroke)) {
                        // found the other end of the linear gradient;
                        dr_snap = &(d_new->point);
                    }
                }
            } else if (draggable->point_num == POINT_RG_R1 || draggable->point_num == POINT_RG_R2 || draggable->point_num == POINT_RG_FOCUS) {
                for (GSList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item, 
                                    POINT_RG_CENTER,
                                    draggable->fill_or_stroke)) {
                        // found the center of the radial gradient;
                        dr_snap = &(d_new->point);
                    }
                }
            } else if (draggable->point_num == POINT_RG_CENTER) {
                // radial center snaps to hor/vert relative to its original position
                dr_snap = &(dragger->point_original);
            }

            NR::Point *snap_vector = NULL;
            if (dr_snap) {
                if (state & GDK_MOD1_MASK) {
                    // with Alt, snap to the original angle and its perpendiculars
                    snap_vector = get_snap_vector (p, *dr_snap, M_PI/2, NR::atan2 (dragger->point_original - *dr_snap));
                } else {
                    // with Ctrl, snap to M_PI/snaps
                    snap_vector = get_snap_vector (p, *dr_snap, M_PI/snaps, 0);
                }
            }
            if (snap_vector) {
                snap_vectors = g_slist_prepend (snap_vectors, snap_vector);
            }
        }

        // Move by the smallest of vectors:
        NR::Point move(9999, 9999);
        for (GSList const *i = snap_vectors; i != NULL; i = i->next) {
            NR::Point *snap_vector = (NR::Point *) i->data;
            if (NR::L2(*snap_vector) < NR::L2(move))
                move = *snap_vector;
        }
        if (move[NR::X] < 9999) {
            p += move;
            sp_knot_moveto (knot, &p);
        }
    }

    { // See if we need to snap to any of the levels
        for (int i = 0; i < dragger->parent->hor_levels.size(); i++) {
            if (fabs(p[NR::Y] - dragger->parent->hor_levels[i]) < snap_dist) {
                p[NR::Y] = dragger->parent->hor_levels[i];
                sp_knot_moveto (knot, &p);
            }
        }
        for (int i = 0; i < dragger->parent->vert_levels.size(); i++) {
            if (fabs(p[NR::X] - dragger->parent->vert_levels[i]) < snap_dist) {
                p[NR::X] = dragger->parent->vert_levels[i];
                sp_knot_moveto (knot, &p);
            }
        }
    }

    dragger->point = p;

    for (GSList const* i = dragger->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        dragger->parent->local_change = true;
        sp_item_gradient_set_coords (draggable->item, draggable->point_num, p, draggable->fill_or_stroke, false);
    }

    if (state & GDK_SHIFT_MASK) {
        if (dragger->draggables && dragger->draggables->next) {
            // create a new dragger
            GrDragger *dr_new = new GrDragger (dragger->parent, p, NULL, 
                                               gr_knot_shapes[((GrDraggable *) dragger->draggables->next->data)->point_num]);
            dragger->parent->draggers = g_slist_prepend (dragger->parent->draggers, dr_new);
            // relink to it all but the first draggable in the list
            for (GSList const* i = dragger->draggables->next; i != NULL; i = i->next) {
                GrDraggable *draggable = (GrDraggable *) i->data;
                dr_new->addDraggable (draggable);
            }
            g_slist_free (dragger->draggables->next);
            dragger->draggables->next = NULL;
            dragger->updateTip();
        }
    } else {
    // without Shift
    // TODO: snap to bboxes, centers of all selected objects; lower priority than dragger snap
    // see if we need to snap to another dragger
    for (GSList *di = dragger->parent->draggers; di != NULL; di = di->next) {
        GrDragger *d_new = (GrDragger *) di->data;
        if (d_new == dragger)
            continue;
        if (NR::L2 (d_new->point - p) < snap_dist) {

            bool incest = false;
            for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
                GrDraggable *d1 = (GrDraggable *) i->data;
                for (GSList const* j = d_new->draggables; j != NULL; j = j->next) { // for all draggables of dragger
                    GrDraggable *d2 = (GrDraggable *) j->data;
                    if ((d1->item == d2->item) && (d1->fill_or_stroke == d2->fill_or_stroke)) {
                        // we must not snap together the points of the same gradient!
                        incest = true;
                    }
                }
            }
            if (incest)
                continue;

            // Now actually snap:
            for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
                GrDraggable *draggable = (GrDraggable *) i->data;
                // copy draggable to d_new:
                GrDraggable *da_new = new GrDraggable (draggable->item, draggable->point_num, draggable->fill_or_stroke);
                d_new->addDraggable (da_new); 
                // move to the exact position of d_new, writing to repr:
                sp_item_gradient_set_coords (da_new->item, da_new->point_num, d_new->point, da_new->fill_or_stroke, true);
            }
            // unlink and delete this dragger
            dragger->parent->draggers = g_slist_remove (dragger->parent->draggers, dragger);
            delete dragger;
            d_new->parent->updateLines();
            d_new->parent->setSelected (d_new);
            sp_document_done (SP_DT_DOCUMENT (d_new->parent->desktop));
            // do the same as on ungrabbing
            if (d_new->isA(POINT_RG_R1) || d_new->isA(POINT_RG_R2) || d_new->isA(POINT_RG_CENTER)) {
                // only if this is center or one of the radii; focus does not affect other draggers
                d_new->parent->updateDraggersReselect();
            }
            return;
        }
    }
    }
}

/**
Called when the mouse releases a dragger knot; changes gradient writing to repr, updates other draggers if needed
*/
static void
gr_knot_ungrabbed_handler (SPKnot *knot, unsigned int state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->point_original = dragger->point;

    // Act upon all draggables of the dragger:
    for (GSList const* i = dragger->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        // set local_change flag so that selection_changed callback does not regenerate draggers
        dragger->parent->local_change = true;
        // change gradient, writing to repr
        sp_item_gradient_set_coords (draggable->item, draggable->point_num, knot->pos, draggable->fill_or_stroke, true);
    }

    // make this dragger selected
    dragger->parent->setSelected (dragger);

    // we did an undoable action
    sp_document_done (SP_DT_DOCUMENT (dragger->parent->desktop));

    // check if this draggable is attached to a radial gradient; if so, we need to regenerate all
    // draggers because moving this one affects positions of others. BAD because 1) others will not
    // be updated until mouse is released, 2) we'll need to clumsily save and restore selected
    // dragger, 3) others will unsnap without warning; 4) the order of draggables may change which
    // affects knot shape. However, this is by far the simplest method.
    if (dragger->isA(POINT_RG_R1) || dragger->isA(POINT_RG_R2) || dragger->isA(POINT_RG_CENTER)) {
        // only if this is center or one of the radii; focus does not affect other draggers
        dragger->parent->updateDraggersReselect();
    }
}

/**
Called when a dragger knot is clicked; selects the dragger
*/
static void
gr_knot_clicked_handler(SPKnot *knot, guint state, gpointer data)
{
   GrDragger *dragger = (GrDragger *) data;

   dragger->point_original = dragger->point;

   dragger->parent->setSelected (dragger);
}

/**
Checks if the dragger has a draggable with this point_num
 */
bool
GrDragger::isA (guint point_num)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if (draggable->point_num == point_num) {
            return true;
        }
    }
    return false;
}

/**
Checks if the dragger has a draggable with this item, point_num, fill_or_stroke
 */
bool
GrDragger::isA (SPItem *item, guint point_num, bool fill_or_stroke)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if (draggable->point_num == point_num && draggable->item == item && draggable->fill_or_stroke == fill_or_stroke) {
            return true;
        }
    }
    return false;
}

/**
Updates the statusbar tip of the dragger knot, based on its draggables
 */
void
GrDragger::updateTip ()
{
	if (this->knot && this->knot->tip) {
		g_free (this->knot->tip);
		this->knot->tip = NULL;
	}

    if (g_slist_length (this->draggables) == 1) {
        GrDraggable *draggable = (GrDraggable *) this->draggables->data;
        this->knot->tip = g_strdup_printf (_("%s for: %s"), 
                                           gr_knot_descr[draggable->point_num], sp_item_description (draggable->item));
    } else {
        this->knot->tip = g_strdup_printf (_("Gradient point shared by <b>%d</b> gradients; drag with <b>Shift</b> to separate"), 
                                           g_slist_length (this->draggables));
    }
}

/**
Adds a draggable to the dragger
 */
void
GrDragger::addDraggable (GrDraggable *draggable)
{
    this->draggables = g_slist_prepend (this->draggables, draggable);

    this->updateTip();
}


GrDragger::GrDragger (GrDrag *parent, NR::Point p, GrDraggable *draggable, SPKnotShapeType shape) 
{
    this->draggables = NULL;

    this->parent = parent;

    this->point = p;
    this->point_original = p;

    // create the knot
    this->knot = sp_knot_new (parent->desktop, NULL);
    g_object_set (G_OBJECT (this->knot->item), "shape", shape, NULL);
    g_object_set (G_OBJECT (this->knot->item), "mode", SP_KNOT_MODE_XOR, NULL);
    this->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;

    // move knot to the given point
    sp_knot_set_position (this->knot, &p, SP_KNOT_STATE_NORMAL);
    sp_knot_show (this->knot);

    // connect knot's signals
    this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "clicked", G_CALLBACK (gr_knot_clicked_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "ungrabbed", G_CALLBACK (gr_knot_ungrabbed_handler), this);

    // add the initial draggable
    if (draggable)
        this->addDraggable (draggable);
}

GrDragger::~GrDragger ()
{
    /* unref should call destroy */
    g_object_unref (G_OBJECT (this->knot));

    // unselect if it was selected
    if (this->parent->selected == this)
        this->parent->selected = NULL;

    // delete all draggables
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        delete ((GrDraggable *) i->data);
    }
    g_slist_free (this->draggables);
    this->draggables = NULL;
}

/**
Select the dragger which has the given draggable.
*/
void
GrDrag::restoreSelected (GrDraggable *da1) 
{
    for (GSList const* i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {
            GrDraggable *da2 = (GrDraggable *) j->data;
            if (da1->item == da2->item && da1->point_num == da2->point_num && da1->fill_or_stroke == da2->fill_or_stroke) {
                setSelected (dragger);
                return;
            }
        }
    }
}

/**
Set selected dragger
*/
void
GrDrag::setSelected (GrDragger *dragger) 
{
    if (this->selected) {
       this->selected->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;
       g_object_set (G_OBJECT (this->selected->knot->item), "fill_color", GR_KNOT_COLOR_NORMAL, NULL);
    }
    if (dragger) {
        dragger->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_SELECTED;
        g_object_set (G_OBJECT (dragger->knot->item), "fill_color", GR_KNOT_COLOR_SELECTED, NULL);
    }
    this->selected = dragger;
}

/**
Create a line from p1 to p2 and add it to the lines list
 */
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

/**
If there already exists a dragger within MERGE_DIST of p, add the draggable to it; otherwise create
new dragger and add it to draggers list
 */
void 
GrDrag::addDragger (NR::Point p, GrDraggable *draggable, SPKnotShapeType shape)
{
    for (GSList *i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        if (NR::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addDraggable (draggable);
            return;
        }
    }

    this->draggers = g_slist_prepend (this->draggers, new GrDragger(this, p, draggable, shape));
}

/**
Add draggers for the radial gradient rg on item
*/
void 
GrDrag::addDraggersRadial (SPRadialGradient *rg, SPItem *item, bool fill_or_stroke)
{
    addDragger (sp_rg_get_center (item, rg), new GrDraggable (item, POINT_RG_CENTER, fill_or_stroke), gr_knot_shapes[POINT_RG_CENTER]);
    addDragger (sp_rg_get_r1(item, rg), new GrDraggable (item, POINT_RG_R1, fill_or_stroke), gr_knot_shapes[POINT_RG_R1]);
    addDragger (sp_rg_get_r2(item, rg), new GrDraggable (item, POINT_RG_R2, fill_or_stroke), gr_knot_shapes[POINT_RG_R2]);
}

/**
Add draggers for the linear gradient lg on item
*/
void 
GrDrag::addDraggersLinear (SPLinearGradient *lg, SPItem *item, bool fill_or_stroke)
{
    addDragger (sp_lg_get_p1 (item, lg), new GrDraggable (item, POINT_LG_P1, fill_or_stroke), gr_knot_shapes[POINT_LG_P1]);
    addDragger (sp_lg_get_p2 (item, lg), new GrDraggable (item, POINT_LG_P2, fill_or_stroke), gr_knot_shapes[POINT_LG_P2]);
}


/**
Same as updateDraggers, but also restores selection, if possible, to the same dragger
*/
void
GrDrag::updateDraggersReselect ()
{
    if (selected) {
        // remember the first draggable of the selected knot
        GrDraggable *select = (GrDraggable *) selected->draggables->data;
        GrDraggable *select_copy = new GrDraggable (select->item, select->point_num, select->fill_or_stroke);
        // update all knots; this destroys dragger!
        updateDraggers();
        // restore selection
        restoreSelected(select_copy);
    } else {
        updateDraggers();
    }
}

/**
Regenerates the draggers list from the current selection; is called when selection is changed or
modified, also when a radial dragger needs to update positions of other draggers in the gradient
*/
void
GrDrag::updateDraggers ()
{
    // delete old draggers and deselect
    for (GSList const* i = this->draggers; i != NULL; i = i->next) {
        delete ((GrDragger *) i->data);
    }
    g_slist_free (this->draggers);
    this->draggers = NULL;
    this->selected = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {

        SPItem *item = SP_ITEM(i->data);
        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                addDraggersLinear (SP_LINEARGRADIENT (server), item, true);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                addDraggersRadial (SP_RADIALGRADIENT (server), item, true);
            }
        }

        if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                addDraggersLinear (SP_LINEARGRADIENT (server), item, false);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                addDraggersRadial (SP_RADIALGRADIENT (server), item, false);
            }
        }


    }
}

/**
Regenerates the lines list from the current selection; is called on each move of a dragger, so that
lines are always in sync with the actual gradient
*/
void
GrDrag::updateLines ()
{
    // delete old lines
    for (GSList const *i = this->lines; i != NULL; i = i->next) {
        gtk_object_destroy( GTK_OBJECT (i->data));
    }
    g_slist_free (this->lines);
    this->lines = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {

        SPItem *item = SP_ITEM(i->data);

        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                SPLinearGradient *lg = SP_LINEARGRADIENT (server);
                this->addLine (sp_lg_get_p1 (item, lg), sp_lg_get_p2 (item, lg));
            } else if (SP_IS_RADIALGRADIENT (server)) {
                SPRadialGradient *rg = SP_RADIALGRADIENT (server);
                NR::Point center = sp_rg_get_center (item, rg);
                this->addLine (center, sp_rg_get_r1 (item, rg));
                this->addLine (center, sp_rg_get_r2 (item, rg));
            }
        }

        if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                SPLinearGradient *lg = SP_LINEARGRADIENT (server);

                this->addLine (sp_lg_get_p1 (item, lg), sp_lg_get_p2 (item, lg));
            } else if (SP_IS_RADIALGRADIENT (server)) {
                SPRadialGradient *rg = SP_RADIALGRADIENT (server);
                NR::Point center = sp_rg_get_center (item, rg);
                this->addLine (center, sp_rg_get_r1 (item, rg));
                this->addLine (center, sp_rg_get_r2 (item, rg));
            }
        }
    }
}

/**
Regenerates the levels list from the current selection
*/
void
GrDrag::updateLevels ()
{
    hor_levels.clear();
    vert_levels.clear();

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        NR::Rect rect = sp_item_bbox_desktop (item);
        // Remember the edges of the bbox and the center axis
        hor_levels.push_back(rect.min()[NR::Y]);
        hor_levels.push_back(rect.max()[NR::Y]);
        hor_levels.push_back(0.5 * (rect.min()[NR::Y] + rect.max()[NR::Y]));
        vert_levels.push_back(rect.min()[NR::X]);
        vert_levels.push_back(rect.max()[NR::X]);
        vert_levels.push_back(0.5 * (rect.min()[NR::X] + rect.max()[NR::X]));
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
