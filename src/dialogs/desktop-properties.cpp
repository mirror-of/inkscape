#define __SP_DESKTOP_PROPERTIES_C__

/** \file
 * \brief  Desktop configuration dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   bulia byak <buliabyak@users.sf.net>
 * 
 * Copyright (C) Jon Phillips 2005
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <locale>
#include <sstream>
#include <utility>  // pair
#include <algorithm>  // swap

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "helper/window.h"
#include "svg/svg.h"
#include "color-picker.h"
#include "../inkscape.h"
#include "../document.h"
#include "../desktop-handles.h"
#include "../sp-namedview.h"
#include "widgets/spw-utilities.h"
#include "dialog-events.h"

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"

#include "../xml/repr.h"
#include "../xml/node-event-vector.h"

#include "desktop-properties.h"
#include "svg/stringstream.h"

#include "rdf.h"

using std::pair;

static void sp_dtw_activate_desktop(Inkscape::Application *inkscape,
                                    SPDesktop *desktop, GtkWidget *dialog);

static void sp_dtw_deactivate_desktop(Inkscape::Application *inkscape,
                                      SPDesktop *desktop,
                                      GtkWidget *dialog);

static void sp_dtw_update(GtkWidget *dialog,
                          SPDesktop *desktop);

static GtkWidget *dlg = NULL;
static GtkTooltips *tooltips = NULL;
static win_data wd;

/* impossible original values to make sure they are read from prefs */
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.documentoptions";

struct PaperSize {
    char const * const name;
    double const smaller;
    double const larger;
    SPUnitId const unit;
};

    /** \note
     * The ISO page sizes in the table below differ from ghostscript's idea of page sizes (by
     * less than 1pt).  Being off by <1pt should be OK for most purposes, but may cause fuzziness
     * (antialiasing) problems when printing to 72dpi or 144dpi printers or bitmap files due to
     * postscript's different coordinate system (y=0 meaning bottom of page in postscript and top
     * of page in SVG).  I haven't looked into whether this does in fact cause fuzziness, I merely
     * note the possibility.  Rounding done by extension/internal/ps.cpp (e.g. floor/ceil calls)
     * will also affect whether fuzziness occurs.
     *
     * The remainder of this comment discusses the origin of the numbers used for ISO page sizes in
     * this table and in ghostscript.
     *
     * The versions here, in mm, are the official sizes according to
     * <a href="http://en.wikipedia.org/wiki/Paper_sizes">http://en.wikipedia.org/wiki/Paper_sizes</a> 
     * at 2005-01-25.  (The ISO entries in the below table
     * were produced mechanically from the table on that page.)
     *
     * (The rule seems to be that A0, B0, ..., D0. sizes are rounded to the nearest number of mm
     * from the "theoretical size" (i.e. 1000 * sqrt(2) or pow(2.0, .25) or the like), whereas
     * going from e.g. A0 to A1 always take the floor of halving -- which by chance coincides
     * exactly with flooring the "theoretical size" for n != 0 instead of the rounding to nearest
     * done for n==0.)
     *
     * Ghostscript paper sizes are given in gs_statd.ps according to gs(1).  gs_statd.ps always
     * uses an integer number of pt: sometimes gs_statd.ps rounds to nearest (e.g. a1), sometimes
     * floors (e.g. a10), sometimes ceils (e.g. a8).
     *
     * I'm not sure how ghostscript's gs_statd.ps was calculated: it isn't just rounding the
     * "theoretical size" of each page to pt (see a0), nor is it rounding the a0 size times an
     * appropriate power of two (see a1).  Possibly it was prepared manually, with a human applying
     * inconsistent rounding rules when converting from mm to pt.
     */
    /** \todo
     * Should we include the JIS B series (used in Japan)  
     * (JIS B0 is sometimes called JB0, and similarly for JB1 etc)?
     * Should we exclude B7--B10 and A7--10 to make the list smaller ?
     * Should we include any of the ISO C, D and E series (see below) ?
     */

