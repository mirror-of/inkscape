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
#include "../dropper-context.h"
#include "../enums.h"
#include "../selcue.h"

#include "display-settings.h"



static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0; 
static gchar *prefs_path = "dialogs.preferences";

extern gint nr_arena_image_x_sample;
extern gint nr_arena_image_y_sample;
extern gdouble nr_arena_global_delta;

#define SB_WIDTH 90
#define SB_LONG_ADJUSTMENT 20
#define SB_MARGIN 1
#define SUFFIX_WIDTH 70
#define HB_MARGIN 4
#define VB_MARGIN 4
#define VB_SKIP 1

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
options_selector_show_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const gchar *val;
		val = (const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value");
		prefs_set_string_attribute ("tools.select", "show", val);
	}
}

static void
options_store_transform_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const guint val = GPOINTER_TO_INT((const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value"));
		prefs_set_int_attribute ("options.preservetransform", "value", val);
	}
}

static void
options_clone_compensation_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const guint val = GPOINTER_TO_INT((const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value"));
		prefs_set_int_attribute ("options.clonecompensation", "value", val);
	}
}

static void
options_clone_orphans_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const guint val = GPOINTER_TO_INT((const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value"));
		prefs_set_int_attribute ("options.cloneorphans", "value", val);
	}
}

static void
options_selcue_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const guint val = GPOINTER_TO_INT((const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value"));
		prefs_set_int_attribute ("options.selcue", "value", val);
	}
}

static void
options_selector_move_with_grid_toggled (GtkToggleButton *button)
{
        if (gtk_toggle_button_get_active (button)) {
                const gchar *val = (const gchar *) gtk_object_get_data (GTK_OBJECT (button), "value");
                prefs_set_string_attribute ("tools.select", "move_with_grid", val);
        }
}

