#define __SP_EXPORT_C__

/*
 * PNG export dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktable.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkprogressbar.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "widgets/spw-utilities.h"
#include "inkscape.h"
#include "dir-util.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-item.h"
#include "selection.h"
#include "file.h"
#include "macros.h"

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"

#include "export.h"

#define SP_EXPORT_MIN_SIZE 1.0

static void sp_export_area_toggled (GtkToggleButton *tb, GtkObject *base);
static void sp_export_export_clicked (GtkButton *button, GtkObject *base);
static void sp_export_browse_clicked (GtkButton *button, gpointer userdata);
static void sp_export_browse_store (GtkButton *button, gpointer userdata);

static void sp_export_area_x_value_changed (GtkAdjustment *adj, GtkObject *base);
static void sp_export_area_y_value_changed (GtkAdjustment *adj, GtkObject *base);
static void sp_export_area_width_value_changed (GtkAdjustment *adj, GtkObject *base);
static void sp_export_area_height_value_changed (GtkAdjustment *adj, GtkObject *base);
static void sp_export_bitmap_width_value_changed (GtkAdjustment *adj, GtkObject *base);
static void sp_export_xdpi_value_changed (GtkAdjustment *adj, GtkObject *base);
static void sp_export_selection_changed (Inkscape::Application *inkscape, SPDesktop *desktop, GtkObject *base);

static void sp_export_set_area (GtkObject *base, float x0, float y0, float x1, float y1);
static void sp_export_value_set (GtkObject *base, const gchar *key, float val);
static void sp_export_value_set_pt (GtkObject *base, const gchar *key, float val);
static float sp_export_value_get (GtkObject *base, const gchar *key);
static float sp_export_value_get_pt (GtkObject *base, const gchar *key);

typedef struct {
  GtkToggleButton *tb;
  GtkObject *base;
  SPSelection* selection;
  guint changedId;
} ActiveSelection;

static ActiveSelection activeSelection={0,0,0,0};

static GtkWidget *dlg = NULL;
static win_data wd;
static gint x = -1000, y = -1000, w = 0, h = 0; // impossible original values to make sure they are read from prefs
static gchar *prefs_path = "dialogs.export";

static void
sp_export_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (INKSCAPE, dlg);
	wd.win = dlg = NULL;
	wd.stop = 0;
}

static gboolean
sp_export_dialog_delete (GtkObject *object, GdkEvent *event, gpointer data)
{
	gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
	gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

	prefs_set_int_attribute (prefs_path, "x", x);
	prefs_set_int_attribute (prefs_path, "y", y);
	prefs_set_int_attribute (prefs_path, "w", w);
	prefs_set_int_attribute (prefs_path, "h", h);

	return FALSE; // which means, go ahead and destroy it
}

static void
sp_export_spinbutton_new (gchar *key, float val, float min, float max, float step, float page, GtkWidget *us,
			  GtkWidget *t, int x, int y, const gchar *ll, const gchar *lr,
			  int digits, unsigned int sensitive,
			  GCallback cb, GtkWidget *dlg)
{
	GtkWidget *l, *sb;
	GtkObject *a;
	int pos;

	a = gtk_adjustment_new (val, min, max, step, page, page);
	gtk_object_set_data (a, "key", key);
	gtk_object_set_data (GTK_OBJECT (dlg), (const gchar *)key, a);
	if (us) sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (us), GTK_ADJUSTMENT (a));

	pos = 0;

	if (ll) {
		l = gtk_label_new ((const gchar *)ll);
		gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
		gtk_table_attach (GTK_TABLE (t), l, x + pos, x + pos + 1, y, y + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
		gtk_widget_set_sensitive (l, sensitive);
		pos += 1;
	}

	sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, digits);
	gtk_table_attach (GTK_TABLE (t), sb, x + pos, x + pos + 1, y, y + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
	gtk_widget_set_usize (sb, 64, -1);
	gtk_widget_set_sensitive (sb, sensitive);
	pos += 1;

	if (lr) {
		l = gtk_label_new ((const gchar *)lr);
		gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
		gtk_table_attach (GTK_TABLE (t), l, x + pos, x + pos + 1, y, y + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
		gtk_widget_set_sensitive (l, sensitive);
		pos += 1;
	}

	if (cb) gtk_signal_connect (a, "value_changed", cb, dlg);
}

void
sp_export_dialog (void)
{
	if (!dlg) {
		GtkWidget *vb, *f, *t, *hb, *us, *l, *fe, *hs, *b;

		gchar title[500];
		sp_ui_dialog_title_string (SP_VERB_FILE_EXPORT, title);

		dlg = sp_window_new (title, TRUE);
		if (x == -1000 || y == -1000) {
			x = prefs_get_int_attribute (prefs_path, "x", 0);
			y = prefs_get_int_attribute (prefs_path, "y", 0);
		}
		if (w ==0 || h == 0) {
			w = prefs_get_int_attribute (prefs_path, "w", 0);
			h = prefs_get_int_attribute (prefs_path, "h", 0);
		}
		if (x != 0 || y != 0) 
			gtk_window_move ((GtkWindow *) dlg, x, y);
		else
			gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
		if (w && h) gtk_window_resize ((GtkWindow *) dlg, w, h);
		sp_transientize (dlg);
		wd.win = dlg;
		wd.stop = 0;
		g_signal_connect (G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
		gtk_signal_connect (GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
		gtk_signal_connect (GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_export_dialog_destroy), dlg);
		gtk_signal_connect (GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_export_dialog_delete), dlg);
		g_signal_connect (G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_export_dialog_delete), dlg);
		g_signal_connect (G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
		g_signal_connect (G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

		vb = gtk_vbox_new (FALSE, 4);
		gtk_widget_show (vb);
		gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
		gtk_container_add (GTK_CONTAINER (dlg), vb);

		/* Export area frame */
		f = gtk_frame_new (_("Export area"));
		gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);
		t = gtk_table_new (3, 6, FALSE);
		gtk_table_set_row_spacings (GTK_TABLE (t), 4);
		gtk_table_set_col_spacings (GTK_TABLE (t), 4);
		gtk_container_set_border_width (GTK_CONTAINER (t), 4);
		gtk_container_add (GTK_CONTAINER (f), t);

		hb = gtk_hbox_new (FALSE, 0);
		gtk_table_attach (GTK_TABLE (t), hb, 0, 6, 0, 1, (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), (GtkAttachOptions)0, 0, 0);

		b = gtk_toggle_button_new_with_label (_("Page"));
		gtk_object_set_data (GTK_OBJECT (b), "key", (void *)"page");
		gtk_object_set_data (GTK_OBJECT (dlg), "page", b);
		gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_export_area_toggled), dlg);
		b = gtk_toggle_button_new_with_label (_("Drawing"));
		gtk_object_set_data (GTK_OBJECT (b), "key", (void *)"drawing");
		gtk_object_set_data (GTK_OBJECT (dlg), "drawing", b);
		gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_export_area_toggled), dlg);
		b = gtk_toggle_button_new_with_label (_("Selection"));
		gtk_object_set_data (GTK_OBJECT (b), "key", (void *)"selection");
		gtk_object_set_data (GTK_OBJECT (dlg), "selection", b);
		gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_export_area_toggled), dlg);

		g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (sp_export_selection_changed), dlg);
		g_signal_connect (G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_export_selection_changed), dlg);

		us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
		gtk_box_pack_end (GTK_BOX (hb), us, FALSE, FALSE, 0);
		l = gtk_label_new (_("Units:"));
		gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
		gtk_object_set_data (GTK_OBJECT (dlg), "units", us);

		sp_export_spinbutton_new ("x0", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, t, 0, 1, _("x0:"), NULL, 2, 1,
					  G_CALLBACK (sp_export_area_x_value_changed), dlg);

		sp_export_spinbutton_new ("x1", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, t, 2, 1, _("x1:"), NULL, 2, 1,
					  G_CALLBACK (sp_export_area_x_value_changed), dlg);

		sp_export_spinbutton_new ("width", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, t, 4, 1, _("Width:"), NULL, 2, 1,
					  G_CALLBACK (sp_export_area_width_value_changed), dlg);

		sp_export_spinbutton_new ("y0", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, t, 0, 2, _("y0:"), NULL, 2, 1,
					  G_CALLBACK (sp_export_area_y_value_changed), dlg);

		sp_export_spinbutton_new ("y1", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, t, 2, 2, _("y1:"), NULL, 2, 1,
					  G_CALLBACK (sp_export_area_y_value_changed), dlg);

		sp_export_spinbutton_new ("height", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, t, 4, 2, _("Height:"), NULL, 2, 1,
					  G_CALLBACK (sp_export_area_height_value_changed), dlg);

		gtk_widget_show_all (f);

		//for now, make sekection toggled by default
		//fixme: make it remember user choice between invoications
		//sp_export_area_toggled ((GtkToggleButton *) b, (GtkObject *) dlg); 

		/* Bitmap size frame */
		f = gtk_frame_new (_("Bitmap size"));
		gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);
		t = gtk_table_new (2, 5, FALSE);
		gtk_table_set_row_spacings (GTK_TABLE (t), 4);
		gtk_table_set_col_spacings (GTK_TABLE (t), 4);
		gtk_container_set_border_width (GTK_CONTAINER (t), 4);
		gtk_container_add (GTK_CONTAINER (f), t);

		sp_export_spinbutton_new ("bmwidth", 16.0, 1.0, 1000000.0, 1.0, 10.0, NULL, t, 0, 0,
					  _("Width:"), _("pixels"), 0, 1,
					  G_CALLBACK (sp_export_bitmap_width_value_changed), dlg);

		sp_export_spinbutton_new ("xdpi", 72.0, 1.0, 9600.0, 0.1, 1.0, NULL, t, 3, 0,
					  NULL, _("dpi"), 2, 1,
					  G_CALLBACK (sp_export_xdpi_value_changed), dlg);

		sp_export_spinbutton_new ("bmheight", 16.0, 1.0, 1000000.0, 1, 10.0, NULL, t, 0, 1,
					  _("Height:"), _("pixels"), 0, 0,
					  NULL, dlg);

		sp_export_spinbutton_new ("ydpi", 72.0, 1.0, 9600.0, 0.1, 1.0, NULL, t, 3, 1,
					  NULL, _("dpi"), 2, 0,
					  NULL, dlg);

		gtk_widget_show_all (f);

		/* File entry */
		f = gtk_frame_new (_("Filename"));
		gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);
		fe = gtk_entry_new ();

		// set the default filename to be that of the current path + document with .png extension
		if (SP_ACTIVE_DOCUMENT && SP_DOCUMENT_URI (SP_ACTIVE_DOCUMENT)) {
			gchar *name, *dot;
			gchar c[1024];
			int len;
			const gchar *uri = SP_DOCUMENT_URI (SP_ACTIVE_DOCUMENT);
			
			name = g_strdup(uri);
			
			len = strlen (name);
			dot = strrchr (name, '.');
			if (dot && (dot > name)) len = dot - name;
			len = MIN (len, 1019);
			memcpy (c, name, len);
			memcpy (c + len, ".png", 4);
			c[len + 4] = 0;
			gtk_entry_set_text (GTK_ENTRY (fe), c);

			g_free(name);
		}

		hb = gtk_hbox_new (FALSE, 5);
		gtk_container_add (GTK_CONTAINER (f), hb);
		gtk_container_set_border_width (GTK_CONTAINER (hb), 4);

		b = gtk_button_new_with_label (_("Browse..."));
		gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 4);
		g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (sp_export_browse_clicked), NULL);
		
		gtk_box_pack_start (GTK_BOX (hb), fe, TRUE, TRUE, 0);
		gtk_object_set_data (GTK_OBJECT (dlg), "filename", fe);
		gtk_widget_show_all (f);
		// enter in filename field is the same as clicking export:
		g_signal_connect (G_OBJECT (fe), "activate", G_CALLBACK (sp_export_export_clicked), dlg);

		/* Buttons */
		hb = gtk_hbox_new (FALSE, 0);
		gtk_widget_show (hb);
		gtk_box_pack_end (GTK_BOX (vb), hb, FALSE, FALSE, 0);

		b = gtk_button_new_with_label (_("Export"));
		gtk_widget_show (b);
		gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_export_export_clicked), dlg);

		hs = gtk_hseparator_new ();
		gtk_widget_show (hs);
		gtk_box_pack_end (GTK_BOX (vb), hs, FALSE, FALSE, 0);
	}

	gtk_window_present ((GtkWindow *) dlg);

	// if there's a selection, set up to export it by default
	if (!sp_selection_is_empty (SP_DT_SELECTION (SP_ACTIVE_DESKTOP))) {
		GtkWidget *button = sp_search_by_value_recursive (dlg, "key", "selection");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	}
}