static PaperSize const inkscape_papers[] = {
    { "A4", 210, 297, SP_UNIT_MM },
    { "US Letter", 8.5, 11, SP_UNIT_IN },
    { "US Legal", 8.5, 14, SP_UNIT_IN },
    { "US Executive", 7.25, 10.5, SP_UNIT_IN },
    { "A0", 841, 1189, SP_UNIT_MM },
    { "A1", 594, 841, SP_UNIT_MM },
    { "A2", 420, 594, SP_UNIT_MM },
    { "A3", 297, 420, SP_UNIT_MM },
    { "A5", 148, 210, SP_UNIT_MM },
    { "A6", 105, 148, SP_UNIT_MM },
    { "A7", 74, 105, SP_UNIT_MM },
    { "A8", 52, 74, SP_UNIT_MM },
    { "A9", 37, 52, SP_UNIT_MM },
    { "A10", 26, 37, SP_UNIT_MM },
    { "B0", 1000, 1414, SP_UNIT_MM },
    { "B1", 707, 1000, SP_UNIT_MM },
    { "B2", 500, 707, SP_UNIT_MM },
    { "B3", 353, 500, SP_UNIT_MM },
    { "B4", 250, 353, SP_UNIT_MM },
    { "B5", 176, 250, SP_UNIT_MM },
    { "B6", 125, 176, SP_UNIT_MM },
    { "B7", 88, 125, SP_UNIT_MM },
    { "B8", 62, 88, SP_UNIT_MM },
    { "B9", 44, 62, SP_UNIT_MM },
    { "B10", 31, 44, SP_UNIT_MM },

#if 0 /* Whether to include or exclude these depends on how big we mind our page size menu
         becoming.  C series is used for envelopes; don't know what D and E series are used for. */
    { "C0", 917, 1297, SP_UNIT_MM },
    { "C1", 648, 917, SP_UNIT_MM },
    { "C2", 458, 648, SP_UNIT_MM },
    { "C3", 324, 458, SP_UNIT_MM },
    { "C4", 229, 324, SP_UNIT_MM },
    { "C5", 162, 229, SP_UNIT_MM },
    { "C6", 114, 162, SP_UNIT_MM },
    { "C7", 81, 114, SP_UNIT_MM },
    { "C8", 57, 81, SP_UNIT_MM },
    { "C9", 40, 57, SP_UNIT_MM },
    { "C10", 28, 40, SP_UNIT_MM },
    { "D1", 545, 771, SP_UNIT_MM },
    { "D2", 385, 545, SP_UNIT_MM },
    { "D3", 272, 385, SP_UNIT_MM },
    { "D4", 192, 272, SP_UNIT_MM },
    { "D5", 136, 192, SP_UNIT_MM },
    { "D6", 96, 136, SP_UNIT_MM },
    { "D7", 68, 96, SP_UNIT_MM },
    { "E3", 400, 560, SP_UNIT_MM },
    { "E4", 280, 400, SP_UNIT_MM },
    { "E5", 200, 280, SP_UNIT_MM },
    { "E6", 140, 200, SP_UNIT_MM },
#endif

    { "CSE", 462, 649, SP_UNIT_PT },
    { "US #10 Envelope", 4.125, 9.5, SP_UNIT_IN }, // TODO: Select landscape by default.
    /* See http://www.hbp.com/content/PCR_envelopes.cfm for a much larger list of US envelope
       sizes. */
    { "DL Envelope", 110, 220, SP_UNIT_MM }, // TODO: Select landscape by default.
    { "Ledger/Tabloid", 11, 17, SP_UNIT_IN },
    /* Note that `Folio' (used in QPrinter/KPrinter) is deliberately absent from this list, as it
       means different sizes to different people: different people may expect the width to be
       either 8, 8.25 or 8.5 inches, and the height to be either 13 or 13.5 inches, even
       restricting our interpretation to foolscap folio.  If you wish to introduce a folio-like
       page size to the list, then please consider using a name more specific than just `Folio' or
       `Foolscap Folio'. */
    { "Banner 468x60", 60, 468, SP_UNIT_PX },  // TODO: Select landscape by default.
    { "Icon 16x16", 16, 16, SP_UNIT_PX },
    { "Icon 32x32", 32, 32, SP_UNIT_PX },
    { NULL, 0, 0, SP_UNIT_PX },
};

/** 
 * Returns an index into inkscape_papers of a paper of the specified 
 * size (specified in px), or -1 if there's no such paper.
 */
