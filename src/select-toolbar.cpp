/*
 * Selector aux toolbar
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkselection.h>
#include <gtk/gtktable.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhandlebox.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "helper/window.h"
#include "widgets/icon.h"
#include "widgets/button.h"
#include "widgets/spw-utilities.h"
#include "widgets/widget-sizes.h"
#include "widgets/spinbutton-events.h"

#include "prefs-utils.h"
#include "inkscape-stock.h"
#include "file.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "inkscape-private.h"
#include "document.h"
#include "inkscape.h"
#include "sp-object.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "interface.h"
#include "toolbox.h"
#include "xml/repr-private.h"
#include "helper/gnome-utils.h"
#include "helper/sp-intl.h"

#include "widgets/sp-widget.h"
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "widgets/sp-widget.h"
#include "macros.h"
#include "inkscape.h"
#include "verbs.h"
#include "interface.h"
#include "inkscape-stock.h"
#include "prefs-utils.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "sp-item-transform.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-ops.h"
//#include <libart_lgpl/art_affine.h>

#include "select-toolbar.h"

static void
sp_selection_layout_widget_update (SPWidget *spw, SPSelection *sel)
{
	GtkWidget *f;

	if (gtk_object_get_data (GTK_OBJECT (spw), "update")) return;

	gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

	f = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (spw), "frame");

	if ( sel && !sel->isEmpty() ) {
		NRRect bbox;

		sel->bounds(&bbox);

		if ((bbox.x1 - bbox.x0 > 1e-6) || (bbox.y1 - bbox.y0 > 1e-6)) {
			GtkWidget *us;
			GtkAdjustment *a;
			const SPUnit *unit;

			us = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (spw), "units");
			unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));

			if (unit->base == SP_UNIT_ABSOLUTE) {
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "X");
				gtk_adjustment_set_value (a, sp_points_get_units (bbox.x0, unit));
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "Y");
				gtk_adjustment_set_value (a, sp_points_get_units (bbox.y0, unit));
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "width");
				gtk_adjustment_set_value (a, sp_points_get_units (bbox.x1 - bbox.x0, unit));
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "height");
				gtk_adjustment_set_value (a, sp_points_get_units (bbox.y1 - bbox.y0, unit));
			} else {
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "X");
				gtk_adjustment_set_value (a, 100.0);
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "Y");
				gtk_adjustment_set_value (a, 100.0);
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "width");
				gtk_adjustment_set_value (a, 100.0);
				a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "height");
				gtk_adjustment_set_value (a, 100.0);

			}

			gtk_widget_set_sensitive (f, TRUE);
		} else {
			gtk_widget_set_sensitive (f, FALSE);
		}
	} else {
		gtk_widget_set_sensitive (f, FALSE);
	}

	gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
}


static void
sp_selection_layout_widget_modify_selection (SPWidget *spw, SPSelection *selection, guint flags, gpointer data)
{
	SPDesktop *desktop = (SPDesktop *) data;
	if (desktop->selection == selection) { // only respond to changes in our desktop
		if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG)) {
			sp_selection_layout_widget_update (spw, selection);
		}
	}
}

static void
sp_selection_layout_widget_change_selection (SPWidget *spw, SPSelection *selection, gpointer data)
{

	SPDesktop *desktop = (SPDesktop *) data;
	if (desktop->selection == selection) // only respond to changes in our desktop
		sp_selection_layout_widget_update (spw, selection);
}

static void
sp_object_layout_any_value_changed (GtkAdjustment *adj, SPWidget *spw)
{
	if (gtk_object_get_data (GTK_OBJECT (spw), "update")) {
		return;
	}

	GtkWidget *us = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (spw), "units");
	const SPUnit *unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));
	if (sp_unit_selector_update_test (SP_UNIT_SELECTOR (us))) {
		/*
		 * When only units are being changed, don't treat changes
		 * to adjuster values as object changes.
		 */
		return;
	}
	gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

	SPSelection *selection = SP_WIDGET_SELECTION (spw);
	NR::Rect bbox = selection->bounds();

	if (!((bbox.max()[NR::X] - bbox.min()[NR::X] > 1e-6) || (bbox.max()[NR::Y] - bbox.min()[NR::Y] > 1e-6))) {
		return;
	}

	GtkAdjustment *a;
	gdouble x0, y0, x1, y1, dx0, dy0, dx1, dy1, xrel, yrel;

	if (unit->base == SP_UNIT_ABSOLUTE) {
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "X");
		x0 = sp_units_get_points (a->value, unit);
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "Y");
		y0 = sp_units_get_points (a->value, unit);
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "width");
		x1 = x0 + sp_units_get_points (a->value, unit);
		xrel = sp_units_get_points (a->value, unit) / bbox.extent(NR::X);
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "height");
		y1 = y0 + sp_units_get_points (a->value, unit);
		yrel = sp_units_get_points (a->value, unit) / bbox.extent(NR::Y);
	} else {
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "X");
		x0 = a->value * bbox.min()[NR::X] / 100.0;
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "Y");
		y0 = a->value * bbox.min()[NR::Y] / 100.0;
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "width");
		xrel = a->value * 0.01;
		x1 = x0 + xrel * bbox.extent(NR::X);
		a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (spw), "height");
		yrel = a->value * 0.01;
		y1 = y0 + yrel * bbox.extent(NR::Y);
	}

	GtkWidget *lock = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (spw), "lock"));
	if (SP_BUTTON_IS_DOWN (lock)) {
		if (fabs (xrel - 1.0) < 1e-6) {
			x1 = x0 + yrel * bbox.extent(NR::X);
		} else if (fabs (yrel - 1.0) < 1e-6) {
			y1 = y0 + xrel * bbox.extent(NR::Y);
		}
	}

	dx0 = fabs (x0 - bbox.min()[NR::X]) > 1e-6 ? fabs (x0 - bbox.min()[NR::X]) : 0;
	dx1 = fabs (x1 - bbox.max()[NR::X]) > 1e-6 ? fabs (x1 - bbox.max()[NR::X]) : 0;
	dy0 = fabs (y0 - bbox.min()[NR::Y]) > 1e-6 ? fabs (y0 - bbox.min()[NR::Y]) : 0;
	dy1 = fabs (y1 - bbox.max()[NR::Y]) > 1e-6 ? fabs (y1 - bbox.max()[NR::Y]) : 0;

	if (dx0 || dx1 || dy0 || dy1) {
		NR::translate const p2o(-bbox.min());
		NR::scale scale(1, 1);
		if ( fabs( bbox.extent(NR::X) ) <= 1e-06 ) {
			scale = NR::scale(1, ( y1 - y0 ) / ( bbox.extent(NR::Y) ));
		} else if ( fabs( bbox.extent(NR::Y) ) <= 1e-06 ) {
			scale = NR::scale(( x1 - x0 ) / ( bbox.extent(NR::X) ), 1);
		} else {
			scale = NR::scale(( x1 - x0 ) / ( bbox.extent(NR::X) ), ( y1 - y0 ) / ( bbox.extent(NR::Y) ));
		}
		NR::translate const o2n(x0, y0);
		sp_selection_apply_affine(selection, p2o * scale * o2n);

 		if (dx0) 
 			sp_document_maybe_done (SP_WIDGET_DOCUMENT (spw), "selector:toolbar:move:horizontal");
 		else if (dx1)
 			sp_document_maybe_done (SP_WIDGET_DOCUMENT (spw), "selector:toolbar:scale:horizontal");
 		else if (dy0)
 			sp_document_maybe_done (SP_WIDGET_DOCUMENT (spw), "selector:toolbar:move:vertical");
 		else if (dy1)
 			sp_document_maybe_done (SP_WIDGET_DOCUMENT (spw), "selector:toolbar:scale:vertical");
		else sp_document_done (SP_WIDGET_DOCUMENT (spw));

		// defocus spinbuttons by moving focus to the canvas, unless "stay" is on
		spinbutton_defocus (GTK_OBJECT (spw));
	}

	gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
}

