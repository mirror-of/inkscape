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
#include "../sp-text.h"
#include "../selection-chemistry.h"
#include "../sp-defs.h"
#include "../sp-rect.h"
#include "../sp-ellipse.h"
#include "../sp-star.h"
#include "../sp-spiral.h"
#include "../sp-polygon.h"
#include "../sp-path.h"
#include "../sp-line.h"
#include "../sp-polyline.h"
#include "../sp-item-group.h"
#include "../sp-use.h"
#include "../sp-image.h"
#include "../sp-offset.h"

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

void
sp_find_squeeze_window()
{
    GtkRequisition r;
    gtk_widget_size_request(dlg, &r);
    gtk_window_resize ((GtkWindow *) dlg, r.width, r.height);
}

bool
item_id_match (SPItem *item, const gchar *id, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    if (SP_IS_STRING(item)) // SPStrings have "on demand" ids which are useless for searching
        return false;

    const gchar *item_id = sp_repr_attr (SP_OBJECT_REPR (item), "id");
    if (item_id == NULL)
        return false;

    if (exact) {
        return ((bool) !strcmp(item_id, id));
    } else {
//        g_print ("strstr: %s %s: %s\n", item_id, id, strstr(item_id, id) != NULL? "yes":"no");
        return ((bool) (strstr(item_id, id) != NULL));
    }
}

bool
item_text_match (SPItem *item, const gchar *text, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    const gchar *item_text = sp_repr_content (SP_OBJECT_REPR (item));
    if (item_text == NULL)
        return false;

    if (exact) {
        return ((bool) !strcasecmp(item_text, text));
    } else {
        //FIXME: strcasestr
        return ((bool) (strstr(item_text, text) != NULL));
    }
}

bool
item_style_match (SPItem *item, const gchar *text, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    const gchar *item_text = sp_repr_attr (SP_OBJECT_REPR (item), "style");
    if (item_text == NULL)
        return false;

    if (exact) {
        return ((bool) !strcmp(item_text, text));
    } else {
        return ((bool) (strstr(item_text, text) != NULL));
    }
}

bool
item_attr_match (SPItem *item, const gchar *name, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    if (exact) {
        const gchar *attr_value = sp_repr_attr (SP_OBJECT_REPR (item), name);
        return ((bool) (attr_value != NULL));
    } else {
        return sp_repr_has_attr (SP_OBJECT_REPR (item), name);
    }
}


GSList *
filter_onefield (GSList *l, GObject *dlg, const gchar *field, bool (*match_function)(SPItem *, const gchar *, bool), bool exact)
{
    GtkWidget *widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), field));
    const gchar *text = gtk_entry_get_text (GTK_ENTRY(widget));

    if (strlen (text) != 0) {
        GSList *n = NULL;
        for (GSList *i = l; i != NULL; i = i->next) {
            if (match_function (SP_ITEM(i->data), text, exact)) {
                n = g_slist_prepend (n, i->data);
            }
        }
        return n;
    } else {
        return l;
    }

    return NULL;
}


bool 
type_checkbox (GtkWidget *widget, const gchar *data)
{
    return  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (widget), data)));
}

bool 
item_type_match (SPItem *item, GtkWidget *widget)
{
    if (SP_IS_RECT(item)) {
        return (type_checkbox (widget, "shapes") || type_checkbox (widget, "rects"));

    } else if (SP_IS_GENERICELLIPSE(item) || SP_IS_ELLIPSE(item) || SP_IS_ARC(item) || SP_IS_CIRCLE(item)) {
        return (type_checkbox (widget, "shapes") || type_checkbox (widget, "ellipses"));

    } else if (SP_IS_STAR(item) || SP_IS_POLYGON(item)) {
        return (type_checkbox (widget, "shapes") || type_checkbox (widget, "stars"));

    } else if (SP_IS_SPIRAL(item)) {
        return (type_checkbox (widget, "shapes") || type_checkbox (widget, "spirals"));

    } else if (SP_IS_PATH(item) || SP_IS_LINE(item) || SP_IS_POLYLINE(item)) {
        return (type_checkbox (widget, "paths"));

    } else if (SP_IS_TEXT(item) || SP_IS_TSPAN(item) || SP_IS_STRING(item)) {
        return (type_checkbox (widget, "texts"));

    } else if (SP_IS_GROUP(item)) {
        return (type_checkbox (widget, "groups"));

    } else if (SP_IS_USE(item)) {
        return (type_checkbox (widget, "clones"));

    } else if (SP_IS_IMAGE(item)) {
        return (type_checkbox (widget, "images"));

    } else if (SP_IS_OFFSET(item)) {
        return (type_checkbox (widget, "offsets"));
    }

    return false;
}

