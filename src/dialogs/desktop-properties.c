#define __SP_DESKTOP_PROPERTIES_C__

/*
 * Desktop configuration dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <gtk/gtknotebook.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhseparator.h>

#include "macros.h"
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "svg/svg.h"
#include "widgets/sp-color-selector.h"
#include "widgets/sp-color-preview.h"
#include "../sodipodi.h"
#include "../document.h"
#include "../desktop.h"
#include "../desktop-handles.h"
#include "../sp-namedview.h"
#include "widgets/spw-utilities.h"

#include "desktop-properties.h"

static GtkWidget *sp_desktop_dialog_new (void);

static void sp_dtw_activate_desktop (Sodipodi *sodipodi, SPDesktop *desktop, GtkWidget *dialog);
static void sp_dtw_desactivate_desktop (Sodipodi *sodipodi, SPDesktop *desktop, GtkWidget *dialog);
static void sp_dtw_update (GtkWidget *dialog, SPDesktop *desktop);

static GtkWidget *sp_color_picker_new (unsigned char *colorkey, unsigned char *alphakey, unsigned char *title, guint32 rgba);
static void sp_color_picker_set_rgba32 (GtkWidget *cp, guint32 rgba);
static void sp_color_picker_clicked (GObject *cp, void *data);
void sp_color_picker_button(GtkWidget * dialog, GtkWidget * t, const guchar * label, guchar * key, guchar * color_dialog_label, guchar * opacity_key, int row);

static GtkWidget *dlg = NULL;

static void
sp_dtw_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (SODIPODI, dlg);

	dlg = NULL;
}

void
sp_desktop_dialog (void)
{
	if (!dlg) {
		dlg = sp_desktop_dialog_new ();
		g_signal_connect (G_OBJECT (dlg), "destroy", G_CALLBACK (sp_dtw_dialog_destroy), NULL);
		gtk_widget_show (dlg);
	}

	gtk_window_present (GTK_WINDOW (dlg));
}

static void
sp_dtw_whatever_toggled (GtkToggleButton *tb, GtkWidget *dialog)
{
	SPDesktop *dt;
	SPDocument *doc;
	SPRepr *repr;
	const guchar *key;

	if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	doc = SP_DT_DOCUMENT (dt);

	repr = SP_OBJECT_REPR (dt->namedview);
	key = gtk_object_get_data (GTK_OBJECT (tb), "key");

	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_boolean (repr, key, gtk_toggle_button_get_active (tb));
	sp_document_set_undo_sensitive (doc, TRUE);
}

static void
sp_dtw_border_layer_toggled (GtkToggleButton *tb, GtkWidget *dialog)
{
	SPDesktop *dt;
	SPDocument *doc;
	SPRepr *repr;

	if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	doc = SP_DT_DOCUMENT (dt);

	repr = SP_OBJECT_REPR (dt->namedview);

	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_attr (repr, "borderlayer", gtk_toggle_button_get_active (tb) ? "top" : NULL);
	sp_document_set_undo_sensitive (doc, TRUE);
}

static void
sp_dtw_whatever_changed (GtkAdjustment *adjustment, GtkWidget *dialog)
{
	SPDesktop *dt;
	SPDocument *doc;
	SPRepr *repr;
	SPUnitSelector *us;
	const guchar *key;
	guchar c[32];

	if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	doc = SP_DT_DOCUMENT (dt);

	repr = SP_OBJECT_REPR (dt->namedview);
	key = gtk_object_get_data (GTK_OBJECT (adjustment), "key");
	us = gtk_object_get_data (GTK_OBJECT (adjustment), "unit_selector");

	g_snprintf (c, 32, "%g%s", adjustment->value, sp_unit_selector_get_unit (us)->abbr);

	sp_document_set_undo_sensitive (doc, FALSE);
	sp_repr_set_attr (repr, key, c);
	sp_document_set_undo_sensitive (doc, TRUE);
}

static void
sp_dtw_grid_snap_distance_changed (GtkAdjustment *adjustment, GtkWidget *dialog)
{
	SPRepr *repr;
	SPUnitSelector *us;
	guchar c[32];

	if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

	if (!SP_ACTIVE_DESKTOP) return;

	repr = SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview);

	us = gtk_object_get_data (GTK_OBJECT (dialog), "grid_snap_units");

	g_snprintf (c, 32, "%g%s", adjustment->value, sp_unit_selector_get_unit (us)->abbr);
	sp_repr_set_attr (repr, "gridtolerance", c);
}

static void
sp_dtw_guides_snap_distance_changed (GtkAdjustment *adjustment, GtkWidget *dialog)
{
	SPRepr *repr;
	SPUnitSelector *us;
	guchar c[32];

	if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

	if (!SP_ACTIVE_DESKTOP) return;

	repr = SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview);

	us = gtk_object_get_data (GTK_OBJECT (dialog), "guide_snap_units");

	g_snprintf (c, 32, "%g%s", adjustment->value, sp_unit_selector_get_unit (us)->abbr);
	sp_repr_set_attr (repr, "guidetolerance", c);
}

static GtkWidget *
sp_desktop_dialog_new (void)
{
	GtkWidget *dialog, *nb, *l, *t, *b, *us, *sb, *cp, *i;
	GtkObject *a;
	GCallback cb;
	int row;

	cb = G_CALLBACK(sp_dtw_whatever_changed);

	dialog = sp_window_new (_("Desktop settings"), FALSE);

	nb = gtk_notebook_new ();
	gtk_widget_show (nb);
	gtk_container_add (GTK_CONTAINER (dialog), nb);

	/* Grid settings */

	/* Notebook tab */
	l = gtk_label_new (_("Grid"));
	gtk_widget_show (l);
	t = gtk_table_new (9, 2, FALSE);
	gtk_widget_show (t);
	gtk_container_set_border_width (GTK_CONTAINER (t), 4);
	gtk_table_set_row_spacings (GTK_TABLE (t), 4);
	gtk_table_set_col_spacings (GTK_TABLE (t), 4);
	gtk_notebook_append_page (GTK_NOTEBOOK (nb), t, l);

	/* Checkbuttons */
	row = 0;
	spw_checkbutton(dialog, t, _("Show grid"), "showgrid", 0, row, 0, cb);
	spw_checkbutton(dialog, t, _("Snap to grid"), "snaptogrid", 1, row++, 0, cb);

	spw_checkbutton(dialog, t, _("Horizontal lines"), "vertgrid", 0, row, 0, cb);
	spw_checkbutton(dialog, t, _("Vertical lines"), "horizgrid", 1, row++, 0, cb);

	spw_checkbutton(dialog, t, _("Iso grid"), "isogrid", 0, row, 0, cb);
	spw_checkbutton(dialog, t, _("Hex grid"), "hexgrid", 1, row++, 0, cb);

	us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
	spw_dropdown(dialog, t, _("Grid units:"), "grid_units", row++, us);

	spw_unit_selector(dialog, t, _("Origin X:"), "gridoriginx", row++, us,
			  G_CALLBACK (sp_dtw_whatever_changed));
	spw_unit_selector(dialog, t, _("Origin Y:"), "gridoriginy", row++, us,
			  G_CALLBACK (sp_dtw_whatever_changed));
	
	spw_unit_selector(dialog, t, _("Spacing X:"), "gridspacingx", row++, us,
			  G_CALLBACK (sp_dtw_whatever_changed));
	spw_unit_selector(dialog, t, _("Spacing Y:"), "gridspacingy", row++, us,
			  G_CALLBACK (sp_dtw_whatever_changed));

 	us = sp_unit_selector_new (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
	
	spw_unit_selector(dialog, t, _("Snap distance:"), "gridtolerance", row++, us,
			  G_CALLBACK (sp_dtw_grid_snap_distance_changed) );

	sp_color_picker_button(dialog, t, _("Grid color:"), "gridcolor",
			       _("Grid color"), "gridhicolor", row++);
	
	row=0;
 	/* Guidelines page */
	l = gtk_label_new (_("Guides"));
	gtk_widget_show (l);
	t = gtk_table_new (5, 2, FALSE);
	gtk_widget_show (t);
	gtk_container_set_border_width (GTK_CONTAINER (t), 4);
	gtk_table_set_row_spacings (GTK_TABLE (t), 4);
	gtk_table_set_col_spacings (GTK_TABLE (t), 4);
	gtk_notebook_append_page (GTK_NOTEBOOK (nb), t, l);

	spw_checkbutton(dialog, t, _("Show guides"), "showguides", 0, row, 1, cb);
	spw_checkbutton(dialog, t, _("Snap to guides"), "snaptoguides", 1, row++, 0, cb);

	us = sp_unit_selector_new (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
	spw_dropdown(dialog, t, _("Snap units:"), "guide_snap_units", row++, us);

	spw_unit_selector(dialog, t, _("Snap distance:"), "guidetolerance", row++, us,
			  G_CALLBACK (sp_dtw_guides_snap_distance_changed) );

	sp_color_picker_button(dialog, t, _("Guides color:"), "guidecolor",
			       _("Guideline color"), "guideopacity", row++);

	sp_color_picker_button(dialog, t, _("Highlight color:"), "guidehicolor",
			       _("Highlighted guideline color"), "guidehiopacity", row++);

	row=0;
	/* Page page */
	l = gtk_label_new (_("Page"));
	gtk_widget_show (l);
	t = gtk_table_new (2, 1, FALSE);
	gtk_widget_show (t);
	gtk_container_set_border_width (GTK_CONTAINER (t), 4);
	gtk_table_set_row_spacings (GTK_TABLE (t), 4);
	gtk_table_set_col_spacings (GTK_TABLE (t), 4);
	gtk_notebook_append_page (GTK_NOTEBOOK (nb), t, l);

	spw_checkbutton(dialog, t, _("Show border"), "showborder", 0, row, 0, cb);

	b = gtk_check_button_new_with_label (_("Border on top of drawing"));
	gtk_widget_show (b);
	gtk_table_attach (GTK_TABLE (t), b, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_object_set_data (GTK_OBJECT (dialog), "borderlayer", b);
	g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (sp_dtw_border_layer_toggled), dialog);

	/* fixme: We should listen namedview changes here as well */
	g_signal_connect (G_OBJECT (SODIPODI), "activate_desktop", G_CALLBACK (sp_dtw_activate_desktop), dialog);
	g_signal_connect (G_OBJECT (SODIPODI), "desactivate_desktop", G_CALLBACK (sp_dtw_desactivate_desktop), dialog);
	sp_dtw_update (dialog, SP_ACTIVE_DESKTOP);

	return dialog;
}

static void
sp_dtw_activate_desktop (Sodipodi *sodipodi, SPDesktop *desktop, GtkWidget *dialog)
{
	sp_dtw_update (dialog, desktop);
}

static void
sp_dtw_desactivate_desktop (Sodipodi *sodipodi, SPDesktop *desktop, GtkWidget *dialog)
{
	sp_dtw_update (dialog, NULL);
}

static void
sp_dtw_update (GtkWidget *dialog, SPDesktop *desktop)
{
	if (!desktop) {
		GObject *cp, *w;
		gtk_widget_set_sensitive (dialog, FALSE);
		cp = g_object_get_data (G_OBJECT (dialog), "gridcolor");
		w = g_object_get_data (cp, "window");
		if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
		cp = g_object_get_data (G_OBJECT (dialog), "guidecolor");
		w = g_object_get_data (cp, "window");
		if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
		cp = g_object_get_data (G_OBJECT (dialog), "guidecolor");
		w = g_object_get_data (cp, "window");
		if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
	} else {
		static const SPUnit *pt;
		SPNamedView *nv;
		GtkWidget *cp, *w;
		GtkObject *o;
		gdouble val;

		if (!pt) pt = sp_unit_get_by_abbreviation ("pt");

		nv = desktop->namedview;

		gtk_object_set_data (GTK_OBJECT (dialog), "update", GINT_TO_POINTER (TRUE));
		gtk_widget_set_sensitive (dialog, TRUE);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "showgrid");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->showgrid);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "snaptogrid");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->snaptogrid);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "grid_units");
		sp_unit_selector_set_unit (SP_UNIT_SELECTOR (o), nv->gridunit);

		val = nv->gridoriginx;
		sp_convert_distance (&val, pt, nv->gridunit);
		o = gtk_object_get_data (GTK_OBJECT (dialog), "gridoriginx");
		gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);
		val = nv->gridoriginy;
		sp_convert_distance (&val, pt, nv->gridunit);
		o = gtk_object_get_data (GTK_OBJECT (dialog), "gridoriginy");
		gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);
		val = nv->gridspacingx;
		sp_convert_distance (&val, pt, nv->gridunit);
		o = gtk_object_get_data (GTK_OBJECT (dialog), "gridspacingx");
		gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);
		val = nv->gridspacingy;
		sp_convert_distance (&val, pt, nv->gridunit);
		o = gtk_object_get_data (GTK_OBJECT (dialog), "gridspacingy");
		gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "grid_snap_units");
		sp_unit_selector_set_unit (SP_UNIT_SELECTOR (o), nv->gridtoleranceunit);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "gridtolerance");
		gtk_adjustment_set_value (GTK_ADJUSTMENT (o), nv->gridtolerance);

		cp = gtk_object_get_data (GTK_OBJECT (dialog), "gridcolor");
		sp_color_picker_set_rgba32 (cp, nv->gridcolor);
		w = g_object_get_data (G_OBJECT (cp), "window");
		if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "showguides");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->showgrid);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "snaptoguides");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->snaptogrid);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "guide_snap_units");
		sp_unit_selector_set_unit (SP_UNIT_SELECTOR (o), nv->guidetoleranceunit);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "guidetolerance");
		gtk_adjustment_set_value (GTK_ADJUSTMENT (o), nv->guidetolerance);

		cp = g_object_get_data (G_OBJECT (dialog), "guidecolor");
		sp_color_picker_set_rgba32 (cp, nv->guidecolor);
		w = g_object_get_data (G_OBJECT (cp), "window");
		if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

		cp = g_object_get_data (G_OBJECT (dialog), "guidehicolor");
		sp_color_picker_set_rgba32 (cp, nv->guidehicolor);
		w = g_object_get_data (G_OBJECT (cp), "window");
		if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "showborder");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->showborder);

		o = gtk_object_get_data (GTK_OBJECT (dialog), "borderlayer");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), (nv->borderlayer == SP_BORDER_LAYER_TOP));

		gtk_object_set_data (GTK_OBJECT (dialog), "update", GINT_TO_POINTER (FALSE));
	}
}