GtkWidget *
sp_select_toolbox_spinbutton (gchar *label, gchar *data, float lower_limit, GtkWidget *us, GtkWidget *spw, gchar *tooltip, gboolean altx)
{
	GtkTooltips *tt = gtk_tooltips_new ();

	GtkWidget *hb = gtk_hbox_new (FALSE, 1);
	GtkWidget *l = gtk_label_new (_(label));
	gtk_tooltips_set_tip (tt, l, tooltip, NULL);
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_container_add (GTK_CONTAINER (hb), l);

	GtkObject *a = gtk_adjustment_new (0.0, lower_limit, 1e6, SPIN_STEP, SPIN_PAGE_STEP, SPIN_PAGE_STEP);
	sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), GTK_ADJUSTMENT (a));
	gtk_object_set_data (GTK_OBJECT (spw), data, a);

	GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), SPIN_STEP, 2);
	gtk_tooltips_set_tip (tt, sb, tooltip, NULL);
	gtk_widget_set_size_request (sb, AUX_SPINBUTTON_WIDTH, AUX_SPINBUTTON_HEIGHT);
	gtk_widget_show (sb);
	gtk_signal_connect (GTK_OBJECT (sb), "focus-in-event", GTK_SIGNAL_FUNC (spinbutton_focus_in), spw);
	gtk_signal_connect (GTK_OBJECT (sb), "key-press-event", GTK_SIGNAL_FUNC (spinbutton_keypress), spw);

	gtk_container_add (GTK_CONTAINER (hb), sb);
	gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (sp_object_layout_any_value_changed), spw);

	if (altx) { // this spinbutton will be activated by alt-x
		gtk_object_set_data (GTK_OBJECT (sb), "altx", sb);
	}

	return hb;
}

