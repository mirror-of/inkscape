#define __SP_QUICK_ALIGN_C__

/**
 * \brief  Object align dialog
 *
 * Authors:
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <glib.h>
#include <math.h>
#include <stdlib.h>
#include <libnr/nr-macros.h>
#include <gtk/gtksignal.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtklabel.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "widgets/button.h"
#include "widgets/sp-widget.h"
#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include <sp-item.h>
#include "sp-item-transform.h"
#include "selection.h"
#include "dialog-events.h"
#include "macros.h"
#include <libnr/nr-point-fns.h>

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"

#include "align.h"

/*
 * handler functions for quick align dialog
 *
 * todo: dialog with more aligns
 * - more aligns (30 % left from center ...)
 * - aligns for nodes
 *
 */ 

enum {
    SP_ALIGN_LAST,
    SP_ALIGN_FIRST,
    SP_ALIGN_BIGGEST,
    SP_ALIGN_SMALLEST,
    SP_ALIGN_PAGE,
    SP_ALIGN_DRAWING,
    SP_ALIGN_SELECTION
};

enum align_ixT {
    SP_ALIGN_TOP_IN,
    SP_ALIGN_TOP_OUT,
    SP_ALIGN_RIGHT_IN,
    SP_ALIGN_RIGHT_OUT,
    SP_ALIGN_BOTTOM_IN,
    SP_ALIGN_BOTTOM_OUT,
    SP_ALIGN_LEFT_IN,
    SP_ALIGN_LEFT_OUT,
    SP_ALIGN_CENTER_HOR,
    SP_ALIGN_CENTER_VER
};

enum {
    SP_DISTRIBUTE_LEFT,
    SP_DISTRIBUTE_HCENTRE,
    SP_DISTRIBUTE_RIGHT,
    SP_DISTRIBUTE_HDIST
};

enum {
    SP_DISTRIBUTE_TOP,
    SP_DISTRIBUTE_VCENTRE,
    SP_DISTRIBUTE_BOTTOM,
    SP_DISTRIBUTE_VDIST
};

static struct AlignCoeffs {
    double mx0;
    double mx1;
    double my0;
    double my1;
    double sx0;
    double sx1;
    double sy0;
    double sy1;
} const aligns[10] = {
    {0., 0., 0., 1., 0., 0., 0., 1.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 1., 0., 0., 0., 1., 0., 0.},
    {0., 1., 0., 0., 1., 0., 0., 0.},
    {0., 0., 1., 0., 0., 0., 1., 0.},
    {0., 0., 1., 0., 0., 0., 0., 1.},
    {1., 0., 0., 0., 1., 0., 0., 0.},
    {1., 0., 0., 0., 0., 1., 0., 0.},
    {.5, .5, 0., 0., .5, .5, 0., 0.},
    {0., 0., .5, .5, 0., 0., .5, .5}
};

static const gchar hdist[4][3] = {
    {2, 0, 0},
    {1, 1, 0},
    {0, 2, 0},
    {1, 1, 1}
};

static const gchar vdist[4][3] = {
    {0, 2, 0},
    {1, 1, 0},
    {2, 0, 0},
    {1, 1, 1}
};

static void sp_align_arrange_clicked 
    (GtkWidget *widget, gconstpointer data);

static void sp_align_distribute_h_clicked 
    (GtkWidget *widget, const gchar *layout);

static void sp_align_distribute_v_clicked 
    (GtkWidget *widget, const gchar *layout);

static GtkWidget *sp_align_dialog_create_base_menu (void);
static void set_base (GtkMenuItem * menuitem, gpointer data);

static SPItem * sp_quick_align_find_master 
    (const GSList * slist, gboolean horizontal);

static GtkWidget *dlg = NULL;
static win_data wd;

/* impossible original values to make sure they are read from prefs */
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.align";

static unsigned int base = SP_ALIGN_LAST;



static void
sp_quick_align_dialog_destroy (void)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);

    wd.win = dlg = NULL;
    wd.stop = 0;
    
} // end of sp_quick_align_dialog_destroy()



