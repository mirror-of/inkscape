#define __SP_DESKTOP_PROPERTIES_C__

/**
 * \brief  Desktop configuration dialog
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
#include <gtk/gtksignal.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkframe.h>

#include "macros.h"
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "svg/svg.h"
#include "widgets/sp-color-selector.h"
#include "widgets/sp-color-notebook.h"
#include "widgets/sp-color-preview.h"
#include "../inkscape.h"
#include "../document.h"
#include "../desktop.h"
#include "../desktop-handles.h"
#include "../sp-namedview.h"
#include "widgets/spw-utilities.h"
#include "dialog-events.h"

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"

#include "../xml/repr.h"
#include "../xml/repr-private.h"

#include "desktop-properties.h"


static void sp_dtw_activate_desktop    ( Inkscape::Application *inkscape, 
                                         SPDesktop *desktop, GtkWidget *dialog);
                                      
static void sp_dtw_deactivate_desktop  ( Inkscape::Application *inkscape, 
                                         SPDesktop *desktop, 
                                         GtkWidget *dialog );
                                        
static void sp_dtw_update              ( GtkWidget *dialog, 
                                         SPDesktop *desktop );

static GtkWidget *sp_color_picker_new  ( gchar *colorkey, gchar *alphakey, 
                                         gchar *title, guint32 rgba );
                                        
static void sp_color_picker_set_rgba32 ( GtkWidget *cp, guint32 rgba );
static void sp_color_picker_clicked    ( GObject *cp, void *data );

static void sp_color_picker_button     ( GtkWidget * dialog, GtkWidget * t, 
                                         const gchar * label, gchar * key, 
                                         gchar * color_dialog_label, 
                                         gchar * opacity_key, int row );

static GtkWidget *dlg = NULL;
static win_data wd;

/* impossible original values to make sure they are read from prefs */
static gint x = -1000, y = -1000, w = 0, h = 0; 
static gchar *prefs_path = "dialogs.documentoptions";

static void 
docoptions_event_attr_changed (SPRepr * repr, const gchar * name, const gchar * old_value, const gchar * new_value, gpointer data)
{
    if (dlg) sp_dtw_update (dlg, SP_ACTIVE_DESKTOP);
}

static SPReprEventVector docoptions_repr_events = {
	NULL, /* destroy */
	NULL, /* add_child */
	NULL, /* child_added */
	NULL, /* remove_child */
	NULL, /* child_removed */
	NULL, /* change_attr */
	docoptions_event_attr_changed,
	NULL, /* change_list */
	NULL, /* content_changed */
	NULL, /* change_order */
	NULL  /* order_changed */
};

static void
sp_dtw_dialog_destroy (GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    sp_repr_remove_listener_by_data (SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview), dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_dtw_dialog_delete ( GtkObject *object, GdkEvent *event, gpointer data )
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
sp_dtw_whatever_toggled ( GtkToggleButton *tb, GtkWidget *dialog )
{
    SPDesktop *dt;
    SPDocument *doc;
    SPRepr *repr;
    const gchar *key;

    if ( gtk_object_get_data (GTK_OBJECT (dialog), "update") ) {
        return;
    }
    
    dt = SP_ACTIVE_DESKTOP;
    
    if (!dt)
        return;
    
    doc = SP_DT_DOCUMENT (dt);

    repr = SP_OBJECT_REPR (dt->namedview);
    key = (const gchar *)gtk_object_get_data (GTK_OBJECT (tb), "key");

    sp_document_set_undo_sensitive (doc, FALSE);
    sp_repr_set_boolean (repr, key, gtk_toggle_button_get_active (tb));
    sp_document_set_undo_sensitive (doc, TRUE);
}


static void
sp_dtw_border_layer_toggled ( GtkToggleButton *tb, GtkWidget *dialog )
{
    SPDesktop *dt;
    SPDocument *doc;
    SPRepr *repr;

    if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) {
        return;
    }
    
    dt = SP_ACTIVE_DESKTOP;
    
    if (!dt)
        return;
    
    doc = SP_DT_DOCUMENT (dt);

    repr = SP_OBJECT_REPR (dt->namedview);

    sp_document_set_undo_sensitive (doc, FALSE);
    sp_repr_set_attr ( repr, "borderlayer", 
                       gtk_toggle_button_get_active (tb) ? "top" : NULL );
                       
    sp_document_set_undo_sensitive (doc, TRUE);
}