static void
options_dropper_pick_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		const guint val = GPOINTER_TO_INT((const gchar*)gtk_object_get_data (GTK_OBJECT (button), "value"));
		prefs_set_int_attribute ("tools.dropper", "pick", val);
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
    const char* v_string,
    guint v_uint,
    bool isint,
    gboolean s,
    void (*h)(GtkToggleButton*)
    )
{
	GtkWidget* r = gtk_radio_button_new_with_label (
            b ? gtk_radio_button_group (GTK_RADIO_BUTTON (b)) : NULL, n
            );
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), r, tip, NULL);
	gtk_widget_show (r);

  if (isint)
	gtk_object_set_data (GTK_OBJECT (r), "value", GUINT_TO_POINTER (v_uint));
  else 
	gtk_object_set_data (GTK_OBJECT (r), "value", (void*) v_string);

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

    vb = gtk_vbox_new (FALSE, VB_MARGIN);

    f = gtk_frame_new (_("When transforming, show:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const *show = prefs_get_string_attribute ("tools.select", "show");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("Objects"), _("Show the actual objects when moving or transforming"), "content", 0, false,
        (show == NULL) || !strcmp (show, "content"),
        options_selector_show_toggled
        );

    sp_select_context_add_radio(
        b, fb, tt, _("Box outline"), _("Show only a box outline of the objects when moving or transforming"), "outline",  0, false,
        show && !strcmp (show, "outline"),
        options_selector_show_toggled
        );

    f = gtk_frame_new (_("Per-object selection cue:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gint cue = prefs_get_int_attribute ("options.selcue", "value", SP_SELCUE_MARK);

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("None"), _("No per-object selection indication"), NULL, SP_SELCUE_NONE, true,
        cue == SP_SELCUE_NONE,
        options_selcue_toggled
        );

    b = sp_select_context_add_radio (
        b, fb, tt, _("Mark"), _("Each selected object has a diamond mark in the top left corner"), NULL, SP_SELCUE_MARK, true,
        cue == SP_SELCUE_MARK,
        options_selcue_toggled
        );

    sp_select_context_add_radio (
        b, fb, tt, _("Box"), _("Each selected object displays its bounding box"), NULL, SP_SELCUE_BBOX, true,
        cue == SP_SELCUE_BBOX,
        options_selcue_toggled
        );

    f = gtk_frame_new (_("Default action when moving with grid enabled:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const* move_with_grid = prefs_get_string_attribute ("tools.select", "move_with_grid");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("Snap points to the grid"),
        _("Default action is to snap the points of objects to the grid, even if the object was not "
          "previously snapped.  Shift-drag selects the alternative behaviour."),
        "snap", 0, false, move_with_grid == NULL || !strcmp(move_with_grid, "snap"),
        options_selector_move_with_grid_toggled
        );
        
    b = sp_select_context_add_radio (
        b, fb, tt, _("Snap objects to their current grid offset"),
        _("Default action is to snap objects to having the same offset from the grid as they did "
          "before the move.  Shift-drag selects the alternative behaviour."),
        "keep_offset", 0, false, move_with_grid && !strcmp(move_with_grid, "keep_offset"),
        options_selector_move_with_grid_toggled
        );
    
    return vb;
}


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

    prefs_set_int_attribute ( "options.bitmapoversample", "value", os );
    
} 


static void
options_rotation_steps_changed (GtkMenuItem *item, gpointer data)
{
    gint snaps_new = GPOINTER_TO_INT (data);
    prefs_set_int_attribute ( "options.rotationsnapsperpi", "value", snaps_new );
} 

static void
options_dialogs_ontop_changed (GtkMenuItem *item, gpointer data)
{
    gint policy_new = GPOINTER_TO_INT (data);
    prefs_set_int_attribute ( "options.transientpolicy", "value", policy_new );
} 

void 
options_rotation_steps (GtkWidget *vb, GtkTooltips *tt)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

    {
        GtkWidget *l = gtk_label_new (_("degrees"));
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        gtk_widget_set_usize (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkWidget *om = gtk_option_menu_new ();
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), om, _("Rotating with Ctrl pressed snaps every that much degrees; also, pressing [ or ] rotates by this amount"), NULL);
        gtk_widget_set_usize (om, SB_WIDTH, -1);
        gtk_widget_show (om);
        gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, SB_MARGIN);

        GtkWidget *m = gtk_menu_new ();
        gtk_widget_show (m);

        int snaps_current = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);
        int position_current = 0;

        struct RotSteps {
            double degrees;
            int snaps;
        } const rot_snaps[] = {
            {90, 2},
            {60, 3},
            {45, 4},
            {30, 6},
            {15, 12},
            {10, 18},
            {7.5, 24},
            {6, 30},
            {3, 60},
            {2, 90},
            {1, 180},
            {1, 0},
        };

        for (unsigned j = 0; j < G_N_ELEMENTS(rot_snaps); ++j) {
            RotSteps const &rs = rot_snaps[j];

            const gchar *label = NULL;
            if (rs.snaps == 0) {
                // sorationsnapsperpi == 0 means no snapping
                label = _("None");
            } else {
                label = g_strdup_printf ("%.2g", rs.degrees);
            }

            if (rs.snaps == snaps_current)
                position_current = j;

            GtkWidget *item = gtk_menu_item_new_with_label (label);
            gtk_signal_connect ( GTK_OBJECT (item), "activate", 
                                 GTK_SIGNAL_FUNC (options_rotation_steps_changed),
                                 GINT_TO_POINTER (rs.snaps) );
            gtk_widget_show (item);
            gtk_menu_append (GTK_MENU (m), item);
        }

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
        gtk_option_menu_set_history ( GTK_OPTION_MENU (om), position_current);
    }

    {
        GtkWidget *l = gtk_label_new (_("Rotation snaps every:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }
}

void 
options_dialogs_ontop (GtkWidget *vb, GtkTooltips *tt)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

    { // empty label for alignment
        GtkWidget *l = gtk_label_new ("");
        gtk_widget_set_usize (l, SUFFIX_WIDTH - SB_LONG_ADJUSTMENT, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkWidget *om = gtk_option_menu_new ();
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), om, _("None: dialogs are treated as regular windows; Normal: dialogs stay on top of document windows; Aggressive: same as Normal but may work better with some window managers."), NULL);
        gtk_widget_set_usize (om, SB_WIDTH + SB_LONG_ADJUSTMENT, -1);
        gtk_widget_show (om);
        gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, SB_MARGIN);

        GtkWidget *m = gtk_menu_new ();
        gtk_widget_show (m);

        int current = prefs_get_int_attribute ("options.transientpolicy", "value", 1);

        {
        const gchar *label = _("None");
        GtkWidget *item = gtk_menu_item_new_with_label (label);
        gtk_signal_connect ( GTK_OBJECT (item), "activate", 
                                 GTK_SIGNAL_FUNC (options_dialogs_ontop_changed),
                                 GINT_TO_POINTER (0) );
        gtk_widget_show (item);
        gtk_menu_append (GTK_MENU (m), item);
        }

        {
        const gchar *label = _("Normal");
        GtkWidget *item = gtk_menu_item_new_with_label (label);
        gtk_signal_connect ( GTK_OBJECT (item), "activate", 
                                 GTK_SIGNAL_FUNC (options_dialogs_ontop_changed),
                                 GINT_TO_POINTER (1) );
        gtk_widget_show (item);
        gtk_menu_append (GTK_MENU (m), item);
        }

        {
        const gchar *label = _("Aggressive");
        GtkWidget *item = gtk_menu_item_new_with_label (label);
        gtk_signal_connect ( GTK_OBJECT (item), "activate", 
                                 GTK_SIGNAL_FUNC (options_dialogs_ontop_changed),
                                 GINT_TO_POINTER (2) );
        gtk_widget_show (item);
        gtk_menu_append (GTK_MENU (m), item);
        }

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
        gtk_option_menu_set_history ( GTK_OPTION_MENU (om), current);
    }

    {
        GtkWidget *l = gtk_label_new (_("Dialogs on top:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }
}


static void
sp_display_dialog_cursor_tolerance_changed (GtkAdjustment *adj, gpointer data)
{
    nr_arena_global_delta = adj->value;
    prefs_set_double_attribute ( "options.cursortolerance", "value", 
                                 nr_arena_global_delta );
} 

static void
options_freehand_tolerance_changed (GtkAdjustment *adj, gpointer data)
{
    prefs_set_double_attribute ("tools.freehand.pencil", "tolerance",  adj->value);
}

static void
options_changed_double (GtkAdjustment *adj, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    prefs_set_double_attribute (prefs_path, "value",  adj->value);
}

static void
options_changed_int (GtkAdjustment *adj, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    prefs_set_int_attribute (prefs_path, "value",  (int) adj->value);
}

static void
options_changed_percent (GtkAdjustment *adj, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    prefs_set_double_attribute (prefs_path, "value",  (adj->value)/100.0);
}

static void
options_changed_boolean (GtkToggleButton *tb, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    const gchar *prefs_attr = (const gchar *) g_object_get_data (G_OBJECT(tb), "attr");
    prefs_set_int_attribute (prefs_path, prefs_attr, gtk_toggle_button_get_active (tb));
}


void 
options_sb (
    gchar const *label, 
    gchar const *tooltip, GtkTooltips *tt,
    gchar const *suffix,
    GtkWidget *box,
    gdouble lower, gdouble upper, gdouble step_increment, gdouble page_increment, gdouble page_size,
    gchar const *prefs_path, gchar const *attr, gdouble def,
    bool isint, bool ispercent,
    void (*changed)(GtkAdjustment *, gpointer)
)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (box), hb, FALSE, FALSE, VB_SKIP);

    {
        GtkWidget *l = gtk_label_new (suffix);
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        gtk_widget_set_usize (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkObject *a = gtk_adjustment_new (0.0, lower, upper, step_increment, page_increment, page_size);

        gdouble value; 
        if (isint)
            if (ispercent) 
                value = 100 * (gdouble) prefs_get_double_attribute_limited (prefs_path, attr, def, lower/100.0, upper/100.0);
            else 
                value = (gdouble) prefs_get_int_attribute_limited (prefs_path, attr, (int) def, (int) lower, (int) upper);
        else 
            value = prefs_get_double_attribute_limited (prefs_path, attr, def, lower, upper);

        gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);

        GtkWidget *sb;
        if (isint) {
            sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
        } else {
            if (step_increment < 0.1)
                sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 3);
            else 
                sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 2);
        }

        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, tooltip, NULL);
        gtk_entry_set_width_chars (GTK_ENTRY (sb), 6);
        gtk_widget_set_usize (sb, SB_WIDTH, -1);
        gtk_widget_show (sb);
        gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, SB_MARGIN);

        gtk_signal_connect (GTK_OBJECT (a), "value_changed", GTK_SIGNAL_FUNC (changed), (gpointer) prefs_path);
    }

    {
        GtkWidget *l = gtk_label_new (label);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }

}


