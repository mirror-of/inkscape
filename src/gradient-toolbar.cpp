/*
 * Gradient aux toolbar
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <sigc++/sigc++.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkselection.h>
#include <gtk/gtktable.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhandlebox.h>

#include "macros.h"
#include "helper/window.h"
#include "widgets/icon.h"
#include "widgets/button.h"
#include "widgets/widget-sizes.h"
#include "widgets/spw-utilities.h"
#include "widgets/sp-widget.h"
#include "widgets/spinbutton-events.h"
#include "widgets/gradient-vector.h"
#include "style.h"

#include "prefs-utils.h"
#include "inkscape-stock.h"
#include "verbs.h"
#include "file.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "inkscape-private.h"
#include "document.h"
#include "inkscape.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "interface.h"
#include "nodepath.h"
#include "helper/gnome-utils.h"
#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"

#include "select-toolbar.h"
#include "star-context.h"
#include "spiral-context.h"
#include "gradient-context.h"
#include "sp-desktop-widget.h"
#include "sp-rect.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-pattern.h"
#include "sp-ellipse.h"
#include "sp-gradient.h"
#include "gradient-chemistry.h"
#include "selection.h"

#include "toolbox.h"

#include "gradient-toolbar.h"

//########################
//##       Gradient     ##
//########################

static void gr_toggle_type (GtkWidget *button, gpointer data) {
    GtkWidget *linear = (GtkWidget *) g_object_get_data (G_OBJECT(data), "linear");
    GtkWidget *radial = (GtkWidget *) g_object_get_data (G_OBJECT(data), "radial");
    if (button == linear && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (linear))) {
        prefs_set_int_attribute ("tools.gradient", "newgradient", SP_GRADIENT_TYPE_LINEAR);
        if (radial) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radial), FALSE);
    } else if (button == radial && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radial))) {
        prefs_set_int_attribute ("tools.gradient", "newgradient", SP_GRADIENT_TYPE_RADIAL);
        if (linear) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linear), FALSE);
    }
}

static void gr_toggle_fillstroke (GtkWidget *button, gpointer data) {
    GtkWidget *fill = (GtkWidget *) g_object_get_data (G_OBJECT(data), "fill");
    GtkWidget *stroke = (GtkWidget *) g_object_get_data (G_OBJECT(data), "stroke");
    if (button == fill && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fill))) {
        prefs_set_int_attribute ("tools.gradient", "newfillorstroke", 1);
        if (stroke) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (stroke), FALSE);
    } else if (button == stroke && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (stroke))) {
        prefs_set_int_attribute ("tools.gradient", "newfillorstroke", 0);
        if (fill) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fill), FALSE);
    }
}

static void
gr_selector_vector_set (SPGradientVectorSelector *gvs, SPGradient *gr, gpointer data)
{
		gr = sp_gradient_ensure_vector_normalized (gr);

    g_print ("setting %p\n", gr);
}

static void 
gr_tb_sel_changed(SPSelection *selection, gpointer data)
{
    SPGradientVectorSelector *vectors =       (SPGradientVectorSelector *) data;

    if (g_slist_length((GSList *) selection->itemList()) == 1) {

    SPItem *item = selection->singleItem();

        SPStyle *style = SP_OBJECT_STYLE (item);

        SPGradient *gr;

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) { 
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_GRADIENT (server)) {
                gr = sp_gradient_get_vector (SP_GRADIENT (server), FALSE);
            }
        }

        sp_gradient_vector_selector_set_gradient  (vectors, SP_DT_DOCUMENT (selection->desktop()), gr);
    } else if (g_slist_length((GSList *)selection->itemList()) == 0) {
        sp_gradient_vector_selector_set_gradient  (vectors, SP_DT_DOCUMENT (selection->desktop()), NULL);
    }
}



GtkWidget *
sp_gradient_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->owner->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();

    sp_toolbox_add_label(tbl, _("<b>New:</b>"));

    aux_toolbox_space(tbl, AUX_SPACING);

    {
    GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    {
    GtkWidget *button = sp_button_new_from_data( GTK_ICON_SIZE_SMALL_TOOLBAR,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "fill_gradient",
                                              _("Create linear gradient"),
                                              tt);
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_type), tbl);
    g_object_set_data(G_OBJECT(tbl), "linear", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), 
                                  prefs_get_int_attribute ("tools.gradient", "newgradient", 1) == SP_GRADIENT_TYPE_LINEAR);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( GTK_ICON_SIZE_SMALL_TOOLBAR,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "fill_radial",
                                              _("Create radial (elliptic or circular) gradient"),
                                              tt);
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_type), tbl);
    g_object_set_data(G_OBJECT(tbl), "radial", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), 
                                  prefs_get_int_attribute ("tools.gradient", "newgradient", 1) == SP_GRADIENT_TYPE_RADIAL);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tbl), cvbox, FALSE, FALSE, 0);
    }

    aux_toolbox_space(tbl, AUX_SPACING);

    sp_toolbox_add_label(tbl, _("on"), false);

    aux_toolbox_space(tbl, AUX_SPACING);

    {
    GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    {
    GtkWidget *button = sp_button_new_from_data( GTK_ICON_SIZE_SMALL_TOOLBAR,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "controls_fill",
                                              _("Create gradient in the fill"),
                                              tt);
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_fillstroke), tbl);
    g_object_set_data(G_OBJECT(tbl), "fill", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), 
                                  prefs_get_int_attribute ("tools.gradient", "newfillorstroke", 1) == 1);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( GTK_ICON_SIZE_SMALL_TOOLBAR,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "controls_stroke",
                                              _("Create gradient in the stroke"),
                                              tt);
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_fillstroke), tbl);
    g_object_set_data(G_OBJECT(tbl), "stroke", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), 
                                  prefs_get_int_attribute ("tools.gradient", "newfillorstroke", 1) == 0);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tbl), cvbox, FALSE, FALSE, 0);
    }


    sp_toolbox_add_label(tbl, _("<b>Change:</b>"));

    aux_toolbox_space(tbl, AUX_SPACING);


    {
/*
        SPSelection *selection = SP_DT_SELECTION (desktop);
      GtkWidget *vectors = sp_gradient_vector_selector_new (SP_DT_DOCUMENT(desktop), NULL);
	gtk_widget_show (vectors);
	gtk_box_pack_start (GTK_BOX (tbl), vectors, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (vectors), "vector_set", G_CALLBACK (gr_selector_vector_set), NULL);
  sigc::connection conn = selection->connectChanged(
        sigc::bind (
            sigc::ptr_fun(&gr_tb_sel_changed), 
            (gpointer)vectors )
        );
*/
    }


    gtk_widget_show_all(tbl);
    sp_set_font_size(tbl, AUX_FONT_SIZE);

/* // wait when we have the Change part
    sigc::connection *connection = new sigc::connection(
        SP_DT_SELECTION(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_gradient_toolbox_selection_changed), (GtkObject *)tbl))
        );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);
*/

    return tbl;
}




/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
