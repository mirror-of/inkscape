#define __SP_EXPORT_C__

/**
 * \brief  PNG export dialog
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

#include <iostream>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "xml/repr.h"
#include "helper/sp-intl.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "helper/window.h"
#include "widgets/spw-utilities.h"
#include "inkscape.h"
#include "inkscape-private.h"
#include "dir-util.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-item.h"
#include "selection.h"
#include "file.h"
#include "macros.h"
#include "desktop.h"
#include "sp-namedview.h"

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"

#include "extension/extension.h"
#include "extension/db.h"

#include "export.h"

#define SP_EXPORT_MIN_SIZE 1.0



static void sp_export_area_toggled   ( GtkToggleButton *tb, GtkObject *base );
static void sp_export_export_clicked ( GtkButton *button, GtkObject *base );
static void sp_export_browse_clicked ( GtkButton *button, gpointer userdata );
static void sp_export_browse_store   ( GtkButton *button, gpointer userdata );


static void sp_export_area_x_value_changed       ( GtkAdjustment *adj, 
                                                   GtkObject *base);
                                             
static void sp_export_area_y_value_changed       ( GtkAdjustment *adj, 
                                                   GtkObject *base);
                                             
static void sp_export_area_width_value_changed   ( GtkAdjustment *adj, 
                                                   GtkObject *base);
                                                 
static void sp_export_area_height_value_changed  ( GtkAdjustment *adj, 
                                                   GtkObject *base);
                                                  
static void sp_export_bitmap_width_value_changed ( GtkAdjustment *adj, 
                                                   GtkObject *base);
                                                   
static void sp_export_xdpi_value_changed         ( GtkAdjustment *adj, 
                                                   GtkObject *base);
                                           
static void sp_export_selection_changed ( Inkscape::Application *inkscape, 
                                          SPDesktop *desktop, 
                                          GtkObject *base);
static void sp_export_selection_modified ( Inkscape::Application *inkscape, 
                                           SPDesktop *desktop, 
                                           guint flags,
                                           GtkObject *base );

static void sp_export_set_area      ( GtkObject *base, float x0, float y0, 
                                      float x1, float y1 );
static void sp_export_value_set     ( GtkObject *base, const gchar *key, 
                                      float val );
static void sp_export_value_set_pt  ( GtkObject *base, const gchar *key, 
                                      float val );
static float sp_export_value_get    ( GtkObject *base, const gchar *key );
static float sp_export_value_get_pt ( GtkObject *base, const gchar *key );

static void sp_export_filename_modified (GtkObject * object, gpointer data);
static inline void sp_export_find_default_selection(GtkWidget * dlg);
static void sp_export_detect_size(GtkObject * base);

static GtkWidget *dlg = NULL;
static win_data wd;
static gint x = -1000, y = -1000, w = 0, h = 0; // impossible original values to make sure they are read from prefs
static const gchar *prefs_path = "dialogs.export";
static gchar * original_name = NULL;

/** What type of button is being pressed */
enum selection_type {
    SELECTION_PAGE = 0,  /**< Export the whole page */
    SELECTION_DRAWING,   /**< Export everything drawn on the page */
    SELECTION_SELECTION, /**< Export everything that is selected */
    SELECTION_CUSTOM,    /**< Allows the user to set the region exported */
    SELECTION_NUMBER_OF  /**< A counter for the number of these guys */
};

/** A list of strings that is used both in the preferences, and in the
    data fields to describe the various values of \c selection_type. */
static const char * selection_names[SELECTION_NUMBER_OF] = {
    "page", "drawing", "selection", "custom"};

/** The names on the buttons for the various selection types. */
static const char * selection_labels[SELECTION_NUMBER_OF] = {
    N_("Page"), N_("Drawing"), N_("Selection"), N_("Custom")};

static void
sp_export_dialog_destroy ( GtkObject *object, gpointer data )
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
    g_free(original_name);

    return;
} // end of sp_export_dialog_destroy()