void
sp_color_picker_button(GtkWidget * dialog, GtkWidget * t,
		       const guchar * label, guchar * key,
		       guchar * color_dialog_label, guchar * opacity_key,
		       int row)
{
  GtkWidget *l, *cp;
  l = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
  gtk_widget_show (l);
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  cp = sp_color_picker_new (key, opacity_key, color_dialog_label, 0);
  gtk_widget_show (cp);
  gtk_table_attach (GTK_TABLE (t), cp, 1, 2, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  g_object_set_data (G_OBJECT (dialog), key, cp);
}
                                                                                                
static void
sp_color_picker_destroy (GtkObject *cp, gpointer data)
{
	GtkObject *w;

	w = g_object_get_data (G_OBJECT (cp), "window");

	if (w) gtk_object_destroy (w);
}

static GtkWidget *
sp_color_picker_new (unsigned char *colorkey, unsigned char *alphakey, unsigned char *title, guint32 rgba)
{
	GtkWidget *b, *cpv;

	b = gtk_button_new ();

	g_object_set_data (G_OBJECT (b), "title", title);

	cpv = sp_color_preview_new (rgba);
#if 0
	sp_color_preview_set_show_solid  (SP_COLOR_PREVIEW (rgba), FALSE);
#endif
	gtk_widget_show (cpv);
	gtk_container_add (GTK_CONTAINER (b), cpv);
	g_object_set_data (G_OBJECT (b), "preview", cpv);

	g_object_set_data (G_OBJECT (b), "colorkey", colorkey);
	g_object_set_data (G_OBJECT (b), "alphakey", alphakey);

	g_signal_connect (G_OBJECT (b), "destroy", G_CALLBACK (sp_color_picker_destroy), NULL);
	g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_color_picker_clicked), NULL);

	return b;
}