static void
sp_export_selection_changed (Inkscape::Application *inkscape, SPDesktop *desktop, GtkObject *base)
{
	GtkWidget *button = sp_search_by_value_recursive ((GtkWidget *) base, "key", "selection");
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
			sp_export_area_toggled (GTK_TOGGLE_BUTTON (button), base);
}

static void
sp_export_area_toggled (GtkToggleButton *tb, GtkObject *base)
{
	if (gtk_toggle_button_get_active (tb)) {
		const gchar *key;
		key = (const gchar *)gtk_object_get_data (GTK_OBJECT (tb), "key");
		if (strcmp (key, "page")) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (base, "page")), FALSE);
		}
		if (strcmp (key, "drawing")) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (base, "drawing")), FALSE);
		}
		if (strcmp (key, "selection")) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (base, "selection")), FALSE);
		}
		gtk_object_set_data (base, "area", (gpointer) key);
		if (SP_ACTIVE_DESKTOP) {
			SPDocument *doc;
			NRRect bbox;
			doc = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
			if (!strcmp (key, "page")) {
				bbox.x0 = 0.0;
				bbox.y0 = 0.0;
				bbox.x1 = sp_document_width (doc);
				bbox.y1 = sp_document_height (doc);
			} else if (!strcmp (key, "drawing")) {
				sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)), &bbox);
			} else {
				sp_selection_bbox (SP_DT_SELECTION (SP_ACTIVE_DESKTOP), &bbox);
			}
			sp_export_set_area (base, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
		}
	}
}