static gboolean sp_align_dialog_delete(GtkObject *, GdkEvent *, gpointer data)
{
    gtk_window_get_position(GTK_WINDOW (dlg), &x, &y);
    gtk_window_get_size(GTK_WINDOW (dlg), &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_align_dialog_delete()



static void
sp_align_add_button ( GtkWidget *t, int col, int row, 
                      GCallback handler, 
                      gconstpointer data, 
                      const gchar *px, const gchar *tip,
                      GtkTooltips * tt )
{
    GtkWidget *b;
    b = sp_button_new_from_data ( 24, SP_BUTTON_TYPE_NORMAL, NULL, 
                                  px, tip, tt );
    gtk_widget_show (b);
    if (handler) g_signal_connect ( G_OBJECT (b), "clicked", 
                                    handler, (gpointer) data );
    gtk_table_attach ( GTK_TABLE (t), b, col, col + 1, row, row + 1, 
                       (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );

} // end of sp_align_add_button()



void
sp_quick_align_dialog (void)
{
    if (!dlg) {
        GtkTooltips * tt = gtk_tooltips_new ();

        gchar title[500];
        sp_ui_dialog_title_string (SP_VERB_DIALOG_ALIGN_DISTRIBUTE, title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }
        
        if (w == 0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
        }
        
        if (x != 0 || y != 0) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }
        
        if (w && h) {
            gtk_window_resize (GTK_WINDOW (dlg), w, h);
        }
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        
        g_signal_connect (   G_OBJECT (INKSCAPE), "activate_desktop", 
                             G_CALLBACK (sp_transientize_callback), &wd );
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", 
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", 
                             G_CALLBACK (sp_quick_align_dialog_destroy), dlg );
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", 
                             G_CALLBACK (sp_align_dialog_delete), dlg );
        
        g_signal_connect (   G_OBJECT (INKSCAPE), "shut_down", 
                             G_CALLBACK (sp_align_dialog_delete), dlg );
        
        g_signal_connect (   G_OBJECT (INKSCAPE), "dialogs_hide", 
                             G_CALLBACK (sp_dialog_hide), dlg);
                             
        g_signal_connect (   G_OBJECT (INKSCAPE), "dialogs_unhide", 
                             G_CALLBACK (sp_dialog_unhide), dlg);

        GtkWidget *nb = gtk_notebook_new ();
        gtk_container_add (GTK_CONTAINER (dlg), nb);

        /* Align */

        GtkWidget *vb = gtk_vbox_new (FALSE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);

        GtkWidget *om = gtk_option_menu_new ();
        gtk_box_pack_start (GTK_BOX (vb), om, FALSE, FALSE, 0);
        gtk_option_menu_set_menu ( GTK_OPTION_MENU (om), 
                                   sp_align_dialog_create_base_menu () );

        GtkWidget *t = gtk_table_new (2, 5, TRUE);
        gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);

        struct {
        
            int col;
            int row;
            align_ixT ix;
            gchar const *px;
            gchar const *tip;
            
        } const align_buttons[] = {
            {0, 0, SP_ALIGN_LEFT_OUT, "al_left_out", 
                _("Right side of aligned objects to left side of anchor")},
            {1, 0, SP_ALIGN_LEFT_IN, "al_left_in", 
                _("Left side of aligned objects to left side of anchor")},
            {2, 0, SP_ALIGN_CENTER_HOR, "al_center_hor", 
                _("Center horizontally")},
            {3, 0, SP_ALIGN_RIGHT_IN, "al_right_in", 
                _("Right side of aligned objects to right side of anchor")},
            {4, 0, SP_ALIGN_RIGHT_OUT, "al_right_out", 
                _("Left side of aligned objects to right side of anchor")},

            {0, 1, SP_ALIGN_TOP_OUT, "al_top_out", 
                _("Bottom of aligned objects to top of anchor")},
            {1, 1, SP_ALIGN_TOP_IN, "al_top_in", 
                _("Top of aligned objects to top of anchor")},
            {2, 1, SP_ALIGN_CENTER_VER, "al_center_ver", 
                _("Center vertically")},
            {3, 1, SP_ALIGN_BOTTOM_IN, "al_bottom_in", 
                _("Bottom of aligned objects to bottom of anchor")},
            {4, 1, SP_ALIGN_BOTTOM_OUT, "al_bottom_out", 
                _("Top of aligned objects to bottom of anchor")},
                
        };
        
        for (unsigned int i = 0 ; i < G_N_ELEMENTS(align_buttons) ; ++i) {
        
            sp_align_add_button ( t, align_buttons[i].col,
                                  align_buttons[i].row,
                                  G_CALLBACK( sp_align_arrange_clicked ),
                                  &aligns[align_buttons[i].ix],
                                  align_buttons[i].px,
                                  align_buttons[i].tip,
                                  tt );
        }

        GtkWidget *l = gtk_label_new (_("Align"));
        gtk_widget_show (l);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        /* Distribute */

        vb = gtk_vbox_new (FALSE, 4);
        gtk_container_set_border_width ( GTK_CONTAINER (vb), 4 );

        om = gtk_option_menu_new ();
        gtk_box_pack_start ( GTK_BOX (vb), om, FALSE, FALSE, 0 );
        gtk_option_menu_set_menu ( GTK_OPTION_MENU (om), 
                                   sp_align_dialog_create_base_menu () );
        gtk_widget_set_sensitive ( om, FALSE );

        t = gtk_table_new (2, 4, TRUE);
        gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);

        sp_align_add_button ( t, 0, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_LEFT], "distribute_left",
                              _("Distribute left sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 1, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_HCENTRE], 
                              "distribute_hcentre",
                              _("Distribute centers of objects at even " 
                                "distances horizontally"),
                              tt );
                              
        sp_align_add_button ( t, 2, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_RIGHT], "distribute_right",
                              _("Distribute right sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 3, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_HDIST], "distribute_hdist",
                              _("Distribute horizontal distance between " 
                                "objects equally"),
                              tt );

        sp_align_add_button ( t, 0, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_TOP], "distribute_top",
                              _("Distribute top sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 1, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_VCENTRE], 
                              "distribute_vcentre",
                              _("Distribute centers of objects at even " 
                                "distances vertically"),
                              tt );
                              
        sp_align_add_button ( t, 2, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_BOTTOM], "distribute_bottom",
                              _("Distribute bottom sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 3, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_VDIST], "distribute_vdist",
                              _("Distribute vertical distance between objects "
                                "equally"),
                              tt );

        l = gtk_label_new (_("Distribute"));
        gtk_widget_show (l);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        gtk_widget_show_all (nb);
    
    } // end of if (!dlg)

    gtk_window_present ((GtkWindow *) dlg);
    
} // end of sp_quick_align_dialog()