static gboolean aux_set_unit(SPUnitSelector *,
                                                 SPUnit const *old,
                                                 SPUnit const *new_units,
                                                 GObject *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    
    if (!desktop) {
        return FALSE;
    }
    
    SPSelection *selection = SP_DT_SELECTION (desktop);
    
    if (selection->isEmpty())
        return FALSE;

    if ((old->base == SP_UNIT_ABSOLUTE) && 
       (new_units->base == SP_UNIT_DIMENSIONLESS)) {
       
        /* Absolute to percentage */
        g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));
 
        GtkAdjustment *ax = GTK_ADJUSTMENT(g_object_get_data (dlg, "X"));
        GtkAdjustment *ay = GTK_ADJUSTMENT(g_object_get_data (dlg, "Y"));
        GtkAdjustment *aw = GTK_ADJUSTMENT(g_object_get_data (dlg, "width"));
        GtkAdjustment *ah = GTK_ADJUSTMENT(g_object_get_data (dlg, "height"));

        float x = sp_units_get_points (ax->value, old);
        float y = sp_units_get_points (ay->value, old);
        float w = sp_units_get_points (aw->value, old);
        float h = sp_units_get_points (ah->value, old);

        NR::Rect bbox = selection->bounds();

        gtk_adjustment_set_value (ax, 100.0 * x / bbox.min()[NR::X]);
        gtk_adjustment_set_value (ay, 100.0 * y / bbox.min()[NR::Y]);
        gtk_adjustment_set_value (aw, 100.0 * w / bbox.extent(NR::X));
        gtk_adjustment_set_value (ah, 100.0 * h / bbox.extent(NR::Y));

        g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
        return TRUE;
        
    } else if ((old->base == SP_UNIT_DIMENSIONLESS) && 
              (new_units->base == SP_UNIT_ABSOLUTE)) {
              
        /* Percentage to absolute */
        g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));

        GtkAdjustment *ax = GTK_ADJUSTMENT(g_object_get_data (dlg, "X"));
        GtkAdjustment *ay = GTK_ADJUSTMENT(g_object_get_data (dlg, "Y"));
        GtkAdjustment *aw = GTK_ADJUSTMENT(g_object_get_data (dlg, "width"));
        GtkAdjustment *ah = GTK_ADJUSTMENT(g_object_get_data (dlg, "height"));

        NR::Rect bbox = selection->bounds();

        gtk_adjustment_set_value (ax, sp_points_get_units (0.01 * ax->value * bbox.min()[NR::X], new_units));
        gtk_adjustment_set_value (ay, sp_points_get_units (0.01 * ay->value * bbox.min()[NR::Y], new_units));
        gtk_adjustment_set_value (aw, sp_points_get_units (0.01 * aw->value * bbox.extent(NR::X), new_units));
        gtk_adjustment_set_value (ah, sp_points_get_units (0.01 * ah->value * bbox.extent(NR::Y), new_units));

        g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
        return TRUE;
    }

    return FALSE;
}



