#define __SP_TRANSFORMATION_C__

/*
 * Object transformation dialog
 *
 * Authors:
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkstock.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "widgets/icon.h"
#include "macros.h"
#include "sodipodi.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "desktop-handles.h"

/* Notebook pages */
/* These are hardcoded so do not play with them */

enum {
	SP_TRANSFORMATION_MOVE,
	SP_TRANSFORMATION_SCALE,
	SP_TRANSFORMATION_ROTATE,
	SP_TRANSFORMATION_SKEW
};

static void sp_transformation_dialog_present (unsigned int page);
static GtkWidget *sp_transformation_dialog_new (void);

static void sp_transformation_dialog_apply (GObject *object, GObject *dlg);
static void sp_transformation_dialog_close (GObject *object, GtkWidget *dlg);

static GtkWidget *sp_transformation_page_move_new (GObject *obj);
static void sp_transformation_move_update (GObject *dlg, SPSelection *selection);
static void sp_transformation_move_apply (GObject *dlg, SPSelection *selection, unsigned int copy);

static GtkWidget *sp_transformation_page_scale_new (GObject *obj);
static void sp_transformation_scale_update (GObject *dlg, SPSelection *selection);
static void sp_transformation_scale_apply (GObject *dlg, SPSelection *selection, unsigned int copy);

static GtkWidget *sp_transformation_page_rotate_new (GObject *obj);
static void sp_transformation_rotate_update (GObject *dlg, SPSelection *selection);
static void sp_transformation_rotate_apply (GObject *dlg, SPSelection *selection, unsigned int copy);

static void sp_transformation_skew_apply (GObject *dlg, SPSelection *selection, unsigned int copy);

static GtkWidget *dlg = NULL;

void
sp_transformation_dialog_move (void)
{
	sp_transformation_dialog_present (SP_TRANSFORMATION_MOVE);
}

void
sp_transformation_dialog_scale (void)
{
	sp_transformation_dialog_present (SP_TRANSFORMATION_SCALE);
}

void
sp_transformation_dialog_rotate (void)
{
	sp_transformation_dialog_present (SP_TRANSFORMATION_ROTATE);
}

void
sp_transformation_dialog_skew (void)
{
	sp_transformation_dialog_present (SP_TRANSFORMATION_SKEW);
}

static void
sp_transformation_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (SODIPODI, object);

	dlg = NULL;
}

static void
sp_transformation_dialog_present (unsigned int page)
{
	GtkWidget *nbook;

	if (!dlg) {
		dlg = sp_transformation_dialog_new ();
	}

	nbook = g_object_get_data (G_OBJECT (dlg), "notebook");
	gtk_notebook_set_page (GTK_NOTEBOOK (nbook), page);

#if 0
	sp_transformation_apply_button_reset ();
#endif

	gtk_widget_show (dlg);
	gtk_window_present (GTK_WINDOW (dlg));
}

static void
sp_transformation_dialog_update_selection (GObject *dlg, unsigned int page, SPSelection *selection)
{
	GtkWidget *apply;

	switch (page) {
	case SP_TRANSFORMATION_MOVE:
		sp_transformation_move_update (dlg, selection);
		break;
	case SP_TRANSFORMATION_SCALE:
		sp_transformation_scale_update (dlg, selection);
		break;
	case SP_TRANSFORMATION_ROTATE:
		sp_transformation_rotate_update (dlg, selection);
		break;
	case SP_TRANSFORMATION_SKEW:
	default:
		break;
	}

	apply = g_object_get_data (dlg, "apply");
	if (selection && !sp_selection_is_empty (selection)) {
		gtk_widget_set_sensitive (apply, TRUE);
	} else {
		gtk_widget_set_sensitive (apply, FALSE); 
	}
}

static void
sp_transformation_dialog_selection_changed (Sodipodi *sodipodi, SPSelection *selection, GObject *obj)
{
	GObject *notebook;
	int page;

	notebook = g_object_get_data (obj, "notebook");
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

	sp_transformation_dialog_update_selection (obj, page, selection);
}

static void
sp_transformation_dialog_selection_modified (Sodipodi *sodipodi, SPSelection *selection, unsigned int flags, GObject *obj)
{
	GObject *notebook;
	int page;

	notebook = g_object_get_data (obj, "notebook");
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

	sp_transformation_dialog_update_selection (obj, page, selection);
}

static void
sp_transformation_dialog_switch_page (GtkNotebook *notebook, GtkNotebookPage *page, guint pagenum, GObject *dlg)
{
	SPSelection *sel;

	sel = (SP_ACTIVE_DESKTOP) ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL;

	sp_transformation_dialog_update_selection (dlg, pagenum, sel);
}

static GtkWidget *
sp_transformation_dialog_new (void)
{
	GtkWidget *dlg;
	GtkWidget *hb, *vb, *nbook, *page, *img, *hs, *bb, *b;
	SPSelection *sel;

	dlg = sp_window_new (_("Transform selection"), FALSE);

	/* Toplevel hbox */
	hb = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hb);
	gtk_container_add (GTK_CONTAINER (dlg), hb);

	/* Toplevel vbox */
	vb = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vb);
	gtk_box_pack_start (GTK_BOX (hb), vb, TRUE, TRUE, 0);
	
	/* Notebook for individual transformations */
	nbook = gtk_notebook_new ();
	gtk_widget_show (nbook);
	gtk_box_pack_start (GTK_BOX (vb), nbook, TRUE, TRUE, 0);
	g_object_set_data (G_OBJECT (dlg), "notebook", nbook);
	/* Separator */
	hs = gtk_hseparator_new ();
	gtk_widget_show (hs);
	gtk_box_pack_start (GTK_BOX (vb), hs, FALSE, FALSE, 0);
	/* Buttons */
	bb = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (bb);
	gtk_box_pack_start (GTK_BOX (vb), bb, FALSE, FALSE, 0);
	b = gtk_button_new_from_stock (GTK_STOCK_APPLY);
	g_object_set_data (G_OBJECT (dlg), "apply", b);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (bb), b, TRUE, TRUE, 0);
	g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_transformation_dialog_apply), dlg);
	b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (b);
	gtk_box_pack_start (GTK_BOX (bb), b, TRUE, TRUE, 0);
	g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_transformation_dialog_close), dlg);

	/* Move page */
	img = sp_icon_new (SP_ICON_SIZE_NOTEBOOK, "move");
	gtk_widget_show (img);
	page = sp_transformation_page_move_new (G_OBJECT (dlg));
	gtk_widget_show (page);
	gtk_notebook_append_page (GTK_NOTEBOOK (nbook), page, img);
	g_object_set_data (G_OBJECT (dlg), "move", page);

	/* Scale page */
	img = sp_icon_new (SP_ICON_SIZE_NOTEBOOK, "scale");
	gtk_widget_show (img);
	page = sp_transformation_page_scale_new (G_OBJECT (dlg));
	gtk_widget_show (page);
	gtk_notebook_append_page (GTK_NOTEBOOK (nbook), page, img);
	g_object_set_data (G_OBJECT (dlg), "scale", page);

	/* Rotate page */
	img = sp_icon_new (SP_ICON_SIZE_NOTEBOOK, "rotate");
	gtk_widget_show (img);
	page = sp_transformation_page_rotate_new (G_OBJECT (dlg));
	gtk_widget_show (page);
	gtk_notebook_append_page (GTK_NOTEBOOK (nbook), page, img);
	g_object_set_data (G_OBJECT (dlg), "rotate", page);

	/* Connect signals */
	g_signal_connect (G_OBJECT (dlg), "destroy", G_CALLBACK (sp_transformation_dialog_destroy), NULL);
	g_signal_connect (G_OBJECT (SODIPODI), "change_selection", G_CALLBACK (sp_transformation_dialog_selection_changed), dlg);
	g_signal_connect (G_OBJECT (SODIPODI), "modify_selection", G_CALLBACK (sp_transformation_dialog_selection_modified), dlg);
	g_signal_connect (G_OBJECT (nbook), "switch_page", G_CALLBACK (sp_transformation_dialog_switch_page), dlg);

	sel = (SP_ACTIVE_DESKTOP) ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL;
	sp_transformation_dialog_update_selection (G_OBJECT (dlg), 0, sel);

	return dlg;
}

