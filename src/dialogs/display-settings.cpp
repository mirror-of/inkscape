#define __SP_DISPLAY_SETTINGS_C__

/*
* Display settings dialog
*
* Author:
*   Lauris Kaplinski <lauris@ximian.com>
*
* Copyright (C) 2001 Ximian, Inc.
*
*/

#include <config.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "../inkscape.h"
#include "../prefs-utils.h"
#include "dialog-events.h"
#include "../macros.h"

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"
#include "../seltrans.h"

#include "display-settings.h"



static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0; 
static gchar *prefs_path = "dialogs.preferences";

extern gint nr_arena_image_x_sample;
extern gint nr_arena_image_y_sample;
extern gdouble nr_arena_global_delta;



static void
sp_display_dialog_destroy (GtkObject *object, gpointer data)
{

    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
    
} // edn of sp_display_dialog_destroy()



static gboolean
sp_display_dialog_delete (GtkObject *object, GdkEvent *event, gpointer data)
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_display_dialog_delete()



static void
sp_display_dialog_set_oversample (GtkMenuItem *item, gpointer data)
{
    gint os;

    os = GPOINTER_TO_INT (data);

    g_return_if_fail (os >= 0);
    g_return_if_fail (os <= 4);

    nr_arena_image_x_sample = os;
    nr_arena_image_y_sample = os;

    inkscape_refresh_display (INKSCAPE);
    
} // end of sp_display_dialog_set_oversample()



static void
sp_display_dialog_cursor_tolerance_changed (GtkAdjustment *adj, gpointer data)
{

    nr_arena_global_delta = adj->value;
    prefs_set_double_attribute ( "options.cursortolerance", "value", 
                                 nr_arena_global_delta );

} // end of sp_display_dialog_cursor_tolerance_changed()










static void
options_selector_show_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const gchar *val;
		val = (const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value");
		prefs_set_string_attribute ("tools.select", "show", val);
	}
}

static void
options_selector_transform_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const gchar *val;
		val = (const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value");
		prefs_set_string_attribute ("tools.select", "transform", val);
	}
}

static void
options_selector_cue_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const gchar *val;
		val = (const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value");
		prefs_set_string_attribute ("tools.select", "cue", val);
	}
}

/**
* Small helper function to make options_selector a little less
* verbose.
*
* \param b Another radio button in the group, or NULL for the first.
* \param fb Box to add the button to.
* \param n Name for the button.
* \param v Key for the button's value.
* \param s Initial state of the button.
* \param h Toggled handler function.
*/
static GtkWidget* sp_select_context_add_radio (
    GtkWidget* b,
    GtkWidget* fb,
    GtkTooltips* tt,
    const gchar* n,
    const gchar* tip,
    const char* v,
    gboolean s,
    void (*h)(GtkToggleButton*)
    )
{
	GtkWidget* r = gtk_radio_button_new_with_label (
            b ? gtk_radio_button_group (GTK_RADIO_BUTTON (b)) : NULL, n
            );
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), r, tip, NULL);
	gtk_widget_show (r);
	gtk_object_set_data (GTK_OBJECT (r), "value", (void*) v);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (r), s);
	gtk_box_pack_start (GTK_BOX (fb), r, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (r), "toggled", GTK_SIGNAL_FUNC (h), NULL);

       return r;
}