static gboolean
sp_export_dialog_delete ( GtkObject *object, GdkEvent *event, gpointer data )
{

    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_export_dialog_delete()

/**
    \brief  Creates a new spin button for the export dialog
    \param  key  The name of the spin button
    \param  val  A default value for the spin button
    \param  min  Minimum value for the spin button
    \param  max  Maximum value for the spin button
    \param  step The step size for the spin button
    \param  page Size of the page increment
    \param  us   Unit selector that effects this spin button
    \param  t    Table to put the spin button in
    \param  x    X location in the table \c t to start with
    \param  y    Y location in the table \c t to start with
    \param  ll   Text to put on the left side of the spin button (optional)
    \param  lr   Text to put on the right side of the spin button (optional)
    \param  digits  Number of digits to display after the decimal
    \param  sensitive  Whether the spin button is sensitive or not
    \param  cb   Callback for when this spin button is changed (optional)
    \param  dlg  Export dialog the spin button is being placed in
 
*/
static void
sp_export_spinbutton_new ( gchar *key, float val, float min, float max, 
                           float step, float page, GtkWidget *us,
                           GtkWidget *t, int x, int y, 
                           const gchar *ll, const gchar *lr,
                           int digits, unsigned int sensitive,
                           GCallback cb, GtkWidget *dlg )
{

    GtkWidget *l, *sb;
    GtkObject *a;
    int pos;

    a = gtk_adjustment_new (val, min, max, step, page, page);
    gtk_object_set_data (a, "key", key);
    gtk_object_set_data (GTK_OBJECT (dlg), (const gchar *)key, a);
    
    if (us) {
        sp_unit_selector_add_adjustment ( SP_UNIT_SELECTOR (us), 
                                          GTK_ADJUSTMENT (a) );
    }
    
    pos = 0;

    if (ll) {
    
        l = gtk_label_new ((const gchar *)ll);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_table_attach ( GTK_TABLE (t), l, x + pos, x + pos + 1, y, y + 1, 
                           (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );
        gtk_widget_set_sensitive (l, sensitive);
        pos += 1;
    
    }

    sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, digits);
    gtk_table_attach ( GTK_TABLE (t), sb, x + pos, x + pos + 1, y, y + 1, 
                       (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );
    gtk_widget_set_usize (sb, 64, -1);
    gtk_widget_set_sensitive (sb, sensitive);
    pos += 1;

    if (lr) {
    
        l = gtk_label_new ((const gchar *)lr);
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        gtk_table_attach ( GTK_TABLE (t), l, x + pos, x + pos + 1, y, y + 1, 
                           (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );
        gtk_widget_set_sensitive (l, sensitive);
        pos += 1;
    
    }

    if (cb) 
        gtk_signal_connect (a, "value_changed", cb, dlg);

    return;
} // end of sp_export_spinbutton_new()


GtkWidget *
sp_export_dialog_area_frame (GtkWidget * dlg)
{
    GtkWidget * f, * t, * hb, * b, * us, * l, * vb, * unitbox;

    f = gtk_frame_new (_("Export area"));
    vb = gtk_vbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (f), vb);

    /* Units box */
    unitbox = gtk_hbox_new (FALSE, 0);
    /* gets added to the vbox later, but the unit selector is needed
       earlier than that */

    us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
    gtk_box_pack_end (GTK_BOX (unitbox), us, FALSE, FALSE, 0);
    l = gtk_label_new (_("Units:"));
    gtk_box_pack_end (GTK_BOX (unitbox), l, FALSE, FALSE, 3);
    gtk_object_set_data (GTK_OBJECT (dlg), "units", us);

    hb = gtk_hbox_new (TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 3);

    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        b = gtk_toggle_button_new_with_label (_(selection_labels[i]));
        gtk_object_set_data (GTK_OBJECT (b), "key", (void *)i);
        gtk_object_set_data (GTK_OBJECT (dlg), selection_names[i], b);
        gtk_box_pack_start (GTK_BOX (hb), b, FALSE, TRUE, 0);
        gtk_signal_connect ( GTK_OBJECT (b), "clicked", 
                             GTK_SIGNAL_FUNC (sp_export_area_toggled), dlg );
    }

    g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", 
                       G_CALLBACK (sp_export_selection_changed), dlg );
    g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection", 
                       G_CALLBACK (sp_export_selection_modified), dlg );
    g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", 
                       G_CALLBACK (sp_export_selection_changed), dlg );
    
    t = gtk_table_new (2, 6, FALSE);
    gtk_box_pack_start(GTK_BOX(vb), t, FALSE, FALSE, 0);
    gtk_table_set_row_spacings (GTK_TABLE (t), 4);
    gtk_table_set_col_spacings (GTK_TABLE (t), 4);
    gtk_container_set_border_width (GTK_CONTAINER (t), 4);

    sp_export_spinbutton_new ( "x0", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, 
                               t, 0, 0, _("x0:"), NULL, 2, 1,
                               G_CALLBACK ( sp_export_area_x_value_changed), 
                               dlg );

    sp_export_spinbutton_new ( "x1", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, 
                               t, 2, 0, _("x1:"), NULL, 2, 1,
                               G_CALLBACK (sp_export_area_x_value_changed), 
                               dlg );

    sp_export_spinbutton_new ( "width", 0.0, -10000.0, 10000.0, 0.1, 1.0, 
                               us, t, 4, 0, _("Width:"), NULL, 2, 1,
                               G_CALLBACK 
                                   (sp_export_area_width_value_changed), 
                               dlg );

    sp_export_spinbutton_new ( "y0", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, 
                               t, 0, 1, _("y0:"), NULL, 2, 1,
                               G_CALLBACK (sp_export_area_y_value_changed), 
                               dlg );

    sp_export_spinbutton_new ( "y1", 0.0, -10000.0, 10000.0, 0.1, 1.0, us, 
                               t, 2, 1, _("y1:"), NULL, 2, 1,
                               G_CALLBACK (sp_export_area_y_value_changed), 
                               dlg );

    sp_export_spinbutton_new ( "height", 0.0, -10000.0, 10000.0, 0.1, 1.0, 
                               us, t, 4, 1, _("Height:"), NULL, 2, 1,
                               G_CALLBACK 
                                   (sp_export_area_height_value_changed), 
                               dlg );

    /* Adding in the unit box */
    gtk_box_pack_start(GTK_BOX(vb), unitbox, FALSE, FALSE, 0);

    gtk_widget_show_all (f);

    return f;
} // end of sp_export_dialog_area_frame