static gint
sp_export_progress_delete (GtkWidget *widget, GdkEvent *event, GObject *base)
{
      g_object_set_data (base, "cancel", (gpointer) 1);
      //g_print ("_progress_delete\n");
      return TRUE;
}

static void
sp_export_progress_cancel (GtkWidget *widget, GObject *base)
{
      g_object_set_data (base, "cancel", (gpointer) 1);
      //g_print ("_progress_cancel\n");
}

static unsigned int
sp_export_progress_callback (float value, void *data)
{
      GtkWidget *prg;
      int evtcount;
      if (g_object_get_data ((GObject *) data, "cancel")) return FALSE;
      prg = (GtkWidget *) g_object_get_data ((GObject *) data, "progress");
      gtk_progress_bar_set_fraction ((GtkProgressBar *) prg, value);
      evtcount = 0;
      while ((evtcount < 16) && gdk_events_pending ()) {
              //g_print ("Iteration %d\n", evtcount);
              gtk_main_iteration_do (FALSE);
              evtcount += 1;
      }
      gtk_main_iteration_do (FALSE);

      //g_print ("Done.\n");

      return TRUE;
}

static void
sp_export_export_clicked (GtkButton *button, GtkObject *base)
{
	GtkWidget *fe;
	const gchar *filename;
	float x0, y0, x1, y1;
	int width, height;

	if (!SP_ACTIVE_DESKTOP) return;

	fe = (GtkWidget *)gtk_object_get_data (base, "filename");
#if 0
	filename = gnome_file_entry_get_full_path (GNOME_FILE_ENTRY (fe), FALSE);
#else
	filename = gtk_entry_get_text (GTK_ENTRY (fe));
#endif

	x0 = sp_export_value_get_pt (base, "x0");
	y0 = sp_export_value_get_pt (base, "y0");
	x1 = sp_export_value_get_pt (base, "x1");
	y1 = sp_export_value_get_pt (base, "y1");
	width = (int) (sp_export_value_get (base, "bmwidth") + 0.5);
	height = (int) (sp_export_value_get (base, "bmheight") + 0.5);

	if (strlen (filename) == 0) {
		sp_ui_error_dialog (_("You have to enter a filename"));
	} else {
		if ((x1 > x0) && (y1 > y0) && (width > 0) && (height > 0)) {
			GtkWidget *dlg, *prg, *btn; /* progressbar-stuff */
			char *fn;
			gchar *text;
			
			dlg = gtk_dialog_new ();
			gtk_window_set_title (GTK_WINDOW (dlg), _("Export in progress"));
			prg = gtk_progress_bar_new ();
			sp_transientize (dlg);
			gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
			g_object_set_data ((GObject *) base, "progress", prg);
			fn = g_path_get_basename (filename);
			text = g_strdup_printf (_("Exporting [%d x %d] %s"), width, height, fn);
			g_free (fn);
			gtk_progress_bar_set_text ((GtkProgressBar *) prg, text);
			g_free (text);
			gtk_progress_bar_set_orientation ((GtkProgressBar *) prg, GTK_PROGRESS_LEFT_TO_RIGHT);
			gtk_box_pack_start ((GtkBox *) ((GtkDialog *) dlg)->vbox, prg, FALSE, FALSE, 4);
			btn = gtk_dialog_add_button (GTK_DIALOG (dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
			g_signal_connect ((GObject *) dlg, "delete_event", (GCallback) sp_export_progress_delete, base);
			g_signal_connect ((GObject *) btn, "clicked", (GCallback) sp_export_progress_cancel, base);
			gtk_window_set_modal ((GtkWindow *) dlg, TRUE);
			gtk_widget_show_all (dlg);
			/* Do export */
			sp_export_png_file (SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP), filename, 
					x0, y0, x1, y1, width, height, 
					0x00000000, 
					sp_export_progress_callback, base);
			gtk_widget_destroy (dlg);
			g_object_set_data (G_OBJECT (base), "cancel", (gpointer) 0);
		} else {
			sp_ui_error_dialog (_("The chosen area to be exported is invalid"));
		}
	}
}

static void
sp_export_browse_clicked (GtkButton *button, gpointer userdata)
{
	GtkWidget *fs, *fe;
	const gchar *filename;
	
	fs = gtk_file_selection_new (_("Select a filename for exporting"));
	fe = (GtkWidget *)g_object_get_data (G_OBJECT (dlg), "filename");

	sp_transientize (fs);

	gtk_window_set_modal(GTK_WINDOW (fs), true);
	
	filename = gtk_entry_get_text (GTK_ENTRY (fe));
	
	if(strlen(filename) == 0) {
		filename = g_build_filename (g_get_home_dir(), "/", NULL);
	}	

	gtk_file_selection_set_filename (GTK_FILE_SELECTION (fs), filename);
	
	g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
				"clicked",
				G_CALLBACK (sp_export_browse_store),
				(gpointer) fs);

	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
				"clicked",
				G_CALLBACK (gtk_widget_destroy),
				(gpointer) fs);
	
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fs)->cancel_button),
				"clicked",
				G_CALLBACK (gtk_widget_destroy),
				(gpointer) fs);

	gtk_widget_show (fs);
}

