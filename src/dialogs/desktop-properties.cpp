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

#include <locale>
#include <sstream>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
/*
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
*/

#include "macros.h"
#include "helper/sp-intl.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "helper/window.h"
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
#include "svg/stringstream.h"

#include "rdf.h"

static void sp_dtw_activate_desktop(Inkscape::Application *inkscape,
                                    SPDesktop *desktop, GtkWidget *dialog);

static void sp_dtw_deactivate_desktop(Inkscape::Application *inkscape,
                                      SPDesktop *desktop,
                                      GtkWidget *dialog);

static void sp_dtw_update(GtkWidget *dialog,
                          SPDesktop *desktop);

static GtkWidget *sp_color_picker_new(gchar *colorkey, gchar *alphakey,
                                      gchar *title, guint32 rgba);

static void sp_color_picker_set_rgba32(GtkWidget *cp, guint32 rgba);
static void sp_color_picker_clicked(GObject *cp, void *data);

static void sp_color_picker_button(GtkWidget *dialog, GtkWidget *t,
                                   gchar const *label, gchar *key,
                                   gchar *color_dialog_label,
                                   gchar *opacity_key, int row);

static GtkWidget *dlg = NULL;
static win_data wd;

/* impossible original values to make sure they are read from prefs */
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.documentoptions";

struct inkscape_papers_t {
    gchar const * const name;
    gdouble const height;
    gdouble const width;
};

static inkscape_papers_t const inkscape_papers[] = {
    { "A4", 842, 595 },
    { "US Letter", 792, 612 },
    { "US Legal", 1008, 612 },
    { "US Executive", 720, 542 },
    { "A0", 3368, 2380 },
    { "A1", 2380, 1684 },
    { "A2", 1684, 1190 },
    { "A3", 1190, 842 },
    { "A5", 595, 421 },
    { "A6", 421, 297 },
    { "A7", 297, 210 },
    { "A8", 210, 148 },
    { "A9", 148, 105 },
    { "B0", 4008, 2836 },
    { "B1", 2836, 2004 },
    { "B2", 2004, 1418 },
    { "B3", 1418, 1002 },
    { "B4", 1002, 709 },
    { "B5", 709, 501 },
    { "B6", 501, 355 },
    { "B7", 355, 250 },
    { "B8", 250, 178 },
    { "B9", 178, 125 },
    { "B10", 125, 89 },
    { "CSE", 649, 462 },
    { "Comm10E", 683, 298 },
    { "DLE", 624, 312 },
    { "Folio", 935, 595 },
    { "Ledger", 792, 1224 },
    { "Tabloid", 1225, 792 },
    { NULL, 0, 0 },
};

static void
docoptions_event_attr_changed(SPRepr *, gchar const *, gchar const *, gchar const *, bool, gpointer)
{
    if (dlg) {
        sp_dtw_update(dlg, SP_ACTIVE_DESKTOP);
    }
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
sp_dtw_dialog_destroy(GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data(INKSCAPE, dlg);
    sp_repr_remove_listener_by_data(SP_OBJECT_REPR(SP_ACTIVE_DESKTOP->namedview), dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_dtw_dialog_delete(GtkObject *object, GdkEvent *event, gpointer data)
{
    gtk_window_get_position((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size((GtkWindow *) dlg, &w, &h);

    prefs_set_int_attribute(prefs_path, "x", x);
    prefs_set_int_attribute(prefs_path, "y", y);
    prefs_set_int_attribute(prefs_path, "w", w);
    prefs_set_int_attribute(prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it
}

static void
sp_dtw_whatever_toggled(GtkToggleButton *tb, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dialog), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;

    if (!dt) {
        return;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    SPRepr *repr = SP_OBJECT_REPR(dt->namedview);
    gchar const *key = (gchar const *)gtk_object_get_data(GTK_OBJECT(tb), "key");

    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_boolean(repr, key, gtk_toggle_button_get_active(tb));
    sp_document_set_undo_sensitive(doc, TRUE);
    sp_document_done(doc);
}


static void
sp_dtw_border_layer_toggled(GtkToggleButton *tb, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dialog), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    SPRepr *repr = SP_OBJECT_REPR(dt->namedview);

    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_attr(repr, "borderlayer",
                     gtk_toggle_button_get_active(tb) ? "top" : NULL);

    sp_document_set_undo_sensitive(doc, TRUE);
    sp_document_done(doc);
}


/**
 * \brief  Writes the change into the corresponding attribute of the
 *         sodipodi:namedview element.
 *
 */
static void
sp_dtw_whatever_changed(GtkAdjustment *adjustment, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dialog), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }
    SPDocument *doc = SP_DT_DOCUMENT(dt);

    SPRepr *repr = SP_OBJECT_REPR(dt->namedview);
    gchar const *key = (gchar const *)gtk_object_get_data(GTK_OBJECT(adjustment), "key");
    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(adjustment),
                                                                "unit_selector");

    Inkscape::SVGOStringStream os;
    os << adjustment->value << sp_unit_selector_get_unit(us)->abbr;

    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_attr(repr, key, os.str().c_str());
    sp_document_set_undo_sensitive(doc, TRUE);
    sp_document_done(doc);
}


/**
 *  \brief Exists to stop tabs from working until we just GTK 2.4+
 */
gboolean text_stop_tab (GtkWidget * widget, GdkEventKey *event, gpointer user_data)
{
    if ( event->keyval == GDK_Tab ) return TRUE;
    return FALSE;
}

/**
 *  \brief   Handles a dialog entry box changing and updates the XML
 *  \param   widget  The GtkEntry widget that changed
 *  \param   data    The pointer to the entity
 *
 */
static void
sp_doc_dialog_work_entity_changed ( GtkWidget *widget, gpointer data )
{
    if (!dlg || g_object_get_data(G_OBJECT(dlg), "update")) {
        return;
    }

    struct rdf_work_entity_t * entity = (struct rdf_work_entity_t *)data;
    g_assert ( entity != NULL );

    //printf("signal for '%s' (0x%08x)\n",entity->name,(unsigned int)data);

    GtkWidget *e;
    GtkTextBuffer *buf;
    GtkTextIter start, end;
    gchar const * text = NULL;

    switch ( entity->format ) {
    case RDF_FORMAT_LINE:
        e = GTK_WIDGET (widget);

        text = (gchar const *)gtk_entry_get_text ( GTK_ENTRY (e) );
        break;
    case RDF_FORMAT_MULTILINE:
        buf = GTK_TEXT_BUFFER (widget);

        gtk_text_buffer_get_start_iter(buf, &start);
        gtk_text_buffer_get_end_iter(buf, &end);

        text = (gchar const *)gtk_text_buffer_get_text( buf, &start,
                                                        &end, FALSE );
        break;
    default:
        break;
    }

    if (rdf_set_work_entity( SP_ACTIVE_DOCUMENT, entity, text )) {
        /* if we changed an RDF entity, mark the document as changed */
        sp_document_done ( SP_ACTIVE_DOCUMENT );
    }
}

/**
 *\brief   Writes the change into the corresponding attribute of the document
 *         root (svg element); moved here from the former document settings
 *         dialog.
 *
 */
static void
sp_doc_dialog_whatever_changed(GtkAdjustment *adjustment, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dialog), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    SPRepr *repr = sp_document_repr_root(doc);
    gchar const *key = (gchar const *) gtk_object_get_data(GTK_OBJECT(adjustment), "key");
    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(adjustment),
                                                                "unit_selector");
    SPUnit const *unit = sp_unit_selector_get_unit(us);


    /* SVG does not support meters as a unit, so we must translate meters to
     * cm when writing
     */
    Inkscape::SVGOStringStream os;
    if (!strcmp(unit->abbr, "m")) {
        os << 100*adjustment->value << "cm";
    } else {
        os << adjustment->value << unit->abbr;
        /* FIXME: unit->abbr is a translated string, whereas we want a recognized SVG unit.
         * (Possibly we should just avoid abbr being a translated string.)
         *
         * Also, we usually avoid using units at all in SVG other than in the outermost <svg>
         * element, in line with the recommendation in
         * http://www.w3.org/TR/SVG11/coords.html#Units.
         *
         * Similarly elsewhere in this file: search for abbr.
         */
    }

    sp_repr_set_attr(repr, key, os.str().c_str());

    /* Save this for later
    if (!strcmp(key, "width") || !strcmp(key, "height")) {

        //A short-term hack to set the viewBox to be 1.25 x the size
        // of the page.
        gdouble vbWidth  = (gdouble)g_ascii_strtod(sp_repr_attr(repr, "width"), NULL)  * 1.25;
        gdouble vbHeight = (gdouble)g_ascii_strtod(sp_repr_attr(repr, "height"), NULL) * 1.25;
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os.setf(std::ios::showpoint);
        os.precision(8);
        os << "0 0 " << vbWidth << " " << vbHeight;
        gchar const *strVal = (gchar const *)os.str().c_str();
        sp_repr_set_attr(repr, "viewBox", g_strdup(strVal));

    }
    */
    sp_document_done(doc);
}