static void
sp_transformation_dialog_apply (GObject *object, GObject *dlg)
{
	SPDesktop *desktop;
	SPSelection *selection;
	GtkWidget *nbookw, *apply;
	int page;

	desktop = SP_ACTIVE_DESKTOP;
	g_return_if_fail (desktop != NULL);
	selection = SP_DT_SELECTION (desktop);
	g_return_if_fail (!sp_selection_is_empty (selection));

	nbookw = g_object_get_data (dlg, "notebook");
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (nbookw));

	switch (page) {
	case SP_TRANSFORMATION_MOVE:
		sp_transformation_move_apply (dlg, selection, FALSE);
		break;
	case SP_TRANSFORMATION_ROTATE:
		sp_transformation_rotate_apply (dlg, selection, FALSE);
		break;
	case SP_TRANSFORMATION_SCALE:
		sp_transformation_scale_apply (dlg, selection, FALSE);
		break;
	case SP_TRANSFORMATION_SKEW:
		sp_transformation_skew_apply (dlg, selection, FALSE);
		break;
	}

	apply = g_object_get_data (dlg, "apply");
	gtk_widget_set_sensitive (apply, TRUE);
}

static void
sp_transformation_dialog_close (GObject *object, GtkWidget *dlg)
{
	gtk_widget_destroy (dlg);
}

/*
 * Move implementation
 */

static void
sp_transformation_move_value_changed (GtkAdjustment *adj, GObject *dlg)
{
	GtkWidget *apply;

	if (g_object_get_data (dlg, "update")) return;

	apply = g_object_get_data (dlg, "apply");
	gtk_widget_set_sensitive (apply, TRUE);
}

static void
sp_transformation_move_relative_toggled (GtkToggleButton *tb, GObject *dlg)
{
	SPDesktop *desktop;
	SPSelection *selection;
	SPUnitSelector *us;
	GtkAdjustment *ax, *ay;
	NRRectF bbox;
	float x, y;

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return;
	selection = SP_DT_SELECTION (desktop);
	if (sp_selection_is_empty (selection)) return;

	/* Read values from widget */
	us = g_object_get_data (dlg, "move_units");
	ax = g_object_get_data (dlg, "move_position_x");
	ay = g_object_get_data (dlg, "move_position_y");
	x = sp_unit_selector_get_value_in_points (us, ax);
	y = sp_unit_selector_get_value_in_points (us, ay);

	sp_selection_bbox (selection, &bbox);

	g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));

	if (gtk_toggle_button_get_active (tb)) {
		/* From absolute to relative */
		sp_unit_selector_set_value_in_points (us, ax, x - bbox.x0);
		sp_unit_selector_set_value_in_points (us, ay, y - bbox.y0);
	} else {
		/* From relative to absolute */
		sp_unit_selector_set_value_in_points (us, ax, bbox.x0 + x);
		sp_unit_selector_set_value_in_points (us, ay, bbox.y0 + y);
	}

	g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
}

static GtkWidget *
sp_transformation_page_move_new (GObject *obj)
{
	GtkWidget *frame, *vb, *tbl, *lbl, *img, *sb, *us, *cb;
	GtkAdjustment *adj;

	frame = gtk_frame_new (_("Move"));

	vb = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
	gtk_container_add (GTK_CONTAINER (frame), vb);

	tbl = gtk_table_new (4, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (tbl), 4);
	gtk_table_set_col_spacings (GTK_TABLE (tbl), 4);
	gtk_box_pack_start (GTK_BOX (vb), tbl, FALSE, FALSE, 0);

	/* Unit selector */
	us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
	g_object_set_data (obj, "move_units", us);
	if (SP_ACTIVE_DESKTOP) {
		sp_unit_selector_set_unit (SP_UNIT_SELECTOR (us), sp_desktop_get_default_unit (SP_ACTIVE_DESKTOP));
	}
	/* Horizontal */
	img = sp_icon_new (SP_ICON_SIZE_BUTTON, "arrows_hor");
	gtk_table_attach (GTK_TABLE (tbl), img, 0, 1, 0, 1, 0, 0, 0, 0);
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -1e6, 1e6, 0.01, 0.1, 0.1);
	g_object_set_data (obj, "move_position_x", adj);
	sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), adj);
	g_signal_connect (G_OBJECT (adj), "value_changed", G_CALLBACK (sp_transformation_move_value_changed), obj);
	sb = gtk_spin_button_new (adj, 0.1, 2);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	/* Vertical */
	img = sp_icon_new (SP_ICON_SIZE_BUTTON, "arrows_ver");
	gtk_table_attach (GTK_TABLE (tbl), img, 0, 1, 1, 2, 0, 0, 0, 0);
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -1e6, 1e6, 0.01, 0.1, 0.1);
	g_object_set_data (obj, "move_position_y", adj);
	sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), adj);
	g_signal_connect (G_OBJECT (adj), "value_changed", G_CALLBACK (sp_transformation_move_value_changed), obj);
	sb = gtk_spin_button_new (adj, 0.1, 2);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	/* Unit selector */
	lbl = gtk_label_new (_("Units:"));
	gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (tbl), us, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	/* Relative moves */
	cb = gtk_check_button_new_with_label (_("Relative move"));
	g_object_set_data (obj, "move_relative", cb);
	gtk_table_attach (GTK_TABLE (tbl), cb, 1, 2, 3, 4, 0, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb), TRUE);
	g_signal_connect (G_OBJECT (cb), "toggled", G_CALLBACK (sp_transformation_move_relative_toggled), obj);

	gtk_widget_show_all (vb);

	return frame;
}

static void
sp_transformation_move_update (GObject *dlg, SPSelection *selection)
{
	GtkWidget *page;

	page = g_object_get_data (dlg, "move");

	if (selection && !sp_selection_is_empty (selection)) {
		GtkToggleButton *cb;
		cb = g_object_get_data (dlg, "move_relative");
		if (!gtk_toggle_button_get_active (cb)) {
			GtkAdjustment *ax, *ay;
			SPUnitSelector *us;
			NRRectF bbox;
			ax = g_object_get_data (dlg, "move_position_x");
			ay = g_object_get_data (dlg, "move_position_y");
			us = g_object_get_data (dlg, "move_units");
			sp_selection_bbox (selection, &bbox);
			sp_unit_selector_set_value_in_points (us, ax, bbox.x0);
			sp_unit_selector_set_value_in_points (us, ay, bbox.y0);
		}
		gtk_widget_set_sensitive (page, TRUE);
	} else {
		gtk_widget_set_sensitive (page, FALSE);
	}
}

static void
sp_transformation_move_apply (GObject *dlg, SPSelection *selection, unsigned int copy)
{
	SPUnitSelector *us;
	GtkAdjustment *ax, *ay;
	GtkToggleButton *cb;
	float x, y;

	us = g_object_get_data (dlg, "move_units");
	ax = g_object_get_data (dlg, "move_position_x");
	ay = g_object_get_data (dlg, "move_position_y");
	x = sp_unit_selector_get_value_in_points (us, ax);
	y = sp_unit_selector_get_value_in_points (us, ay);

	cb = g_object_get_data (dlg, "move_relative");
	if (gtk_toggle_button_get_active (cb)) {
		sp_selection_move_relative (selection, x, y);
	} else {
		NRRectF bbox;
		sp_selection_bbox (selection, &bbox);
		sp_selection_move_relative (selection, x - bbox.x0, y - bbox.y0);
	}

	if (selection) sp_document_done (SP_DT_DOCUMENT (selection->desktop));
}

/*
 * Scale implementation
 */