static void
sp_export_browse_store (GtkButton *button, gpointer userdata)
{
	GtkWidget *fs = (GtkWidget *)userdata, *fe;
	const gchar *file;

	fe = (GtkWidget *)g_object_get_data (G_OBJECT (dlg), "filename");
	
	file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));

	gtk_entry_set_text (GTK_ENTRY (fe), file);
	
	g_object_set_data (G_OBJECT (dlg), "filename", fe);
}

static void
sp_export_area_x_value_changed (GtkAdjustment *adj, GtkObject *base)
{
	float x0, x1, xdpi, width;

	if (gtk_object_get_data (base, "update")) return;
	if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data (base, "units"))) return;
	gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

	x0 = sp_export_value_get_pt (base, "x0");
	x1 = sp_export_value_get_pt (base, "x1");
	xdpi = sp_export_value_get (base, "xdpi");

	width = floor ((x1 - x0) * xdpi / 72.0 + 0.5);

	if (width < SP_EXPORT_MIN_SIZE) {
		const gchar *key;
		width = SP_EXPORT_MIN_SIZE;
		key = (const gchar *)gtk_object_get_data (GTK_OBJECT (adj), "key");
		if (!strcmp (key, "x0")) {
			x1 = x0 + width * 72.0 / xdpi;
			sp_export_value_set_pt (base, "x1", x1);
		} else {
			x0 = x1 - width * 72.0 / xdpi;
			sp_export_value_set_pt (base, "x0", x0);
		}
	}

	sp_export_value_set_pt (base, "width", x1 - x0);
	sp_export_value_set (base, "bmwidth", width);

	gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
}