void 
options_checkbox (
    gchar const *label, 
    gchar const *tooltip, GtkTooltips *tt,
    GtkWidget *box,
    gchar const *prefs_path, gchar const *attr, gint def,
    void (*changed)(GtkToggleButton *, gpointer)
)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (box), hb, FALSE, FALSE, VB_SKIP);

    { // empty label for alignment
        GtkWidget *l = gtk_label_new ("");
        gtk_widget_set_usize (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {

        GtkWidget *b  = gtk_check_button_new ();

        gint value = prefs_get_int_attribute (prefs_path, attr, def);

        gtk_toggle_button_set_active ((GtkToggleButton *) b, value != 0);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, tooltip, NULL);

        gtk_widget_set_usize (b, SB_WIDTH, -1);
        gtk_widget_show (b);
        gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, SB_MARGIN);

        g_object_set_data (G_OBJECT(b), "attr", (void *) attr);

        gtk_signal_connect (GTK_OBJECT (b), "toggled", GTK_SIGNAL_FUNC (changed), (gpointer) prefs_path);
    }

    {
        GtkWidget *l = gtk_label_new (label);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }
}

void
selcue_checkbox (GtkWidget *vb, GtkTooltips *tt, const gchar *path)
{
    options_checkbox (
        _("Show selection cue"), 
        _("Whether selected objects display a selection cue (the same as in selector)"), tt,
        vb,
        path, "selcue", 1,
        options_changed_boolean
        );
}