/**
 * \brief  Writes the change into the corresponding attribute of the 
 *         sodipodi:namedview element.
 *
 */
static void
sp_dtw_whatever_changed (GtkAdjustment *adjustment, GtkWidget *dialog)
{
    SPDesktop *dt;
    SPDocument *doc;
    SPRepr *repr;
    SPUnitSelector *us;
    const gchar *key;
    gchar c[32];

    if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

    dt = SP_ACTIVE_DESKTOP;
    if (!dt) return;
    doc = SP_DT_DOCUMENT (dt);

    repr = SP_OBJECT_REPR (dt->namedview);
    key = (const gchar *)gtk_object_get_data (GTK_OBJECT (adjustment), "key");
    us = (SPUnitSelector *)gtk_object_get_data ( GTK_OBJECT (adjustment), 
                                                 "unit_selector" );

    g_snprintf ( c, 32, "%g%s", adjustment->value, 
                 sp_unit_selector_get_unit (us)->abbr );

    sp_document_set_undo_sensitive (doc, FALSE);
    sp_repr_set_attr (repr, key, c);
    sp_document_set_undo_sensitive (doc, TRUE);
}


/**
 *\brief   Writes the change into the corresponding attribute of the document 
 *         root (svg element); moved here from the former document settings 
 *         dialog.
 *
 */
static void
sp_doc_dialog_whatever_changed ( GtkAdjustment *adjustment, GtkWidget *dialog )
{
    SPDesktop *dt;
    SPDocument *doc;
    SPRepr *repr;
    SPUnitSelector *us;
    const gchar *key;
    gchar c[32];

    if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) return;

    dt = SP_ACTIVE_DESKTOP;
    if (!dt) return;
    doc = SP_DT_DOCUMENT (dt);

    repr = sp_document_repr_root (doc);
    key = (const gchar *)gtk_object_get_data (GTK_OBJECT (adjustment), "key");
    us = (SPUnitSelector *)gtk_object_get_data ( GTK_OBJECT (adjustment), 
                                                 "unit_selector" );

    /* SVG does not support meters as a unit, so we must translate meters to 
     * cm when writing
     */
    if (!strcmp(sp_unit_selector_get_unit (us)->abbr, "m")) {
        g_snprintf (c, 32, "%g%s", 100*adjustment->value, "cm");
    } else {
        g_snprintf ( c, 32, "%g%s", adjustment->value, 
                     sp_unit_selector_get_unit (us)->abbr );
    }

    sp_repr_set_attr (repr, key, c);

    sp_document_done (doc);
}

static void
sp_dtw_grid_snap_distance_changed ( GtkAdjustment *adjustment, 
                                    GtkWidget *dialog )
{
    SPRepr *repr;
    SPUnitSelector *us;
    gchar c[32];

    if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) {
        return;
    }
    
    if (!SP_ACTIVE_DESKTOP)
        return;

    repr = SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview);

    us = (SPUnitSelector *)gtk_object_get_data ( GTK_OBJECT (dialog), 
                                                 "grid_snap_units" );

    g_snprintf ( c, 32, "%g%s", adjustment->value, 
                 sp_unit_selector_get_unit (us)->abbr );
    
    sp_repr_set_attr (repr, "gridtolerance", c);
}


static void
sp_dtw_guides_snap_distance_changed ( GtkAdjustment *adjustment, 
                                      GtkWidget *dialog )
{
    SPRepr *repr;
    SPUnitSelector *us;
    gchar c[32];

    if (gtk_object_get_data (GTK_OBJECT (dialog), "update")) {
        return;
    }
    
    if (!SP_ACTIVE_DESKTOP) 
        return;

    repr = SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview);

    us = (SPUnitSelector *)gtk_object_get_data ( GTK_OBJECT (dialog), 
                                                 "guide_snap_units" );

    g_snprintf ( c, 32, "%g%s", adjustment->value, 
                 sp_unit_selector_get_unit (us)->abbr );

    sp_repr_set_attr (repr, "guidetolerance", c);
}