static void
sp_export_area_y_value_changed (GtkAdjustment *adj, GtkObject *base)
{
	float y0, y1, ydpi, height;

	if (gtk_object_get_data (base, "update")) return;
	if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data (base, "units"))) return;
	gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

	y0 = sp_export_value_get_pt (base, "y0");
	y1 = sp_export_value_get_pt (base, "y1");
	ydpi = sp_export_value_get (base, "ydpi");

	height = floor ((y1 - y0) * ydpi / 72.0 + 0.5);

	if (height < SP_EXPORT_MIN_SIZE) {
		const gchar *key;
		height = SP_EXPORT_MIN_SIZE;
		key = (const gchar *)gtk_object_get_data (GTK_OBJECT (adj), "key");
		if (!strcmp (key, "y0")) {
			y1 = y0 + height * 72.0 / ydpi;
			sp_export_value_set_pt (base, "y1", y1);
		} else {
			y0 = y1 - height * 72.0 / ydpi;
			sp_export_value_set_pt (base, "y0", y0);
		}
	}

	sp_export_value_set_pt (base, "height", y1 - y0);
	sp_export_value_set (base, "bmheight", height);

	gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
}

static void
sp_export_area_width_value_changed (GtkAdjustment *adj, GtkObject *base)
{
	float x0, x1, xdpi, width, bmwidth;

	if (gtk_object_get_data (base, "update")) return;
	if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data (base, "units"))) return;
	gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

	x0 = sp_export_value_get_pt (base, "x0");
	x1 = sp_export_value_get_pt (base, "x1");
	xdpi = sp_export_value_get (base, "xdpi");
	width = sp_export_value_get_pt (base, "width");
	bmwidth = floor (width * xdpi / 72.0 + 0.5);

	if (bmwidth < SP_EXPORT_MIN_SIZE) {
		bmwidth = SP_EXPORT_MIN_SIZE;
		width = bmwidth * 72.0 / xdpi;
		sp_export_value_set_pt (base, "width", width);
	}

	sp_export_value_set_pt (base, "x1", x0 + width);
	sp_export_value_set (base, "bmwidth", bmwidth);

	gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
}

