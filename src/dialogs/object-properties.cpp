#define __OBJECT_PROPERTIES_C__

/**
 * \brief  Shape and tool style dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <glib.h>
#include <libart_lgpl/art_affine.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkiconfactory.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "widgets/sp-widget.h"
#include "macros.h"
#include "inkscape.h"
#include "fill-style.h"
#include "stroke-style.h"
#include "dialog-events.h"
#include "verbs.h"
#include "interface.h"
#include "object-properties.h"
#include "inkscape-stock.h"
#include "prefs-utils.h"

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0; 
static gchar *prefs_path = "dialogs.fillstroke";



static void
sp_object_properties_dialog_destroy (GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
    
}



static gboolean
sp_object_properties_dialog_delete ( GtkObject *object, 
                                     GdkEvent *event, 
                                     gpointer data )
{

    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_object_properties_dialog_delete()




static void
sp_object_properties_style_activate ( GtkMenuItem *menuitem, const gchar *key )
{
    GtkWidget *fs, *sp, *sl;

    fs = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "fill");
    sp = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "stroke-paint");
    sl = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "stroke-line");

    if (key) {
        SPRepr *repr;
        repr = inkscape_get_repr (INKSCAPE, key);
        if (repr) {
            sp_widget_construct_repr (SP_WIDGET (fs), repr);
            sp_widget_construct_repr (SP_WIDGET (sp), repr);
            sp_widget_construct_repr (SP_WIDGET (sl), repr);
            gtk_widget_set_sensitive (GTK_WIDGET (fs), TRUE);
            gtk_widget_set_sensitive (GTK_WIDGET (sp), TRUE);
            gtk_widget_set_sensitive (GTK_WIDGET (sl), TRUE);
        } else {
            /* Big trouble */
            gtk_widget_set_sensitive (GTK_WIDGET (fs), FALSE);
            gtk_widget_set_sensitive (GTK_WIDGET (sp), FALSE);
            gtk_widget_set_sensitive (GTK_WIDGET (sl), FALSE);
        }
    } else {
        sp_widget_construct_global (SP_WIDGET (fs), INKSCAPE);
        sp_widget_construct_global (SP_WIDGET (sp), INKSCAPE);
        sp_widget_construct_global (SP_WIDGET (sl), INKSCAPE);
        gtk_widget_set_sensitive (GTK_WIDGET (fs), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (sp), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (sl), TRUE);
    
    } // end of if()

} // end of sp_object_properties_style_activate()



int
sp_object_properties_page( GtkWidget *nb, 
                           GtkWidget *page,
                           char *label, 
                           char *dlg_name, 
                           char *label_image )
{

    GtkWidget *hb, *l, *px;

    hb = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hb);
    l = gtk_label_new (_(label));
    gtk_widget_show (l);
    gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
   // px = gtk_image_new_from_file (label_image);
    px = gtk_image_new_from_stock ( label_image, GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_widget_show (px);
    gtk_box_pack_start (GTK_BOX (hb), px, FALSE, FALSE, 0);
    gtk_widget_show (page);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), page, hb);
    gtk_object_set_data (GTK_OBJECT (dlg), dlg_name, page);
    
    return 0;
    
} // end of sp_object_properties_page()



static void
sp_object_properties_color_set ( Inkscape::Application *inkscape, 
                                 SPColor *color, 
                                 double opacity, 
                                 GObject *dlg )
{

    GtkNotebook *nb;
    int pnum;

    nb = (GtkNotebook *)g_object_get_data (dlg, "notebook");
    pnum = gtk_notebook_get_current_page (nb);

    if (pnum == 0) {
    
        GtkWidget *fs;
        fs = (GtkWidget *)g_object_get_data (dlg, "fill");
        sp_fill_style_widget_system_color_set (fs, color, opacity);
    
    } else if (pnum == 1) {
        
        GtkWidget *ss;
        ss = (GtkWidget *)g_object_get_data (dlg, "stroke-paint");
        sp_stroke_style_paint_system_color_set (ss, color, opacity);
    }
    
} // end of sp_object_properties_color_set()