static void
sp_doc_dialog_paper_selected ( GtkWidget *widget, gpointer data )
{
    GtkWidget *ww, *hw;

    if (gtk_object_get_data (GTK_OBJECT (dlg), "update")) {
        return;
    }
    
    ww = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "widthsb");
    hw = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "heightsb");

    gtk_widget_set_sensitive (ww, TRUE);
    gtk_widget_set_sensitive (hw, TRUE);
}



void
sp_desktop_dialog (void)
{
    GtkWidget *nb, *l, *t, *b, *us;
    GCallback cb;
    int row;
    
    if (!dlg)
    {

        gchar title[500];
        sp_ui_dialog_title_string (SP_VERB_DIALOG_NAMEDVIEW, title);

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
        
        if (w && h) 
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", 
                             G_CALLBACK (sp_transientize_callback), &wd );
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", 
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", 
                             G_CALLBACK (sp_dtw_dialog_destroy), dlg );
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", 
                             G_CALLBACK (sp_dtw_dialog_delete), dlg );
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", 
                             G_CALLBACK (sp_dtw_dialog_delete), dlg );
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", 
                             G_CALLBACK (sp_dialog_hide), dlg );
        
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", 
                             G_CALLBACK (sp_dialog_unhide), dlg );
                             

        nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_container_add (GTK_CONTAINER (dlg), nb);

        
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
        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_checkbutton( dlg, t, _("Show grid"), "showgrid", 0, row, 0, cb);
        spw_checkbutton( dlg, t, _("Snap to grid"), "snaptogrid", 
                         1, row++, 0, cb);

        spw_checkbutton( dlg, t, _("Horizontal lines"), "vertgrid", 
                         0, row, 0, cb);
        spw_checkbutton( dlg, t, _("Vertical lines"), "horizgrid", 
                         1, row++, 0, cb);

        /*   Commenting out until Nathan implements the grids -- bryce
         *   spw_checkbutton(dlg, t, _("Iso grid"), "isogrid", 0, row, 0, cb);
         *   spw_checkbutton(dlg, t, _("Hex grid"), "hexgrid", 1, row++, 0, cb);
         */
         
        cb = G_CALLBACK(sp_dtw_whatever_changed);

        us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
        spw_dropdown(dlg, t, _("Grid units:"), "grid_units", row++, us);

        spw_unit_selector( dlg, t, _("Origin X:"), "gridoriginx", 
                           row++, us, cb );
                           
        spw_unit_selector( dlg, t, _("Origin Y:"), "gridoriginy", 
                           row++, us, cb );
                           
        spw_unit_selector( dlg, t, _("Spacing X:"), "gridspacingx", 
                           row++, us, cb );
        
        spw_unit_selector( dlg, t, _("Spacing Y:"), "gridspacingy", 
                           row++, us, cb );

        us = sp_unit_selector_new (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        spw_dropdown( dlg, t, _("Snap units:"), "grid_snap_units", 
                      row++, us );

        spw_unit_selector( dlg, t, _("Snap distance:"), "gridtolerance", row++, us,
                           G_CALLBACK (sp_dtw_grid_snap_distance_changed) );

        sp_color_picker_button( dlg, t, _("Grid color:"), "gridcolor",
                                _("Grid color"), "gridhicolor", row++ );

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

        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        
        spw_checkbutton( dlg, t, _("Show guides"), "showguides", 0, 
                         row, 0, cb );
        spw_checkbutton( dlg, t, _("Snap to guides"), "snaptoguides", 1, 
                         row++, 0, cb);

        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        us = sp_unit_selector_new (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        spw_dropdown( dlg, t, _("Snap units:"), "guide_snap_units", 
                      row++, us);

        spw_unit_selector( dlg, t, _("Snap distance:"), "guidetolerance", 
                           row++, us,
                           G_CALLBACK (sp_dtw_guides_snap_distance_changed) );

        sp_color_picker_button( dlg, t, _("Guides color:"), "guidecolor",
                                _("Guideline color"), "guideopacity", row++ );

        sp_color_picker_button( dlg, t, _("Highlight color:"), "guidehicolor",
                                _("Highlighted guideline color"), 
                                "guidehiopacity", row++ );

        row=0;
        /* Page page */
        l = gtk_label_new (_("Page"));
        gtk_widget_show (l);
        t = gtk_table_new (1, 5, FALSE);
        gtk_widget_show (t);
        gtk_container_set_border_width (GTK_CONTAINER (t), 6);
        gtk_table_set_row_spacings (GTK_TABLE (t), 6);
        gtk_notebook_prepend_page (GTK_NOTEBOOK (nb), t, l);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), 0);

        sp_color_picker_button ( dlg, t, _("Background (also for export):"), 
                                 "pagecolor", _("Background color"), 
                                 "inkscape:pageopacity", 0 );

        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_checkbutton( dlg, t, _("Show canvas border"), 
                         "showborder", 0, 1, 0, cb );

        b = gtk_check_button_new_with_label (_("Border on top of drawing"));
        gtk_widget_show (b);
        gtk_table_attach ( GTK_TABLE (t), b, 0, 2, 2, 3, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );
                           
        gtk_object_set_data (GTK_OBJECT (dlg), "borderlayer", b);
        g_signal_connect ( G_OBJECT (b), "toggled", 
                           G_CALLBACK (sp_dtw_border_layer_toggled), dlg );

        sp_color_picker_button ( dlg, t, _("Border color:"), 
                                 "bordercolor", _("Canvas border color"), 
                                 "borderopacity", 4 );

                                 
        // The following comes from the former "document settings" dialog

        GtkWidget *hb, *vb, *l, *om, *m, *i, *f, *tt, *us, *sb;
        GtkObject *a;

        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_table_attach ( GTK_TABLE (t), vb, 0, 2, 5, 6, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );

        hb = gtk_hbox_new (FALSE, 4);
        gtk_widget_show (hb);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        
        l = gtk_label_new (_("Paper size:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
        om = gtk_option_menu_new ();
        gtk_widget_show (om);
        gtk_box_pack_start (GTK_BOX (hb), om, TRUE, TRUE, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "papers", om);

        m = gtk_menu_new ();
        gtk_widget_show (m);

        i = gtk_menu_item_new_with_label (_("Custom"));
        gtk_widget_show (i);
        g_signal_connect ( G_OBJECT (i), "activate", 
                           G_CALLBACK (sp_doc_dialog_paper_selected), NULL);
        gtk_menu_prepend (GTK_MENU (m), i);
        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
       
        
        /* Custom paper frame */
        f = gtk_frame_new (_("Custom paper"));
        gtk_widget_show (f);
        gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

        tt = gtk_table_new (9, 2, FALSE);
        gtk_widget_show (tt);
        gtk_container_set_border_width (GTK_CONTAINER (tt), 4);
        gtk_table_set_row_spacings (GTK_TABLE (tt), 4);
        gtk_table_set_col_spacings (GTK_TABLE (tt), 4);
        gtk_container_add (GTK_CONTAINER (f), tt);
        
        
        l = gtk_label_new (_("Units:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_table_attach ( GTK_TABLE (tt), l, 0, 1, 0, 1, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );
        us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
        gtk_widget_show (us);
        gtk_table_attach ( GTK_TABLE (tt), us, 1, 2, 0, 1, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "units", us);
        
        
        l = gtk_label_new (_("Width:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_table_attach ( GTK_TABLE (tt), l, 0, 1, 1, 2, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0);
        a = gtk_adjustment_new (0.0, 1e-6, 1e6, 1.0, 10.0, 10.0);
        gtk_object_set_data (GTK_OBJECT (a), "key", (void *)"width");
        gtk_object_set_data (GTK_OBJECT (a), "unit_selector", us);
        gtk_object_set_data (GTK_OBJECT (dlg), "width", a);
        sp_unit_selector_add_adjustment ( SP_UNIT_SELECTOR (us), 
                                          GTK_ADJUSTMENT (a) );
        sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 2);
        gtk_widget_show (sb);
        gtk_table_attach ( GTK_TABLE (tt), sb, 1, 2, 1, 2, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );
        gtk_object_set_data (GTK_OBJECT (dlg), "widthsb", sb);
        g_signal_connect ( G_OBJECT (a), "value_changed", 
                           G_CALLBACK (sp_doc_dialog_whatever_changed), dlg );
                        
                          
        l = gtk_label_new (_("Height:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_table_attach ( GTK_TABLE (tt), l, 0, 1, 2, 3, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );
        a = gtk_adjustment_new (0.0, 1e-6, 1e6, 1.0, 10.0, 10.0);
        gtk_object_set_data (GTK_OBJECT (a), "key", (void *)"height");
        gtk_object_set_data (GTK_OBJECT (a), "unit_selector", us);
        gtk_object_set_data (GTK_OBJECT (dlg), "height", a);
        sp_unit_selector_add_adjustment ( SP_UNIT_SELECTOR (us), 
                                          GTK_ADJUSTMENT (a));
        sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 2);
        gtk_widget_show (sb);
        gtk_table_attach ( GTK_TABLE (tt), sb, 1, 2, 2, 3, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );
        gtk_object_set_data (GTK_OBJECT (dlg), "heightsb", sb);
        g_signal_connect ( G_OBJECT (a), "value_changed", 
                           G_CALLBACK (sp_doc_dialog_whatever_changed), dlg );

        // end of former "document settings" stuff
        
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", 
                           G_CALLBACK (sp_dtw_activate_desktop), dlg);
                           
        g_signal_connect ( G_OBJECT (INKSCAPE), "deactivate_desktop", 
                           G_CALLBACK (sp_dtw_deactivate_desktop), dlg);
        sp_dtw_update (dlg, SP_ACTIVE_DESKTOP);

        sp_repr_add_listener (SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview), &docoptions_repr_events, dlg);
        
    } // end of if (!dlg)
    
    gtk_window_present ((GtkWindow *) dlg);


} // end of sp_desktop_dialog (void)



static void
sp_dtw_activate_desktop ( Inkscape::Application *inkscape, 
                          SPDesktop *desktop, 
                          GtkWidget *dialog )
{
    if (desktop && dlg)
        sp_repr_add_listener (SP_OBJECT_REPR (desktop->namedview), &docoptions_repr_events, dlg);
    sp_dtw_update (dialog, desktop);
}



static void
sp_dtw_deactivate_desktop ( Inkscape::Application *inkscape, 
                            SPDesktop *desktop, 
                            GtkWidget *dialog )
{
    if (desktop && SP_IS_DESKTOP (desktop) && SP_IS_NAMEDVIEW (desktop->namedview) && dlg) {
        // all these checks prevent crash when you close inkscape with the dialog open
        sp_repr_remove_listener_by_data (SP_OBJECT_REPR (desktop->namedview), dlg);
    }
    sp_dtw_update (dialog, NULL);
}

static void
sp_dtw_update (GtkWidget *dialog, SPDesktop *desktop)
{
    if (!desktop) {
        GObject *cp, *w;
        gtk_widget_set_sensitive (dialog, FALSE);
        cp = (GObject *)g_object_get_data (G_OBJECT (dialog), "gridcolor");
        w = (GObject *)g_object_get_data (cp, "window");
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
        cp = (GObject *)g_object_get_data (G_OBJECT (dialog), "guidecolor");
        w = (GObject *)g_object_get_data (cp, "window");
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
        cp = (GObject *)g_object_get_data (G_OBJECT (dialog), "guidecolor");
        w = (GObject *)g_object_get_data (cp, "window");
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

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "showgrid");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->showgrid);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "snaptogrid");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->snaptogrid);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "grid_units");
        sp_unit_selector_set_unit (SP_UNIT_SELECTOR (o), nv->gridunit);

        val = nv->gridorigin[NR::X];
        sp_convert_distance (&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "gridoriginx");
        gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);
        val = nv->gridorigin[NR::Y];
        sp_convert_distance (&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "gridoriginy");
        gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);
        val = nv->gridspacing[NR::X];
        sp_convert_distance (&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "gridspacingx");
        gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);
        val = nv->gridspacing[NR::Y];
        sp_convert_distance (&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "gridspacingy");
        gtk_adjustment_set_value (GTK_ADJUSTMENT (o), val);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "grid_snap_units");
        sp_unit_selector_set_unit (SP_UNIT_SELECTOR (o), nv->gridtoleranceunit);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "gridtolerance");
        gtk_adjustment_set_value (GTK_ADJUSTMENT (o), nv->gridtolerance);

        cp = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dialog), "gridcolor");
        sp_color_picker_set_rgba32 (cp, nv->gridcolor);
        w = (GtkWidget *)g_object_get_data (G_OBJECT (cp), "window");
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "showguides");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->showguides);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "snaptoguides");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->snaptoguides);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "guide_snap_units");
        sp_unit_selector_set_unit (SP_UNIT_SELECTOR (o), nv->guidetoleranceunit);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "guidetolerance");
        gtk_adjustment_set_value (GTK_ADJUSTMENT (o), nv->guidetolerance);

        cp = (GtkWidget *)g_object_get_data (G_OBJECT (dialog), "guidecolor");
        sp_color_picker_set_rgba32 (cp, nv->guidecolor);
        w = (GtkWidget *)g_object_get_data (G_OBJECT (cp), "window");
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

        cp = (GtkWidget *)g_object_get_data (G_OBJECT (dialog), "guidehicolor");
        sp_color_picker_set_rgba32 (cp, nv->guidehicolor);
        w = (GtkWidget *)g_object_get_data (G_OBJECT (cp), "window");
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "showborder");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), nv->showborder);

        o = (GtkObject *)gtk_object_get_data (GTK_OBJECT (dialog), "borderlayer");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (o), (nv->borderlayer == SP_BORDER_LAYER_TOP));

        cp = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dialog), "bordercolor"));
        sp_color_picker_set_rgba32 (cp, nv->bordercolor);
        w = GTK_WIDGET (g_object_get_data (G_OBJECT (cp), "window"));
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

        cp = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dialog), "pagecolor"));
        sp_color_picker_set_rgba32 (cp, nv->pagecolor);
        w = GTK_WIDGET (g_object_get_data (G_OBJECT (cp), "window"));
        if (w) gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);

        // Start of former "document settings" stuff

        gdouble docw, doch;
        GList *l;
        gint pos;
        GtkWidget *ww, *hw, *om;
        SPUnitSelector *us;
        const SPUnit *unit;
        GtkAdjustment *a;

        gtk_object_set_data (GTK_OBJECT (dialog), "update", GINT_TO_POINTER (TRUE));
        gtk_widget_set_sensitive (dialog, TRUE);

        docw = sp_document_width (SP_DT_DOCUMENT (desktop));
        doch = sp_document_height (SP_DT_DOCUMENT (desktop));

        pos = 1;
        l = NULL;

        ww = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dialog), "widthsb");
        hw = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dialog), "heightsb");
        om = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dialog), "papers");

        if (l != NULL) {
            gtk_option_menu_set_history (GTK_OPTION_MENU (om), pos);
            gtk_widget_set_sensitive (ww, FALSE);
            gtk_widget_set_sensitive (hw, FALSE);
        } else {
            gtk_option_menu_set_history (GTK_OPTION_MENU (om), 0);
            gtk_widget_set_sensitive (ww, TRUE);
            gtk_widget_set_sensitive (hw, TRUE);
        }

        if (!pt) pt = sp_unit_get_by_abbreviation ("pt");
        us = (SPUnitSelector *)gtk_object_get_data (GTK_OBJECT (dialog), "units");
        unit = sp_unit_selector_get_unit (us);
        a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (dialog), "width");
        sp_convert_distance (&docw, pt, unit);
        gtk_adjustment_set_value (a, docw);
        a = (GtkAdjustment *)gtk_object_get_data (GTK_OBJECT (dialog), "height");
        sp_convert_distance (&doch, pt, unit);
        gtk_adjustment_set_value (a, doch);

        // end of "document settings" stuff

        gtk_object_set_data (GTK_OBJECT (dialog), "update", GINT_TO_POINTER (FALSE));
    }
}