static void
sp_align_add_menuitem ( GtkWidget *menu, const gchar *label, 
                        GCallback handler, int value )
{
    GtkWidget *menuitem = gtk_menu_item_new_with_label (label);
    gtk_widget_show (menuitem);
    
    if (handler) {
        g_signal_connect ( G_OBJECT (menuitem), "activate", 
                           handler, GINT_TO_POINTER (value) );
    }
    
    gtk_menu_append (GTK_MENU (menu), menuitem);

} // end of sp_align_add_menuitem()



static GtkWidget *
sp_align_dialog_create_base_menu (void)
{
    GtkWidget *menu = gtk_menu_new ();

    sp_align_add_menuitem ( menu, _("Last selected"), 
                            G_CALLBACK (set_base), SP_ALIGN_LAST);
                            
    sp_align_add_menuitem ( menu, _("First selected"), 
                            G_CALLBACK (set_base), SP_ALIGN_FIRST);
                            
    sp_align_add_menuitem ( menu, _("Biggest item"), 
                            G_CALLBACK (set_base), SP_ALIGN_BIGGEST);
                            
    sp_align_add_menuitem ( menu, _("Smallest item"), 
                            G_CALLBACK (set_base), SP_ALIGN_SMALLEST);
                            
    sp_align_add_menuitem ( menu, _("Page"), 
                            G_CALLBACK (set_base), SP_ALIGN_PAGE);
                            
    sp_align_add_menuitem ( menu, _("Drawing"), 
                            G_CALLBACK (set_base), SP_ALIGN_DRAWING);
                            
    sp_align_add_menuitem ( menu, _("Selection"), 
                            G_CALLBACK (set_base), SP_ALIGN_SELECTION);

    gtk_widget_show (menu);

    return menu;
    
} // end of sp_align_dialog_create_base_menu()