static GtkWidget *
options_selector ()
{
    GtkWidget *vb, *f, *fb, *b;

    GtkTooltips *tt = gtk_tooltips_new();

    vb = gtk_vbox_new (FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);

    f = gtk_frame_new (_("When transforming, show:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const *show = prefs_get_string_attribute ("tools.select", "show");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("Objects"), _("Show the actual objects when moving or transforming"), "content",
        (show == NULL) || !strcmp (show, "content"),
        options_selector_show_toggled
        );

    sp_select_context_add_radio(
        b, fb, tt, _("Box outline"), _("Show only a box outline of the objects when moving or transforming"), "outline",
        show && !strcmp (show, "outline"),
        options_selector_show_toggled
        );

    f = gtk_frame_new (_("Store transformation:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const *transform = prefs_get_string_attribute ("tools.select", "transform");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("Optimized"), _("If possible, apply transformation to objects without adding a transform= attribute"), "optimize",
        (transform == NULL) || !strcmp (transform, "optimize"),
        options_selector_transform_toggled
        );

    sp_select_context_add_radio (
        b, fb, tt, _("Preserved"), _("Always store transformation as a transform= attribute on objects"), "keep",
        transform && !strcmp (transform, "keep"),
        options_selector_transform_toggled
        );
        
    f = gtk_frame_new (_("Per-object selection cue:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const *cue = prefs_get_string_attribute ("tools.select", "cue");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("None"), _("No per-object selection indication"), "none",
        cue && !strcmp (cue, "none"),
        options_selector_cue_toggled
        );

    b = sp_select_context_add_radio (
        b, fb, tt, _("Mark"), _("Each selected object has a diamond mark in the top left corner"), "mark",
        (cue == NULL) || !strcmp (cue, "mark"),
        options_selector_cue_toggled
        );

    sp_select_context_add_radio (
        b, fb, tt, _("Box"), _("Each selected object displays its bounding box"), "bbox",
        cue && !strcmp (cue, "bbox"),
        options_selector_cue_toggled
        );        

    return vb;
}




void
sp_display_dialog (void)
{

    GtkWidget *nb, *l, *vb, *hb, *om, *m, *i, *sb;
    GtkObject *a;

    if (!dlg)
    {

        gchar title[500];
        sp_ui_dialog_title_string (SP_VERB_DIALOG_DISPLAY, title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }
        
        if (w ==0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
        }
        
        if (x != 0 || y != 0) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        
        } else {
        
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }
        
        if (w && h) {
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        }
        
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", 
                             G_CALLBACK (sp_transientize_callback), &wd);
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", 
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", 
                             G_CALLBACK (sp_display_dialog_destroy), dlg);
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", 
                             G_CALLBACK (sp_display_dialog_delete), dlg);
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", 
                             G_CALLBACK (sp_display_dialog_delete), dlg);
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", 
                             G_CALLBACK (sp_dialog_hide), dlg);
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", 
                             G_CALLBACK (sp_dialog_unhide), dlg);

	GtkTooltips *tt = gtk_tooltips_new();
                             
        nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_container_add (GTK_CONTAINER (dlg), nb);

// Display        
        l = gtk_label_new (_("Display"));
        gtk_widget_show (l);
        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);
     
        /* Oversample */
        hb = gtk_hbox_new (FALSE, 4);
        gtk_widget_show (hb);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new (_("Oversample bitmaps:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
        om = gtk_option_menu_new ();
        gtk_widget_show (om);
        gtk_box_pack_start (GTK_BOX (hb), om, TRUE, TRUE, 0);

        m = gtk_menu_new ();
        gtk_widget_show (m);

        i = gtk_menu_item_new_with_label (_("None"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate", 
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (0) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("2x2"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate", 
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (1) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("4x4"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate", 
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (2) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("8x8"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate", 
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (3));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("16x16"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate", 
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (4) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);

        gtk_option_menu_set_history ( GTK_OPTION_MENU (om), 
                                      nr_arena_image_x_sample);


// Mouse                                      
        l = gtk_label_new (_("Mouse"));
        gtk_widget_show (l);
        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        hb = gtk_hbox_new (FALSE, 4);
        gtk_widget_show (hb);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new (_("Grab sensitivity:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

        a = gtk_adjustment_new (0.0, 0.0, 10.0, 0.1, 1.0, 1.0);
        gtk_adjustment_set_value (GTK_ADJUSTMENT (a), nr_arena_global_delta);
        sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 1);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, _("How close you need to be to an object to be able to grab it with mouse (in pixels)"), NULL);
        gtk_widget_show (sb);
        gtk_box_pack_start (GTK_BOX (hb), sb, TRUE, TRUE, 0);

        gtk_signal_connect ( GTK_OBJECT (a), "value_changed",
                             GTK_SIGNAL_FUNC 
                                (sp_display_dialog_cursor_tolerance_changed), 
                             NULL);


// Tools
        l = gtk_label_new (_("Tools"));
        gtk_widget_show (l);
        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        GtkWidget *nb_tools = gtk_notebook_new ();
        gtk_widget_show (nb_tools);
        gtk_container_add (GTK_CONTAINER (vb), nb_tools);

        // Selector        
        l = gtk_label_new (_("Selector"));
        gtk_widget_show (l);
        GtkWidget *vb_sel = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb_sel);
        gtk_container_set_border_width (GTK_CONTAINER (vb_sel), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb_tools), vb_sel, l);

        GtkWidget *selector_page = options_selector ();
        gtk_widget_show (selector_page);
        gtk_container_add (GTK_CONTAINER (vb_sel), selector_page);
                             
    } // end of if (!dlg)
    
    gtk_window_present ((GtkWindow *) dlg);

} // end of sp_display_dialog()


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
