#define __SP_SELCUE_C__

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
#include "display/sodipodi-ctrlrect.h"
#include "prefs-utils.h"
#include "sp-item.h"
#include "selcue.h"

static void 
sp_sel_cue_sel_changed(SPSelection *selection, gpointer data)
{
	SPSelCue *selcue = (SPSelCue *) data;
	sp_sel_cue_update_item_bboxes (selcue);
}

static void
sp_sel_cue_sel_modified (SPSelection *selection, guint flags, gpointer data)
{
	SPSelCue *selcue = (SPSelCue *) data;
	sp_sel_cue_update_item_bboxes (selcue);
}


SPSelCue::SPSelCue(SPDesktop *desktop) {
	this->desktop = desktop;
	this->selection = SP_DT_SELECTION(desktop);
        this->item_bboxes = NULL;

	this->sel_changed_connection = this->selection->connectChanged(
            SigC::bind(
                SigC::slot(&sp_sel_cue_sel_changed),
                (gpointer)this
            )
	);
	this->sel_modified_connection = this->selection->connectModified(
            SigC::bind(
                SigC::slot(&sp_sel_cue_sel_modified),
                (gpointer)this
            )
	);
	sp_sel_cue_update_item_bboxes (this);
}

SPSelCue::~SPSelCue() {
	this->sel_changed_connection.disconnect();
	this->sel_modified_connection.disconnect();

	for (GSList *l = this->item_bboxes; l != NULL; l = l->next) {
		gtk_object_destroy( GTK_OBJECT (l->data));
	}
	g_slist_free (this->item_bboxes);
	this->item_bboxes = NULL;
}

void
sp_sel_cue_update_item_bboxes (SPSelCue * selcue)
{
       g_return_if_fail (selcue != NULL);

       GSList const* l;
       for (l = selcue->item_bboxes; l != NULL; l = l->next) {
               gtk_object_destroy( GTK_OBJECT (l->data));
       }
       g_slist_free (selcue->item_bboxes);
       selcue->item_bboxes = NULL;

	 gint mode = prefs_get_int_attribute ("options.selcue", "value", SP_SELCUE_MARK);
	 if (mode == SP_SELCUE_NONE)
		 return;

       g_return_if_fail (selcue->selection != NULL);

       NRRect b;
       SPCanvasItem* box = NULL;
       for (l = selcue->selection->itemList(); l != NULL; l = l->next) {

		 sp_item_bbox_desktop ((SPItem *) l->data, &b);

		 if (mode == SP_SELCUE_MARK) {
                     
                     box = sp_canvas_item_new (SP_DT_CONTROLS (selcue->desktop),
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
                     SP_CTRL(box)->moveto (NR::Point(b.x0, b.y1));

                     sp_canvas_item_move_to_z (box, 0); // just low enough to not get in the way of other draggable knots
		 }
		 else if (mode == SP_SELCUE_BBOX)
		 {
                     
                     box = sp_canvas_item_new (
                         SP_DT_CONTROLS (selcue->desktop),
                         SP_TYPE_CTRLRECT,
                         NULL
                         );
                     
                     sp_ctrlrect_set_area (SP_CTRLRECT (box), b.x0, b.y0, b.x1, b.y1);
                     sp_ctrlrect_set_color (SP_CTRLRECT (box), 0x000000a0, 0, 0);
                     sp_ctrlrect_set_dashed (SP_CTRLRECT (box), 1);

                     sp_canvas_item_move_to_z (box, 0);
		 }
                 
		 if (box)
			 selcue->item_bboxes = g_slist_append (selcue->item_bboxes, box);
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