static GtkWidget *
options_dropper ()
{
    GtkWidget *vb, *f, *fb, *b;

    GtkTooltips *tt = gtk_tooltips_new();

    vb = gtk_vbox_new (FALSE, VB_MARGIN);

    {
        f = gtk_frame_new (_("Picking colors:"));
        gtk_widget_show (f);
        gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

        fb = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (fb);
        gtk_container_add (GTK_CONTAINER (f), fb);

        guint pick = prefs_get_int_attribute ("tools.dropper", "pick", 0);

        b = sp_select_context_add_radio (
            NULL, fb, tt, _("Pick visible color (no alpha)"), _("Pick the visible color under cursor, taking into account the page background and disregarding the transparency of objects"), 
            NULL, SP_DROPPER_PICK_VISIBLE, true,
            (pick == SP_DROPPER_PICK_VISIBLE),
            options_dropper_pick_toggled
            );

        b = sp_select_context_add_radio (
            b, fb, tt, _("Pick objects' color (including alpha)"), _("Pick the actual color of object(s) under cursor, including their accumulated transparency"), 
            NULL, SP_DROPPER_PICK_ACTUAL, true,
            (pick == SP_DROPPER_PICK_ACTUAL),
            options_dropper_pick_toggled
            );
    }

    selcue_checkbox (vb, tt, "tools.dropper");

    return vb;
}

GtkWidget *
new_tab (GtkWidget *nb, const gchar *label)
{
     GtkWidget *l = gtk_label_new (label);
     gtk_widget_show (l);
     GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
     gtk_widget_show (vb);
     gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
     gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);
     return vb;
}