static void
sp_dtw_grid_snap_distance_changed(GtkAdjustment *adjustment,
                                  GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dialog), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPRepr *repr = SP_OBJECT_REPR(dt->namedview);

    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(dialog),
                                                                "grid_snap_units");

    Inkscape::SVGOStringStream os;
    os << adjustment->value << sp_unit_selector_get_unit(us)->abbr;

    sp_repr_set_attr(repr, "gridtolerance", os.str().c_str());
    sp_document_done(SP_DT_DOCUMENT(dt));
}


static void
sp_dtw_guides_snap_distance_changed(GtkAdjustment *adjustment,
                                    GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dialog), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPRepr *repr = SP_OBJECT_REPR(dt->namedview);

    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(dialog),
                                                                "guide_snap_units");

    Inkscape::SVGOStringStream os;
    os << adjustment->value << sp_unit_selector_get_unit(us)->abbr;

    sp_repr_set_attr(repr, "guidetolerance", os.str().c_str());
}



static void
sp_doc_dialog_paper_selected(GtkWidget *widget, gpointer data)
{
    GtkWidget *ww, *hw, *om;
    gint landscape;
    double h, w;
    GtkAdjustment *aw, *ah;
    SPUnitSelector *us;

    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    struct inkscape_papers_t const *paper = (struct inkscape_papers_t const *) data;

    ww = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dlg), "widthsb");
    gtk_widget_set_sensitive(ww, FALSE);
    hw = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dlg), "heightsb");
    gtk_widget_set_sensitive(hw, FALSE);

    om = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dlg), "orientation");
    landscape = gtk_option_menu_get_history(GTK_OPTION_MENU(om));

    us = (SPUnitSelector *)gtk_object_get_data(GTK_OBJECT(dlg), "units");
    SPUnit const *unit = sp_unit_selector_get_unit(us);
    aw = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "width");
    ah = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "height");

    if (paper) {
        if (!landscape) {
            w = paper->width;
            h = paper->height;
        } else {
            w = paper->height;
            h = paper->width;
        }
        SPUnit const * const pt = &sp_unit_get_by_id(SP_UNIT_PT);
        sp_convert_distance(&w, pt, unit);
        gtk_adjustment_set_value(aw, w);
        sp_convert_distance(&h, pt, unit);
        gtk_adjustment_set_value(ah, h);
    } else {
        gtk_widget_set_sensitive(ww, TRUE);
        gtk_widget_set_sensitive(hw, TRUE);
    }

    if (!SP_ACTIVE_DESKTOP) {
        return;
    }

    sp_document_done(SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP));
}

static void
sp_doc_dialog_paper_orientation_selected(GtkWidget *widget, gpointer data)
{
    gdouble w, h, t;
    GtkAdjustment *aw, *ah;

    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    aw = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "width");
    ah = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "height");

    w = gtk_adjustment_get_value(aw);
    h = gtk_adjustment_get_value(ah);

    /* only toggle when we actually swap */
    GtkWidget *om = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dlg), "orientation");
    gint landscape = gtk_option_menu_get_history(GTK_OPTION_MENU(om));

    if ((w > h && !landscape) ||
        (w < h &&  landscape)   )
    {
        t = w; w = h; h = t;
    }

    gtk_adjustment_set_value(aw, w);
    gtk_adjustment_set_value(ah, h);

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(FALSE));
}

/**
 *\brief   Creates a data entry widget in a table for a given RDF entity.
 *
 */