GSList *
filter_types (GSList *l, GObject *dlg, bool (*match_function)(SPItem *, GtkWidget *))
{
    GtkWidget *widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "types"));

    GtkWidget *alltypes = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (widget), "all"));
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (alltypes))) 
        return l;
    

    GSList *n = NULL;
    for (GSList *i = l; i != NULL; i = i->next) {
        if (match_function (SP_ITEM(i->data), widget)) {
            n = g_slist_prepend (n, i->data);
        }
    }
    return n;
}


GSList *
filter_list (GSList *l, GObject *dlg, bool exact)
{
    l = filter_onefield (l, dlg, "text", item_text_match, exact);
    l = filter_onefield (l, dlg, "id", item_id_match, exact);
    l = filter_onefield (l, dlg, "style", item_style_match, exact);
    l = filter_onefield (l, dlg, "attr", item_attr_match, exact);

    l = filter_types (l, dlg, item_type_match);

    return l;
}

GSList *
all_items (SPObject *r, GSList *l)
{
    if (SP_IS_DEFS(r))
        return l; // we're not interested in items in defs 

    for (SPObject *child = sp_object_first_child(r); child; child = SP_OBJECT_NEXT (child)) {
        if (SP_IS_ITEM (child) && !SP_OBJECT_IS_CLONED (child)) {
            l = g_slist_prepend (l, child);
        }
        l = all_items (child, l);
    }
    return l;
}

GSList *
all_selection_items (SPSelection *s, GSList *l)
{
   for (GSList *i = (GSList *) s->itemList(); i != NULL; i = i->next) {
        if (SP_IS_ITEM (i->data) && !SP_OBJECT_IS_CLONED (i->data)) {
            l = g_slist_prepend (l, i->data);
        }
        l = all_items (SP_OBJECT (i->data), l);
    }
    return l;
}


void sp_find_dialog_find(GObject *, GObject *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    GSList *l = NULL;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (dlg), "inselection")))) {
        l = all_selection_items (desktop->selection, l);
    } else {
        l = all_items (SP_DOCUMENT_ROOT (SP_DT_DOCUMENT (desktop)), l);
    }
    guint all = g_slist_length (l);

    bool exact = true;
    GSList *n = NULL;
    n = filter_list (l, dlg, exact);
    if (n == NULL) {
        exact = false;
        n = filter_list (l, dlg, exact);
    }

    if (n != NULL) {
        sp_view_set_statusf_flash(SP_VIEW(desktop), 
                                  _("%d object(s) found (out of %d), %s match."), 
                                  g_slist_length (n), all, exact? _("exact") : _("partial"));
        SPSelection *selection = SP_DT_SELECTION (desktop);
        selection->clear();
        selection->setItemList(n);
        scroll_to_show_item (desktop, SP_ITEM(n->data));
    } else {
        sp_view_set_statusf_flash(SP_VIEW(desktop), _("No objects found."));
    }
}

void
sp_find_reset_searchfield (GObject *dlg, const gchar *field)
{
    GtkWidget *widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), field));
    gtk_entry_set_text (GTK_ENTRY(widget), "");
}


void
sp_find_dialog_reset (GObject *, GObject *dlg)
{
    sp_find_reset_searchfield (dlg, "text");
    sp_find_reset_searchfield (dlg, "id");
    sp_find_reset_searchfield (dlg, "style");
    sp_find_reset_searchfield (dlg, "attr");

    GtkWidget *types = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "types"));
    GtkToggleButton *tb = GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (types), "all"));
    gtk_toggle_button_toggled (tb);
    gtk_toggle_button_set_active (tb, TRUE);
}


#define FIND_LABELWIDTH 80

void
sp_find_new_searchfield (GtkWidget *dlg, GtkWidget *vb, const gchar *label, const gchar *id, GtkTooltips *tt, const gchar *tip)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, 0);
    GtkWidget *l = gtk_label_new_with_mnemonic (label);
    gtk_widget_set_usize (l, FIND_LABELWIDTH, -1);
    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

    GtkWidget *tf = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (tf), 64);
    gtk_box_pack_start (GTK_BOX (hb), tf, TRUE, TRUE, 0);
    gtk_object_set_data (GTK_OBJECT (dlg), id, tf);
    gtk_tooltips_set_tip (tt, tf, tip, NULL);
    g_signal_connect ( G_OBJECT (tf), "activate", G_CALLBACK (sp_find_dialog_find), dlg );
    gtk_label_set_mnemonic_widget   (GTK_LABEL(l), tf);

    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
}