static void
sp_color_picker_button ( GtkWidget *dialog, GtkWidget *t,
                         const gchar *label, gchar *key,
                         gchar *color_dialog_label, 
                         gchar *opacity_key,
                         int row )
{
    GtkWidget *l, *cp;
    l = gtk_label_new (label);
    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
    gtk_widget_show (l);
    gtk_table_attach ( GTK_TABLE (t), l, 0, 1, row, row+1, 
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                       (GtkAttachOptions)0, 0, 0 );
                       
    cp = sp_color_picker_new (key, opacity_key, color_dialog_label, 0);
    gtk_widget_show (cp);
    gtk_table_attach ( GTK_TABLE (t), cp, 1, 2, row, row+1, 
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                       (GtkAttachOptions)0, 0, 0);
                       
    g_object_set_data (G_OBJECT (dialog), key, cp);
    
} // end of sp_color_picker_button



static void
sp_color_picker_destroy ( GtkObject *cp, gpointer data )
{
    GtkObject *w;

    w = (GtkObject *)g_object_get_data (G_OBJECT (cp), "window");

    if (w)
        gtk_object_destroy (w);
    
} // end of sp_color_picker_destroy



/**
 * \brief  Creates a new color picker for the desktop properties dialog.
 *
 */
static GtkWidget *
sp_color_picker_new ( gchar *colorkey, gchar *alphakey, 
                      gchar *title, guint32 rgba )
{

    GtkWidget *b, *cpv;

    b = gtk_button_new ();

    g_object_set_data (G_OBJECT (b), "title", title);

    cpv = sp_color_preview_new (rgba);

    gtk_widget_show (cpv);
    gtk_container_add ( GTK_CONTAINER (b), cpv );
    g_object_set_data ( G_OBJECT (b), "preview", cpv );

    g_object_set_data ( G_OBJECT (b), "colorkey", colorkey );
    g_object_set_data ( G_OBJECT (b), "alphakey", alphakey );

    g_signal_connect  ( G_OBJECT (b), "destroy", 
                        G_CALLBACK (sp_color_picker_destroy), NULL );
    g_signal_connect  ( G_OBJECT (b), "clicked", 
                        G_CALLBACK (sp_color_picker_clicked), NULL );

    return b;
    
} // end of sp_color_picker_new