static gboolean
sp_transformation_scale_set_unit (SPUnitSelector *us, const SPUnit *old, const SPUnit *new, GObject *dlg)
{
	SPDesktop *desktop;
	SPSelection *selection;

	desktop = SP_ACTIVE_DESKTOP;
	if (!desktop) return FALSE;
	selection = SP_DT_SELECTION (desktop);
	if (sp_selection_is_empty (selection)) return FALSE;

	if ((old->base == SP_UNIT_ABSOLUTE) && (new->base == SP_UNIT_DIMENSIONLESS)) {
		SPUnitSelector *us;
		GtkAdjustment *ax, *ay;
		NRRectF bbox;
		float x, y;
		/* Absolute to percentage */
		g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));
		us = g_object_get_data (dlg, "scale_units");
		ax = g_object_get_data (dlg, "scale_dimension_x");
		ay = g_object_get_data (dlg, "scale_dimension_y");
		x = sp_units_get_points (ax->value, old);
		y = sp_units_get_points (ay->value, old);
		sp_selection_bbox (selection, &bbox);
		gtk_adjustment_set_value (ax, 100.0 * x / (bbox.x1 - bbox.x0));
		gtk_adjustment_set_value (ay, 100.0 * y / (bbox.y1 - bbox.y0));
		g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
		return TRUE;
	} else if ((old->base == SP_UNIT_DIMENSIONLESS) && (new->base == SP_UNIT_ABSOLUTE)) {
		SPUnitSelector *us;
		GtkAdjustment *ax, *ay;
		NRRectF bbox;
		/* Percentage to absolute */
		g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));
		us = g_object_get_data (dlg, "scale_units");
		ax = g_object_get_data (dlg, "scale_dimension_x");
		ay = g_object_get_data (dlg, "scale_dimension_y");
		sp_selection_bbox (selection, &bbox);
		gtk_adjustment_set_value (ax, sp_points_get_units (0.01 * ax->value * (bbox.x1 - bbox.x0), new));
		gtk_adjustment_set_value (ay, sp_points_get_units (0.01 * ay->value * (bbox.y1 - bbox.y0), new));
		g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
		return TRUE;
	}

	return FALSE;
}

static void
sp_transformation_scale_value_changed (GtkAdjustment *adj, GObject *dlg)
{
	GtkWidget *apply;

	if (g_object_get_data (dlg, "update")) return;

	apply = g_object_get_data (dlg, "apply");
	gtk_widget_set_sensitive (apply, TRUE);
}

static GtkWidget *
sp_transformation_page_scale_new (GObject *obj)
{
	GtkWidget *frame, *vb, *tbl, *lbl, *img, *sb, *us;
	GtkAdjustment *adj;

	frame = gtk_frame_new (_("Scale"));

	vb = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
	gtk_container_add (GTK_CONTAINER (frame), vb);

	tbl = gtk_table_new (4, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (tbl), 4);
	gtk_table_set_col_spacings (GTK_TABLE (tbl), 4);
	gtk_box_pack_start (GTK_BOX (vb), tbl, FALSE, FALSE, 0);

	/* Unit selector */
	/* fixme: Default has to be percentage */
	us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
	g_object_set_data (obj, "scale_units", us);
	sp_unit_selector_add_unit (SP_UNIT_SELECTOR (us), sp_unit_get_by_abbreviation ("%"), 0);
	sp_unit_selector_set_unit (SP_UNIT_SELECTOR (us), sp_unit_get_by_abbreviation ("%"));
	g_signal_connect (G_OBJECT (us), "set_unit", G_CALLBACK (sp_transformation_scale_set_unit), obj);
	/* Horizontal */
	img = sp_icon_new (SP_ICON_SIZE_BUTTON, "scale_hor");
	gtk_table_attach (GTK_TABLE (tbl), img, 0, 1, 0, 1, 0, 0, 0, 0);
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -1e6, 1e6, 0.01, 0.1, 0.1);
	g_object_set_data (obj, "scale_dimension_x", adj);
	sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), adj);
	g_signal_connect (G_OBJECT (adj), "value_changed", G_CALLBACK (sp_transformation_scale_value_changed), obj);
	sb = gtk_spin_button_new (adj, 0.1, 2);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	/* Vertical */
	img = sp_icon_new (SP_ICON_SIZE_BUTTON, "scale_ver");
	gtk_table_attach (GTK_TABLE (tbl), img, 0, 1, 1, 2, 0, 0, 0, 0);
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -1e6, 1e6, 0.01, 0.1, 0.1);
	g_object_set_data (obj, "scale_dimension_y", adj);
	sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), adj);
	g_signal_connect (G_OBJECT (adj), "value_changed", G_CALLBACK (sp_transformation_scale_value_changed), obj);
	sb = gtk_spin_button_new (adj, 0.1, 2);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	/* Unit selector */
	lbl = gtk_label_new (_("Units:"));
	gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5);
	gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (tbl), us, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	gtk_widget_show_all (vb);

	return frame;
}

static void
sp_transformation_scale_update (GObject *dlg, SPSelection *selection)
{
	GtkWidget *page;

	page = g_object_get_data (dlg, "scale");

	if (selection && !sp_selection_is_empty (selection)) {
		GtkAdjustment *ax, *ay;
		SPUnitSelector *us;
		const SPUnit *unit;
		NRRectF bbox;
		ax = g_object_get_data (dlg, "scale_dimension_x");
		ay = g_object_get_data (dlg, "scale_dimension_y");
		us = g_object_get_data (dlg, "scale_units");
		sp_selection_bbox (selection, &bbox);
		unit = sp_unit_selector_get_unit (us);
		if (unit->base == SP_UNIT_ABSOLUTE) {
			sp_unit_selector_set_value_in_points (us, ax, bbox.x1 - bbox.x0);
			sp_unit_selector_set_value_in_points (us, ay, bbox.y1 - bbox.y0);
		} else {
			gtk_adjustment_set_value (ax, 100.0);
			gtk_adjustment_set_value (ay, 100.0);
		}
		gtk_widget_set_sensitive (page, TRUE);
	} else {
		gtk_widget_set_sensitive (page, FALSE);
	}
}

static void
sp_transformation_scale_apply (GObject *dlg, SPSelection *selection, unsigned int copy)
{
	GtkAdjustment *ax, *ay;
	SPUnitSelector *us;
	const SPUnit *unit;
	NRRectF bbox;
	NRPointF c;
	float x, y;

	us = g_object_get_data (dlg, "scale_units");
	ax = g_object_get_data (dlg, "scale_dimension_x");
	ay = g_object_get_data (dlg, "scale_dimension_y");

	sp_selection_bbox (selection, &bbox);
	c.x = 0.5 * (bbox.x0 + bbox.x1);
	c.y = 0.5 * (bbox.y0 + bbox.y1);
	unit = sp_unit_selector_get_unit (us);
	if (unit->base == SP_UNIT_ABSOLUTE) {
		x = sp_unit_selector_get_value_in_points (us, ax);
		y = sp_unit_selector_get_value_in_points (us, ay);
		sp_selection_scale_relative (selection, &c, x / (bbox.x1 - bbox.x0), y / (bbox.y1 - bbox.y0));
	} else {
		sp_selection_scale_relative (selection, &c, 0.01 * ax->value, 0.01 * ay->value);
	}

	if (selection) sp_document_done (SP_DT_DOCUMENT (selection->desktop));
}

/*
 * Rotate implementation
 */

static void
sp_transformation_rotate_value_changed (GtkAdjustment *adj, GObject *dlg)
{
	GtkWidget *apply;

	if (g_object_get_data (dlg, "update")) return;

	apply = g_object_get_data (dlg, "apply");
	gtk_widget_set_sensitive (apply, TRUE);
}