static void
sp_color_picker_set_rgba32 (GtkWidget *cp, guint32 rgba)
{
	SPColorPreview *cpv;
	SPColorSelector *csel;

	cpv = g_object_get_data (G_OBJECT (cp), "preview");
	sp_color_preview_set_rgba32 (cpv, rgba);

	csel = g_object_get_data (G_OBJECT (cp), "selector");
	if (csel) sp_color_selector_set_rgba32 (csel, rgba);

	g_object_set_data (G_OBJECT (cp), "color", GUINT_TO_POINTER (rgba));
}

static void
sp_color_picker_window_destroy (GtkObject *object, GObject *cp)
{
	g_object_set_data (G_OBJECT (cp), "window", NULL);
	g_object_set_data (G_OBJECT (cp), "selector", NULL);
}

static void
sp_color_picker_color_mod (SPColorSelector *csel, GObject *cp)
{
	guint32 rgba;
	SPColorPreview *cpv;
	SPRepr *repr;
	guchar c[32];
	unsigned char *colorkey, *alphakey;

	if (g_object_get_data (G_OBJECT (cp), "update")) return;

	rgba = sp_color_selector_get_rgba32 (csel);

	cpv = g_object_get_data (G_OBJECT (cp), "preview");
	colorkey = g_object_get_data (G_OBJECT (cp), "colorkey");
	alphakey = g_object_get_data (G_OBJECT (cp), "alphakey");
	sp_color_preview_set_rgba32 (cpv, rgba);

	if (!SP_ACTIVE_DESKTOP) return;

	repr = SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview);

	sp_svg_write_color (c, 32, rgba);
	sp_repr_set_attr (repr, colorkey, c);
	sp_repr_set_double (repr, alphakey, (rgba & 0xff) / 255.0);

}