void
sp_export_dialog (void)
{
    if (!dlg) {
        GtkWidget *vb, *f, *t, *hb, *fe, *hs, *b;

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
                             G_CALLBACK (sp_transientize_callback), &wd);
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", 
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", 
                             G_CALLBACK (sp_export_dialog_destroy), dlg);
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", 
                             G_CALLBACK (sp_export_dialog_delete), dlg);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", 
                             G_CALLBACK (sp_export_dialog_delete), dlg);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", 
                             G_CALLBACK (sp_dialog_hide), dlg);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", 
                             G_CALLBACK (sp_dialog_unhide), dlg);

                             
        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
        gtk_container_add (GTK_CONTAINER (dlg), vb);

        /* Export area frame */
        f = sp_export_dialog_area_frame(dlg);
        gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

        /* Bitmap size frame */
        f = gtk_frame_new (_("Bitmap size"));
        gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);
        t = gtk_table_new (2, 5, FALSE);
        gtk_table_set_row_spacings (GTK_TABLE (t), 4);
        gtk_table_set_col_spacings (GTK_TABLE (t), 4);
        gtk_container_set_border_width (GTK_CONTAINER (t), 4);
        gtk_container_add (GTK_CONTAINER (f), t);

        sp_export_spinbutton_new ( "bmwidth", 16.0, 1.0, 1000000.0, 1.0, 10.0, 
                                   NULL, t, 0, 0,
                                   _("Width:"), _("pixels"), 0, 1,
                                   G_CALLBACK 
                                       (sp_export_bitmap_width_value_changed), 
                                   dlg );

        sp_export_spinbutton_new ( "xdpi", 
                                   prefs_get_double_attribute 
                                       ( "dialogs.export.defaultxdpi", 
                                         "value", 72.0), 
                                   1.0, 9600.0, 0.1, 1.0, NULL, t, 3, 0,
                                   NULL, _("dpi"), 2, 1,
                                   G_CALLBACK (sp_export_xdpi_value_changed), 
                                   dlg );

        sp_export_spinbutton_new ( "bmheight", 16.0, 1.0, 1000000.0, 1, 10.0, 
                                   NULL, t, 0, 1, _("Height:"), _("pixels"), 
                                   0, 0, NULL, dlg );

        /*
         * TODO: Nees fixing: there's no way to set ydpi currently, so we use  
         *       the defaultxdpi value here, too...
         */
        sp_export_spinbutton_new ( "ydpi", prefs_get_double_attribute 
                                               ( "dialogs.export.defaultxdpi", 
                                                 "value", 72.0), 
                                   1.0, 9600.0, 0.1, 1.0, NULL, t, 3, 1,
                                   NULL, _("dpi"), 2, 0, NULL, dlg );

        gtk_widget_show_all (f);

        
        /* File entry */
        f = gtk_frame_new (_("Filename"));
        gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);
        fe = gtk_entry_new ();

        /*
         * set the default filename to be that of the current path + document 
         * with .png extension
         *
         * One thing to notice here is that this filename may get
         * overwritten, but it won't happen here.  The filename gets
         * written into the text field, but then the button to select
         * the area gets set.  In that code the filename can be changed
         * if there are some with presidence in the document.  So, while
         * this code sets the name first, it may not be the one users
         * really see.
         */
        if (SP_ACTIVE_DOCUMENT && SP_DOCUMENT_URI (SP_ACTIVE_DOCUMENT))
        {
            gchar *name;
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            const gchar *uri = SP_DOCUMENT_URI (doc);
            SPRepr * repr = sp_document_repr_root(doc);
            const gchar * text_extension = sp_repr_attr(repr, "inkscape:output_extension");
            Inkscape::Extension::Output * oextension = NULL;

            if (text_extension != NULL) {
                oextension = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(text_extension));
            }

            if (oextension != NULL) {
                gchar * old_extension = oextension->get_extension();
                if (g_str_has_suffix(uri, old_extension)) {
                    gchar * uri_copy;
                    gchar * extension_point;
                    gchar * final_name;

                    uri_copy = g_strdup(uri);
                    extension_point = g_strrstr(uri_copy, old_extension);
                    extension_point[0] = '\0';

                    final_name = g_strconcat(uri_copy, ".png", NULL);
                    gtk_entry_set_text (GTK_ENTRY (fe), final_name);

                    g_free(final_name);
                    g_free(uri_copy);
                }
            } else {
                name = g_strconcat(uri, ".png", NULL);
                gtk_entry_set_text (GTK_ENTRY (fe), name);
                g_free(name);
            } 
        }
        g_signal_connect ( G_OBJECT (fe), "changed", 
                           G_CALLBACK (sp_export_filename_modified), dlg);

        
        hb = gtk_hbox_new (FALSE, 5);
        gtk_container_add (GTK_CONTAINER (f), hb);
        gtk_container_set_border_width (GTK_CONTAINER (hb), 4);

        b = gtk_button_new_with_label (_("Browse..."));
        gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 4);
        g_signal_connect ( G_OBJECT (b), "clicked", 
                           G_CALLBACK (sp_export_browse_clicked), NULL );

        gtk_box_pack_start (GTK_BOX (hb), fe, TRUE, TRUE, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "filename", fe);
        gtk_object_set_data (GTK_OBJECT (dlg), "filename-modified", (gpointer)FALSE);
        original_name = g_strdup(gtk_entry_get_text (GTK_ENTRY (fe)));
        gtk_widget_show_all (f);
        // pressing enter in the filename field is the same as clicking export:
        g_signal_connect ( G_OBJECT (fe), "activate", 
                           G_CALLBACK (sp_export_export_clicked), dlg );

        /* Buttons */
        hb = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hb);
        gtk_box_pack_end (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        b = gtk_button_new_with_label (_("Export"));
        gtk_widget_show (b);
        gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
        gtk_signal_connect ( GTK_OBJECT (b), "clicked", 
                             GTK_SIGNAL_FUNC (sp_export_export_clicked), dlg );

        hs = gtk_hseparator_new ();
        gtk_widget_show (hs);
        gtk_box_pack_end (GTK_BOX (vb), hs, FALSE, FALSE, 0);
    
    } // end of if (!dlg)
    
    sp_export_find_default_selection(dlg);

    gtk_window_present ((GtkWindow *) dlg);
    
    return;
} // end of sp_export_dialog()