static int
find_paper_size(double const w_px, double const h_px)
{
    double given[2];
    if ( w_px < h_px ) {
        given[0] = w_px; given[1] = h_px;
    } else {
        given[0] = h_px; given[1] = w_px;
    }
    g_return_val_if_fail(given[0] <= given[1], -1);
    for (unsigned i = 0; i < G_N_ELEMENTS(inkscape_papers) - 1; ++i) {
        SPUnit const &i_unit = sp_unit_get_by_id(inkscape_papers[i].unit);
        double const i_sizes[2] = { sp_units_get_pixels(inkscape_papers[i].smaller, i_unit),
                                    sp_units_get_pixels(inkscape_papers[i].larger, i_unit) };
        g_return_val_if_fail(i_sizes[0] <= i_sizes[1], -1);
        if ((fabs(given[0] - i_sizes[0]) <= .1) &&
            (fabs(given[1] - i_sizes[1]) <= .1)   )
        {
            return (int) i;
        }
    }
    return -1;
}

/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void
docoptions_event_attr_changed(Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer)
{
    if (g_object_get_data(G_OBJECT(dlg), "update")) {
        return;
    }

    if (dlg) {
        sp_dtw_update(dlg, SP_ACTIVE_DESKTOP);
    }
}

static Inkscape::XML::NodeEventVector docoptions_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    docoptions_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
 * Callback to finalize dialog.
 */
static void
sp_dtw_dialog_destroy(GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data(INKSCAPE, dlg);
    gtk_object_destroy ( GTK_OBJECT (tooltips) );
    tooltips = NULL;
    sp_repr_remove_listener_by_data(SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP)), dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

/**
 * Called when dialog is closed; saves position/size.
 */
static bool
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

/**
 * \brief  Callback that writes the change into the corresponding 
 * attribute of the sodipodi:namedview element.
 */
static void
sp_dtw_whatever_toggled(GtkToggleButton *tb, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;

    if (!dt) {
        return;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));
    gchar const *key = (gchar const *)gtk_object_get_data(GTK_OBJECT(tb), "key");

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(TRUE));

    gboolean saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_boolean(repr, key, gtk_toggle_button_get_active(tb));
    sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
    sp_document_set_undo_sensitive(doc, saved);
    sp_document_done(doc);

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(FALSE));
}

/**
 * Same as sp_dtw_whatever_toggled() plus it toggles canvas border on top.
 */
static void
sp_dtw_border_layer_toggled(GtkToggleButton *tb, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));

    gboolean saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_attr(repr, "borderlayer",
                     gtk_toggle_button_get_active(tb) ? "top" : NULL);
    sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
    sp_document_set_undo_sensitive(doc, saved);
    sp_document_done(doc);
}



/**
 * Same as sp_dtw_whatever_toggled() plus it writes unit_selector.
 */
static void
sp_dtw_whatever_changed(GtkAdjustment *adjustment, GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }
    SPDocument *doc = SP_DT_DOCUMENT(dt);

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));
    gchar const *key = (gchar const *)gtk_object_get_data(GTK_OBJECT(adjustment), "key");
    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(adjustment),
                                                                "unit_selector");

    Inkscape::SVGOStringStream os;
    os << adjustment->value;
    if (us != NULL)
        os << sp_unit_selector_get_unit(us)->abbr;

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(TRUE));

    gboolean saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_attr(repr, key, os.str().c_str());
    sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
    sp_document_set_undo_sensitive(doc, saved);
    sp_document_done(doc);

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(FALSE));
}


/**
 *  \brief Exists to stop tabs from working until we just GTK 2.4+
 */
/*
gboolean text_stop_tab (GtkWidget * widget, GdkEventKey *event, gpointer user_data)
{
    if ( event->keyval == GDK_Tab ) return TRUE;
    return FALSE;
}
*/

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
    gchar * text = NULL;

    switch ( entity->format ) {
    case RDF_FORMAT_LINE:
        e = GTK_WIDGET (widget);

        text = g_strdup(gtk_entry_get_text ( GTK_ENTRY (e) ));
        break;
    case RDF_FORMAT_MULTILINE:
        buf = GTK_TEXT_BUFFER (widget);

        gtk_text_buffer_get_start_iter(buf, &start);
        gtk_text_buffer_get_end_iter(buf, &end);

        text = gtk_text_buffer_get_text( buf, &start, &end, FALSE );
        break;
    default:
        break;
    }

    if (rdf_set_work_entity( SP_ACTIVE_DOCUMENT, entity, text )) {
        /* if we changed an RDF entity, mark the document as changed */
        sp_document_done ( SP_ACTIVE_DOCUMENT );
    }
    if (text) g_free(text);
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
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    gchar const *key = (gchar const *) gtk_object_get_data(GTK_OBJECT(adjustment), "key");
    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(adjustment),
                                                                "unit_selector");
    SPUnit const *unit = sp_unit_selector_get_unit(us);

    if (!strcmp (key, "width")) {
        sp_document_set_width (doc, adjustment->value, unit);
    } else if (!strcmp (key, "height")) {
        sp_document_set_height (doc, adjustment->value, unit);
    } else 
        g_warning ("sp_doc_dialog_whatever_changed should only be used for width/height");

    sp_dtw_update (dialog, dt);

    sp_document_done(doc);
}