static GtkWidget *
sp_transformation_page_rotate_new (GObject *obj)
{
	GtkWidget *frame, *vb, *tbl, *lbl, *img, *sb;
	GtkAdjustment *adj;

	frame = gtk_frame_new (_("Rotate"));

	vb = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
	gtk_container_add (GTK_CONTAINER (frame), vb);

	tbl = gtk_table_new (1, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (tbl), 4);
	gtk_table_set_col_spacings (GTK_TABLE (tbl), 4);
	gtk_box_pack_start (GTK_BOX (vb), tbl, FALSE, FALSE, 0);

	img = sp_icon_new (SP_ICON_SIZE_BUTTON, "rotate_left");
	gtk_table_attach (GTK_TABLE (tbl), img, 0, 1, 0, 1, 0, 0, 0, 0);
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -1e6, 1e6, 0.01, 0.1, 0.1);
	g_object_set_data (obj, "rotate_angle", adj);
	g_signal_connect (G_OBJECT (adj), "value_changed", G_CALLBACK (sp_transformation_rotate_value_changed), obj);
	sb = gtk_spin_button_new (adj, 0.1, 2);
	gtk_table_attach (GTK_TABLE (tbl), sb, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	lbl = gtk_label_new (_("deg"));
	gtk_table_attach (GTK_TABLE (tbl), lbl, 2, 3, 0, 1, 0, 0, 0, 0);

	gtk_widget_show_all (vb);

	return frame;
}

static void
sp_transformation_rotate_update (GObject *dlg, SPSelection *selection)
{
	GtkWidget *page;

	page = g_object_get_data (dlg, "rotate");

	if (selection && !sp_selection_is_empty (selection)) {
		gtk_widget_set_sensitive (page, TRUE);
	} else {
		gtk_widget_set_sensitive (page, FALSE);
	}
}

static void
sp_transformation_rotate_apply (GObject *dlg, SPSelection *selection, unsigned int copy)
{
	GtkAdjustment *a;
	NRRectF bbox;
	NRPointF c;

	a = g_object_get_data (dlg, "rotate_angle");

	sp_selection_bbox (selection, &bbox);
	c.x = 0.5 * (bbox.x0 + bbox.x1);
	c.y = 0.5 * (bbox.y0 + bbox.y1);
	sp_selection_rotate_relative (selection, &c, a->value);

	if (selection) sp_document_done (SP_DT_DOCUMENT (selection->desktop));
}

static void
sp_transformation_skew_apply (GObject *dlg, SPSelection *selection, unsigned int copy)
{
}

#if 0
typedef enum {
	ABSOLUTE,
	RELATIVE
} SPTransformationType;

typedef enum {
	SELECTION,
	DESKTOP
} SPReferType;

/*
 * handler functions for transformation dialog
 *
 * - maybe we should convert spinbutton values when metrics and metric types change
 * - error messages for extremes like width==0 etc 
 */ 

GladeXML * transformation_xml = NULL;
GladeXML * move_metric_xml = NULL, * scale_metric_xml = NULL, * skew_metric_xml = NULL, * center_metric_xml = NULL;

GtkWidget * transformation_dialog = NULL;

GtkWidget * apply_button;
GtkSpinButton * rotate_angle, * skew_value;
GtkSpinButton * move_hor, * move_ver;
GtkSpinButton * scale_hor, * scale_ver;
GtkSpinButton * center_x, * center_y;
GtkToggleButton * flip_hor, * flip_ver;
GtkToggleButton * use_align, * use_center;
GtkToggleButton * keep_width, * keep_height;
GtkToggleButton * skew_rotate;
GtkFrame * align_frame, * center_frame;
GtkWidget * rotate_left, * rotate_right;
GtkWidget * skew_hor, * skew_ver;
GtkWidget * scale_locked, * scale_unlocked;
GtkLabel * old_x, * old_y, * old_width, * old_height;
GtkButton * rotate_direction, * skew_direction;
GtkButton * move_type, * scale_type, * center_type;
GtkButton * lock_scale, * lock_skew;
GtkButton * angle_0, * angle_90, * angle_180, * angle_270;
GtkNotebook * trans_notebook;
GtkToggleButton * make_copy;
GtkToggleButton * align_tl, * align_tc, * align_tr, * align_cl, * align_cc, * align_cr, * align_bl, * align_bc, * align_br;
GtkButton * expand;
GtkHBox * expansion;
GtkArrow * arrow_expand;
GtkOptionMenu * move_metric_om;
GtkWidget * move_metrics, * move_pt, * move_mm, * move_cm, * move_in;
GtkOptionMenu * scale_metric_om;
GtkWidget * scale_metrics, * scale_pr, * scale_pt, * scale_mm, * scale_cm, * scale_in;
GtkOptionMenu * center_metric_om;
GtkWidget * center_metrics, * center_pr, * center_pt, * center_mm, * center_cm, * center_in;
GtkOptionMenu * skew_metric_om;
GtkWidget * skew_metrics, * skew_deg, * skew_pt, * skew_mm, * skew_cm, * skew_in;


static SPTransformationType tr_move_type=ABSOLUTE, tr_scale_type=ABSOLUTE;
static SPReferType tr_center_type=DESKTOP;
static guint sel_changed_id = 0;

// move, scale, rotate, skew, center
void sp_transformation_apply_move (SPSelection * selection);
void sp_transformation_apply_scale (SPSelection * selection);
void sp_transformation_apply_rotate (SPSelection * selection);
void sp_transformation_apply_skew (SPSelection * selection);
void sp_transformation_move_update (SPSelection * selection);
void sp_transformation_scale_update (SPSelection * selection);
void sp_transformation_rotate_update (SPSelection * selection);
void sp_transformation_skew_update (SPSelection * selection);
void sp_transformation_select_move_metric (GtkWidget * widget);
void sp_transformation_select_scale_metric (GtkWidget * widget);
void sp_transformation_select_center_metric (GtkWidget * widget);
void sp_transformation_select_skew_metric (GtkWidget * widget);
void sp_transformation_set_move_metric (SPMetric metric);
void sp_transformation_set_scale_metric (SPMetric metric);
void sp_transformation_set_skew_metric (SPMetric metric);
void sp_transformation_set_center_metric (SPMetric metric);
SPMetric sp_transformation_get_move_metric (void);
SPMetric sp_transformation_get_scale_metric (void);
SPMetric sp_transformation_get_skew_metric (void);
SPMetric sp_transformation_get_center_metric (void);
//handlers
static void sp_transformation_selection_changed (Sodipodi * sodipodi, SPSelection * selection);
void sp_transformation_dialog_apply (void);
void sp_transformation_display_position (ArtDRect * bbox, SPMetric metric);
void sp_transformation_display_dimension (ArtDRect * bbox, SPMetric metric);
void sp_transformation_dialog_reset (GtkWidget * widget);
void sp_transformation_set_angle (GtkButton * widget);
void sp_transformation_direction_change (GtkButton * widget);
void sp_transformation_scale_changed (GtkWidget * widget);
void sp_transformation_scale_lock (void);
void sp_transformation_metric_type (GtkButton * widget);
void sp_transformation_fixpoint_toggle (GtkWidget * widget);
void sp_transformation_keep_toggle (GtkWidget * widget);
void sp_transformation_expand_dialog (void);
void sp_transformation_dialog_set_flip (GtkToggleButton * button);
void sp_transformation_notebook_switch (GtkNotebook *notebook,
					GtkNotebookPage *page,
					gint page_num,
					gpointer user_data);
gboolean sp_transformation_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
//helpers
ArtPoint * sp_transformation_get_align (SPSelection * selection, ArtPoint * p);
ArtPoint * sp_transformation_get_center (SPSelection * selection, ArtPoint * p);
void sp_transformation_apply_button_reset (void);


void
sp_transformation_dialog_close (void) {
  g_assert (transformation_dialog != NULL);

  gtk_widget_hide (transformation_dialog);
  if (sel_changed_id > 0) {
    gtk_signal_disconnect (GTK_OBJECT (sodipodi), sel_changed_id);
    sel_changed_id = 0;
  }
}