static void set_base(GtkMenuItem *, gpointer data)
{
    base = GPOINTER_TO_UINT (data);
}



static void sp_align_arrange_clicked(GtkWidget *, gconstpointer data)
{
    AlignCoeffs const &a = *static_cast<AlignCoeffs const *>(data);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }

    SPSelection *selection = SP_DT_SELECTION(desktop);
    GSList *slist = (GSList *) selection->itemList();
    
    if (!slist) {
        return;
    }

    NR::Point mp;
    switch (base) {
    
        case SP_ALIGN_LAST:
        case SP_ALIGN_FIRST:
        case SP_ALIGN_BIGGEST:
        
        case SP_ALIGN_SMALLEST:
        {
            if (!slist->next)
                return;

            slist = g_slist_copy (slist);
            SPItem *master =
                sp_quick_align_find_master ( slist, 
                                             (a.mx0 != 0.0) || (a.mx1 != 0.0) );
            slist = g_slist_remove (slist, master);
            NR::Rect b = sp_item_bbox_desktop (master);
            mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                           a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
            break;
        }
            
        case SP_ALIGN_PAGE:
            slist = g_slist_copy (slist);
            mp = NR::Point(a.mx1 * sp_document_width(SP_DT_DOCUMENT(desktop)),
                           a.my1 * sp_document_height(SP_DT_DOCUMENT(desktop)));
            break;
        
        case SP_ALIGN_DRAWING:
        {
            slist = g_slist_copy (slist);
            NR::Rect b = sp_item_bbox_desktop 
                ( (SPItem *) sp_document_root (SP_DT_DOCUMENT (desktop)) );
            mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                           a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
            break;
        }

        case SP_ALIGN_SELECTION:
        {
            slist = g_slist_copy (slist);
            NR::Rect b =  selection->bounds();
            mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                           a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
            break;
        }

        default:
            g_assert_not_reached ();
            break;
    };  // end of switch

    bool changed = false;
    for (GSList *l = slist; l != NULL; l = l->next) {
        SPItem *item = (SPItem *) l->data;
        NR::Rect b = sp_item_bbox_desktop (item);
        NR::Point const sp(a.sx0 * b.min()[NR::X] + a.sx1 * b.max()[NR::X],
                           a.sy0 * b.min()[NR::Y] + a.sy1 * b.max()[NR::Y]);
        NR::Point const mp_rel( mp - sp );
        if (LInfty(mp_rel) > 1e-9) {
            sp_item_move_rel(item, NR::translate(mp_rel));
            changed = true;
        }
    }

    g_slist_free (slist);

    if (changed) {
        sp_document_done ( SP_DT_DOCUMENT (desktop) );
    }
    
} // end of sp_align_arrange_clicked()



static SPItem *
sp_quick_align_find_master (const GSList *slist, gboolean horizontal)
{
    switch (base) {
        case SP_ALIGN_LAST:
            return (SPItem *) slist->data;
            break;
        
        case SP_ALIGN_FIRST: 
            return (SPItem *) g_slist_last ((GSList *) slist)->data;
            break;
        
        case SP_ALIGN_BIGGEST:
        {
            gdouble max = -1e18;
            SPItem *master = NULL;
            for (const GSList *l = slist; l != NULL; l = l->next) {
                SPItem *item = (SPItem *) l->data;
                NR::Rect b = sp_item_bbox_desktop (item);
                gdouble dim = b.extent(horizontal ? NR::X : NR::Y);
                if (dim > max) {
                    max = dim;
                    master = item;
                }
            }
            return master;
            break;
        }
        
        case SP_ALIGN_SMALLEST:
        {
            gdouble max = 1e18;
            SPItem *master = NULL;
            for (const GSList *l = slist; l != NULL; l = l->next) {
                SPItem *item = (SPItem *) l->data;
                NR::Rect b = sp_item_bbox_desktop (item);
                gdouble dim = b.extent(horizontal ? NR::X : NR::Y);
                if (dim < max) {
                    max = dim;
                    master = item;
                }
            }
            return master;
            break;
        }
        
        default:
            g_assert_not_reached ();
            break;
            
    } // end of switch statement

    return NULL;
    
} //end of sp_quick_align_find_master()