static GtkWidget *
sp_doc_dialog_add_work_entity( struct rdf_work_entity_t * entity,
                                GtkWidget * t, GtkTooltips * tt, int row )
{
    g_assert ( entity != NULL );
    g_assert ( t != NULL );
    g_assert ( tt != NULL );

    GtkWidget * packable = NULL;  /* Widget to pack into the table */
    GtkWidget * interface = NULL; /* Widget we interact with */
    GObject * changable = NULL;   /* Object that gets "change" notices */

    if (dlg) {
        GtkWidget *l = gtk_label_new (entity->title);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_table_attach( GTK_TABLE(t), l, 0, 1, row, row+1,
                          (GtkAttachOptions)( GTK_SHRINK ),
                          (GtkAttachOptions)0, 0, 0 );

        GtkWidget * e, * scroller, * view;
        GtkTextBuffer * buf;

        switch ( entity->format ) {
        case RDF_FORMAT_LINE:
            // single line entry
            e = gtk_entry_new ();
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), e, entity->tip, NULL );
            gtk_widget_show (e);
            gtk_object_set_data (GTK_OBJECT (dlg), entity->name, e);

            packable = GTK_WIDGET (e);
            interface = GTK_WIDGET (e);
            changable = G_OBJECT (e);
            break;
        case RDF_FORMAT_MULTILINE:
            scroller = gtk_scrolled_window_new ( NULL, NULL );
            gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (scroller),
                                             GTK_POLICY_AUTOMATIC,
                                             GTK_POLICY_AUTOMATIC );
            gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroller),
                                                  GTK_SHADOW_IN );
            gtk_widget_show (scroller);

            view = gtk_text_view_new ();
            gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (view), GTK_WRAP_WORD );
// FIXME: available from GTK 2.4 on...
//            gtk_text_view_set_accepts_tab ( GTK_TEXT_VIEW (view), FALSE );
//          until then, just kill tabs...
            g_signal_connect ( G_OBJECT (view), "key-press-event",
                               G_CALLBACK (text_stop_tab), NULL );

            buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
            // FIXME: looks like tool tips don't show up for GtkTextViews
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), scroller, entity->tip, NULL );
            gtk_widget_show (view);
            gtk_container_add ( GTK_CONTAINER (scroller), view );

            packable = GTK_WIDGET (scroller);
            interface = GTK_WIDGET (view);
            changable = G_OBJECT (buf);
            break;
        default:
            break;
        }

        g_assert ( packable != NULL );
        g_assert ( interface != NULL );
        g_assert ( changable != NULL );

        gtk_object_set_data ( GTK_OBJECT (dlg), entity->name, interface);

        g_signal_connect( G_OBJECT(changable), "changed",
                          G_CALLBACK(sp_doc_dialog_work_entity_changed),
                          (gpointer)(entity));

        gtk_table_attach( GTK_TABLE(t), packable, 1, 2, row, row+1,
                          (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                          (GtkAttachOptions)0, 0, 0 );
    }

    return interface;
}

/*
 * \brief   Populates the GUI text entries with RDF data from the document.
 *
 */
static void
sp_doc_dialog_update_work_entity( struct rdf_work_entity_t * entity )
{
    if (dlg) {
        g_assert ( entity != NULL );

        GtkWidget *e, *view;
        GtkTextBuffer *buf;

        const gchar * text = rdf_get_work_entity( SP_ACTIVE_DOCUMENT, entity );
    
        switch ( entity->format ) {
        case RDF_FORMAT_LINE:
            e = (GtkWidget *)g_object_get_data(G_OBJECT(dlg), entity->name);
            gtk_entry_set_text ( GTK_ENTRY (e), text ? text : "" );
            break;
        case RDF_FORMAT_MULTILINE:
            view = (GtkWidget *)g_object_get_data(G_OBJECT(dlg), entity->name);
            buf = gtk_text_view_get_buffer ( GTK_TEXT_VIEW (view) );
            gtk_text_buffer_set_text ( buf, text ? text : "", -1 );
            break;
        default:
            break;
        }
    } 
}

/**
 *\brief   Makes changes to the GUI for a given license entity
 *
 */
static void
sp_doc_dialog_license_update ( struct rdf_work_entity_t * entity,
                               gchar const * text,
                               bool editable )
{
        g_assert ( entity != NULL );
        g_assert ( dlg != NULL );

        /* find widget */
        GtkWidget *w = (GtkWidget*)g_object_get_data ( G_OBJECT(dlg),
                                                       entity->name );
        g_assert ( w != NULL );

        /*
        printf("\tedit: %s '%s' -> '%s'\n", editable ? "yes" : "no",
                                            entity->name,text);
        */

        if (!editable) {
            /* must mark as update or bad things happen */
            gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(TRUE));
            /* load license info info RDF */
            rdf_set_work_entity( SP_ACTIVE_DOCUMENT, entity, text );
            /* update GUI */
            sp_doc_dialog_update_work_entity( entity );
            gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(FALSE));
        }
        gtk_widget_set_sensitive ( GTK_WIDGET (w), editable );
}

/**
 *\brief   Notices changed license pulldown and changes RDF entries.
 *
 */
static void
sp_doc_dialog_license_selected ( GtkWidget *widget, gpointer data )
{
    struct rdf_license_t * license = (struct rdf_license_t*) data;

    if (!dlg || g_object_get_data(G_OBJECT(dlg), "update")) {
        return;
    }

    /*
    if (license) {
        printf("selected license '%s'\n",license->name);
    }
    else {
        printf("selected license 'Proprietary'\n");
    }
    */

    sp_doc_dialog_license_update ( rdf_find_entity( "license_uri" ),
                                   license ? license->uri : NULL,
                                   license ? FALSE : TRUE ); 
    /*
    sp_doc_dialog_license_update ( rdf_find_entity( "license_fragment" ),
                                   license ? license->fragment : NULL,
                                   license ? FALSE : TRUE ); 
                                   */

    rdf_set_license ( SP_ACTIVE_DOCUMENT, license );

    sp_document_done ( SP_ACTIVE_DOCUMENT );
}

/*
 * \brief   Creates the desktop properties dialog
 *
 */