static void
sp_color_picker_window_close (GtkButton * button, GtkWidget * w)
{
  gtk_widget_destroy (w);
}

static void
sp_color_picker_clicked (GObject *cp, void *data)
{
	GtkWidget *w;

	w = g_object_get_data (cp, "window");
	if (!w) {
		GtkWidget *vb, *csel, *hs, *hb, *b;
		w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (w), g_object_get_data (cp, "title"));
		gtk_container_set_border_width (GTK_CONTAINER (w), 4);
		g_object_set_data (cp, "window", w);
		g_signal_connect (G_OBJECT (w), "destroy", G_CALLBACK (sp_color_picker_window_destroy), cp);

		vb = gtk_vbox_new (FALSE, 4);
		gtk_container_add (GTK_CONTAINER (w), vb);

		csel = sp_color_selector_new ();
		gtk_box_pack_start (GTK_BOX (vb), csel, TRUE, TRUE, 0);
		sp_color_selector_set_rgba32 (SP_COLOR_SELECTOR (csel), GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (cp), "color")));
		g_signal_connect (G_OBJECT (csel), "dragged", G_CALLBACK (sp_color_picker_color_mod), cp);
		g_signal_connect (G_OBJECT (csel), "changed", G_CALLBACK (sp_color_picker_color_mod), cp);
		g_object_set_data (cp, "selector", csel);

		hs = gtk_hseparator_new ();
		gtk_box_pack_start (GTK_BOX (vb), hs, FALSE, FALSE, 0);

		hb = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

		b = gtk_button_new_with_label (_("Close"));
		gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK(sp_color_picker_window_close), w);

		gtk_widget_show_all (w);
	} else {
		gtk_window_present (GTK_WINDOW (w));
	}
}

