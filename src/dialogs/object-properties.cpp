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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "widgets/sp-widget.h"
#include "widgets/icon.h"
#include "macros.h"
#include "inkscape.h"
#include "fill-style.h"
#include "stroke-style.h"
#include "dialog-events.h"
#include "verbs.h"
#include "interface.h"
#include "selection.h"
#include "style.h"
#include "object-properties.h"
#include "inkscape-stock.h"
#include "prefs-utils.h"
#include "../svg/stringstream.h"
#include "desktop-style.h"
#include "document.h"


static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0; 
static gchar *prefs_path = "dialogs.fillstroke";


static void sp_fillstroke_selection_modified ( Inkscape::Application *inkscape, SPDesktop *desktop, guint flags, GtkObject *base );
static void sp_fillstroke_selection_changed ( Inkscape::Application *inkscape, SPDesktop *desktop, GtkObject *base );
static void sp_fillstroke_opacity_changed (GtkAdjustment *a, SPWidget *dlg);


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

}


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

    px = sp_icon_new_scaled (16, label_image);
    gtk_widget_show (px);
    gtk_box_pack_start (GTK_BOX (hb), px, FALSE, FALSE, 0);

    l = gtk_label_new (_(label));
    gtk_widget_show (l);
    gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

    gtk_widget_show (page);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), page, hb);
    gtk_object_set_data (GTK_OBJECT (dlg), dlg_name, page);
    
    return 0;
    
}


void
sp_object_properties_dialog (void)
{
    if (!dlg) {
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_FILL_STROKE), title);

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
        
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd );
                           
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_object_properties_dialog_destroy), dlg );
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_object_properties_dialog_delete), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_object_properties_dialog_delete), dlg );
                           
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vb);
        gtk_container_add (GTK_CONTAINER (dlg), vb); 

        GtkWidget *nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_box_pack_start (GTK_BOX (vb), nb, TRUE, TRUE, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "notebook", nb);

        /* Fill page */
        {
        GtkWidget *page = sp_fill_style_widget_new ();
        sp_object_properties_page(nb, page, _("Fill"), "fill",
                    INKSCAPE_STOCK_PROPERTIES_FILL_PAGE);
        }

        /* Stroke paint page */
        {
        GtkWidget *page = sp_stroke_style_paint_widget_new ();
        sp_object_properties_page(nb, page, _("Stroke paint"), "stroke-paint",
                    INKSCAPE_STOCK_PROPERTIES_STROKE_PAINT_PAGE);
        }

        /* Stroke line page */
        {
        GtkWidget *page = sp_stroke_style_line_widget_new ();
        sp_object_properties_page(nb, page, _("Stroke style"), "stroke-line",
                    INKSCAPE_STOCK_PROPERTIES_STROKE_PAGE);
        }

    /* Opacity */

    GtkWidget *o_vb = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vb), o_vb, FALSE, FALSE, 2);
    gtk_object_set_data (GTK_OBJECT (dlg), "master_opacity", o_vb);

    GtkWidget *l_hb = gtk_hbox_new (FALSE, 4);
    GtkWidget *l = gtk_label_new (_("Master opacity"));
    gtk_misc_set_alignment (GTK_MISC (l), 0.0, 1.0);
    gtk_box_pack_start (GTK_BOX (l_hb), l, FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX (o_vb), l_hb, FALSE, FALSE, 0);
    
    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (o_vb), hb, FALSE, FALSE, 0);

    GtkObject *a = gtk_adjustment_new (1.0, 0.0, 1.0, 0.01, 0.1, 0.0);
    gtk_object_set_data(GTK_OBJECT(dlg), "master_opacity_adjustment", a);

    GtkWidget *s = gtk_hscale_new (GTK_ADJUSTMENT (a));
    gtk_scale_set_draw_value (GTK_SCALE (s), FALSE);
    gtk_box_pack_start (GTK_BOX (hb), s, TRUE, TRUE, 4);

    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 2);
    gtk_box_pack_start (GTK_BOX (hb), sb, FALSE, FALSE, 0);

    gtk_signal_connect ( a, "value_changed", 
                         GTK_SIGNAL_FUNC (sp_fillstroke_opacity_changed),
                         dlg );

    gtk_widget_show_all (o_vb);

       // these callbacks are only for the master opacity update; the tabs above take care of themselves
        g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (sp_fillstroke_selection_changed), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (sp_fillstroke_selection_modified), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_fillstroke_selection_changed), dlg );

        sp_fillstroke_selection_changed ( NULL, NULL, NULL );

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


static void
sp_fillstroke_selection_modified ( Inkscape::Application *inkscape, 
                              SPDesktop *desktop, 
                              guint flags,
                              GtkObject *base )
{
    sp_fillstroke_selection_changed ( NULL, NULL, NULL ); 
}


static void
sp_fillstroke_selection_changed ( Inkscape::Application *inkscape, 
                              SPDesktop *desktop, 
                              GtkObject *base )
{
    if (gtk_object_get_data (GTK_OBJECT (dlg), "blocked"))
        return;
    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (TRUE));

    SPSelection *sel = SP_DT_SELECTION (SP_ACTIVE_DESKTOP);

    GtkWidget *opa = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "master_opacity"));
    GtkAdjustment *a = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(dlg), "master_opacity_adjustment"));

    if ( !sel || sel->isEmpty() ) {
        gtk_widget_set_sensitive (opa, FALSE);
        gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (FALSE));
        return;
    }

    gtk_widget_set_sensitive (opa, TRUE);

    // calculate the average opacity of items
    gdouble opacity = 0;
    guint opacity_items = 0;
    for (GSList const *i = sel->itemList(); i != NULL; i = i->next) {
        SPStyle *style = SP_OBJECT_STYLE (i->data);
        opacity += SP_SCALE24_TO_FLOAT(style->opacity.value);
        opacity_items ++;
    }
    opacity /= opacity_items;

    gtk_adjustment_set_value(a, opacity);

    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (FALSE));
}

static void
sp_fillstroke_opacity_changed (GtkAdjustment *a, SPWidget *dlg)
{
    if (gtk_object_get_data (GTK_OBJECT (dlg), "blocked"))
        return;

    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (TRUE));

    SPCSSAttr *css = sp_repr_css_attr_new ();
    
    Inkscape::SVGOStringStream os;	
    os << CLAMP (a->value, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());

    sp_desktop_set_style (SP_ACTIVE_DESKTOP, css);

    sp_repr_css_attr_unref (css);

    sp_document_maybe_done (SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP), "fillstroke:opacity");

    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (FALSE));
} 


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