static void
sp_export_area_height_value_changed (GtkAdjustment *adj, GtkObject *base)
{
	float y0, y1, ydpi, height, bmheight;

	if (gtk_object_get_data (base, "update")) return;
	if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data (base, "units"))) return;
	gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

	y0 = sp_export_value_get_pt (base, "y0");
	y1 = sp_export_value_get_pt (base, "y1");
	ydpi = sp_export_value_get (base, "ydpi");
	height = sp_export_value_get_pt (base, "height");
	bmheight = floor (height * ydpi / 72.0 + 0.5);

	if (bmheight < SP_EXPORT_MIN_SIZE) {
		bmheight = SP_EXPORT_MIN_SIZE;
		height = bmheight * 72.0 / ydpi;
		sp_export_value_set_pt (base, "height", height);
	}

	sp_export_value_set_pt (base, "y1", y0 + height);
	sp_export_value_set (base, "bmheight", bmheight);

	gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
}

static void
sp_export_set_image_y (GtkObject *base)
{
	float y0, y1, xdpi;

	y0 = sp_export_value_get_pt (base, "y0");
	y1 = sp_export_value_get_pt (base, "y1");
	xdpi = sp_export_value_get (base, "xdpi");

	sp_export_value_set (base, "ydpi", xdpi);
	sp_export_value_set (base, "bmheight", (y1 - y0) * xdpi / 72.0);
}

