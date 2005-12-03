/** \file
 *
 * Paper-size widget and helper functions
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/menuitem.h>

#include "ui/widget/scalar-unit.h"
#include "ui/widget/unit-menu.h"
#include "helper/units.h"
#include "page-sizer.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Widget {

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

//===================================================

class SizeMenuItem : public Gtk::MenuItem {
public:
    SizeMenuItem (const Glib::ustring &s);
    virtual ~SizeMenuItem();
protected:
    void on_activate_item();
};

SizeMenuItem::SizeMenuItem (const Glib::ustring &s)
: Gtk::MenuItem(s)
{
}

SizeMenuItem::~SizeMenuItem()
{
}

void
SizeMenuItem::on_activate_item()
{
}

class OrientationMenuItem : public Gtk::MenuItem {
public:
    OrientationMenuItem (const Glib::ustring &s);
    virtual ~OrientationMenuItem();
protected:
    void on_activate_item();
};

OrientationMenuItem::OrientationMenuItem (const Glib::ustring &s) 
: Gtk::MenuItem(s) 
{
}

OrientationMenuItem::~OrientationMenuItem()
{
}

void OrientationMenuItem::on_activate_item()
{
}

//---------------------------------------------------

PageSizer::PageSizer()
: Gtk::VBox(false,4)
{
    Gtk::HBox *hbox_size = manage (new Gtk::HBox (false, 4));
    pack_start (*hbox_size, false, false, 0);
    Gtk::Label *label_size = manage (new Gtk::Label (_("Page size:"), 1.0, 0.5)); 
    hbox_size->pack_start (*label_size, false, false, 0);
    Gtk::OptionMenu *omenu_size = manage (new Gtk::OptionMenu);
    hbox_size->pack_start (*omenu_size, true, true, 0);
    Gtk::Menu *menu_size = manage (new Gtk::Menu);

    for (PaperSize const *paper = inkscape_papers; paper->name; paper++) {
        SizeMenuItem *item = manage (new SizeMenuItem (paper->name));
        menu_size->append (*item);
    }
    SizeMenuItem *item = manage (new SizeMenuItem (_("Custom")));
    menu_size->prepend (*item);
    omenu_size->set_menu (*menu_size);

    Gtk::HBox *hbox_ori = manage (new Gtk::HBox (false, 4));
    pack_start (*hbox_ori, false, false, 0);
    Gtk::Label *label_ori = manage (new Gtk::Label (_("Page orientation:"), 1.0, 0.5)); 
    hbox_ori->pack_start (*label_ori, false, false, 0);
    Gtk::OptionMenu *omenu_ori = manage (new Gtk::OptionMenu);
    hbox_ori->pack_start (*omenu_ori, true, true, 0);
    Gtk::Menu *menu_ori = manage (new Gtk::Menu);

    OrientationMenuItem *oitem;
    oitem = manage (new OrientationMenuItem (_("Landscape")));
    menu_ori->prepend (*oitem);
    oitem = manage (new OrientationMenuItem (_("Portrait")));
    menu_ori->prepend (*oitem);
    omenu_ori->set_menu (*menu_ori);

    show_all_children();
}

PageSizer::~PageSizer()
{
}

void
PageSizer::init (Registry& reg)
{
    /* Custom paper frame */
    Gtk::Frame *frame = manage (new Gtk::Frame(_("Custom size")));
    pack_start (*frame, false, false, 0);
    frame->show();
    Gtk::Table *table = manage (new Gtk::Table (4, 2, false));
    table->set_border_width (4);
    table->set_row_spacings (4);
    table->set_col_spacings (4);
    table->show();
    frame->add (*table);
    
    _wr = reg;

    _rum.init (_("Units:"), "units", _wr);
    _rusw.init (_("Width:"), "", "width", _rum, _wr);
    _rush.init (_("Height:"), "", "height", _rum, _wr);

    const Gtk::Widget* arr[] =
    {
        _rum._label, _rum._sel,
        0,           _rusw.getSU(),
        0,           _rush.getSU(),
    };

    for (unsigned i=0, r=0; i<sizeof(arr)/sizeof(Gtk::Widget*); i+=2)
    {
        if (arr[i])
        {
            table->attach (const_cast<Gtk::Widget&>(*arr[i]),   0, 1, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table->attach (const_cast<Gtk::Widget&>(*arr[i+1]), 1, 2, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
        else
            table->attach (const_cast<Gtk::Widget&>(*arr[i+1]), 0, 2, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        ++r;
    }
}

#if 0
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
#endif


} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