static inline void
sp_export_find_default_selection(GtkWidget * dlg)
{
    selection_type key = SELECTION_NUMBER_OF;

    if ((SP_DT_SELECTION(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
        key = SELECTION_SELECTION;
    }

    /* Try using the preferences */
    if (key == SELECTION_NUMBER_OF) {
        const gchar *what = NULL;
        int i = SELECTION_NUMBER_OF;

        what = prefs_get_string_attribute ("dialogs.export.exportarea", "value");
        
        if (what != NULL) {
            for (i = 0; i < SELECTION_NUMBER_OF; i++) {
                if (!strcmp (what, selection_names[i])) {
                    break;
                }
            }
        }

        key = (selection_type)i;
    }

    if (key == SELECTION_NUMBER_OF) {
        key = SELECTION_SELECTION;
    }

    if (key != SELECTION_NUMBER_OF) {
        GtkWidget *button;
    
        button = (GtkWidget *)g_object_get_data(G_OBJECT(dlg), selection_names[key]);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    }

    return;
}


/**
 * \brief  If selection changed or a different document activated, we must 
 * recalculate any chosen areas
 *
 */
static void
sp_export_selection_changed ( Inkscape::Application *inkscape, 
                              SPDesktop *desktop, 
                              GtkObject *base )
{
//  std::cout << "Selection Changed" << std::endl;
    selection_type current_key;
    current_key = (selection_type)((int)gtk_object_get_data(GTK_OBJECT(base), "selection-type"));
    if (inkscape &&
            SP_IS_INKSCAPE (inkscape) &&
            desktop &&
            SELECTION_CUSTOM != current_key) {
        GtkToggleButton * button;
        button = (GtkToggleButton *)gtk_object_get_data(base, selection_names[current_key]);
        sp_export_area_toggled(button, base);
    } // end of if()

    return;
} // end of sp_export_selection_changed()

static void
sp_export_selection_modified ( Inkscape::Application *inkscape, 
                               SPDesktop *desktop,  /* Not sure that this is right */
                               guint flags,
                               GtkObject *base )
{
    selection_type current_key;
    current_key = (selection_type)((int)gtk_object_get_data(GTK_OBJECT(base), "selection-type"));

    switch (current_key) {
        case SELECTION_DRAWING:
            if ( SP_ACTIVE_DESKTOP ) {
                SPDocument *doc;
                NRRect bbox;
                doc = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
                sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)), &bbox);

                if (!(bbox.x0 > bbox.x1 && bbox.y0 > bbox.y1)) { 
                    sp_export_set_area (base, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
                }
            }
            break;
        case SELECTION_SELECTION:
            if ((SP_DT_SELECTION(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
                NRRect bbox;
                (SP_DT_SELECTION (SP_ACTIVE_DESKTOP))->bounds(&bbox);
                sp_export_set_area (base, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
            }
            break;
        default:
            /* Do nothing for page or for custom */
            break;
    }

    return;
}

static void
sp_export_area_toggled (GtkToggleButton *tb, GtkObject *base)
{
    selection_type key;
    key = (selection_type)((int)gtk_object_get_data (GTK_OBJECT (tb), "key"));

    /* We're going to have to toggle the other buttons, but we
       only care about the one that is depressed */
    if (!gtk_toggle_button_get_active (tb) ) {
        selection_type current_key;
        current_key = (selection_type)((int)gtk_object_get_data(GTK_OBJECT(base), "selection-type"));

        /* Don't let the current selection be deactived - but rerun the
           activate to allow the user to renew the values */
        if (key == current_key) {
            gtk_toggle_button_set_active 
                ( GTK_TOGGLE_BUTTON ( gtk_object_get_data (base, selection_names[key])), 
                  TRUE );
        }

        return;
    }
        
    gtk_object_set_data(GTK_OBJECT(base), "selection-type", (gpointer)key);

    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        if (i != key) {
            gtk_toggle_button_set_active 
                ( GTK_TOGGLE_BUTTON ( gtk_object_get_data (base, selection_names[i])), 
                  FALSE );
            /* printf("Pushing up: %s\n", selection_names[i]); */
        }
    }

    if (SP_ACTIVE_DESKTOP && !gtk_object_get_data(GTK_OBJECT(base), "filename-modified")) {
        GtkWidget * file_entry;
        const gchar * filename = NULL;
        float xdpi = 0.0, ydpi = 0.0;

        file_entry = (GtkWidget *)gtk_object_get_data (base, "filename");

        switch (key) {
            case SELECTION_PAGE:
            case SELECTION_DRAWING: {
                SPDocument * doc = SP_ACTIVE_DOCUMENT;
                SPRepr * repr = sp_document_repr_root(doc);
                const gchar * dpi_string;

                filename = sp_repr_attr(repr, "inkscape:export-filename");

                dpi_string = NULL;
                dpi_string = sp_repr_attr(repr, "inkscape:export-xdpi");
                if (dpi_string != NULL) {
                    xdpi = atof(dpi_string);
                }

                dpi_string = NULL;
                dpi_string = sp_repr_attr(repr, "inkscape:export-ydpi");
                if (dpi_string != NULL) {
                    ydpi = atof(dpi_string);
                }

                break;
            }
            case SELECTION_SELECTION:
                if ((SP_DT_SELECTION(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
                    const GSList * reprlst;
                    bool filename_search = TRUE;
                    bool xdpi_search = TRUE;
                    bool ydpi_search = TRUE;

                    reprlst = SP_DT_SELECTION(SP_ACTIVE_DESKTOP)->reprList();
                    for(; reprlst != NULL &&
                            filename_search &&
                            xdpi_search &&
                            ydpi_search;
                            reprlst = reprlst->next) {
                        const gchar * dpi_string;
                        SPRepr * repr = (SPRepr *)reprlst->data;

                        if (filename_search) {
                            filename = sp_repr_attr(repr, "inkscape:export-filename");
                            if (filename != NULL)
                                filename_search = FALSE;
                        }

                        if (xdpi_search) {
                            dpi_string = NULL;
                            dpi_string = sp_repr_attr(repr, "inkscape:export-xdpi");
                            if (dpi_string != NULL) {
                                xdpi = atof(dpi_string);
                                xdpi_search = FALSE;
                            }
                        }

                        if (ydpi_search) {
                            dpi_string = NULL;
                            dpi_string = sp_repr_attr(repr, "inkscape:export-ydpi");
                            if (dpi_string != NULL) {
                                ydpi = atof(dpi_string);
                                ydpi_search = FALSE;
                            }
                        }
                    }
                }
                break;
            case SELECTION_CUSTOM:
            default:
                break;
        }

        if (filename != NULL) {
            g_free(original_name);
            original_name = g_strdup(filename);
            gtk_entry_set_text(GTK_ENTRY(file_entry), filename);
        }

        if (xdpi != 0.0) {
            sp_export_value_set(base, "xdpi", xdpi);
        }

        /* These can't be seperate, and setting x sets y, so for
           now setting this is disabled.  Hopefully it won't be in
           the future */
        if (FALSE && ydpi != 0.0) {
            sp_export_value_set(base, "ydpi", ydpi);
        }
    }

    if (gtk_object_get_data (base, "update"))
        return;

    if ( SP_ACTIVE_DESKTOP )
    {
        SPDocument *doc;
        NRRect bbox;
        doc = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);
        
        /* Notice how the switch is used to 'fall through' here to get
           various backups.  If you modify this without noticing you'll
           probabaly screw something up. */
        switch (key) {
            case SELECTION_SELECTION:
                if ((SP_DT_SELECTION(SP_ACTIVE_DESKTOP))->isEmpty() == false)
                {
                    (SP_DT_SELECTION (SP_ACTIVE_DESKTOP))->bounds(&bbox);
                    /* Only if there is a selection that we can set
                       do we break, otherwise we fall through to the
                       drawing */
                    // std::cout << "Using selection: SELECTION" << std::endl;
                    key = SELECTION_SELECTION;
                    break;
                }
            case SELECTION_DRAWING:
                /* 
                 * TODO: this returns wrong values if the document has a 
                 * viewBox
                 */
                sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)), &bbox);
                
                /* If the drawing is valid, then we'll use it and break
                   otherwise we drop through to the page settings */
                if (!(bbox.x0 > bbox.x1 && bbox.y0 > bbox.y1)) { 
                    // std::cout << "Using selection: DRAWING" << std::endl;
                    key = SELECTION_DRAWING;
                    break;
                }
            case SELECTION_PAGE:
                bbox.x0 = 0.0;
                bbox.y0 = 0.0;
                bbox.x1 = sp_document_width (doc);
                bbox.y1 = sp_document_height (doc);
                // std::cout << "Using selection: PAGE" << std::endl;
                key = SELECTION_PAGE;
                break;
            case SELECTION_CUSTOM:
            default:
                break;
        } // switch
        
        // remember area setting
        prefs_set_string_attribute ( "dialogs.export.exportarea", 
                                     "value", selection_names[key]);

        if (key != SELECTION_CUSTOM)
            sp_export_set_area (base, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
    
    } // end of if ( SP_ACTIVE_DESKTOP )
    
    return;
} // end of sp_export_area_toggled()


static gint
sp_export_progress_delete ( GtkWidget *widget, GdkEvent *event, GObject *base )
{
    g_object_set_data (base, "cancel", (gpointer) 1);
    return TRUE;
} // end of sp_export_progress_delete()


static void
sp_export_progress_cancel ( GtkWidget *widget, GObject *base )
{
    g_object_set_data (base, "cancel", (gpointer) 1);
} // end of sp_export_progress_cancel()


static unsigned int
sp_export_progress_callback (float value, void *data)
{
    GtkWidget *prg;
    int evtcount;
    
    if (g_object_get_data ((GObject *) data, "cancel")) 
        return FALSE;
    
    prg = (GtkWidget *) g_object_get_data ((GObject *) data, "progress");
    gtk_progress_bar_set_fraction ((GtkProgressBar *) prg, value);
    
    evtcount = 0;
    while ((evtcount < 16) && gdk_events_pending ()) {
            gtk_main_iteration_do (FALSE);
            evtcount += 1;
    }
    
    gtk_main_iteration_do (FALSE);

    return TRUE;
    
} // end of sp_export_progress_callback()


static void
sp_export_export_clicked (GtkButton *button, GtkObject *base)
{
    GtkWidget *fe;
    const gchar *filename;
    float x0, y0, x1, y1, xdpi, ydpi;
    int width, height;

    if (!SP_ACTIVE_DESKTOP) return;

    fe = (GtkWidget *)gtk_object_get_data (base, "filename");
    filename = gtk_entry_get_text (GTK_ENTRY (fe));

    x0 = sp_export_value_get_pt (base, "x0");
    y0 = sp_export_value_get_pt (base, "y0");
    x1 = sp_export_value_get_pt (base, "x1");
    y1 = sp_export_value_get_pt (base, "y1");
    xdpi = sp_export_value_get (base, "xdpi");
    ydpi = sp_export_value_get (base, "ydpi");
    width = (int) (sp_export_value_get (base, "bmwidth") + 0.5);
    height = (int) (sp_export_value_get (base, "bmheight") + 0.5);

    if (filename == NULL || strlen (filename) == 0) {
        sp_ui_error_dialog (_("You have to enter a filename"));
        return;
    }
    
    if (!((x1 > x0) && (y1 > y0) && (width > 0) && (height > 0))) {
        sp_ui_error_dialog (_("The chosen area to be exported is invalid"));
        return;
    }

    gchar * dirname = g_dirname(filename);
    if (dirname == NULL || !g_file_test(dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
        gchar * error;
        error = g_strdup_printf(_("Directory %s does not exist or is not a directory.\n"), dirname);
        sp_ui_error_dialog(error);
        g_free(error);
        g_free(dirname);
        return;
    }
    g_free(dirname);

    SPNamedView *nv = (SP_ACTIVE_DESKTOP)->namedview;
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
    text = g_strdup_printf ( _("Exporting [%d x %d] %s"), 
                             width, height, fn);
    g_free (fn);
    gtk_progress_bar_set_text ((GtkProgressBar *) prg, text);
    g_free (text);
    gtk_progress_bar_set_orientation ( (GtkProgressBar *) prg, 
                                       GTK_PROGRESS_LEFT_TO_RIGHT);
    gtk_box_pack_start ((GtkBox *) ((GtkDialog *) dlg)->vbox, 
                        prg, FALSE, FALSE, 4 );
    btn = gtk_dialog_add_button ( GTK_DIALOG (dlg), 
                                  GTK_STOCK_CANCEL, 
                                  GTK_RESPONSE_CANCEL );
                                  
    g_signal_connect ( (GObject *) dlg, "delete_event", 
                       (GCallback) sp_export_progress_delete, base);
    g_signal_connect ( (GObject *) btn, "clicked", 
                       (GCallback) sp_export_progress_cancel, base);
    gtk_window_set_modal ((GtkWindow *) dlg, TRUE);
    gtk_widget_show_all (dlg);
    
    /* Do export */
    sp_export_png_file (SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP), filename, 
            x0, y0, x1, y1, width, height, 
            nv->pagecolor, 
            sp_export_progress_callback, base);
            
    gtk_widget_destroy (dlg);
    g_object_set_data (G_OBJECT (base), "cancel", (gpointer) 0);

    /* Setup the values in the document */
    switch ((selection_type)((int)gtk_object_get_data(GTK_OBJECT(base), "selection-type"))) {
        case SELECTION_PAGE:
        case SELECTION_DRAWING: {
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            SPRepr * repr = sp_document_repr_root(doc);

            sp_document_set_undo_sensitive(doc, FALSE);
            sp_repr_set_attr(repr, "inkscape:export-filename", filename);
            sp_repr_set_double(repr, "inkscape:export-xdpi", xdpi);
            sp_repr_set_double(repr, "inkscape:export-ydpi", ydpi);
            sp_repr_set_attr(repr, "sodipodi:modified", "TRUE");
            sp_document_set_undo_sensitive(doc, TRUE);
            break;
        }
        case SELECTION_SELECTION: {
            const GSList * reprlst;
            SPDocument * doc = SP_ACTIVE_DOCUMENT;

            sp_document_set_undo_sensitive(doc, FALSE);
            reprlst = SP_DT_SELECTION(SP_ACTIVE_DESKTOP)->reprList();

            if (reprlst != NULL) {
                SPRepr * repr = sp_document_repr_root(doc);
                sp_repr_set_attr(repr, "sodipodi:modified", "TRUE");
            }

            for(; reprlst != NULL; reprlst = reprlst->next) {
                SPRepr * repr = (SPRepr *)reprlst->data;

                sp_repr_set_attr(repr, "inkscape:export-filename", filename);
                sp_repr_set_double(repr, "inkscape:export-xdpi", xdpi);
                sp_repr_set_double(repr, "inkscape:export-ydpi", ydpi);
            }

            sp_document_set_undo_sensitive(doc, TRUE);
            break;
        }
        default:
            break;
    }
    
    return;
} // end of sp_export_export_clicked()


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
        filename = g_build_filename (g_get_home_dir(), G_DIR_SEPARATOR_S, NULL);
    }

    gtk_file_selection_set_filename (GTK_FILE_SELECTION (fs), filename);

    g_signal_connect ( GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                       "clicked",
                       G_CALLBACK (sp_export_browse_store),
                       (gpointer) fs );

    g_signal_connect_swapped ( GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                               "clicked",
                               G_CALLBACK (gtk_widget_destroy),
                               (gpointer) fs );

    g_signal_connect_swapped ( GTK_OBJECT 
                                   (GTK_FILE_SELECTION (fs)->cancel_button),
                               "clicked",
                               G_CALLBACK (gtk_widget_destroy),
                               (gpointer) fs );

    gtk_widget_show (fs);
    
    return;
} // end of sp_export_browse_clicked()