static void
sp_export_bitmap_width_value_changed (GtkAdjustment *adj, GtkObject *base)
{
	float x0, x1, bmwidth, xdpi;

	if (gtk_object_get_data (base, "update")) return;
	if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data (base, "units"))) return;
	gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

	x0 = sp_export_value_get_pt (base, "x0");
	x1 = sp_export_value_get_pt (base, "x1");
	bmwidth = sp_export_value_get (base, "bmwidth");

	if (bmwidth < SP_EXPORT_MIN_SIZE) {
		bmwidth = SP_EXPORT_MIN_SIZE;
		sp_export_value_set (base, "bmwidth", bmwidth);
	}

	xdpi = bmwidth * 72.0 / (x1 - x0);
	sp_export_value_set (base, "xdpi", xdpi);

	sp_export_set_image_y (base);

	gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
}

void
sp_export_xdpi_value_changed (GtkAdjustment *adj, GtkObject *base)
{
	float x0, x1, xdpi, bmwidth;

	if (gtk_object_get_data (base, "update")) return;
	if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data (base, "units"))) return;
	gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

	x0 = sp_export_value_get_pt (base, "x0");
	x1 = sp_export_value_get_pt (base, "x1");
	xdpi = sp_export_value_get (base, "xdpi");

	bmwidth = (x1 - x0) * xdpi / 72.0;

	if (bmwidth < SP_EXPORT_MIN_SIZE) {
		bmwidth = SP_EXPORT_MIN_SIZE;
		xdpi = bmwidth * 72.0 / (x1 - x0);
		sp_export_value_set (base, "xdpi", xdpi);
	}

	sp_export_value_set (base, "bmwidth", bmwidth);

	sp_export_set_image_y (base);

	gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
}

static void
sp_export_set_area (GtkObject *base, float x0, float y0, float x1, float y1)
{
	sp_export_value_set_pt (base, "x1", x1);
	sp_export_value_set_pt (base, "y1", y1);
	sp_export_value_set_pt (base, "x0", x0);
	sp_export_value_set_pt (base, "y0", y0);
}

static void
sp_export_value_set (GtkObject *base, const gchar *key, float val)
{
	GtkAdjustment *adj;

	adj = (GtkAdjustment *)gtk_object_get_data (base, key);

	gtk_adjustment_set_value (adj, val);
}

static void
sp_export_value_set_pt (GtkObject *base, const gchar *key, float val)
{
	GtkAdjustment *adj;
	const SPUnit *unit;

	adj = (GtkAdjustment *)gtk_object_get_data (base, key);

	unit = sp_unit_selector_get_unit ((SPUnitSelector *)gtk_object_get_data (base, "units"));

	gtk_adjustment_set_value (adj, sp_points_get_units (val, unit));
}

static float
sp_export_value_get (GtkObject *base, const gchar *key)
{
	GtkAdjustment *adj;

	adj = (GtkAdjustment *)gtk_object_get_data (base, key);

	return adj->value;
}

static float
sp_export_value_get_pt (GtkObject *base, const gchar *key)
{
	GtkAdjustment *adj;
	const SPUnit *unit;

	adj = (GtkAdjustment *)gtk_object_get_data (base, key);

	unit = sp_unit_selector_get_unit ((SPUnitSelector *)gtk_object_get_data (base, "units"));

	return sp_units_get_points (adj->value, unit);
}