static void
sp_color_picker_set_rgba32 ( GtkWidget *cp, guint32 rgba )
{
    SPColorPreview *cpv;
    SPColorSelector *csel;
    SPColor color;

    cpv = (SPColorPreview *)g_object_get_data (G_OBJECT (cp), "preview");
    sp_color_preview_set_rgba32 (cpv, rgba);

    csel = (SPColorSelector *)g_object_get_data (G_OBJECT (cp), "selector");
    
    if (csel)
    {
        sp_color_set_rgb_rgba32 (&color, rgba);
        csel->base->setColorAlpha( color, SP_RGBA32_A_F(rgba) );
    }

    g_object_set_data (G_OBJECT (cp), "color", GUINT_TO_POINTER (rgba));
    
} // end of sp_color_picker_set_rgba32



static void
sp_color_picker_window_destroy ( GtkObject *object, GObject *cp )
{
    GtkWidget *w;

    /* remove window object */
    w = (GtkWidget*) g_object_get_data (G_OBJECT (cp), "window");
    if (w) gtk_widget_destroy(GTK_WIDGET (w));

    g_object_set_data (G_OBJECT (cp), "window", NULL);
    g_object_set_data (G_OBJECT (cp), "selector", NULL);
    
} // end of sp_color_picker_window_destroy