void
sp_desktop_dialog(void)
{
    if (!dlg) {
        gchar title[500];
        sp_ui_dialog_title_string(SP_VERB_DIALOG_NAMEDVIEW, title);

        dlg = sp_window_new(title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute(prefs_path, "x", 0);
            y = prefs_get_int_attribute(prefs_path, "y", 0);
        }

        if (w == 0 || h == 0) {
            w = prefs_get_int_attribute(prefs_path, "w", 0);
            h = prefs_get_int_attribute(prefs_path, "h", 0);
        }

        if (x != 0 || y != 0) {
            gtk_window_move((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }

        if (w && h) {
            gtk_window_resize((GtkWindow *) dlg, w, h);
        }

        sp_transientize(dlg);
        wd.win = dlg;
        wd.stop = 0;

        g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK(sp_transientize_callback), &wd);

        gtk_signal_connect(GTK_OBJECT(dlg), "event", GTK_SIGNAL_FUNC(sp_dialog_event_handler), dlg);

        gtk_signal_connect(GTK_OBJECT(dlg), "destroy", G_CALLBACK(sp_dtw_dialog_destroy), dlg);
        gtk_signal_connect(GTK_OBJECT(dlg), "delete_event", G_CALLBACK(sp_dtw_dialog_delete), dlg);
        g_signal_connect(G_OBJECT(INKSCAPE), "shut_down", G_CALLBACK(sp_dtw_dialog_delete), dlg);

        g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(sp_dialog_hide), dlg);
        g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(sp_dialog_unhide), dlg);

        GtkWidget *nb = gtk_notebook_new();
        gtk_widget_show(nb);
        gtk_container_add(GTK_CONTAINER(dlg), nb);


        /* Grid settings */

        GCallback cb = G_CALLBACK(sp_dtw_whatever_toggled);

        /* Notebook tab */
        GtkWidget *l = gtk_label_new(_("Grid"));
        gtk_widget_show(l);
        GtkWidget *v = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(v);
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), v, l);


        /* Checkbuttons */
        spw_vbox_checkbutton(dlg, v, _("Show grid"), "showgrid", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap to grid"), "snaptogrid", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap bounding boxes to grid"), "inkscape:grid-bbox", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap points to grid"), "inkscape:grid-points", cb);

        /*   Commenting out until Nathan implements the grids -- bryce
         *   spw_checkbutton(dlg, t, _("Iso grid"), "isogrid", 0, row, 0, cb);
         *   spw_checkbutton(dlg, t, _("Hex grid"), "hexgrid", 1, row++, 0, cb);
         */

        GtkWidget *t = gtk_table_new(8, 2, FALSE);
        gtk_widget_show(t);
        gtk_container_set_border_width(GTK_CONTAINER(t), 4);
        gtk_table_set_row_spacings(GTK_TABLE(t), 4);
        gtk_table_set_col_spacings(GTK_TABLE(t), 4);
        gtk_box_pack_start(GTK_BOX(v), t, TRUE, TRUE, 0);

        cb = G_CALLBACK(sp_dtw_whatever_changed);

        GtkWidget *us = sp_unit_selector_new(SP_UNIT_ABSOLUTE);
        int row = 0;
        spw_dropdown(dlg, t, _("Grid units:"), "grid_units", row++, us);

        spw_unit_selector(dlg, t, _("Origin X:"), "gridoriginx",
                          row++, us, cb);

        spw_unit_selector(dlg, t, _("Origin Y:"), "gridoriginy",
                          row++, us, cb);

        spw_unit_selector(dlg, t, _("Spacing X:"), "gridspacingx",
                          row++, us, cb);

        spw_unit_selector(dlg, t, _("Spacing Y:"), "gridspacingy",
                          row++, us, cb);

        us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        spw_dropdown(dlg, t, _("Snap units:"), "grid_snap_units",
                     row++, us);

        spw_unit_selector(dlg, t, _("Snap distance:"), "gridtolerance", row++, us,
                          G_CALLBACK(sp_dtw_grid_snap_distance_changed));

        sp_color_picker_button(dlg, t, _("Grid color:"), "gridcolor",
                               _("Grid color"), "gridhicolor", row++);

        /* Guidelines page */

        l = gtk_label_new(_("Guides"));
        gtk_widget_show(l);
        v = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(v);
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), v, l);

        /* Checkbuttons */
        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_vbox_checkbutton(dlg, v, _("Show guides"), "showguides", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap to guides"), "snaptoguides", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap bounding boxes to guides"), "inkscape:guide-bbox", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap points to guides"), "inkscape:guide-points", cb);

        t = gtk_table_new(4, 2, FALSE);
        gtk_widget_show(t);
        gtk_container_set_border_width(GTK_CONTAINER(t), 4);
        gtk_table_set_row_spacings(GTK_TABLE(t), 4);
        gtk_table_set_col_spacings(GTK_TABLE(t), 4);
        gtk_box_pack_start(GTK_BOX(v), t, TRUE, TRUE, 0);

        row = 0;
        us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        spw_dropdown(dlg, t, _("Snap units:"), "guide_snap_units",
                     row++, us);

        spw_unit_selector(dlg, t, _("Snap distance:"), "guidetolerance",
                          row++, us,
                          G_CALLBACK(sp_dtw_guides_snap_distance_changed));

        sp_color_picker_button(dlg, t, _("Guide color:"), "guidecolor",
                               _("Guideline color"), "guideopacity", row++);

        sp_color_picker_button(dlg, t, _("Highlight color:"), "guidehicolor",
                               _("Highlighted guideline color"),
                               "guidehiopacity", row++);

        row=0;
        /* Page page */
        l = gtk_label_new(_("Page"));
        gtk_widget_show(l);
        t = gtk_table_new(1, 5, FALSE);
        gtk_widget_show(t);
        gtk_container_set_border_width(GTK_CONTAINER(t), 6);
        gtk_table_set_row_spacings(GTK_TABLE(t), 6);
        gtk_notebook_prepend_page(GTK_NOTEBOOK(nb), t, l);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), 0);

        sp_color_picker_button(dlg, t, _("Background (also for export):"),
                               "pagecolor", _("Background color"),
                               "inkscape:pageopacity", 0);

        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_checkbutton(dlg, t, _("Show canvas border"),
                        "showborder", 0, 1, 0, cb);

        GtkWidget *b = gtk_check_button_new_with_label(_("Border on top of drawing"));
        gtk_widget_show(b);
        gtk_table_attach(GTK_TABLE(t), b, 0, 2, 2, 3,
                         (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                         (GtkAttachOptions)0, 0, 0);

        gtk_object_set_data(GTK_OBJECT(dlg), "borderlayer", b);
        g_signal_connect(G_OBJECT(b), "toggled",
                         G_CALLBACK(sp_dtw_border_layer_toggled), dlg);

        sp_color_picker_button(dlg, t, _("Border color:"),
                               "bordercolor", _("Canvas border color"),
                               "borderopacity", 4);


        // The following comes from the former "document settings" dialog

        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
        gtk_widget_show(vb);
        gtk_table_attach(GTK_TABLE(t), vb, 0, 2, 5, 6,
                         (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                         (GtkAttachOptions)0, 0, 0);

        GtkWidget *hb = gtk_hbox_new(FALSE, 4);
        gtk_widget_show(hb);
        gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new(_("Paper size:"));
        gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
        gtk_widget_show(l);
        gtk_box_pack_start(GTK_BOX(hb), l, FALSE, FALSE, 0);
        GtkWidget *om = gtk_option_menu_new();
        gtk_widget_show(om);
        gtk_box_pack_start(GTK_BOX(hb), om, TRUE, TRUE, 0);
        gtk_object_set_data(GTK_OBJECT(dlg), "papers", om);

        GtkWidget *m = gtk_menu_new();
        gtk_widget_show(m);

        GtkWidget *i;
        for (struct inkscape_papers_t const *paper = inkscape_papers;
             paper && paper->name;
             paper++) {
            i = gtk_menu_item_new_with_label(paper->name);
            gtk_widget_show(i);
            g_signal_connect(G_OBJECT(i), "activate",
                             G_CALLBACK(sp_doc_dialog_paper_selected),
                             (gpointer) paper);
            gtk_menu_append(GTK_MENU(m), i);
        }

        i = gtk_menu_item_new_with_label(_("Custom"));
        gtk_widget_show(i);
        g_signal_connect(G_OBJECT(i), "activate",
                         G_CALLBACK(sp_doc_dialog_paper_selected), NULL);
        gtk_menu_prepend(GTK_MENU(m), i);
        gtk_option_menu_set_menu(GTK_OPTION_MENU(om), m);

        hb = gtk_hbox_new(FALSE, 4);
        gtk_widget_show(hb);
        gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new(_("Paper orientation:"));
        gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
        gtk_widget_show(l);
        gtk_box_pack_start(GTK_BOX(hb), l, FALSE, FALSE, 0);
        om = gtk_option_menu_new();
        gtk_widget_show(om);
        gtk_box_pack_start(GTK_BOX(hb), om, TRUE, TRUE, 0);
        gtk_object_set_data(GTK_OBJECT(dlg), "orientation", om);

        m = gtk_menu_new();
        gtk_widget_show(m);

        i = gtk_menu_item_new_with_label(_("Landscape"));
        gtk_widget_show(i);
        g_signal_connect(G_OBJECT(i), "activate",
                         G_CALLBACK(sp_doc_dialog_paper_orientation_selected), NULL);
        gtk_menu_prepend(GTK_MENU(m), i);

        i = gtk_menu_item_new_with_label(_("Portrait"));
        gtk_widget_show(i);
        g_signal_connect(G_OBJECT(i), "activate",
                         G_CALLBACK(sp_doc_dialog_paper_orientation_selected), NULL);
        gtk_menu_prepend(GTK_MENU(m), i);

        gtk_option_menu_set_menu(GTK_OPTION_MENU(om), m);

        /* Custom paper frame */
        GtkWidget *f = gtk_frame_new(_("Custom paper"));
        gtk_widget_show(f);
        gtk_box_pack_start(GTK_BOX(vb), f, FALSE, FALSE, 0);

        GtkWidget *tt = gtk_table_new(4, 2, FALSE);
        gtk_widget_show(tt);
        gtk_container_set_border_width(GTK_CONTAINER(tt), 4);
        gtk_table_set_row_spacings(GTK_TABLE(tt), 4);
        gtk_table_set_col_spacings(GTK_TABLE(tt), 4);
        gtk_container_add(GTK_CONTAINER(f), tt);

        us = sp_unit_selector_new(SP_UNIT_ABSOLUTE);
        spw_dropdown(dlg, tt, _("Units:"), "units", 0, us);

        l = gtk_label_new(_("Width:"));
        gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
        gtk_widget_show(l);
        gtk_table_attach( GTK_TABLE(tt), l, 0, 1, 1, 2,
                          (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                          (GtkAttachOptions)0, 0, 0);
        GtkObject *a = gtk_adjustment_new(0.0, 1e-6, 1e6, 1.0, 10.0, 10.0);
        gtk_object_set_data(GTK_OBJECT(a), "key", (void *)"width");
        gtk_object_set_data(GTK_OBJECT(a), "unit_selector", us);
        gtk_object_set_data(GTK_OBJECT(dlg), "width", a);
        sp_unit_selector_add_adjustment(SP_UNIT_SELECTOR(us),
                                        GTK_ADJUSTMENT(a));
        GtkWidget *sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), 1.0, 2);
        gtk_widget_show(sb);
        gtk_table_attach(GTK_TABLE(tt), sb, 1, 2, 1, 2,
                         (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                         (GtkAttachOptions)0, 0, 0);
        gtk_object_set_data(GTK_OBJECT(dlg), "widthsb", sb);
        g_signal_connect(G_OBJECT(a), "value_changed",
                         G_CALLBACK(sp_doc_dialog_whatever_changed), dlg);


        l = gtk_label_new(_("Height:"));
        gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
        gtk_widget_show(l);
        gtk_table_attach( GTK_TABLE(tt), l, 0, 1, 2, 3,
                          (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                          (GtkAttachOptions)0, 0, 0 );
        a = gtk_adjustment_new(0.0, 1e-6, 1e6, 1.0, 10.0, 10.0);
        gtk_object_set_data(GTK_OBJECT(a), "key", (void *)"height");
        gtk_object_set_data(GTK_OBJECT(a), "unit_selector", us);
        gtk_object_set_data(GTK_OBJECT(dlg), "height", a);
        sp_unit_selector_add_adjustment( SP_UNIT_SELECTOR(us),
                                         GTK_ADJUSTMENT(a));
        sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), 1.0, 2);
        gtk_widget_show(sb);
        gtk_table_attach( GTK_TABLE(tt), sb, 1, 2, 2, 3,
                          (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                          (GtkAttachOptions)0, 0, 0 );
        gtk_object_set_data(GTK_OBJECT(dlg), "heightsb", sb);
        g_signal_connect( G_OBJECT(a), "value_changed",
                          G_CALLBACK(sp_doc_dialog_whatever_changed), dlg );

        // end of former "document settings" stuff

        /*
         * Ownership metadata tab
         */
        l = gtk_label_new (_("Metadata"));
        gtk_widget_show (l);
        t = gtk_table_new (5, 2, FALSE);
        gtk_widget_show (t);
        gtk_container_set_border_width (GTK_CONTAINER (t), 4);
        gtk_table_set_row_spacings (GTK_TABLE (t), 4);
        gtk_table_set_col_spacings (GTK_TABLE (t), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), t, l);

        row=0;
        /* add generic metadata entry areas */
        GtkTooltips * tip = gtk_tooltips_new ();
        struct rdf_work_entity_t * entity;
        for (entity = rdf_work_entities; entity && entity->name; entity++) {
            if ( entity->editable == RDF_EDIT_GENERIC ) {
                sp_doc_dialog_add_work_entity ( entity, t, tip, row++ );
            }
        }

        /* add license selector pull-down */
        f = gtk_frame_new(_("License"));
        gtk_widget_show(f);

        gtk_table_attach ( GTK_TABLE (t), f, 0, 2, row, row+1, 
                           (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                           (GtkAttachOptions)0, 0, 0 );
        row++;

        vb = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vb);
        gtk_container_add(GTK_CONTAINER(f), vb);

        om = gtk_option_menu_new ();
        gtk_widget_show (om);
        gtk_box_pack_start (GTK_BOX (vb), om, TRUE, TRUE, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "licenses", om);

        m = gtk_menu_new ();
        gtk_widget_show (m);

        for (struct rdf_license_t * license = rdf_licenses;
             license && license->name;
             license++) {
            i = gtk_menu_item_new_with_label (license->name);
            gtk_widget_show (i);
            g_signal_connect ( G_OBJECT (i), "activate",
                G_CALLBACK (sp_doc_dialog_license_selected),
                (gpointer)(license));
            gtk_menu_append (GTK_MENU (m), i);
        }
        i = gtk_menu_item_new_with_label (_("Proprietary"));
        gtk_widget_show (i);
        g_signal_connect ( G_OBJECT (i), "activate", 
                           G_CALLBACK (sp_doc_dialog_license_selected), NULL);
        gtk_menu_prepend (GTK_MENU (m), i);
        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);

        t = gtk_table_new (5, 2, FALSE);
        gtk_widget_show (t);
        gtk_container_set_border_width (GTK_CONTAINER (t), 4);
        gtk_table_set_row_spacings (GTK_TABLE (t), 4);
        gtk_table_set_col_spacings (GTK_TABLE (t), 4);

        gtk_box_pack_start (GTK_BOX (vb), t, TRUE, TRUE, 0);
        
        row = 0;
        /* add license-specific metadata entry areas */
        entity = rdf_find_entity ( "license_uri" );
        GtkWidget * w = sp_doc_dialog_add_work_entity ( entity, t, tip, row++ );
        gtk_widget_set_sensitive ( w, FALSE );
        /*
        entity = rdf_find_entity ( "license_fragment" );
        w = sp_doc_dialog_add_work_entity ( entity, t, tip, row++ );
        gtk_widget_set_sensitive ( w, FALSE );
        */

        // end "Metadata" tab


        g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop",
                          G_CALLBACK(sp_dtw_activate_desktop), dlg);

        g_signal_connect( G_OBJECT(INKSCAPE), "deactivate_desktop",
                          G_CALLBACK(sp_dtw_deactivate_desktop), dlg);
        sp_dtw_update(dlg, SP_ACTIVE_DESKTOP);

        sp_repr_add_listener(SP_OBJECT_REPR(SP_ACTIVE_DESKTOP->namedview), &docoptions_repr_events, dlg);

    } // end of if (!dlg)

    gtk_window_present((GtkWindow *) dlg);


} // end of sp_desktop_dialog(void)