/**
 * Callback to set gridtolerance attribute from adjustment and 
 * grid_snap_units widget.
 */
static void
sp_dtw_grid_snap_distance_changed(GtkAdjustment *adjustment,
                                  GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));

    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(dialog),
                                                                "grid_snap_units");

    Inkscape::SVGOStringStream os;
    os << adjustment->value << sp_unit_selector_get_unit(us)->abbr;

    sp_repr_set_attr(repr, "gridtolerance", os.str().c_str());
    sp_document_done(SP_DT_DOCUMENT(dt));
}

/** 
 * Callback to set gridempspacing attribute from adjustment widget.
 */
static void
sp_dtw_grid_emp_spacing_changed (GtkAdjustment *adjustment,
                                 GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));

    Inkscape::SVGOStringStream os;
    int value = int(adjustment->value);
    os << value;

    sp_repr_set_attr(repr, "gridempspacing", os.str().c_str());
    sp_document_done(SP_DT_DOCUMENT(dt));
}

/**
 * Callback to set guidetolerance attribute from adjustment
 * and guide_snap_units widgets.
 */
static void
sp_dtw_guides_snap_distance_changed(GtkAdjustment *adjustment,
                                    GtkWidget *dialog)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));

    SPUnitSelector *us = (SPUnitSelector *) gtk_object_get_data(GTK_OBJECT(dialog),
                                                                "guide_snap_units");

    Inkscape::SVGOStringStream os;
    os << adjustment->value << sp_unit_selector_get_unit(us)->abbr;

    sp_repr_set_attr(repr, "guidetolerance", os.str().c_str());
}

/**
 * Returns paper dimensions using specific unit and orientation.
 */
static pair<double, double>
get_paper_size(PaperSize const &paper, bool const landscape, SPUnit const *const dest_unit)
{
    double h, w;
    if (landscape) {
        w = paper.larger;
        h = paper.smaller;
    } else {
        w = paper.smaller;
        h = paper.larger;
    }
    SPUnit const &src_unit = sp_unit_get_by_id(paper.unit);
    sp_convert_distance(&w, &src_unit, dest_unit);
    sp_convert_distance(&h, &src_unit, dest_unit);
    return pair<double, double>(w, h);
}

/**
 * Callback to set height/width widgets from paper type, orientation,
 * and unit widgets.
 */
static void
sp_doc_dialog_paper_selected(GtkWidget *widget, gpointer data)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    PaperSize const *const paper = (PaperSize const *) data;

    if (paper) {
        GtkWidget *const om = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dlg), "orientation");
        bool const landscape = gtk_option_menu_get_history(GTK_OPTION_MENU(om));

        SPUnitSelector *const us = (SPUnitSelector *)gtk_object_get_data(GTK_OBJECT(dlg), "units");
        SPUnit const *const unit = sp_unit_selector_get_unit(us);

        pair<double, double> const w_h(get_paper_size(*paper, landscape, unit));

        GtkAdjustment *const aw = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "width");
        GtkAdjustment *const ah = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "height");
        gtk_adjustment_set_value(aw, w_h.first);
        gtk_adjustment_set_value(ah, w_h.second);
    } 

    if (!SP_ACTIVE_DESKTOP) {
        return;
    }

    sp_document_done(SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP));
}

/**
 * Callback to set height/width widgets from paper type, orientation,
 * and unit widgets.
 */
static void
sp_doc_dialog_paper_orientation_selected(GtkWidget *widget, gpointer data)
{
    if (gtk_object_get_data(GTK_OBJECT(dlg), "update")) {
        return;
    }

    GtkAdjustment *aw = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "width");
    GtkAdjustment *ah = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dlg), "height");

    gdouble w = gtk_adjustment_get_value(aw);
    gdouble h = gtk_adjustment_get_value(ah);

    /* only toggle when we actually swap */
    GtkWidget *om = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dlg), "orientation");
    gint const landscape = gtk_option_menu_get_history(GTK_OPTION_MENU(om));

    if ( landscape
         ? (w < h)
         : (w > h) )
    {
        std::swap(w, h);
    }

    gtk_adjustment_set_value(aw, w);
    gtk_adjustment_set_value(ah, h);
}