static void
sp_export_browse_store (GtkButton *button, gpointer userdata)
{
    GtkWidget *fs = (GtkWidget *)userdata, *fe;
    const gchar *file;

    fe = (GtkWidget *)g_object_get_data (G_OBJECT (dlg), "filename");

    file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));

    gtk_entry_set_text (GTK_ENTRY (fe), file);

    g_object_set_data (G_OBJECT (dlg), "filename", fe);

    return;
} // end of sp_export_browse_store()

/**
    \brief  This function is used to detect the current selection setting
            based on the values in the x0, y0, x1 and y0 fields.
    \param  base  The export dialog itself

    One of the most confusing parts of this function is why the array
    is built at the beginning.  What needs to happen here is that we
    should always check the current selection to see if it is the valid
    one.  While this is a performance improvement it is also a usability
    one during the cases where things like selections and drawings match
    size.  This way buttons change less 'randomly' (atleast in the eyes
    of the user).  To do this an array is built where the current selection
    type is placed first, and then the others in an order from smallest
    to largest (this can be configured by reshuffling \c test_order).

    All of the values in this function are rounded to two decimal places
    because that is what is shown to the user.  While everything is kept
    more accurate than that, the user can't control more acurrate than
    that, so for this to work for them - it needs to check on that level
    of accuracy.

    \todo finish writing this up 
*/
static void
sp_export_detect_size(GtkObject * base) {
    static const selection_type test_order[SELECTION_NUMBER_OF] = {SELECTION_SELECTION, SELECTION_DRAWING, SELECTION_PAGE, SELECTION_CUSTOM};
    selection_type this_test[SELECTION_NUMBER_OF + 1];
    selection_type key = SELECTION_NUMBER_OF;

    NR::Point x(sp_export_value_get_pt (base, "x0"),
                sp_export_value_get_pt (base, "y0"));
    NR::Point y(sp_export_value_get_pt (base, "x1"),
                sp_export_value_get_pt (base, "y1"));
    NR::Rect current_bbox(x, y);
    current_bbox.round(2);
    // std::cout << "Current " << current_bbox;

    this_test[0] = (selection_type)((int)gtk_object_get_data(GTK_OBJECT(base), "selection-type"));
    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        this_test[i + 1] = test_order[i];
    }

    for (int i = 0;
            i < SELECTION_NUMBER_OF + 1 &&
                key == SELECTION_NUMBER_OF &&
                SP_ACTIVE_DESKTOP != NULL;
            i++) {
        // std::cout << "Looking at: " << selection_names[this_test[i]] << std::endl;
        switch (this_test[i]) {
            case SELECTION_SELECTION:
                if ((SP_DT_SELECTION(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
                    NRRect bbox;

                    (SP_DT_SELECTION (SP_ACTIVE_DESKTOP))->bounds(&bbox);
                    NR::Rect bbox2(bbox);

                    bbox2.round(2);
                    // std::cout << "Selection " << bbox2;
                    if (bbox2 == current_bbox) {
                        key = SELECTION_SELECTION;
                    }
                }
                break;
            case SELECTION_DRAWING: {
                SPDocument *doc;
                NRRect bbox;
                doc = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);

                sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)), &bbox);
                NR::Rect bbox2(bbox);

                bbox2.round(2);
                // std::cout << "Drawing " << bbox2;
                if (bbox2 == current_bbox) {
                    key = SELECTION_DRAWING;
                }
                break;
            }

            case SELECTION_PAGE: {
                SPDocument *doc;

                doc = SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);

                NR::Point x(0.0, 0.0);
                NR::Point y(sp_document_width(doc),
                            sp_document_height(doc));
                NR::Rect bbox(x, y);
                bbox.round(2);

                // std::cout << "Page " << bbox;
                if (bbox == current_bbox) {
                    key = SELECTION_PAGE;
                }

                break;
           }
        default:
           break;
        }
    }
    // std::cout << std::endl;

    if (key == SELECTION_NUMBER_OF) {
        key = SELECTION_CUSTOM;
    }

    /* We're now using a custom size, not a fixed one */
    /* printf("Detecting state: %s\n", selection_names[key]); */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gtk_object_get_data(base, selection_names[key])), TRUE);

    return;
} /* sp_export_detect_size */