void
sp_display_dialog (void)
{

    GtkWidget *nb, *l, *vb, *vbvb, *hb, *om, *m, *i, *frame;

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


// Mouse                                      
        vb = new_tab (nb, _("Mouse"));

        options_sb (
            _("Grab sensitivity:"), 
            _("How close you need to be to an object to be able to grab it with mouse (in pixels)"), tt,
            _("px"),
            vb,
            0.0, 30.0, 1.0, 1.0, 1.0,
            "options.cursortolerance", "value", 8.0,
            true, false,
            sp_display_dialog_cursor_tolerance_changed
            );

        options_sb (
            _("Click/drag threshold:"), 
            _("Maximum mouse drag (in pixels) which is considered a click, not a drag"), tt,
            _("px"),
            vb,
            0.0, 20.0, 1.0, 1.0, 1.0,
            "options.dragtolerance", "value", 4.0,
            true, false,
            options_changed_int
            );


// Scrolling
        vb = new_tab (nb, _("Scrolling"));

        options_sb (
            _("Mouse wheel scrolls by:"), 
            _("One mouse wheel notch scrolls by this distance in pixels (horizontally with Shift)"), tt,
            _("px"),
            vb,
            0.0, 1000.0, 1.0, 1.0, 1.0,
            "options.wheelscroll", "value", 40.0,
            true, false,
            options_changed_int
            );

        frame = gtk_frame_new (_("Ctrl+arrows"));
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, FALSE, 0);
        vbvb = gtk_vbox_new (FALSE, VB_MARGIN);
        gtk_widget_show (vbvb);
        gtk_container_add (GTK_CONTAINER (frame), vbvb);

        options_sb (
            _("Scroll by:"), 
            _("Pressing Ctrl+arrow key scrolls by this distance (in pixels)"), tt,
            _("px"),
            vbvb,
            0.0, 1000.0, 1.0, 1.0, 1.0,
            "options.keyscroll", "value", 10.0,
            true, false,
            options_changed_int
            );

        options_sb (
            _("Acceleration:"), 
            _("Pressing and holding Ctrl+arrow will gradually speed up scrolling (0 for no acceleration)"), tt,
            "",
            vbvb,
            0.0, 5.0, 0.01, 1.0, 1.0,
            "options.scrollingacceleration", "value", 0.35,
            false, false,
            options_changed_double
            );

        frame = gtk_frame_new (_("Autoscrolling"));
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, FALSE, 0);
        vbvb = gtk_vbox_new (FALSE, VB_MARGIN);
        gtk_widget_show (vbvb);
        gtk_container_add (GTK_CONTAINER (frame), vbvb);

        options_sb (
            _("Speed:"), 
            _("How fast the canvas autoscrolls when you drag beyond canvas edge (0 to turn autoscroll off)"), tt,
            "",
            vbvb,
            0.0, 5.0, 0.01, 1.0, 1.0,
            "options.autoscrollspeed", "value", 0.7,
            false, false,
            options_changed_double
            );

        options_sb (
            _("Threshold:"), 
            _("How far (in pixels) you need to be from the canvas edge to trigger autoscroll; positive is outside the canvas, negative is within the canvas"), tt,
            _("px"),
            vbvb,
            -600.0, 600.0, 1.0, 1.0, 1.0,
            "options.autoscrolldistance", "value", -10.0,
            true, false,
            options_changed_int
            );

// Steps
        vb = new_tab (nb, _("Steps"));

        options_sb (
            _("Arrow keys move by:"), 
            _("Pressing an arrow key moves selected object(s) or node(s) by this distance (in points)"), tt,
            _("pt"),
            vb,
            0.0, 3000.0, 0.01, 1.0, 1.0,
            "options.nudgedistance", "value", 2.0,
            false, false,
            options_changed_double
            );

        options_sb (
            _("> and < scale by:"), 
            _("Pressing > or < scales selection up or down by this increment (in points)"), tt,
            _("pt"),
            vb,
            0.0, 3000.0, 0.01, 1.0, 1.0,
            "options.defaultscale", "value", 2.0,
            false, false,
            options_changed_double
            );

        options_sb (
            _("Inset/Outset by:"), 
            _("Inset and Outset commands displace the path by this distance (in points)"), tt,
            _("pt"),
            vb,
            0.0, 3000.0, 0.01, 1.0, 1.0,
            "options.defaultoffsetwidth", "value", 2.0,
            false, false,
            options_changed_double
            );

        options_rotation_steps (vb, tt);

        options_sb (
            _("Zoom in/out by:"), 
            _("Zoom tool click, +/- keys, and middle click zoom in and out by this multiplier"), tt,
            _("%"),
            vb,
            101.0, 500.0, 1.0, 1.0, 1.0,
            "options.zoomincrement", "value", 1.414213562,
            true, true,
            options_changed_percent
            );