/**
 * Callback to set inkscape::document-units attribute from unit widget.
 */
static gboolean set_doc_units (SPUnitSelector *,
                             SPUnit const *old,
                             SPUnit const *new_units,
                             GObject *dlg)
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;

    if (!dt) {
        return FALSE;
    }

    SPDocument *doc = SP_DT_DOCUMENT(dt);

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));

    gboolean saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, FALSE);
    sp_repr_set_attr (repr, "inkscape:document-units", sp_unit_get_abbreviation (new_units));
    sp_repr_set_attr (doc->rroot, "sodipodi:modified", "true");
    sp_document_set_undo_sensitive(doc, saved);
    sp_document_done(doc);

    return TRUE;
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
        GtkWidget *l = gtk_label_new ( _(entity->title) );
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
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), e, _(entity->tip), NULL );
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
            gtk_widget_set_size_request (view, -1, 3);
            gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (view), GTK_WRAP_WORD );
// available from GTK 2.4 on...
            gtk_text_view_set_accepts_tab ( GTK_TEXT_VIEW (view), FALSE );
//          previously, we just killed tabs...
//            g_signal_connect ( G_OBJECT (view), "key-press-event",
//                               G_CALLBACK (text_stop_tab), NULL );

            buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
            /// \todo FIXME: looks like tool tips don't show up for GtkTextViews
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), scroller, _(entity->tip), NULL );
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
sp_doc_dialog_update_license ( struct rdf_work_entity_t * entity,
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

    sp_doc_dialog_update_license ( rdf_find_entity( "license_uri" ),
                                   license ? license->uri : NULL,
                                   license ? FALSE : TRUE ); 
    /*
    sp_doc_dialog_update_license ( rdf_find_entity( "license_fragment" ),
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
        sp_ui_dialog_title_string(Inkscape::Verb::get(SP_VERB_DIALOG_NAMEDVIEW), title);

        dlg = sp_window_new(title, TRUE);
        gtk_window_set_resizable ((GtkWindow *) dlg, FALSE);

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
        tooltips = gtk_tooltips_new ();
        gtk_tooltips_enable (tooltips);

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
        /// \todo FIXME: gray out snapping when grid is off
        spw_vbox_checkbutton(dlg, v, _("Show grid"), _("Show or hide grid"), "showgrid", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap bounding boxes to grid"), _("Snap the edges of the object bounding boxes"), "inkscape:grid-bbox", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap points to grid"), _("Snap path nodes, text baselines, ellipse centers, etc."), "inkscape:grid-points", cb);

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

        GtkWidget *us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        int row = 0;
        spw_dropdown(dlg, t, _("Grid units:"), "grid_units", row++, us);

        spw_unit_selector(dlg, t, _("Origin X:"), "gridoriginx",
                          row++, us, cb, true);

        spw_unit_selector(dlg, t, _("Origin Y:"), "gridoriginy",
                          row++, us, cb, true);

        spw_unit_selector(dlg, t, _("Spacing X:"), "gridspacingx",
                          row++, us, cb);

        spw_unit_selector(dlg, t, _("Spacing Y:"), "gridspacingy",
                          row++, us, cb);

        us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        spw_dropdown(dlg, t, _("Snap units:"), "grid_snap_units",
                     row++, us);

        spw_unit_selector(dlg, t, _("Snap distance:"), "gridtolerance", row++, us,
                          G_CALLBACK(sp_dtw_grid_snap_distance_changed));

        sp_color_picker_button(NULL, true, dlg, t, _("Grid line color:"), "gridcolor",
                               _("Grid line color"), _("Color of grid lines"), "gridopacity", row++);

        sp_color_picker_button(NULL, true, dlg, t, _("Major grid line color:"), "gridempcolor",
                               _("Major grid line color"), _("Color of the major (highlighted) grid lines"), "gridempopacity", row++);

        if (1) {
            spw_label(t, _("Major grid line every:"), 0, row);
            GtkObject * a = gtk_adjustment_new (0.0, 0.0, 25.0, 1.0, 1.0, 1.0);
            gtk_object_set_data(GTK_OBJECT(a), (const gchar *)"key", (gpointer)"gridempspacing");
            gtk_object_set_data(GTK_OBJECT(dlg), "gridempspacing", a);

            GtkWidget * hbox = gtk_hbox_new(FALSE, 2);

            GtkWidget * sb = gtk_spin_button_new (GTK_ADJUSTMENT(a), 1.0, 0);
            gtk_widget_show(sb);

            // TRANSLATORS: This belongs to the "Major grid line every:" string,
            //  see grid settings in the "Document Preferences" dialog
            GtkWidget * label = gtk_label_new(_("lines"));
            gtk_widget_show(label);

            gtk_box_pack_start(GTK_BOX(hbox), sb, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
            gtk_widget_show(hbox);

            gtk_table_attach(GTK_TABLE(t), hbox, 1, 2, row, row+1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
            g_signal_connect(G_OBJECT(a), "value_changed", G_CALLBACK(sp_dtw_grid_emp_spacing_changed), dlg);
            row++;
        }

        /* Guidelines page */

        l = gtk_label_new(_("Guides"));
        gtk_widget_show(l);
        v = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(v);
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), v, l);

        /* Checkbuttons */
        /// \todo FIXME: gray out snapping when guides are off
        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_vbox_checkbutton(dlg, v, _("Show guides"), _("Show or hide guides"), "showguides", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap bounding boxes to guides"),  _("Snap the edges of the object bounding boxes"), "inkscape:guide-bbox", cb);
        spw_vbox_checkbutton(dlg, v, _("Snap points to guides"), _("Snap path nodes, text baselines, ellipse centers, etc."), "inkscape:guide-points", cb);

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

        sp_color_picker_button(NULL, true, dlg, t, _("Guide color:"), "guidecolor",
                               _("Guideline color"), _("Color of guidelines"), "guideopacity", row++);

        sp_color_picker_button(NULL, true, dlg, t, _("Highlight color:"), "guidehicolor",
                               _("Highlighted guideline color"), _("Color of a guideline when it is under mouse"),
                               "guidehiopacity", row++);

        row=0;
        /* Page page */
        l = gtk_label_new(_("Page"));
        gtk_widget_show(l);
        t = gtk_table_new(1, 2, FALSE);
        gtk_widget_show(t);
        gtk_container_set_border_width(GTK_CONTAINER(t), 6);
        gtk_table_set_row_spacings(GTK_TABLE(t), 6);
        gtk_table_set_col_spacings(GTK_TABLE(t), 6);
        gtk_notebook_prepend_page(GTK_NOTEBOOK(nb), t, l);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), 0);

        sp_color_picker_button(NULL, true, dlg, t, _("Background:"),
                               "pagecolor", _("Background color"), 
                               _("Color and transparency of the page background (also used for bitmap export)"),
                               "inkscape:pageopacity", 0);

        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_checkbutton(dlg, t, _("Show canvas border"),
                        "showborder", 0, 1, 0, cb);

        cb = G_CALLBACK(sp_dtw_border_layer_toggled);
        spw_checkbutton(dlg, t, _("Border on top of drawing"),
                        "borderlayer", 0, 2, 0, cb);

        sp_color_picker_button(NULL, true, dlg, t, _("Border color:"),
                               "bordercolor", _("Canvas border color"),
                               _("Color of the canvas border"),
                               "borderopacity", 4);

	/* Page Shadow toggle */
        cb = G_CALLBACK(sp_dtw_whatever_toggled);
        spw_checkbutton(dlg, t, _("Show page shadow"),
		                  "inkscape:showpageshadow", 0, 5, 0, cb);

	
        l = gtk_label_new(_("Default units:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show(l);
        gtk_table_attach (GTK_TABLE (t), l, 0, 1, 6, 7,
                    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);
        GtkWidget *doc_units = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        gtk_tooltips_set_tip(tooltips, doc_units, _("Units for the tool controls, ruler, and the statusbar"), NULL);
        g_signal_connect(G_OBJECT(doc_units), "set_unit", G_CALLBACK(set_doc_units), NULL);
        gtk_object_set_data (GTK_OBJECT (dlg), "doc_units", doc_units);
        gtk_widget_show(doc_units);
        gtk_table_attach (GTK_TABLE (t), doc_units, 1, 2, 6, 7,
                    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)0, 0, 0);

        // The following comes from the former "document settings" dialog

        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
        gtk_widget_show(vb);
        gtk_table_attach(GTK_TABLE(t), vb, 0, 2, 7, 8,
                         (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                         (GtkAttachOptions)0, 0, 0);

        GtkWidget *hb = gtk_hbox_new(FALSE, 4);
        gtk_widget_show(hb);
        gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);

        l = gtk_label_new(_("Canvas size:"));
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
        for (PaperSize const *paper = inkscape_papers;
             paper->name;
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

        l = gtk_label_new(_("Canvas orientation:"));
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
        GtkWidget *f = gtk_frame_new(_("Custom canvas"));
        gtk_widget_show(f);
        gtk_box_pack_start(GTK_BOX(vb), f, FALSE, FALSE, 0);

        GtkWidget *tt = gtk_table_new(4, 2, FALSE);
        gtk_widget_show(tt);
        gtk_container_set_border_width(GTK_CONTAINER(tt), 4);
        gtk_table_set_row_spacings(GTK_TABLE(tt), 4);
        gtk_table_set_col_spacings(GTK_TABLE(tt), 4);
        gtk_container_add(GTK_CONTAINER(f), tt);

        us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
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
        gtk_table_set_row_spacings (GTK_TABLE (t), 1);
        gtk_table_set_col_spacings (GTK_TABLE (t), 4);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), t, l);

        row=0;
        /* add generic metadata entry areas */
        struct rdf_work_entity_t * entity;
        for (entity = rdf_work_entities; entity && entity->name; entity++) {
            if ( entity->editable == RDF_EDIT_GENERIC ) {
                sp_doc_dialog_add_work_entity ( entity, t, tooltips, row++ );
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
        GtkWidget * w = sp_doc_dialog_add_work_entity ( entity, t, tooltips, row++ );
        gtk_widget_set_sensitive ( w, FALSE );
        /*
        entity = rdf_find_entity ( "license_fragment" );
        w = sp_doc_dialog_add_work_entity ( entity, t, tooltips, row++ );
        gtk_widget_set_sensitive ( w, FALSE );
        */

        // end "Metadata" tab


        g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop",
                          G_CALLBACK(sp_dtw_activate_desktop), dlg);

        g_signal_connect( G_OBJECT(INKSCAPE), "deactivate_desktop",
                          G_CALLBACK(sp_dtw_deactivate_desktop), dlg);
        sp_dtw_update(dlg, SP_ACTIVE_DESKTOP);

        sp_repr_add_listener(SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP)), &docoptions_repr_events, dlg);

    } // end of if (!dlg)

    gtk_window_present((GtkWindow *) dlg);


} // end of sp_desktop_dialog(void)