struct SPBBoxSort {
    SPItem *item;
    NR::Rect bbox;
    float anchor;
};



static int
sp_align_bbox_sort ( const void *a, const void *b )
{
    const SPBBoxSort *bbsa = (SPBBoxSort *) a;
    const SPBBoxSort *bbsb = (SPBBoxSort *) b;
    
    if (bbsa->anchor < bbsb->anchor) 
        return -1;
    
    if (bbsa->anchor > bbsb->anchor)
        return 1;
    
    return 0;
} // end of sp_align_bbox_sort()


static void sp_align_distribute_h_or_v_clicked(GtkWidget *, gchar const *layout, NR::Dim2 dim)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    
    if (!desktop)
        return;

    const GSList* slist = SP_DT_SELECTION(desktop)->itemList();
    if (!slist)
        return;
    
    if (!slist->next)
        return;
    
    int len = g_slist_length ((GSList *) slist);
    SPBBoxSort *bbs = g_new (SPBBoxSort, len);
    
    {
        unsigned int pos = 0;
        
        for (const GSList *l = slist; l != NULL; l = l->next) {
            bbs[pos].item = SP_ITEM (l->data);
            bbs[pos].bbox = sp_item_bbox_desktop (bbs[pos].item);
            bbs[pos].anchor =
                0.5 * layout[0] * bbs[pos].bbox.min()[dim] +
                0.5 * layout[0] * bbs[pos].bbox.max()[dim];
            ++pos;
        }
    }
    

    qsort (bbs, len, sizeof (SPBBoxSort), sp_align_bbox_sort);

    bool changed = false;

    if (!layout[2]) {
        float dist = bbs[len - 1].anchor - bbs[0].anchor;
        float step = dist / (len - 1);
        for (int i = 0; i < len; i++) {
            float pos = bbs[0].anchor + i * step;
            if (!NR_DF_TEST_CLOSE (pos, bbs[i].anchor, 1e-6)) {
                NR::Point t(0.0, 0.0);
                t[dim] = pos - bbs[i].anchor;
                sp_item_move_rel(bbs[i].item, NR::translate(t));
                changed = true;
            }
        }
    } else {
        /* Damn I am not sure, how to order them initially (Lauris) */
        float dist = (bbs[len - 1].bbox.max()[dim] - bbs[0].bbox.min()[dim]);
        float span = 0;
        for (int i = 0; i < len; i++) {
            span += bbs[i].bbox.extent(dim);
        }
        
        float step = (dist - span) / (len - 1);
        float pos = bbs[0].bbox.min()[dim];
        for (int i = 0; i < len; i++) {
            if (!NR_DF_TEST_CLOSE (pos, bbs[i].bbox.min()[dim], 1e-6)) {
                NR::Point t(0.0, 0.0);
                t[dim] = pos - bbs[i].bbox.min()[dim];
                sp_item_move_rel(bbs[i].item, NR::translate(t));
                changed = true;
            }
            pos += bbs[i].bbox.extent(dim);
            pos += step;
        }
        
    } // end of if (!layout[2])

    g_free (bbs);

    if ( changed ) {
        sp_document_done ( SP_DT_DOCUMENT (desktop) );
    }
    

}

static void sp_align_distribute_h_clicked(GtkWidget *w, gchar const *layout)
{
    sp_align_distribute_h_or_v_clicked(w, layout, NR::X);
}

static void sp_align_distribute_v_clicked(GtkWidget *w, gchar const *layout)
{
    sp_align_distribute_h_or_v_clicked(w, layout, NR::Y);
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