static void
sp_export_area_x_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float x0, x1, xdpi, width;

    if (gtk_object_get_data (base, "update"))
        return;
        
    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data 
            (base, "units"))) 
    {
        return;
    }
    
    gtk_object_set_data ( base, "update", GUINT_TO_POINTER (TRUE) );

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

    sp_export_detect_size(base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_area_x_value_changed()


static void
sp_export_area_y_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float y0, y1, ydpi, height;

    if (gtk_object_get_data (base, "update"))
        return;
    
    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data 
           (base, "units"))) 
    {
        return;
    }
       
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

    sp_export_detect_size(base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));
    
    return;
} // end of sp_export_area_y_value_changed()


static void
sp_export_area_width_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float x0, x1, xdpi, width, bmwidth;

    if (gtk_object_get_data (base, "update")) 
        return;
    
    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data 
           (base, "units"))) {
        return;
    }
    
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

    return;
} // end of sp_export_area_width_value_changed()


static void
sp_export_area_height_value_changed (GtkAdjustment *adj, GtkObject *base)
{

    float y0, y1, ydpi, height, bmheight;

    if (gtk_object_get_data (base, "update"))
        return;
    
    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data 
           (base, "units"))) {
        return;
    }
    
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
    
    return;
} // end of sp_export_area_height_value_changed()