static void
sp_dtw_activate_desktop(Inkscape::Application *inkscape,
                        SPDesktop *desktop,
                        GtkWidget *dialog)
{
    if (desktop && dlg) {
        sp_repr_add_listener(SP_OBJECT_REPR(desktop->namedview), &docoptions_repr_events, dlg);
    }
    sp_dtw_update(dialog, desktop);
}



static void
sp_dtw_deactivate_desktop(Inkscape::Application *inkscape,
                          SPDesktop *desktop,
                          GtkWidget *dialog)
{
    if (desktop && SP_IS_DESKTOP(desktop) && SP_IS_NAMEDVIEW(desktop->namedview) && dlg) {
        // all these checks prevent crash when you close inkscape with the dialog open
        sp_repr_remove_listener_by_data(SP_OBJECT_REPR(desktop->namedview), dlg);
    }
    sp_dtw_update(dialog, NULL);
}

static void
sp_dtw_update(GtkWidget *dialog, SPDesktop *desktop)
{
    if (!desktop) {
        gtk_widget_set_sensitive(dialog, FALSE);
        GObject *cp = (GObject *)g_object_get_data(G_OBJECT(dialog), "gridcolor");
        GObject *w = (GObject *)g_object_get_data(cp, "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), FALSE);
        }
        cp = (GObject *)g_object_get_data(G_OBJECT(dialog), "guidecolor");
        w = (GObject *)g_object_get_data(cp, "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), FALSE);
        }
        cp = (GObject *)g_object_get_data(G_OBJECT(dialog), "guidecolor");
        w = (GObject *)g_object_get_data(cp, "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), FALSE);
        }
    } else {
        SPNamedView *nv = desktop->namedview;

        gtk_object_set_data(GTK_OBJECT(dialog), "update", GINT_TO_POINTER(TRUE));
        gtk_widget_set_sensitive(dialog, TRUE);

        GtkObject *o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "showgrid");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->showgrid);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "snaptogrid");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->grid_snapper.getEnabled());

        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:grid-bbox");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->grid_snapper.getSnapTo(Snapper::BBOX_POINT));
        gtk_widget_set_sensitive(GTK_WIDGET(o), nv->grid_snapper.getEnabled());
        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:grid-points");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->grid_snapper.getSnapTo(Snapper::SNAP_POINT));
        gtk_widget_set_sensitive(GTK_WIDGET(o), nv->grid_snapper.getEnabled());

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "grid_units");
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(o), nv->gridunit);

        SPUnit const * const pt = &sp_unit_get_by_id(SP_UNIT_PT);
        gdouble val = nv->gridorigin[NR::X];
        sp_convert_distance(&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridoriginx");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);
        val = nv->gridorigin[NR::Y];
        sp_convert_distance(&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridoriginy");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);
        val = nv->gridspacing[NR::X];
        sp_convert_distance(&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridspacingx");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);
        val = nv->gridspacing[NR::Y];
        sp_convert_distance(&val, pt, nv->gridunit);
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridspacingy");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "grid_snap_units");
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(o), nv->gridtoleranceunit);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridtolerance");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), nv->gridtolerance);

        GtkWidget *cp = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dialog), "gridcolor");
        sp_color_picker_set_rgba32(cp, nv->gridcolor);
        GtkWidget *w = (GtkWidget *)g_object_get_data(G_OBJECT(cp), "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "showguides");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->showguides);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "snaptoguides");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->guide_snapper.getEnabled());

        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:guide-bbox");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->guide_snapper.getSnapTo(Snapper::BBOX_POINT));
        gtk_widget_set_sensitive(GTK_WIDGET(o), nv->guide_snapper.getEnabled());
        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:guide-points");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->guide_snapper.getSnapTo(Snapper::SNAP_POINT));
        gtk_widget_set_sensitive(GTK_WIDGET(o), nv->guide_snapper.getEnabled());

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "guide_snap_units");
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(o), nv->guidetoleranceunit);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "guidetolerance");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), nv->guidetolerance);

        cp = (GtkWidget *)g_object_get_data(G_OBJECT(dialog), "guidecolor");
        sp_color_picker_set_rgba32(cp, nv->guidecolor);
        w = (GtkWidget *)g_object_get_data(G_OBJECT(cp), "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        cp = (GtkWidget *)g_object_get_data(G_OBJECT(dialog), "guidehicolor");
        sp_color_picker_set_rgba32(cp, nv->guidehicolor);
        w = (GtkWidget *)g_object_get_data(G_OBJECT(cp), "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "showborder");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->showborder);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "borderlayer");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), (nv->borderlayer == SP_BORDER_LAYER_TOP));

        cp = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dialog), "bordercolor"));
        sp_color_picker_set_rgba32(cp, nv->bordercolor);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(cp), "window"));
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        cp = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dialog), "pagecolor"));
        sp_color_picker_set_rgba32(cp, nv->pagecolor);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(cp), "window"));
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        // Start of former "document settings" stuff

        gtk_object_set_data(GTK_OBJECT(dialog), "update", GINT_TO_POINTER(TRUE));
        gtk_widget_set_sensitive(dialog, TRUE);

        gdouble docw = sp_document_width(SP_DT_DOCUMENT(desktop));
        gdouble doch = sp_document_height(SP_DT_DOCUMENT(desktop));


        GtkWidget *ww = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dialog), "widthsb");
        GtkWidget *hw = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dialog), "heightsb");
        GtkWidget *pm = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dialog), "papers");
        GtkWidget *om = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dialog), "orientation");

        /* select paper orientation */
        gdouble ph, pw;
        if (docw > doch) {
            /* wider than tall: landscape */
            gtk_option_menu_set_history(GTK_OPTION_MENU(om), 1);
            ph = docw;
            pw = doch;
        }
        else {
            /* taller than wide: portrait */
            gtk_option_menu_set_history(GTK_OPTION_MENU(om), 0);
            ph = doch;
            pw = docw;
        }
        //printf("have ph: %f pw: %f\n",ph,pw);

        /* find matching paper size */
        gint pos;
        struct inkscape_papers_t const *paper;
        for (paper = inkscape_papers, pos=1;
             paper && paper->name;
             paper++, pos++) {
            //printf("checking %s (%fx%f)\n",paper->name,paper->height,paper->width);
            if (paper->width == pw
                && paper->height == ph)
            {
                //printf("matched\n");
                break;
            }
        }
        if (paper && paper->name) {
            //printf("using pos %d (%s)\n",pos,paper->name);
            gtk_option_menu_set_history(GTK_OPTION_MENU(pm), pos);
            gtk_widget_set_sensitive(ww, FALSE);
            gtk_widget_set_sensitive(hw, FALSE);
        } else {
            //printf("using custom\n");
            gtk_option_menu_set_history(GTK_OPTION_MENU(pm), 0);
            gtk_widget_set_sensitive(ww, TRUE);
            gtk_widget_set_sensitive(hw, TRUE);
        }

        SPUnitSelector *us = (SPUnitSelector *)gtk_object_get_data(GTK_OBJECT(dialog), "units");
        SPUnit const *unit = sp_unit_selector_get_unit(us);
        GtkAdjustment *a = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dialog), "width");
        sp_convert_distance(&docw, pt, unit);
        gtk_adjustment_set_value(a, docw);
        a = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dialog), "height");
        sp_convert_distance(&doch, pt, unit);
        gtk_adjustment_set_value(a, doch);

        // end of "document settings" stuff

        /* load the RDF entities */
        for (struct rdf_work_entity_t * entity = rdf_work_entities;
             entity && entity->name; entity++) {
            if ( entity->editable == RDF_EDIT_GENERIC ) {
                sp_doc_dialog_update_work_entity( entity );
            }
        }
        /* identify the license info */
        struct rdf_license_t * license = rdf_get_license ( SP_ACTIVE_DOCUMENT );

        GtkWidget *lm = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dialog),
                                                         "licenses");
        for (int i=0; rdf_licenses[i].name; i++) {
            if (license == &rdf_licenses[i]) {
                gtk_option_menu_set_history(GTK_OPTION_MENU(lm), i+1);
                break;
            }
        }

        gtk_object_set_data(GTK_OBJECT(dialog), "update", GINT_TO_POINTER(FALSE));

        /* update the license info outside of the "update"=1 area */
        sp_doc_dialog_license_selected ( NULL, license );
    }
}


