#define __SP_TRANSFORMATION_C__

/**
 * \brief  Find dialog
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gtk/gtk.h>
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

#include "../inkscape-stock.h"
#include "helper/sp-intl.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "libnr/nr-scale.h"
#include "libnr/nr-scale-ops.h"
#include "macros.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "desktop-handles.h"

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"
#include "../sp-object.h"
using NR::X;
using NR::Y;



static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.find";




static void sp_find_dialog_destroy(GtkObject *object, gpointer)
{
    sp_signal_disconnect_by_data (INKSCAPE, object);
    wd.win = dlg = NULL;
    wd.stop = 0;
}



static gboolean sp_find_dialog_delete(GtkObject *, GdkEvent *, gpointer data)
{
    gtk_window_get_position (GTK_WINDOW (dlg), &x, &y);
    gtk_window_get_size (GTK_WINDOW (dlg), &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it
}

bool
item_id_match (SPItem *item, const gchar *id, bool exact)
{
    const gchar *item_id = sp_repr_attr (SP_OBJECT_REPR (item), "id");
    if (exact) {
        return ((bool) !strcmp(item_id, id));
    } else {
//        g_print ("strstr: %s %s: %s\n", item_id, id, strstr(item_id, id) != NULL? "yes":"no");
        return ((bool) (strstr(item_id, id) != NULL));
    }
}

GSList *
filter_id (GSList *l, const gchar *id, bool exact)
{
    GSList *n = NULL;
    for (GSList *i = l; i != NULL; i = i->next) {
        if (item_id_match (SP_ITEM(i->data), id, exact)) {
            n = g_slist_prepend (n, i->data);
        }
    }
    return n;
}

GSList *
filter_list (GSList *l, GObject *dlg, bool exact)
{
    GtkWidget *id_widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "id"));
    const gchar *id = gtk_entry_get_text (GTK_ENTRY(id_widget));
    GSList *n = NULL;
    if (strlen (id) != 0) {
        n = filter_id (l, id, exact);
    }
    return n;
}

GSList *
all_items (SPObject *r, GSList *l)
{
    for (SPObject *child = sp_object_first_child(r); child; child = SP_OBJECT_NEXT (child)) {
        if (SP_IS_ITEM (child) && !SP_OBJECT_IS_CLONED (child)) {
            l = g_slist_prepend (l, child);
        }
        l = all_items (child, l);
    }
    return l;
}

static void sp_find_dialog_find(GObject *, GObject *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    GSList *l = NULL;
    l = all_items (SP_DOCUMENT_ROOT (SP_DT_DOCUMENT (desktop)), l);
    gint all = g_slist_length (l);

    bool exact = true;
    GSList *n = NULL;
    n = filter_list (l, dlg, exact);
    if (n == NULL) {
        exact = false;
        n = filter_list (l, dlg, exact);
    }

    if (n != NULL && g_slist_length (n) != all) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("%d object(s) found, %s match."), g_slist_length (n), exact? _("exact") : _("partial"));
        SPSelection *selection = SP_DT_SELECTION (desktop);
        selection->clear();
        selection->setItemList(n);
    } else {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("No objects found."));
    }
}



GtkWidget *
sp_find_dialog (void)
{
    if  (!dlg)
    {
        gchar title[500];
        sp_ui_dialog_title_string (SP_VERB_DIALOG_FIND, title);

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
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd );
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
                             
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_find_dialog_destroy), NULL );
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_find_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_find_dialog_delete), dlg);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

        GtkTooltips *tt = gtk_tooltips_new ();

        /* Toplevel vbox */
        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (dlg), vb);


        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            GtkWidget *l = gtk_label_new_with_mnemonic (_("_ID"));
            gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

            GtkWidget *tf = gtk_entry_new ();
            gtk_entry_set_max_length (GTK_ENTRY (tf), 64);
            gtk_box_pack_start (GTK_BOX (hb), tf, TRUE, TRUE, 0);
            gtk_object_set_data (GTK_OBJECT (dlg), "id", tf);
            g_signal_connect ( G_OBJECT (tf), "activate", G_CALLBACK (sp_find_dialog_find), dlg );

            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
        }


        {
            GtkWidget *b = gtk_button_new_with_label (_("Find"));
            gtk_tooltips_set_tip (tt, b, _("Find and select objects matching all of the filled fields"), NULL);
            gtk_box_pack_start (GTK_BOX (vb), b, FALSE, FALSE, 0);
    g_signal_connect ( G_OBJECT (b), "clicked", 
                         G_CALLBACK (sp_find_dialog_find), 
                         dlg );

        }

        gtk_widget_show_all (vb);


    }

    gtk_window_present ((GtkWindow *) dlg);

    return dlg;
    
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