void
sp_find_new_button (GtkWidget *dlg, GtkWidget *hb, const gchar *label, GtkTooltips *tt, const gchar *tip, void (*function) (GObject *, GObject *))
{
    GtkWidget *b = gtk_button_new_with_mnemonic (label);
    gtk_tooltips_set_tip (tt, b, tip, NULL);
    gtk_box_pack_start (GTK_BOX (hb), b, TRUE, TRUE, 0);
    g_signal_connect ( G_OBJECT (b), "clicked", G_CALLBACK (function), dlg );
    gtk_widget_show (b);
}

void
toggle_alltypes (GtkToggleButton *tb, gpointer data)
{
    GtkWidget *alltypes_pane =  GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (data), "all-pane"));
    if (gtk_toggle_button_get_active (tb)) {
        gtk_widget_hide_all (alltypes_pane);
    } else {
        gtk_widget_show_all (alltypes_pane);
        
        // excplicit toggle to make sure its handler gets called, no matter what was the original state
        gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "shapes")));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "shapes")), TRUE);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "paths")), TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "texts")), TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "groups")), TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "clones")), TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "images")), TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "offsets")), TRUE);
    }
    sp_find_squeeze_window();
}

void
toggle_shapes (GtkToggleButton *tb, gpointer data)
{
    GtkWidget *shapes_pane =  GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (data), "shapes-pane"));
    if (gtk_toggle_button_get_active (tb)) {
        gtk_widget_hide_all (shapes_pane);
    } else {
        gtk_widget_show_all (shapes_pane);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "rects")), FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "ellipses")), FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "stars")), FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_object_get_data (GTK_OBJECT (data), "spirals")), FALSE);
    }
    sp_find_squeeze_window();
}


GtkWidget *
sp_find_types_checkbox (GtkWidget *w, const gchar *data, gboolean active, 
                        GtkTooltips *tt, const gchar *tip,
                        const gchar *label,
                        void (*toggled)(GtkToggleButton *, gpointer))
{
    GtkWidget *hb = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hb);

    {
        GtkWidget *b  = gtk_check_button_new_with_label (label);
        gtk_widget_show (b);
        gtk_toggle_button_set_active ((GtkToggleButton *) b, active);
        gtk_object_set_data (GTK_OBJECT (w), data, b);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, tip, NULL);
        if (toggled)
            gtk_signal_connect (GTK_OBJECT (b), "toggled", GTK_SIGNAL_FUNC (toggled), w);
        gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
    }

    return hb;
}

GtkWidget *
sp_find_types_checkbox_indented (GtkWidget *w, const gchar *data, gboolean active, 
                                 GtkTooltips *tt, const gchar *tip,
                                 const gchar *label,
                                 void (*toggled)(GtkToggleButton *, gpointer), guint indent)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hb);

    { // empty label for indent
        GtkWidget *l = gtk_label_new ("");
        gtk_widget_show (l);
        gtk_widget_set_usize (l, FIND_LABELWIDTH + indent, -1);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    GtkWidget *c = sp_find_types_checkbox (w, data, active, tt, tip, label, toggled);
    gtk_box_pack_start (GTK_BOX (hb), c, FALSE, FALSE, 0);

    return hb;
}