// dialog generation
void 
sp_transformation_dialog (void)
{
  transformation_xml = glade_xml_new (SODIPODI_GLADEDIR "/transformation.glade", NULL, PACKAGE);
  glade_xml_signal_autoconnect (transformation_xml);
  transformation_dialog = glade_xml_get_widget (transformation_xml, "transform_dialog_small");
  
  apply_button = glade_xml_get_widget (transformation_xml, "apply_button");
  rotate_angle = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "rotate_angle");
  skew_value = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "skew_value");
  move_hor = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "move_hor");
  move_ver = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "move_ver");
  scale_hor = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "scale_hor");
  scale_ver = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "scale_ver");
  center_x = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "center_x");
  center_y = (GtkSpinButton *) glade_xml_get_widget (transformation_xml, "center_y");
  
  flip_hor = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "flip_hor");
  gtk_button_set_relief (GTK_BUTTON(flip_hor), GTK_RELIEF_NONE);
  flip_ver = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "flip_ver");
  gtk_button_set_relief (GTK_BUTTON(flip_ver), GTK_RELIEF_NONE);
  
  rotate_direction = (GtkButton *) glade_xml_get_widget (transformation_xml, "rotate_direction");
  skew_direction = (GtkButton *) glade_xml_get_widget (transformation_xml, "skew_direction");
  gtk_button_set_relief (rotate_direction, GTK_RELIEF_NONE);
  gtk_button_set_relief (skew_direction, GTK_RELIEF_NONE);
  lock_scale = (GtkButton *) glade_xml_get_widget (transformation_xml, "lock_scale");
  gtk_button_set_relief (lock_scale, GTK_RELIEF_NONE);
  
  expansion = (GtkHBox *) glade_xml_get_widget (transformation_xml, "expansion");
  expand = (GtkButton *) glade_xml_get_widget (transformation_xml, "expand");
  arrow_expand = (GtkArrow *) glade_xml_get_widget (transformation_xml, "arrow_expand");
  
  trans_notebook = (GtkNotebook *) glade_xml_get_widget (transformation_xml, "trans_notebook");
  make_copy = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "make_copy");
  align_tl = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_tl");
  align_tc = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_tc");
  align_tr = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_tr");
  align_cl = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_cl");
  align_cc = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_cc");
  align_cr = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_cr");
  align_bl = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_bl");
  align_bc = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_bc");
  align_br = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "align_br");

  use_align = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "use_align");
  use_center = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "use_center");
  keep_width = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "keep_width");
  keep_height = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "keep_height");
  skew_rotate = (GtkToggleButton *) glade_xml_get_widget (transformation_xml, "skew_rotate");
  align_frame = (GtkFrame *) glade_xml_get_widget (transformation_xml, "align_frame");
  center_frame = (GtkFrame *) glade_xml_get_widget (transformation_xml, "center_frame");
  
  move_type = (GtkButton *) glade_xml_get_widget (transformation_xml, "move_type");
  gtk_button_set_relief (move_type, GTK_RELIEF_NONE);
  scale_type = (GtkButton *) glade_xml_get_widget (transformation_xml, "scale_type");
  gtk_button_set_relief (scale_type, GTK_RELIEF_NONE);
  center_type = (GtkButton *) glade_xml_get_widget (transformation_xml, "center_type");
  gtk_button_set_relief (center_type, GTK_RELIEF_NONE);
  
  angle_0 = (GtkButton *) glade_xml_get_widget (transformation_xml, "angle_0");
  gtk_button_set_relief (angle_0, GTK_RELIEF_NONE);
  angle_90 = (GtkButton *) glade_xml_get_widget (transformation_xml, "angle_90");
  gtk_button_set_relief (angle_90, GTK_RELIEF_NONE);
  angle_180 = (GtkButton *) glade_xml_get_widget (transformation_xml, "angle_180");
  gtk_button_set_relief (angle_180, GTK_RELIEF_NONE);
  angle_270 = (GtkButton *) glade_xml_get_widget (transformation_xml, "angle_270");
  gtk_button_set_relief (angle_270, GTK_RELIEF_NONE);
  
  rotate_left = glade_xml_get_widget (transformation_xml, "rotate_left");
  rotate_right = glade_xml_get_widget (transformation_xml, "rotate_right");
  scale_locked = glade_xml_get_widget (transformation_xml, "scale_locked");
  scale_unlocked = glade_xml_get_widget (transformation_xml, "scale_unlocked");
  skew_hor = glade_xml_get_widget (transformation_xml, "skew_hor");
  skew_ver = glade_xml_get_widget (transformation_xml, "skew_ver");
  
  old_x = (GtkLabel *) glade_xml_get_widget (transformation_xml, "old_x");
  old_y = (GtkLabel *) glade_xml_get_widget (transformation_xml, "old_y");
  old_width = (GtkLabel *) glade_xml_get_widget (transformation_xml, "old_width");
  old_height = (GtkLabel *) glade_xml_get_widget (transformation_xml, "old_height");
  // move metrics
  move_metric_om = (GtkOptionMenu *) glade_xml_get_widget (transformation_xml, "move_metric_om");
  move_metrics = glade_xml_get_widget (transformation_xml, "move_metrics");
  gtk_option_menu_set_menu (move_metric_om, move_metrics);
  move_pt = glade_xml_get_widget (transformation_xml, "move_pt");
  move_mm = glade_xml_get_widget (transformation_xml, "move_mm");
  move_cm = glade_xml_get_widget (transformation_xml, "move_cm");
  move_in = glade_xml_get_widget (transformation_xml, "move_in");
  // scale metrics
  scale_metric_om = (GtkOptionMenu *) glade_xml_get_widget (transformation_xml, "scale_metric_om");
  scale_metrics = glade_xml_get_widget (transformation_xml, "scale_metrics");
  gtk_option_menu_set_menu (scale_metric_om, scale_metrics);
  scale_pr = glade_xml_get_widget (transformation_xml, "scale_%");
  scale_pt = glade_xml_get_widget (transformation_xml, "scale_pt");
  scale_mm = glade_xml_get_widget (transformation_xml, "scale_mm");
  scale_cm = glade_xml_get_widget (transformation_xml, "scale_cm");
  scale_in = glade_xml_get_widget (transformation_xml, "scale_in");
  // center metrics
  center_metric_om = (GtkOptionMenu *) glade_xml_get_widget (transformation_xml, "center_metric_om");
  center_metrics = glade_xml_get_widget (transformation_xml, "center_metrics");
  gtk_option_menu_set_menu (center_metric_om, center_metrics);
  center_pt = glade_xml_get_widget (transformation_xml, "center_pt");
  center_mm = glade_xml_get_widget (transformation_xml, "center_mm");
  center_cm = glade_xml_get_widget (transformation_xml, "center_cm");
  center_in = glade_xml_get_widget (transformation_xml, "center_in");
  center_pr = glade_xml_get_widget (transformation_xml, "center_%");
  // skew metrics
  skew_metric_om = (GtkOptionMenu *) glade_xml_get_widget (transformation_xml, "skew_metric_om");
  skew_metrics = glade_xml_get_widget (transformation_xml, "skew_metrics");
  gtk_option_menu_set_menu (skew_metric_om, skew_metrics);
  skew_deg = glade_xml_get_widget (transformation_xml, "skew_deg");
  skew_pt = glade_xml_get_widget (transformation_xml, "skew_pt");
  skew_mm = glade_xml_get_widget (transformation_xml, "skew_mm");
  skew_cm = glade_xml_get_widget (transformation_xml, "skew_cm");
  skew_in = glade_xml_get_widget (transformation_xml, "skew_in");
  sp_transformation_dialog_reset (NULL);
}

/*
 * rotate
 */

void
sp_transformation_rotate_update (SPSelection * selection) {
  g_assert (transformation_dialog != NULL);
}