static void
sp_color_picker_button(GtkWidget *dialog, GtkWidget *t,
                       gchar const *label, gchar *key,
                       gchar *color_dialog_label,
                       gchar *opacity_key,
                       int row)
{
    GtkWidget *l = gtk_label_new(label);
    gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
    gtk_widget_show(l);
    gtk_table_attach(GTK_TABLE(t), l, 0, 1, row, row+1,
                     (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                     (GtkAttachOptions)0, 0, 0);

    GtkWidget *cp = sp_color_picker_new(key, opacity_key, color_dialog_label, 0);
    gtk_widget_show(cp);
    gtk_table_attach(GTK_TABLE(t), cp, 1, 2, row, row+1,
                     (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                     (GtkAttachOptions)0, 0, 0);

    g_object_set_data(G_OBJECT(dialog), key, cp);

} // end of sp_color_picker_button



static void
sp_color_picker_destroy(GtkObject *cp, gpointer data)
{
    GtkObject *w = (GtkObject *) g_object_get_data(G_OBJECT(cp), "window");

    if (w) {
        gtk_object_destroy(w);
    }

} // end of sp_color_picker_destroy



/**
 * \brief  Creates a new color picker for the desktop properties dialog.
 *
 */
static GtkWidget *
sp_color_picker_new(gchar *colorkey, gchar *alphakey,
                    gchar *title, guint32 rgba)
{
    GtkWidget *b = gtk_button_new();

    g_object_set_data(G_OBJECT(b), "title", title);

    GtkWidget *cpv = sp_color_preview_new(rgba);

    gtk_widget_show(cpv);
    gtk_container_add(GTK_CONTAINER(b), cpv);
    g_object_set_data(G_OBJECT(b), "preview", cpv);

    g_object_set_data(G_OBJECT(b), "colorkey", colorkey);
    g_object_set_data(G_OBJECT(b), "alphakey", alphakey);

    g_signal_connect(G_OBJECT(b), "destroy",
                     G_CALLBACK(sp_color_picker_destroy), NULL);
    g_signal_connect(G_OBJECT(b), "clicked",
                     G_CALLBACK(sp_color_picker_clicked), NULL);

    return b;

} // end of sp_color_picker_new



static void
sp_color_picker_set_rgba32(GtkWidget *cp, guint32 rgba)
{
    SPColorPreview *cpv = (SPColorPreview *)g_object_get_data(G_OBJECT(cp), "preview");
    sp_color_preview_set_rgba32(cpv, rgba);

    SPColorSelector *csel = (SPColorSelector *)g_object_get_data(G_OBJECT(cp), "selector");

    if (csel) {
        SPColor color;
        sp_color_set_rgb_rgba32(&color, rgba);
        csel->base->setColorAlpha(color, SP_RGBA32_A_F(rgba));
    }

    g_object_set_data(G_OBJECT(cp), "color", GUINT_TO_POINTER(rgba));

} // end of sp_color_picker_set_rgba32



static void
sp_color_picker_window_destroy(GtkObject *object, GObject *cp)
{
    /* remove window object */
    GtkWidget *w = (GtkWidget*) g_object_get_data(G_OBJECT(cp), "window");
    if (w) {
        gtk_widget_destroy(GTK_WIDGET(w));
    }

    g_object_set_data(G_OBJECT(cp), "window", NULL);
    g_object_set_data(G_OBJECT(cp), "selector", NULL);

} // end of sp_color_picker_window_destroy



static void
sp_color_picker_color_mod(SPColorSelector *csel, GObject *cp)
{
    if (g_object_get_data(G_OBJECT(cp), "update")) {
        return;
    }

    SPColor color;
    float alpha;
    csel->base->getColorAlpha(color, &alpha);
    guint32 rgba = sp_color_get_rgba32_falpha(&color, alpha);

    g_object_set_data(G_OBJECT(cp), "color", GUINT_TO_POINTER(rgba));

    SPColorPreview *cpv = (SPColorPreview *)g_object_get_data(G_OBJECT(cp), "preview");
    gchar *colorkey = (gchar *)g_object_get_data(G_OBJECT(cp), "colorkey");
    gchar *alphakey = (gchar *)g_object_get_data(G_OBJECT(cp), "alphakey");
    sp_color_preview_set_rgba32(cpv, rgba);

    if (!SP_ACTIVE_DESKTOP) {
        return;
    }

    SPRepr *repr = SP_OBJECT_REPR(SP_ACTIVE_DESKTOP->namedview);

    gchar c[32];
    sp_svg_write_color(c, 32, rgba);
    sp_repr_set_attr(repr, colorkey, c);

    if (alphakey) {
        sp_repr_set_double(repr, alphakey, (rgba & 0xff) / 255.0);
    }

    if (!SP_ACTIVE_DESKTOP) {
        return;
    }

    sp_document_done(SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP));

} // end of sp_color_picker_color_mod



static void
sp_color_picker_window_close(GtkButton *button, GtkWidget *w)
{
    gtk_widget_destroy(w);
}



static void
sp_color_picker_clicked(GObject *cp, void *data)
{
    GtkWidget *w = (GtkWidget *) g_object_get_data(cp, "window");

    if (!w) {
        w = sp_window_new(NULL, TRUE);
        gtk_window_set_title( GTK_WINDOW(w), (gchar *)g_object_get_data(cp, "title"));
        gtk_container_set_border_width(GTK_CONTAINER(w), 4);
        g_object_set_data(cp, "window", w);

        gtk_window_set_position(GTK_WINDOW(w), GTK_WIN_POS_CENTER);
        sp_transientize(w);

        gtk_signal_connect(GTK_OBJECT(w), "event", GTK_SIGNAL_FUNC(sp_dialog_event_handler), w);

        g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(sp_dialog_hide), w);
        g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(sp_dialog_unhide), w);

        g_signal_connect(G_OBJECT(w), "destroy", G_CALLBACK(sp_color_picker_window_destroy), cp);

        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
        gtk_container_add(GTK_CONTAINER(w), vb);

        GtkWidget *csel = sp_color_selector_new(SP_TYPE_COLOR_NOTEBOOK,
                                                SP_COLORSPACE_TYPE_UNKNOWN);
        gtk_box_pack_start(GTK_BOX(vb), csel, TRUE, TRUE, 0);
        guint32 rgba = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(cp), "color"));

        SPColor color;
        sp_color_set_rgb_rgba32(&color, rgba);
        SP_COLOR_SELECTOR(csel)->base->setColorAlpha(color,
                                                     SP_RGBA32_A_F(rgba));
        g_signal_connect(G_OBJECT(csel), "dragged",
                         G_CALLBACK(sp_color_picker_color_mod), cp);
        g_signal_connect(G_OBJECT(csel), "changed",
                         G_CALLBACK(sp_color_picker_color_mod), cp);

        g_object_set_data(cp, "selector", csel);

        GtkWidget *hs = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX(vb), hs, FALSE, FALSE, 0);

        GtkWidget *hb = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);

        GtkWidget *b = gtk_button_new_with_label(_("Close"));
        gtk_box_pack_end(GTK_BOX(hb), b, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(b), "clicked",
                         G_CALLBACK(sp_color_picker_window_close), w);

        gtk_widget_show_all(w);

    } else {
        gtk_window_present(GTK_WINDOW(w));
    }

} // end of sp_color_picker_clicked


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