// Tools
        vb = new_tab (nb, _("Tools"));

        GtkWidget *nb_tools = gtk_notebook_new ();
        gtk_widget_show (nb_tools);
        gtk_container_add (GTK_CONTAINER (vb), nb_tools);

        // Selector        
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Selector"));

            GtkWidget *selector_page = options_selector ();
            gtk_widget_show (selector_page);
            gtk_container_add (GTK_CONTAINER (vb_tool), selector_page);
        }

        // Node
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Node"));

            selcue_checkbox (vb_tool, tt, "tools.nodes");
        }

        // Zoom
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Zoom"));

            selcue_checkbox (vb_tool, tt, "tools.zoom");
        }

        { // The 4 shape tools
            GtkWidget *vb_shapes = new_tab (nb_tools, _("Shapes"));

            GtkWidget *nb_shapes = gtk_notebook_new ();
            gtk_widget_show (nb_shapes);
            gtk_container_add (GTK_CONTAINER (vb_shapes), nb_shapes);

            // Rect
            {
                GtkWidget *vb_tool = new_tab (nb_shapes, _("Rect"));
            }

            // Ellipse
            {
                GtkWidget *vb_tool = new_tab (nb_shapes, _("Ellipse"));
            }

            // Star
            {
                GtkWidget *vb_tool = new_tab (nb_shapes, _("Star"));
            }

            // Spiral
            {
                GtkWidget *vb_tool = new_tab (nb_shapes, _("Spiral"));
            }

            // common for all shapes
            selcue_checkbox (vb_shapes, tt, "tools.shapes");
        }

        // Freehand
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Pencil"));

            options_sb (
                _("Tolerance:"), 
                _("This value affects the amount of smoothing applied to freehand lines; lower values produce more uneven paths with more nodes"), tt,
                "",
                vb_tool,
                0.0, 100.0, 0.01, 1.0, 1.0,
                "tools.freehand.pencil", "tolerance", 10.0,
                false, false,
                options_freehand_tolerance_changed
                );

            selcue_checkbox (vb_tool, tt, "tools.freehand.pencil");
        }

        // Pen
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Pen"));

            selcue_checkbox (vb_tool, tt, "tools.freehand.pen");
        }

        // Text
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Text"));

            selcue_checkbox (vb_tool, tt, "tools.text");
        }

        // Dropper
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Dropper"));

            GtkWidget *dropper_page = options_dropper ();
            gtk_widget_show (dropper_page);
            gtk_container_add (GTK_CONTAINER (vb_tool), dropper_page);
        }


// Windows
        vb = new_tab (nb, _("Windows"));

        options_dialogs_ontop (vb, tt);

options_checkbox (
    _("Save window geometry"), 
    _("Save the window size and position with each document (only for Inkscape SVG format)"), tt,
    vb,
    "options.savewindowgeometry", "value", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Dialogs are hidden in taskbar"), 
    _("Whether dialog windows are to be hidden in the window manager taskbar"), tt,
    vb,
    "options.dialogsskiptaskbar", "value", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Zoom when window is resized"), 
    _("Zoom drawing when document window is resized, to keep the same area visible (this is the default which can be changed in any window using the button above the right scrollbar)"), tt,
    vb,
    "options.stickyzoom", "value", 0,
    options_changed_boolean
    );


