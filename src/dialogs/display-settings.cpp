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
#include <gtk/gtksignal.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkspinbutton.h>

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

                             
        nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_container_add (GTK_CONTAINER (dlg), nb);

        
        /* Rendering settings */
        /* Notebook tab */
        l = gtk_label_new (_("Rendering"));
        gtk_widget_show (l);
        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        
        /* Oversampling menu */
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

                                      
        /* Input settings */
        /* Notebook tab */
        l = gtk_label_new (_("Input"));
        gtk_widget_show (l);
        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        hb = gtk_hbox_new (FALSE, 4);
        gtk_widget_show (hb);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new (_("Default cursor tolerance:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

        a = gtk_adjustment_new (0.0, 0.0, 10.0, 0.1, 1.0, 1.0);
        gtk_adjustment_set_value (GTK_ADJUSTMENT (a), nr_arena_global_delta);
        sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 1);
        gtk_widget_show (sb);
        gtk_box_pack_start (GTK_BOX (hb), sb, TRUE, TRUE, 0);

        gtk_signal_connect ( GTK_OBJECT (a), "value_changed",
                             GTK_SIGNAL_FUNC 
                                (sp_display_dialog_cursor_tolerance_changed), 
                             NULL);
                             
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