void
sp_transformation_apply_rotate (SPSelection * selection) {
  ArtPoint bp, ap, p;
  double angle, dx=0, dy=0;
  ArtDRect bbox, bbox2;

  g_assert (transformation_dialog != NULL);
  g_assert (!sp_selection_is_empty (selection));

  //angle
  angle = gtk_spin_button_get_value_as_float (rotate_angle);
  if (GTK_WIDGET_VISIBLE (rotate_right)) angle = -angle;
  // before + default
  sp_selection_bbox (selection, &bbox);
  sp_transformation_get_align (selection,&bp);
  p.x = (bbox.x0 + bbox.x1)/2;
  p.y = (bbox.y0 + bbox.y1)/2;
  // explicit center
  if (GTK_WIDGET_VISIBLE (expansion) && gtk_toggle_button_get_active (use_center)) sp_transformation_get_center (selection,&p);
  // rotate
  sp_selection_rotate_relative (selection, &p, angle);
  //after
  sp_selection_bbox (selection, &bbox2);
  // align
  if (GTK_WIDGET_VISIBLE (expansion) && gtk_toggle_button_get_active (use_align)) {
    sp_transformation_get_align (selection,&ap);
    p.x = bp.x;
    p.y = bp.y;
    dx = bp.x-ap.x;
    dy = bp.y-ap.y;
    sp_selection_move_relative (selection, dx, dy);
  }
  // keep width/height
  if (GTK_WIDGET_VISIBLE (expansion) && (gtk_toggle_button_get_active (keep_width) || 
					 gtk_toggle_button_get_active (keep_height))) {
    if (fabs(bbox2.x1-bbox2.x0)<1e-15 || fabs(bbox2.y1-bbox2.y0)<1e-15) return;
    if (gtk_toggle_button_get_active (keep_width)) dx = dy =fabs(bbox.x1-bbox.x0) / fabs(bbox2.x1-bbox2.x0);
    if (gtk_toggle_button_get_active (keep_height)) dx = dy = fabs(bbox.y1-bbox.y0) / fabs(bbox2.y1-bbox2.y0);

    if (dx < 1e-15 || dy < 1e-15) return;
    sp_selection_scale_relative (selection, &p, dx, dy);
  }
}

/*
 * skew
 */

void
sp_transformation_select_skew_metric (GtkWidget * widget) {
}

void
sp_transformation_set_skew_metric (SPMetric metric){

  if (metric == SP_PT) gtk_option_menu_set_history (skew_metric_om, 0);
  if (metric == SP_MM) gtk_option_menu_set_history (skew_metric_om, 1);
  if (metric == SP_CM) gtk_option_menu_set_history (skew_metric_om, 2);
  if (metric == SP_IN) gtk_option_menu_set_history (skew_metric_om, 3);
  if (metric == NONE) gtk_option_menu_set_history (skew_metric_om, 4);
  sp_transformation_select_skew_metric (NULL);
}

SPMetric 
sp_transformation_get_skew_metric (void){
  GtkWidget * selected;

  selected = gtk_menu_get_active ((GtkMenu *) skew_metrics);

  if (selected == skew_deg) return NONE;
  if (selected == skew_pt) return SP_PT;
  if (selected == skew_mm) return SP_MM;
  if (selected == skew_cm) return SP_CM;
  if (selected == skew_in) return SP_IN;

  return SP_PT;
}

void
sp_transformation_skew_update (SPSelection * selection) {
  g_assert (transformation_dialog != NULL);

}

void
sp_transformation_apply_skew (SPSelection * selection) {
  ArtDRect bbox, bbox2;
  ArtPoint p, bp, ap;
  double dx=0, dy=0, a, b=0, first, snd=0;
  SPMetric metric;

  g_assert (transformation_dialog != NULL);
  g_assert (!sp_selection_is_empty (selection));

  // before + default
  sp_selection_bbox (selection, &bbox);
  sp_transformation_get_align (selection,&bp);
  p.x = (bbox.x0 + bbox.x1)/2;
  p.y = (bbox.y0 + bbox.y1)/2;
  // explicit center
  if (GTK_WIDGET_VISIBLE (expansion) && gtk_toggle_button_get_active (use_center)) sp_transformation_get_center (selection,&p);

  // skew
  metric = sp_transformation_get_skew_metric ();
  a = gtk_spin_button_get_value_as_float (skew_value);
  snd = 0;
  if (!gtk_toggle_button_get_active (skew_rotate)) { // no aspect
    if (metric == NONE) { 
      if (fabs((fabs( remainder(a,180))-90 ))  < 1e-10) return;
      first = tan (M_PI*a/180);
    } else {
      if (fabs(bbox.x1-bbox.x0)<1e-15 || fabs(bbox.y1-bbox.y0)<1e-15) return;
      a = SP_METRIC_TO_PT (gtk_spin_button_get_value_as_float (skew_value), metric);
      if (GTK_WIDGET_VISIBLE (skew_hor)) first = a / fabs(bbox.y1-bbox.y0);
      else first = a / fabs(bbox.x1-bbox.x0);
    }
  } else { // keep aspect
    if (metric == NONE) { 
      b = M_PI * a / 180;
    } else {
      if (fabs(bbox.x1-bbox.x0)<1e-15 || fabs(bbox.y1-bbox.y0)<1e-15) return;
      a = SP_METRIC_TO_PT (a, metric);
      if (GTK_WIDGET_VISIBLE (skew_hor)) b = M_PI * (a / fabs(bbox.y1-bbox.y0))/2;
      else b = M_PI * (a / fabs(bbox.x1-bbox.x0))/2;
    }
    first = sin (b);
    if (GTK_WIDGET_VISIBLE (skew_hor));  
  }
 
  if (GTK_WIDGET_VISIBLE (skew_hor)) sp_selection_skew_relative (selection, &p, first, snd);
  else  sp_selection_skew_relative (selection, &p, snd, first);

  if (gtk_toggle_button_get_active (skew_rotate)) { // keep aspect
    first = cos (b);
    snd = 1;
    if (GTK_WIDGET_VISIBLE (skew_hor)) sp_selection_scale_relative (selection, &p, snd, first);
    else  sp_selection_scale_relative (selection, &p, first, snd);
  }

  //after
  sp_selection_bbox (selection, &bbox2);
  // align
  if (GTK_WIDGET_VISIBLE (expansion) && gtk_toggle_button_get_active (use_align)) {
    sp_transformation_get_align (selection,&ap);
    p.x = bp.x;
    p.y = bp.y;
    dx = bp.x-ap.x;
    dy = bp.y-ap.y;
    sp_selection_move_relative (selection, dx, dy);
  }
  // keep width/height
  if (GTK_WIDGET_VISIBLE (expansion) && (gtk_toggle_button_get_active (keep_width) || 
					 gtk_toggle_button_get_active (keep_height))) {
    if (fabs(bbox2.x1-bbox2.x0)<1e-15 || fabs(bbox2.y1-bbox2.y0)<1e-15) return;
    if (gtk_toggle_button_get_active (keep_width)) dx = dy =fabs(bbox.x1-bbox.x0) / fabs(bbox2.x1-bbox2.x0);
    if (gtk_toggle_button_get_active (keep_height)) dx = dy = fabs(bbox.y1-bbox.y0) / fabs(bbox2.y1-bbox2.y0);
    if (dx < 1e-15 || dy < 1e-15) return;
    sp_selection_scale_relative (selection, &p, dx, dy);
  }
}

/*
 * center
 */

void
sp_transformation_select_center_metric (GtkWidget * widget) {
}

void
sp_transformation_set_center_metric (SPMetric metric){

  if (metric == SP_PT) gtk_option_menu_set_history (center_metric_om, 0);
  if (metric == SP_MM) gtk_option_menu_set_history (center_metric_om, 1);
  if (metric == SP_CM) gtk_option_menu_set_history (center_metric_om, 2);
  if (metric == SP_IN) gtk_option_menu_set_history (center_metric_om, 3);
}

SPMetric 
sp_transformation_get_center_metric (void){
  GtkWidget * selected;

  selected = gtk_menu_get_active ((GtkMenu *) center_metrics);

  if (selected == center_pr) return NONE;
  if (selected == center_pt) return SP_PT;
  if (selected == center_mm) return SP_MM;
  if (selected == center_cm) return SP_CM;
  if (selected == center_in) return SP_IN;

  return SP_PT;
}


    // update handels and undo
  sp_selection_changed (selection);
  sp_document_done (SP_DT_DOCUMENT (desktop));
}