/**
 * Callback to enable notifyig namedview of any changes in dialog widgets.
 */
static void
sp_dtw_activate_desktop(Inkscape::Application *inkscape,
                        SPDesktop *desktop,
                        GtkWidget *dialog)
{
    if (desktop && dlg) {
        sp_repr_add_listener(SP_OBJECT_REPR(SP_DT_NAMEDVIEW(desktop)), &docoptions_repr_events, dlg);
    }
    sp_dtw_update(dialog, desktop);
}

/**
 * Callback to disable notifyig namedview of any changes in dialog widgets.
 */
static void
sp_dtw_deactivate_desktop(Inkscape::Application *inkscape,
                          SPDesktop *desktop,
                          GtkWidget *dialog)
{
    if (desktop && SP_IS_NAMEDVIEW(SP_DT_NAMEDVIEW(desktop)) && dlg) {
        // all these checks prevent crash when you close inkscape with the dialog open
        sp_repr_remove_listener_by_data(SP_OBJECT_REPR(SP_DT_NAMEDVIEW(desktop)), dlg);
    }
    sp_dtw_update(dialog, NULL);
}

/**
 * Update dialog widgets from desktop.
 */
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
        cp = (GObject *)g_object_get_data(G_OBJECT(dialog), "gridempcolor");
        w = (GObject *)g_object_get_data(cp, "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), FALSE);
        }
        cp = (GObject *)g_object_get_data(G_OBJECT(dialog), "guidecolor");
        w = (GObject *)g_object_get_data(cp, "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), FALSE);
        }
        cp = (GObject *)g_object_get_data(G_OBJECT(dialog), "guidehicolor");
        w = (GObject *)g_object_get_data(cp, "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), FALSE);
        }
    } else {
        SPNamedView *nv = SP_DT_NAMEDVIEW(desktop);

        gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(TRUE));
        gtk_widget_set_sensitive(dialog, TRUE);

        GtkObject *o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "showgrid");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->showgrid);

        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:grid-bbox");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->grid_snapper.getSnapTo(Snapper::BBOX_POINT));
        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:grid-points");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->grid_snapper.getSnapTo(Snapper::SNAP_POINT));

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "grid_units");
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(o), nv->gridunit);

        gdouble val = nv->gridorigin[NR::X];
        val = sp_pixels_get_units (val, *(nv->gridunit));
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridoriginx");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);
        val = nv->gridorigin[NR::Y];
        val = sp_pixels_get_units (val, *(nv->gridunit));
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridoriginy");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);
        val = nv->gridspacing[NR::X];
        val = sp_pixels_get_units (val, *(nv->gridunit));
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridspacingx");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);
        val = nv->gridspacing[NR::Y];
        val = sp_pixels_get_units (val, *(nv->gridunit));
        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridspacingy");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), val);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "grid_snap_units");
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(o), nv->gridtoleranceunit);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridtolerance");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), nv->gridtolerance);

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "gridempspacing");
        gtk_adjustment_set_value(GTK_ADJUSTMENT(o), nv->gridempspacing);

        GtkWidget *cp = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dialog), "gridcolor");
        sp_color_picker_set_rgba32(cp, nv->gridcolor);
        GtkWidget *w = (GtkWidget *)g_object_get_data(G_OBJECT(cp), "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        cp = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(dialog), "gridempcolor");
        sp_color_picker_set_rgba32(cp, nv->gridempcolor);
        w = (GtkWidget *)g_object_get_data(G_OBJECT(cp), "window");
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "showguides");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->showguides);

        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:guide-bbox");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->guide_snapper.getSnapTo(Snapper::BBOX_POINT));
        o = (GtkObject *) gtk_object_get_data(GTK_OBJECT(dialog), "inkscape:guide-points");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->guide_snapper.getSnapTo(Snapper::SNAP_POINT));

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

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), 
		"inkscape:showpageshadow");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o), nv->showpageshadow);
	
        cp = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dialog), "pagecolor"));
        sp_color_picker_set_rgba32(cp, nv->pagecolor);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(cp), "window"));
        if (w) {
            gtk_widget_set_sensitive(GTK_WIDGET(w), TRUE);
        }

        o = (GtkObject *)gtk_object_get_data(GTK_OBJECT(dialog), "doc_units");
        if (nv->doc_units) 
            sp_unit_selector_set_unit (SP_UNIT_SELECTOR(o), nv->doc_units);

        // Start of former "document settings" stuff

        gdouble const doc_w_px = sp_document_width(SP_DT_DOCUMENT(desktop));
        gdouble const doc_h_px = sp_document_height(SP_DT_DOCUMENT(desktop));

        GtkWidget *pm = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dialog), "papers");
        GtkWidget *om = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(dialog), "orientation");

        /* select paper orientation */
        gint const landscape = ( doc_w_px > doc_h_px );
        gtk_option_menu_set_history(GTK_OPTION_MENU(om), landscape);

        /* find matching paper size */
        gint const pos = 1 + find_paper_size(doc_w_px, doc_h_px);
        gtk_option_menu_set_history(GTK_OPTION_MENU(pm), pos);

        /* Show document width/height in the requested units. */
        {
            SPUnitSelector const *us = (SPUnitSelector *)gtk_object_get_data(GTK_OBJECT(dialog), "units");
            SPUnit const *unit = sp_unit_selector_get_unit(us);
            gdouble const doc_w_u = sp_pixels_get_units(doc_w_px, *unit);
            gdouble const doc_h_u = sp_pixels_get_units(doc_h_px, *unit);
            GtkAdjustment *w_adj = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dialog), "width");
            GtkAdjustment *h_adj = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(dialog), "height");
            gtk_adjustment_set_value(w_adj, doc_w_u);
            gtk_adjustment_set_value(h_adj, doc_h_u);
        }

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
        if (license) {
            for (int i=0; rdf_licenses[i].name; i++) {
                if (license == &rdf_licenses[i]) {
                    gtk_option_menu_set_history(GTK_OPTION_MENU(lm), i+1);
                    break;
                }
            }
        }
        else {
            gtk_option_menu_set_history(GTK_OPTION_MENU(lm), 0);
        }
        /* update the URI */
        struct rdf_work_entity_t * entity = rdf_find_entity( "license_uri" );
        sp_doc_dialog_update_license ( entity,
                                       license ? license->uri : NULL,
                                       license ? FALSE : TRUE ); 
        sp_doc_dialog_update_work_entity( entity );

        gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(FALSE));
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