static void
sp_color_picker_color_mod (SPColorSelector *csel, GObject *cp)
{
    guint32 rgba;
    SPColorPreview *cpv;
    SPRepr *repr;
    SPColor color;
    float alpha;
    gchar c[32];
    gchar *colorkey, *alphakey;

    if (g_object_get_data (G_OBJECT (cp), "update"))
        return;

    csel->base->getColorAlpha( color, &alpha );
    rgba = sp_color_get_rgba32_falpha (&color, alpha);

    g_object_set_data (G_OBJECT (cp), "color", GUINT_TO_POINTER (rgba));

    cpv = (SPColorPreview *)g_object_get_data (G_OBJECT (cp), "preview");
    colorkey = (gchar *)g_object_get_data (G_OBJECT (cp), "colorkey");
    alphakey = (gchar *)g_object_get_data (G_OBJECT (cp), "alphakey");
    sp_color_preview_set_rgba32 (cpv, rgba);

    if (!SP_ACTIVE_DESKTOP)
        return;

    repr = SP_OBJECT_REPR (SP_ACTIVE_DESKTOP->namedview);

    sp_svg_write_color (c, 32, rgba);
    sp_repr_set_attr (repr, colorkey, c);
    
    if (alphakey) 
        sp_repr_set_double (repr, alphakey, (rgba & 0xff) / 255.0);

} // end of sp_color_picker_color_mod