void
sp_transformation_dialog_set_flip (GtkToggleButton * button) {
  if (button == flip_hor) {
    if (gtk_toggle_button_get_active (flip_hor)) 
      gtk_widget_set_sensitive((GtkWidget *)scale_hor,0);
    else gtk_widget_set_sensitive((GtkWidget *)scale_hor,1);
  }
  if (button == flip_ver) {
    if (gtk_toggle_button_get_active (flip_ver)) 
      gtk_widget_set_sensitive((GtkWidget *)scale_ver,0);
    else gtk_widget_set_sensitive((GtkWidget *)scale_ver,1);
  }
}

void
sp_transformation_expand_dialog (void) {

  if (GTK_WIDGET_VISIBLE ((GtkWidget *) expansion)) {
    gtk_widget_hide (GTK_WIDGET (expansion));
    gtk_arrow_set (arrow_expand, GTK_ARROW_RIGHT, GTK_SHADOW_ETCHED_IN);
  } else {
    gtk_widget_show (GTK_WIDGET (expansion));
    gtk_arrow_set (arrow_expand, GTK_ARROW_LEFT, GTK_SHADOW_ETCHED_IN);
  }
}

void
sp_transformation_fixpoint_toggle (GtkWidget * widget) {
  GtkToggleButton * button;

  button = (GtkToggleButton *) widget;
  if (gtk_toggle_button_get_active (button)) {
    if (button == use_align) {
      // use_align was pressed
      gtk_widget_set_sensitive (GTK_WIDGET (align_frame), TRUE);
      if (gtk_toggle_button_get_active (use_center)) {
	gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
	gtk_toggle_button_set_active (use_center,FALSE);
      }
    } else {
      // use center was pressed
      gtk_widget_set_sensitive (GTK_WIDGET (center_frame), TRUE);
      if (gtk_toggle_button_get_active (use_align)) {
	gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
	gtk_toggle_button_set_active (use_align,FALSE);
      }
    };
  } else {
    if (button == use_align) gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
    else gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
  }
}

void
sp_transformation_keep_toggle (GtkWidget * widget) {
  GtkToggleButton * button;

  button = (GtkToggleButton *) widget;
  if (gtk_toggle_button_get_active (button)) {
    if (button == keep_width) gtk_toggle_button_set_active (keep_height,FALSE);
    else gtk_toggle_button_set_active (keep_width,FALSE);
  }
}

void
sp_transformation_notebook_switch (GtkNotebook *notebook,
				   GtkNotebookPage *page,
				   gint page_num,
				   gpointer user_data) {
  SPDesktop * desktop;
  SPSelection * selection;

  desktop = SP_ACTIVE_DESKTOP;
  if (SP_IS_DESKTOP (desktop)) selection = SP_DT_SELECTION (desktop);
  else selection = NULL;
  
  switch (page_num) {
  case 0:
    gtk_widget_set_sensitive (GTK_WIDGET (keep_width), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (keep_height), FALSE);
    if (tr_move_type == RELATIVE) {
      gtk_widget_set_sensitive (GTK_WIDGET (use_center), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (use_align), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
    } else {
      gtk_widget_set_sensitive (GTK_WIDGET (use_align), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (use_center), TRUE);
      if (gtk_toggle_button_get_active (use_center)) gtk_widget_set_sensitive (GTK_WIDGET (center_frame), TRUE);
      else gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
      if (gtk_toggle_button_get_active (use_align)) gtk_widget_set_sensitive (GTK_WIDGET (align_frame), TRUE);
      else gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
    }
    sp_transformation_move_update (selection);
    break;
  case 1:
    gtk_widget_set_sensitive (GTK_WIDGET (keep_width), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (keep_height), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (use_center), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (use_align), TRUE);
    if (gtk_toggle_button_get_active (use_center)) gtk_widget_set_sensitive (GTK_WIDGET (center_frame), TRUE);
    else gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
    if (gtk_toggle_button_get_active (use_align)) gtk_widget_set_sensitive (GTK_WIDGET (align_frame), TRUE);
    else gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);

    sp_transformation_scale_update (selection);
    break;
  case 2:
    gtk_widget_set_sensitive (GTK_WIDGET (keep_width), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (keep_height), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (use_center), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (use_align), TRUE);
    if (gtk_toggle_button_get_active (use_center)) gtk_widget_set_sensitive (GTK_WIDGET (center_frame), TRUE);
    else gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
    if (gtk_toggle_button_get_active (use_align)) gtk_widget_set_sensitive (GTK_WIDGET (align_frame), TRUE);
    else gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);

    sp_transformation_rotate_update (selection);
    break;
  case 3:
    gtk_widget_set_sensitive (GTK_WIDGET (keep_width), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (keep_height), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (use_center), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (use_align), TRUE);
    if (gtk_toggle_button_get_active (use_center)) gtk_widget_set_sensitive (GTK_WIDGET (center_frame), TRUE);
    else gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
    if (gtk_toggle_button_get_active (use_align)) gtk_widget_set_sensitive (GTK_WIDGET (align_frame), TRUE);
    else gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);

    sp_transformation_skew_update (selection);
    break;
  }
}

void
sp_transformation_metric_type (GtkButton * widget) {
  if (widget == move_type) {
    if (tr_move_type == ABSOLUTE) {
      gtk_object_set(GTK_OBJECT(move_type), 
                  "label", "relative",
                  NULL);
      tr_move_type = RELATIVE;
      gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (use_align), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (use_center), FALSE);
    } else {
      gtk_object_set(GTK_OBJECT(move_type), 
                  "label", "absolute",
                  NULL);
      tr_move_type = ABSOLUTE;
      gtk_widget_set_sensitive (GTK_WIDGET (use_align), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (use_center), TRUE);
      if (gtk_toggle_button_get_active (use_center)) gtk_widget_set_sensitive (GTK_WIDGET (center_frame), TRUE);
      else gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
      if (gtk_toggle_button_get_active (use_align)) gtk_widget_set_sensitive (GTK_WIDGET (align_frame), TRUE);
      else gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
    }
  }
  if (widget == scale_type) {
    if (tr_scale_type == ABSOLUTE) {
      gtk_object_set(GTK_OBJECT(scale_type), 
                  "label", "relative",
                  NULL);
      tr_scale_type = RELATIVE;
    } else {
      gtk_object_set(GTK_OBJECT(scale_type), 
                  "label", "absolute",
                  NULL);
      tr_scale_type = ABSOLUTE;
    }
  }
  if (widget == center_type) {
    if (tr_center_type == SELECTION) {
      gtk_object_set(GTK_OBJECT(center_type), 
                  "label", "desktop",
                  NULL);
      tr_center_type = DESKTOP;
      gtk_widget_hide (center_pr);
      if (sp_transformation_get_center_metric () == NONE) sp_transformation_set_center_metric (SP_DEFAULT_METRIC);
    } else {
      gtk_object_set(GTK_OBJECT(center_type), 
                  "label", "selection",
                  NULL);
      tr_center_type = SELECTION;
      gtk_widget_show (center_pr);
    }
  }
}

void 
sp_transformation_scale_lock (void) {
  gdouble value;
  if (GTK_WIDGET_VISIBLE (scale_unlocked)) {
    gtk_widget_hide (scale_unlocked);
    gtk_widget_show (scale_locked);
    value = gtk_spin_button_get_value_as_float (scale_hor);
    gtk_spin_button_set_value (scale_ver, value);
  } else {
    gtk_widget_hide (scale_locked);
    gtk_widget_show (scale_unlocked);
  }
}

void 
sp_transformation_direction_change (GtkButton * widget) {
  if (widget == rotate_direction) {
    if (GTK_WIDGET_VISIBLE (rotate_left)) {
      gtk_widget_hide (rotate_left);
      gtk_widget_show (rotate_right);
    } else {
      gtk_widget_hide (rotate_right);
      gtk_widget_show (rotate_left);
    }
  }
  if (widget == skew_direction) {
    if (GTK_WIDGET_VISIBLE (skew_hor)) {
      gtk_widget_hide (skew_hor);
      gtk_widget_show (skew_ver);
    } else {
      gtk_widget_hide (skew_ver);
      gtk_widget_show (skew_hor);
    }
  }
}