GtkWidget *
sp_find_types ()
{
    GtkTooltips *tt = gtk_tooltips_new ();

    GtkWidget *vb = gtk_vbox_new (FALSE, 4);
    gtk_widget_show (vb);

    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hb);

        {
            GtkWidget *l = gtk_label_new_with_mnemonic (_("T_ype: "));
            gtk_widget_show (l);
            gtk_widget_set_usize (l, FIND_LABELWIDTH, -1);
            gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
        }

        GtkWidget *alltypes = sp_find_types_checkbox (vb, "all", TRUE, tt, _("Search in all object types"), _("All types"), toggle_alltypes);
        gtk_box_pack_start (GTK_BOX (hb), alltypes, FALSE, FALSE, 0);
   
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
    }

    {
        GtkWidget *vb_all = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vb_all);

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "shapes", FALSE, tt, _("Search all shapes"), _("All shapes"), toggle_shapes, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }


        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            gtk_widget_show (hb);

            { // empty label for alignment
                GtkWidget *l = gtk_label_new ("");
                gtk_widget_show (l);
                gtk_widget_set_usize (l, FIND_LABELWIDTH + 20, -1);
                gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
            }

            {
                GtkWidget *c = sp_find_types_checkbox (vb, "rects", FALSE, tt, _("Search rectangles"), _("Rects"), NULL);
                gtk_box_pack_start (GTK_BOX (hb), c, FALSE, FALSE, 0);
            }

            {
                GtkWidget *c = sp_find_types_checkbox (vb, "ellipses", FALSE, tt, _("Search ellipses, arcs, circles"), _("Ellipses"), NULL);
                gtk_box_pack_start (GTK_BOX (hb), c, FALSE, FALSE, 0);
            }

            {
                GtkWidget *c = sp_find_types_checkbox (vb, "stars", FALSE, tt, _("Search stars and polygons"), _("Stars"), NULL);
                gtk_box_pack_start (GTK_BOX (hb), c, FALSE, FALSE, 0);
            }

            {
                GtkWidget *c = sp_find_types_checkbox (vb, "spirals", FALSE, tt, _("Search spirals"), _("Spirals"), NULL);
                gtk_box_pack_start (GTK_BOX (hb), c, FALSE, FALSE, 0);
            }

            gtk_object_set_data (GTK_OBJECT (vb), "shapes-pane", hb);

            gtk_box_pack_start (GTK_BOX (vb_all), hb, FALSE, FALSE, 0);
            gtk_widget_hide_all (hb);
        }

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "paths", TRUE, tt, _("Search paths, lines, polylines"), _("Paths"), NULL, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "texts", TRUE, tt, _("Search text objects"), _("Texts"), NULL, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "groups", TRUE, tt, _("Search groups"), _("Groups"), NULL, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "clones", TRUE, tt, _("Search clones"), _("Clones"), NULL, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "images", TRUE, tt, _("Search images"), _("Images"), NULL, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }

        {
            GtkWidget *c = sp_find_types_checkbox_indented (vb, "offsets", TRUE, tt, _("Search offset objects"), _("Offsets"), NULL, 10);
            gtk_box_pack_start (GTK_BOX (vb_all), c, FALSE, FALSE, 0);
        }

        gtk_box_pack_start (GTK_BOX (vb), vb_all, FALSE, FALSE, 0);
        gtk_object_set_data (GTK_OBJECT (vb), "all-pane", vb_all);
        gtk_widget_hide_all (vb_all);
    }

    return vb;
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

        gtk_container_set_border_width (GTK_CONTAINER (dlg), 4);

        /* Toplevel vbox */
        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (dlg), vb);

        sp_find_new_searchfield (dlg, vb, _("_Text: "), "text", tt, _("Find objects by their text content (exact or partial match)"));
        sp_find_new_searchfield (dlg, vb, _("_ID: "), "id", tt, _("Find objects by the value of the id attribute (exact or partial match)"));
        sp_find_new_searchfield (dlg, vb, _("_Style: "), "style", tt, _("Find objects by the value of the style attribute (exact or partial match)"));
        sp_find_new_searchfield (dlg, vb, _("_Attribute: "), "attr", tt, _("Find objects by the name of an attribute (exact or partial match)"));

        gtk_widget_show_all (vb);

        GtkWidget *types = sp_find_types ();
        gtk_object_set_data (GTK_OBJECT (dlg), "types", types);
        gtk_box_pack_start (GTK_BOX (vb), types, FALSE, FALSE, 0);

        {
            GtkWidget *w = gtk_hseparator_new (); 
            gtk_widget_show (w);
            gtk_box_pack_start (GTK_BOX (vb), w, FALSE, FALSE, 3);

            GtkWidget *b  = gtk_check_button_new_with_mnemonic (_("Search in s_election"));
            gtk_widget_show (b);
            gtk_toggle_button_set_active ((GtkToggleButton *) b, FALSE);
            gtk_object_set_data (GTK_OBJECT (dlg), "inselection", b);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Limit search to the current selection"), NULL);
            gtk_box_pack_start (GTK_BOX (vb), b, FALSE, FALSE, 0);
        }

        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            gtk_widget_show (hb);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

            sp_find_new_button (dlg, hb, _("_Clear"), tt, _("Clear values"), sp_find_dialog_reset);
            sp_find_new_button (dlg, hb, _("_Find"), tt, _("Select objects matching all of the fields you filled in"), sp_find_dialog_find);
        }
    }


    gtk_window_present ((GtkWindow *) dlg);
    sp_find_dialog_reset (NULL, G_OBJECT (dlg));
    
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