void
sp_object_properties_dialog (void)
{

    if (!dlg) {
        GtkWidget *vb, *nb, *hb, *l, *page, *hs, *om, *m, *mi;

        gchar title[500];
        sp_ui_dialog_title_string (SP_VERB_DIALOG_FILL_STROKE, title);

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
        
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", 
                           G_CALLBACK (sp_transientize_callback), &wd );
                           
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", 
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", 
                             G_CALLBACK (sp_object_properties_dialog_destroy), 
                             dlg );
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", 
                             G_CALLBACK (sp_object_properties_dialog_delete), 
                             dlg );
                             
        g_signal_connect ( G_OBJECT (INKSCAPE), "shut_down", 
                           G_CALLBACK (sp_object_properties_dialog_delete), 
                           dlg );
                           
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", 
                           G_CALLBACK (sp_dialog_hide), dlg );
                           
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", 
                           G_CALLBACK (sp_dialog_unhide), dlg );

        vb = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vb);
        gtk_container_add (GTK_CONTAINER (dlg), vb); 

        nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_box_pack_start (GTK_BOX (vb), nb, TRUE, TRUE, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "notebook", nb);

        /* Fill page */
        page = sp_fill_style_widget_new ();
        sp_object_properties_page(nb, page, "Fill", "fill",
                    INKSCAPE_STOCK_PROPERTIES_FILL_PAGE);

        /* Stroke paint page */
        page = sp_stroke_style_paint_widget_new ();
        sp_object_properties_page(nb, page, "Stroke paint", "stroke-paint",
                    INKSCAPE_STOCK_PROPERTIES_STROKE_PAINT_PAGE);

        /* Stroke line page */
        page = sp_stroke_style_line_widget_new ();
        sp_object_properties_page(nb, page, "Stroke style", "stroke-line",
                    INKSCAPE_STOCK_PROPERTIES_STROKE_PAGE);

        /* Modify style selector */
        hs = gtk_hseparator_new ();
        gtk_widget_show (hs);
        gtk_box_pack_start (GTK_BOX (vb), hs, FALSE, FALSE, 0);

        hb = gtk_hbox_new (FALSE, 4);
        gtk_widget_show (hb);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new (_("Apply to:"));
        gtk_widget_show (l);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 4);
        om = gtk_option_menu_new ();
        gtk_widget_show (om);
        gtk_box_pack_start (GTK_BOX (hb), om, TRUE, TRUE, 0);

        m = gtk_menu_new ();

        mi = gtk_menu_item_new_with_label (_("Selected objects"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             NULL);
        mi = gtk_menu_item_new_with_label (_("All shape tools"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.shapes" );
        mi = gtk_menu_item_new_with_label (_("Rectangle tool"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.shapes.rect" );
        mi = gtk_menu_item_new_with_label (_("Arc tool"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.shapes.arc" );
        mi = gtk_menu_item_new_with_label (_("Star tool"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.shapes.star" );
        mi = gtk_menu_item_new_with_label (_("Spiral tool"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.shapes.spiral");
        mi = gtk_menu_item_new_with_label (_("Freehand and pen"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.freehand");
        mi = gtk_menu_item_new_with_label (_("Calligraphic line"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.calligraphic");
        mi = gtk_menu_item_new_with_label (_("Text"));
        gtk_menu_append (GTK_MENU (m), mi);
        gtk_signal_connect ( GTK_OBJECT (mi), "activate", 
                             GTK_SIGNAL_FUNC 
                                 (sp_object_properties_style_activate), 
                             (void *)"tools.text");

        gtk_widget_show_all (m);

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);

        g_signal_connect ( G_OBJECT (INKSCAPE), 
                           "color_set", 
                           G_CALLBACK (sp_object_properties_color_set), 
                           dlg );

        gtk_widget_show (dlg);
        
    } else {
        
        gtk_window_present (GTK_WINDOW (dlg));
    }

} // end of sp_object_properties_dialog()



void sp_object_properties_stroke (void)
{
    GtkWidget *nb;

    sp_object_properties_dialog ();

    nb = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "notebook");

    gtk_notebook_set_page (GTK_NOTEBOOK (nb), 1);
}



void sp_object_properties_fill (void)
{
    GtkWidget *nb;

    sp_object_properties_dialog ();

    nb = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "notebook");

    gtk_notebook_set_page (GTK_NOTEBOOK (nb), 0);
    
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