void
sp_transformation_set_angle (GtkButton * widget) {
  if (widget == angle_0) gtk_spin_button_set_value (rotate_angle, 0.0);
  if (widget == angle_90) gtk_spin_button_set_value (rotate_angle, 90.0);
  if (widget == angle_180) gtk_spin_button_set_value (rotate_angle, 180.0);
  if (widget == angle_270) gtk_spin_button_set_value (rotate_angle, 270.0);
}

void
sp_transformation_apply_button_reset (void) {
  SPDesktop * desktop;
  SPSelection * selection;

  desktop = SP_ACTIVE_DESKTOP;
  if (desktop != NULL) {
    selection = SP_DT_SELECTION(desktop);
    if (!sp_selection_is_empty (selection)) { 
      gtk_widget_set_sensitive (GTK_WIDGET (apply_button), TRUE);
      return;
    }
  }
  gtk_widget_set_sensitive (GTK_WIDGET (apply_button), FALSE);
}

void
sp_transformation_dialog_reset (GtkWidget * widget) {
  //  gint page;

  g_assert (transformation_dialog != NULL);

  gtk_toggle_button_set_active (align_cc, TRUE);
  gtk_toggle_button_set_active (use_align, FALSE);
  gtk_toggle_button_set_active (use_center, FALSE);
  gtk_toggle_button_set_active (keep_width, FALSE);
  gtk_toggle_button_set_active (keep_height, FALSE);
  gtk_toggle_button_set_active (make_copy, FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (align_frame), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (center_frame), FALSE);
  gtk_object_set(GTK_OBJECT(center_type), 
		 "label", "selection",
		 NULL);
  gtk_spin_button_set_value (center_x, 0.0);
  gtk_spin_button_set_value (center_y, 0.0);
  //  gtk_widget_hide (center_pr);
  tr_center_type = SELECTION;
  sp_transformation_set_center_metric (SP_DEFAULT_METRIC);
  /*
  page = gtk_notebook_get_current_page(trans_notebook);
  switch (page) {
  case 0:
  */
    gtk_spin_button_set_value (move_hor, 0.0);
    gtk_spin_button_set_value (move_ver, 0.0);
    gtk_object_set(GTK_OBJECT(move_type), 
		   "label", "relative",
		   NULL);
    tr_move_type = RELATIVE;
    sp_transformation_set_move_metric (SP_DEFAULT_METRIC);
    /*
    break;
  case 1:
    */
    gtk_spin_button_set_value (scale_hor, 0.0);
    gtk_spin_button_set_value (scale_ver, 0.0);
    gtk_toggle_button_set_active (flip_hor, FALSE);
    gtk_toggle_button_set_active (flip_hor, FALSE);
    gtk_widget_hide (scale_unlocked);
    gtk_widget_show (scale_locked);
    gtk_object_set(GTK_OBJECT(scale_type), 
		   "label", "relative",
		   NULL);
    tr_scale_type = RELATIVE;
    sp_transformation_set_scale_metric (SP_DEFAULT_METRIC);
    /* 
   break;
  case 2:
    */
    gtk_spin_button_set_value (rotate_angle, 0.0);
    gtk_widget_hide (rotate_left);
    gtk_widget_show (rotate_right);
    /*
    break;
  case 3:
    */
    gtk_spin_button_set_value (skew_value, 0.0);
    gtk_widget_hide (skew_ver);
    gtk_widget_show (skew_hor);
    sp_transformation_set_skew_metric (SP_DEFAULT_METRIC);
    gtk_toggle_button_set_active (skew_rotate, FALSE);
    /*
    break;
  }
    */
}

void
sp_transformation_scale_changed (GtkWidget * widget) {
  gdouble value;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (scale_locked))) {
    if (widget == GTK_WIDGET (scale_hor)) {
      value = gtk_spin_button_get_value_as_float (scale_hor);
      gtk_spin_button_set_value (scale_ver, value);
    } 
    if (widget == GTK_WIDGET (scale_ver)) {
      value = gtk_spin_button_get_value_as_float (scale_ver);
      gtk_spin_button_set_value (scale_hor, value);
    } 
  }
}


/*
 * key bindings for transformation dialog
 */

gboolean
sp_transformation_key_press (GtkWidget *widget,
			     GdkEventKey *event,
			     gpointer user_data) {

  g_assert (transformation_dialog != NULL);

  if (event->state & GDK_CONTROL_MASK) switch (event->keyval) {
  case 49:
  case 50:
  case 51:
  case 52:
    gtk_notebook_set_page (trans_notebook, event->keyval-49);
    break;
  case 114:
    sp_transformation_dialog_reset (NULL);
    break;
  case 101:
    sp_transformation_expand_dialog ();
    break;
  case 97:
    sp_transformation_dialog_apply ();
    break;
  case 99:
    sp_transformation_dialog_close ();
    break;
  case 109:
    if (gtk_toggle_button_get_active (make_copy)) gtk_toggle_button_set_active (make_copy, FALSE);
    else gtk_toggle_button_set_active (make_copy, TRUE);
    break;
  }

  return TRUE;
};

/*
 * helpers
 */

ArtPoint *
sp_transformation_get_align (SPSelection * selection, ArtPoint * p) {
  ArtDRect  bbox;

  g_assert (SP_IS_SELECTION (selection));
  sp_selection_bbox (selection, &bbox);

  if (gtk_toggle_button_get_active (align_tl)) { p->x = bbox.x0; p->y = bbox.y1; }
  if (gtk_toggle_button_get_active (align_tc)) { p->x = (bbox.x0+bbox.x1)/2; p->y = bbox.y1; }
  if (gtk_toggle_button_get_active (align_tr)) { p->x = bbox.x1; p->y = bbox.y1; }
  if (gtk_toggle_button_get_active (align_cl)) { p->x = bbox.x0; p->y = (bbox.y0+bbox.y1)/2; }
  if (gtk_toggle_button_get_active (align_cc)) { p->x = (bbox.x0+bbox.x1)/2; p->y = (bbox.y0+bbox.y1)/2; }
  if (gtk_toggle_button_get_active (align_cr)) { p->x = bbox.x1; p->y = (bbox.y0+bbox.y1)/2; }
  if (gtk_toggle_button_get_active (align_bl)) { p->x = bbox.x0; p->y = bbox.y0; }
  if (gtk_toggle_button_get_active (align_bc)) { p->x = (bbox.x0+bbox.x1)/2; p->y = bbox.y0; }
  if (gtk_toggle_button_get_active (align_br)) { p->x = bbox.x1; p->y = bbox.y0; }

  return p;
}

ArtPoint *
sp_transformation_get_center (SPSelection * selection, ArtPoint * p) {
  ArtDRect  bbox;
  SPMetric metric;

  g_assert (SP_IS_SELECTION (selection));
  sp_selection_bbox (selection, &bbox);

  metric = sp_transformation_get_center_metric ();

  if (tr_center_type == DESKTOP) {
    p->x = SP_METRIC_TO_PT (gtk_spin_button_get_value_as_float (center_x), metric);
    p->y = SP_METRIC_TO_PT (gtk_spin_button_get_value_as_float (center_y), metric);
  } else {
    // center is relative to selection
    if (metric == NONE) {
      p->x = bbox.x0 + fabs(bbox.x1 - bbox.x0) * gtk_spin_button_get_value_as_float (center_x)/100;
      p->y = bbox.y0 + fabs(bbox.y1 - bbox.y0) * gtk_spin_button_get_value_as_float (center_y)/100;
    } else {
      p->x = bbox.x0 + SP_METRIC_TO_PT (gtk_spin_button_get_value_as_float (center_x), metric);
      p->y = bbox.y0 + SP_METRIC_TO_PT (gtk_spin_button_get_value_as_float (center_y), metric);
    }
  }
  return p;
}


#endif