/**
    \brief  A function to set the ydpi
    \param  base  The export dialog

    This function grabs all of the y values and then figures out the
    new bitmap size based on the changing dpi value.  The dpi value is
    gotten from the xdpi setting as these can not currently be independent.
*/
static void
sp_export_set_image_y (GtkObject *base)
{
    float y0, y1, xdpi;

    y0 = sp_export_value_get_pt (base, "y0");
    y1 = sp_export_value_get_pt (base, "y1");
    xdpi = sp_export_value_get (base, "xdpi");

    sp_export_value_set (base, "ydpi", xdpi);
    sp_export_value_set (base, "bmheight", (y1 - y0) * xdpi / 72.0);

    return;
} // end of sp_export_set_image_y()


static void
sp_export_bitmap_width_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float x0, x1, bmwidth, xdpi;

    if (gtk_object_get_data (base, "update")) 
        return;
        
    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data 
           (base, "units"))) {
       return;
    }
    
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

    return;
} // end of sp_export_bitmap_width_value_changed()

/**
    \brief  A function to adjust the bitmap width when the xdpi value changes
    \param  adj  The adjustment that was changed
    \param  base The export dialog itself

    The first thing this function checks is to see if we are doing an
    update.  If we are, this function just returns because there is another
    instance of it that will handle everything for us.  If there is a
    units change, we also assume that everyone is being updated appropriately
    and there is nothing for us to do.

    If we're the highest level function, we set the update flag, and 
    continue on our way.

    All of the values are grabbed using the \c sp_export_value_get functions
    (call to the _pt ones for x0 and x1 but just standard for xdpi).  The
    xdpi value is saved in the preferences for the next time the dialog
    is opened.  (does the selection dpi need to be set here?)

    A check is done to to ensure that we aren't outputing an invalid width,
    this is set by SP_EXPORT_MIN_SIZE.  If that is the case the dpi is
    changed to make it valid.

    After all of this the bitmap width is changed.

    We also change the ydpi.  This is a temporary hack as these can not
    currently be independent.  This is likely to change in the future.
*/
void
sp_export_xdpi_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float x0, x1, xdpi, bmwidth;

    if (gtk_object_get_data (base, "update")) 
        return;
    
    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data 
           (base, "units"))) {
       return;
    }
    
    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    x0 = sp_export_value_get_pt (base, "x0");
    x1 = sp_export_value_get_pt (base, "x1");
    xdpi = sp_export_value_get (base, "xdpi");

    // remember xdpi setting
    prefs_set_double_attribute ("dialogs.export.defaultxdpi", "value", xdpi);

    bmwidth = (x1 - x0) * xdpi / 72.0;

    if (bmwidth < SP_EXPORT_MIN_SIZE) {
        bmwidth = SP_EXPORT_MIN_SIZE;
        if (x1 != x0) 
            xdpi = bmwidth * 72.0 / (x1 - x0);
        else 
            xdpi = 72.0;
        sp_export_value_set (base, "xdpi", xdpi);
    }

    sp_export_value_set (base, "bmwidth", bmwidth);

    sp_export_set_image_y (base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_xdpi_value_changed()


/**
    \brief  A function to change the area that is used for the exported
            bitmap.
    \param  base  This is the export dialog
    \param  x0    Horizontal upper left hand corner of the picture in points
    \param  y0    Vertical upper left hand corner of the picture in points
    \param  x1    Horizontal lower right hand corner of the picture in points
    \param  y1    Vertical lower right hand corner of the picture in points

    This function just calls \c sp_export_value_set_pt for each of the
    parameters that is passed in.  This allows for setting them all in
    one convient area.

    Update is set to suspend all of the other test running while all the
    values are being set up.  This allows for a performance increase, but
    it also means that the wrong type won't be detected with only some of
    the values set.  After all the values are set everyone is told that
    there has been an update.
*/
static void
sp_export_set_area ( GtkObject *base, float x0, float y0, float x1, float y1 )
{
    gtk_object_set_data ( base, "update", GUINT_TO_POINTER (TRUE) );
    sp_export_value_set_pt (base, "x1", x1);
    sp_export_value_set_pt (base, "y1", y1);
    sp_export_value_set_pt (base, "x0", x0);
    sp_export_value_set_pt (base, "y0", y0);
    gtk_object_set_data ( base, "update", GUINT_TO_POINTER (FALSE) );

    sp_export_area_x_value_changed ((GtkAdjustment *)gtk_object_get_data (base, "x1"), base);
    sp_export_area_y_value_changed ((GtkAdjustment *)gtk_object_get_data (base, "y1"), base);

    return;
} // end of sp_export_set_area()

/**
    \brief  Sets the value of an adjustment
    \param  base  The export dialog
    \param  key   Which adjustment to set
    \param  val   What value to set it to

    This function finds the adjustment using the data stored in the
    export dialog.  After finding the adjustment it then sets
    the value of it.
*/
static void
sp_export_value_set ( GtkObject *base, const gchar *key, float val )
{
    GtkAdjustment *adj;

    adj = (GtkAdjustment *)gtk_object_get_data (base, key);

    gtk_adjustment_set_value (adj, val);

    return;
} // end of sp_export_value_set()

/**
    \brief  A function to set a value using the units points
    \param  base  The export dialog
    \param  key   Which value should be set
    \param  val   What the value should be in points

    This function first gets the adjustment for the key that is passed
    in.  It then figures out what units are currently being used in the
    dialog.  After doing all of that, it then converts the incoming
    value and sets the adjustment.
*/
static void
sp_export_value_set_pt (GtkObject *base, const gchar *key, float val)
{
    const SPUnit *unit;

    unit = 
        sp_unit_selector_get_unit ((SPUnitSelector *)gtk_object_get_data 
            (base, "units") );

    sp_export_value_set (base, key, sp_points_get_units (val, unit));

    return;
} // end of sp_export_value_set_pt()

/**
    \brief  Get the value of an adjustment in the export dialog
    \param  base  The export dialog
    \param  key   Which adjustment is being looked for
    \return The value in the specified adjustment

    This function gets the adjustment from the data field in the export
    dialog.  It then grabs the value from the adjustment.
*/
static float
sp_export_value_get ( GtkObject *base, const gchar *key )
{
    GtkAdjustment *adj;

    adj = (GtkAdjustment *)gtk_object_get_data (base, key);

    return adj->value;
} // end of sp_export_value_get()

/**
    \brief  Grabs a value in the export dialog and converts the unit
            to points
    \param  base  The export dialog
    \param  key   Which value should be returned
    \return The value in the adjustment in points

    This function, at its most basic, is a call to \c sp_export_value_get
    to get the value of the adjustment.  It then finds the units that
    are being used by looking at the "units" attribute of the export
    dialog.  Using that it converts the returned value into points.
*/
static float
sp_export_value_get_pt ( GtkObject *base, const gchar *key )
{
    float value;
    const SPUnit *unit;

    value = sp_export_value_get(base, key);

    unit = sp_unit_selector_get_unit ((SPUnitSelector *)gtk_object_get_data (base, "units"));

    return sp_units_get_points (value, unit);
} // end of sp_export_value_get_pt()

/**
    \brief  This function is called when the filename is changed by
            anyone.  It resets the virgin bit.
    \param  object  Text entry box
    \param  data    The export dialog
    \return None

    This function gets called when the text area is modified.  It is
    looking for the case where the text area is modified from its
    original value.  In that case it sets the "filename-modified" bit
    to TRUE.  If the text dialog returns back to the original text, the
    bit gets reset.  This should stop simple mistakes.
*/
static void
sp_export_filename_modified (GtkObject * object, gpointer data)
{
    GtkWidget * text_entry = (GtkWidget *)object;
    GtkWidget * export_dialog = (GtkWidget *)data;

    if (!strcmp(original_name, gtk_entry_get_text(GTK_ENTRY(text_entry)))) {
        gtk_object_set_data (GTK_OBJECT (export_dialog), "filename-modified", (gpointer)FALSE);
//        printf("Modified: FALSE\n");
    } else {
        gtk_object_set_data (GTK_OBJECT (export_dialog), "filename-modified", (gpointer)TRUE);
//        printf("Modified: TRUE\n");
    }

    return;
} // end sp_export_filename_modified

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