// Clones
        vb = new_tab (nb, _("Clones"));

        // Clone compensation
        {
            GtkWidget *f = gtk_frame_new (_("When the original moves, its clones:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

            gint compense = prefs_get_int_attribute ("options.clonecompensation", "value", SP_CLONE_COMPENSATION_PARALLEL);

            GtkWidget *b = 
            sp_select_context_add_radio (
                NULL, fb, tt, _("Move in parallel"), _("Clones are translated by the same vector as their original."), NULL, SP_CLONE_COMPENSATION_PARALLEL, true,
                compense == SP_CLONE_COMPENSATION_PARALLEL,
                options_clone_compensation_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Stay unmoved"), _("Clones preserve their positions when their original is moved."), NULL, SP_CLONE_COMPENSATION_UNMOVED, true,
                compense == SP_CLONE_COMPENSATION_UNMOVED,
                options_clone_compensation_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Move according to transform"), _("Each clone moves according to the value of its transform= attribute. For example, a rotated clone will move in a different direction than its original."), NULL, SP_CLONE_COMPENSATION_NONE, true,
                compense == SP_CLONE_COMPENSATION_NONE,
                options_clone_compensation_toggled
                );
        }

        // Original deletion
        {
            GtkWidget *f = gtk_frame_new (_("When the original is deleted, its clones:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

            gint orphans = prefs_get_int_attribute ("options.cloneorphans", "value", SP_CLONE_ORPHANS_UNLINK);

            GtkWidget *b = 
            sp_select_context_add_radio (
                NULL, fb, tt, _("Are unlinked"), _("Orphaned clones are converted to regular objects."), NULL, SP_CLONE_ORPHANS_UNLINK, true,
                orphans == SP_CLONE_ORPHANS_UNLINK,
                options_clone_orphans_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Are deleted"), _("Orphaned clones are deleted along with their original."), NULL, SP_CLONE_ORPHANS_DELETE, true,
                orphans == SP_CLONE_ORPHANS_DELETE,
                options_clone_orphans_toggled
                );

/*
            sp_select_context_add_radio (
                b, fb, tt, _("Ask me"), _("Ask me what to do with the clones when their originals are deleted."), NULL, SP_CLONE_ORPHANS_ASKME, true,
                orphans == SP_CLONE_ORPHANS_ASKME,
                options_clone_orphans_toggled
                );
*/
        }

// Transforms
        vb = new_tab (nb, _("Transforms"));

options_checkbox (
    _("Scale stroke width"), 
    _("When scaling objects, scale the stroke width by the same proportion"), tt,
    vb,
    "options.transform", "stroke", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Scale rounded corners in rects"), 
    _("When scaling rectangles, scale the radii of rounded corners"), tt,
    vb,
    "options.transform", "rectcorners", 0,
    options_changed_boolean
    );

options_checkbox (
    _("Transform pattern fill"), 
    _("Apply the transform of an object to its pattern fill"), tt,
    vb,
    "options.transform", "pattern", 1,
    options_changed_boolean
    );

     // Store transformation (global)
        {
            GtkWidget *f = gtk_frame_new (_("Store transformation:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

            gint preserve = prefs_get_int_attribute ("options.preservetransform", "value", 0);

            GtkWidget *b = sp_select_context_add_radio (
                NULL, fb, tt, _("Optimized"), _("If possible, apply transformation to objects without adding a transform= attribute"), NULL, 0, true,
                preserve == 0,
                options_store_transform_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Preserved"), _("Always store transformation as a transform= attribute on objects"), NULL, 1, true,
                preserve != 0,
                options_store_transform_toggled
                );
        }


// To be broken into: Display, Save, Export, SVG, Commands
        vb = new_tab (nb, _("Misc"));

        options_sb (
            _("Default export resolution:"), 
            _("Default bitmap resolution (in dots per inch) in the Export dialog"), tt, // FIXME: add "Used for new exports; once exported, documents remember this value on per-object basis" when implemented
            _("dpi"),
            vb,
            0.0, 6000.0, 1.0, 1.0, 1.0,
            "dialogs.export.defaultxdpi", "value", 72.0,
            true, false,
            options_changed_int
            );

options_checkbox (
    _("Import bitmap as <image>"), 
    _("When on, an imported bitmap creates an <image> element; otherwise it is a rect with bitmap fill"), tt,
    vb,
    "options.importbitmapsasimages", "value", 1,
    options_changed_boolean
    );

        options_sb (
            _("Max recent documents:"), 
            _("The maximum length of the Open Recent list in the File menu"), tt,
            "",
            vb,
            0.0, 1000.0, 1.0, 1.0, 1.0,
            "options.maxrecentdocuments", "value", 20.0,
            true, false,
            options_changed_int
            );

        options_sb (
            _("Simplification threshold:"), 
            _("How strong is the Simplify command by default. If you invoke this command several times in quick succession, it will act more and more aggressively; invoking it again after a pause restores the default threshold."), tt,
            "",
            vb,
            0.0, 1.0, 0.001, 0.01, 0.01,
            "options.simplifythreshold", "value", 0.002,
            false, false,
            options_changed_double
            );

       
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
        gtk_box_pack_start (GTK_BOX (hb), om, FALSE, FALSE, 0);

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