static void
sp_color_picker_window_close (GtkButton * button, GtkWidget * w)
{

    gtk_widget_destroy (w);

} // end of sp_color_picker_window_close



static void
sp_color_picker_clicked (GObject *cp, void *data)
{

    GtkWidget *w;
    guint32 rgba;
    SPColor color;

    w = (GtkWidget *)g_object_get_data (cp, "window");
    
    if (!w)
    {
        GtkWidget *vb, *csel, *hs, *hb, *b;
        w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title ( GTK_WINDOW (w), 
                               (gchar *)g_object_get_data (cp, "title"));
        gtk_container_set_border_width (GTK_CONTAINER (w), 4);
        g_object_set_data (cp, "window", w);
        g_signal_connect ( G_OBJECT (w), "destroy", 
                           G_CALLBACK (sp_color_picker_window_destroy), cp);

        vb = gtk_vbox_new (FALSE, 4);
        gtk_container_add (GTK_CONTAINER (w), vb);

        csel = sp_color_selector_new ( SP_TYPE_COLOR_NOTEBOOK, 
                                       SP_COLORSPACE_TYPE_UNKNOWN);
        gtk_box_pack_start (GTK_BOX (vb), csel, TRUE, TRUE, 0);
        rgba = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (cp), "color"));
        sp_color_set_rgb_rgba32 (&color, rgba);
        SP_COLOR_SELECTOR(csel)->base->setColorAlpha( color, 
                                                      SP_RGBA32_A_F(rgba) );
        g_signal_connect ( G_OBJECT (csel), "dragged", 
                           G_CALLBACK (sp_color_picker_color_mod), cp);
        g_signal_connect ( G_OBJECT (csel), "changed", 
                           G_CALLBACK (sp_color_picker_color_mod), cp);
                           
        g_object_set_data ( cp, "selector", csel );

        hs = gtk_hseparator_new ();
        gtk_box_pack_start (GTK_BOX (vb), hs, FALSE, FALSE, 0);

        hb = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        b = gtk_button_new_with_label (_("Close"));
        gtk_box_pack_end ( GTK_BOX (hb), b, FALSE, FALSE, 0 );
        g_signal_connect ( G_OBJECT (b), "clicked", 
                           G_CALLBACK(sp_color_picker_window_close), w );

        gtk_widget_show_all (w);
        
    } else {
    
        gtk_window_present (GTK_WINDOW (w));
    
    }

} // end of sp_color_picker_clicked