GtkWidget *
sp_select_toolbox_new (SPDesktop *desktop)
{
	GtkWidget *tb;
	GtkTooltips *tt;
	SPView *view=SP_VIEW (desktop);

	tt = gtk_tooltips_new ();
	tb = gtk_hbox_new (FALSE, 0);

	aux_toolbox_space (tb, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_GROUP, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_UNGROUP, view, tt);

	aux_toolbox_space (tb, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_TO_BACK, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_LOWER, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_RAISE, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_SELECTION_TO_FRONT, view, tt);

	aux_toolbox_space (tb, AUX_BETWEEN_BUTTON_GROUPS);

	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_OBJECT_ROTATE_90_CCW, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_OBJECT_ROTATE_90_CW, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_OBJECT_FLIP_HORIZONTAL, view, tt);
	sp_toolbox_button_normal_new_from_verb(tb, AUX_BUTTON_SIZE, SP_VERB_OBJECT_FLIP_VERTICAL, view, tt);

	GtkWidget *spw, *vb, *us;

	// create the parent widget for x y w h tracker
	spw = sp_widget_new_global (INKSCAPE);

	// remember the desktop's canvas widget, to be used for defocusing
	gtk_object_set_data (GTK_OBJECT (spw), "dtw", desktop->owner->canvas);

	// the vb frame holds all other widgets and is used to set sensitivity depending on selection state
	vb = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (vb);
	gtk_container_add (GTK_CONTAINER (spw), vb);
	gtk_object_set_data (GTK_OBJECT (spw), "frame", vb);

	// create the units menu
	us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
	sp_unit_selector_setsize (us, AUX_OPTION_MENU_WIDTH, AUX_OPTION_MENU_HEIGHT);
	sp_unit_selector_add_unit ( SP_UNIT_SELECTOR (us), sp_unit_get_by_abbreviation ("%"), 0 );
	g_signal_connect ( G_OBJECT (us), "set_unit", G_CALLBACK (aux_set_unit), spw );

	// four spinbuttons

	gtk_container_add (GTK_CONTAINER (vb),
		sp_select_toolbox_spinbutton ("X", "X", -1e6, us, spw, _("Horizontal coordinate of selection"), TRUE));
	aux_toolbox_space (vb, AUX_BETWEEN_SPINBUTTONS);
	gtk_container_add (GTK_CONTAINER (vb),
		sp_select_toolbox_spinbutton ("Y", "Y", -1e6, us, spw, _("Vertical coordinate of selection"), FALSE));
	aux_toolbox_space (vb, AUX_BETWEEN_BUTTON_GROUPS);

	gtk_container_add (GTK_CONTAINER (vb),
		sp_select_toolbox_spinbutton ("W", "width", 1e-3, us, spw, _("Width of selection"), FALSE));

	// lock toggle
	GtkWidget *lockbox = gtk_vbox_new (TRUE, 0);
	GtkWidget *lock = sp_button_new_from_data (9,
                                                 SP_BUTTON_TYPE_TOGGLE, 	 
                                                 NULL, 	 
                                                 "width_height_lock", 	 
                                                 _("Change both width and height by the same proportion"), 	 
                                                 tt); 	 
	gtk_box_pack_start (GTK_BOX (lockbox), lock, TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vb), lockbox, FALSE, FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (spw), "lock", lock);

	gtk_container_add (GTK_CONTAINER (vb),
		sp_select_toolbox_spinbutton ("H", "height", 1e-3, us, spw, _("Height of selection"), FALSE));

	aux_toolbox_space (vb, 2);

	// add the units menu
	gtk_widget_show (us);
	gtk_container_add (GTK_CONTAINER (vb), us);
	gtk_object_set_data (GTK_OBJECT (spw), "units", us);

	// set font size
	sp_set_font_size (vb, AUX_FONT_SIZE);

	// force update when selection changes
	gtk_signal_connect (GTK_OBJECT (spw), "modify_selection", GTK_SIGNAL_FUNC (sp_selection_layout_widget_modify_selection), desktop);
	gtk_signal_connect (GTK_OBJECT (spw), "change_selection", GTK_SIGNAL_FUNC (sp_selection_layout_widget_change_selection), desktop);

	// update now
	sp_selection_layout_widget_update (SP_WIDGET (spw), SP_ACTIVE_DESKTOP ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL);

	// insert spw into the toolbar
	gtk_box_pack_start (GTK_BOX (tb), spw, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

	gtk_widget_show_all (tb);

	return tb;
}
