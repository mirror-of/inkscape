/*
 * A simple dialog for creating grid type arrangements of selected objects
 *
 * Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#define DEBUG_GRID_ARRANGE 0

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tiledialog.h"

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>

#include <gtk/gtk.h>

#include <dialogs/dialog-events.h>
#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <glibmm/i18n.h>
#include "interface.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "inkscape.h"
#include "macros.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "xml/repr.h"
#include "document.h"
#include "sp-item.h"


#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"

#include <glib.h>

namespace Inkscape {
namespace UI {
namespace Dialogs {


//#########################################################################
//## I M P L E M E N T A T I O N
//#########################################################################

/**
 * A dialog for creating grid patterns of selected objects
 */
class TileDialogImpl : public TileDialog, public Gtk::Dialog
{

    public:


    /**
     * Constructor
     */
    TileDialogImpl();

    /**
     * Destructor
     */
    ~TileDialogImpl();


    /**
     * Show the dialog
     */
    void show();
    void showF12();

    /**
     * Do not show the dialog
     */
    void hide();
    void hideF12();

    /**
     * Do the actual work
     */
    void Grid_Arrange();


    /**
     * Respond to selection change
     */
    void TileDialogImpl::updateSelection();


    /**
     * Callback from OK or Cancel
     */
    void responseCallback(int response_id);

    /**
     * Callback from spinbuttons
     */
    void on_row_spinbutton_changed();
    void on_col_spinbutton_changed();
    void on_xpad_spinbutton_changed();
    void on_ypad_spinbutton_changed();
    void on_RowSize_checkbutton_changed();
    void on_ColSize_checkbutton_changed();
    void on_rowSize_spinbutton_changed();
    void on_colSize_spinbutton_changed();
    private:

    bool userHidden;
    bool updating;



    Gtk::Notebook   notebook;
    Gtk::Tooltips   tips;

    Gtk::VBox             TileBox;
    Gtk::Button           *TileOkButton;
    Gtk::Button           *TileCancelButton;

    // Number selected label
    Gtk::Label            SelectionContentsLabel;

    // Number per Row
    Gtk::HBox             NoPerRowBox;
    Gtk::Label            NoPerRowLabel;
    Gtk::SpinButton       NoPerRowSpinner;

    // Number per Column
    Gtk::HBox             NoPerColBox;
    Gtk::Label            NoPerColLabel;
    Gtk::SpinButton       NoPerColSpinner;

    // padding in x
    Gtk::HBox             XPadBox;
    Gtk::Label            XPadLabel;
    Gtk::SpinButton       XPadSpinner;

    // padding in y
    Gtk::HBox             YPadBox;
    Gtk::Label            YPadLabel;
    Gtk::SpinButton       YPadSpinner;

    // Row height
    bool AutoRowSize;
    Gtk::VBox             RowHeightVBox;
    Gtk::HBox             RowHeightBox;
    Gtk::Label            RowHeightLabel;
    Gtk::SpinButton       RowHeightSpinner;
    Gtk::CheckButton      RowHeightButton;

    // Column width
    bool AutoColSize;
    Gtk::VBox             ColumnWidthVBox;
    Gtk::HBox             ColumnWidthBox;
    Gtk::Label            ColumnWidthLabel;
    Gtk::SpinButton       ColumnWidthSpinner;
    Gtk::CheckButton      ColumnWidthButton;

};




//#########################################################################
//## E V E N T S
//#########################################################################

/*
 *
 * This arranges the selection in a grid pattern.
 *
 */

void TileDialogImpl::Grid_Arrange ()
{

    int cnt;
    double grid_left,grid_top,col_width,row_height,paddingx,paddingy,width, height, new_x, new_y,cx,cy;
    col_width = 0;
    row_height = 0;
    paddingx = XPadSpinner.get_value();
    paddingy = YPadSpinner.get_value();
    grid_left = 9999;
    grid_top = 99999;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);
    const GSList *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        NRRect b;
        SPItem *item=SP_ITEM(items->data);
        sp_item_invoke_bbox(item, &b, sp_item_i2doc_affine(item), TRUE);
        width = fabs (b.x1 - b.x0);
        height = fabs (b.y1 - b.y0);
        cx = (b.x1 + b.x0)/2;
        cy = (b.y1 + b.y0)/2;

        #ifdef DEBUG_GRID_ARRANGE
        g_print("\n cx = %f cy= %f gridleft=%f",cx,cy,grid_left);
        #endif

        if (cx < grid_left) grid_left = cx;
        if (cy < grid_top) grid_top = cy;
        if (width > col_width) col_width = width;
        if (height > row_height) row_height = height;
    }
    grid_top = grid_top - (row_height / 2);
    grid_left = grid_left - (col_width/2);

       #ifdef DEBUG_GRID_ARRANGE
            g_print("\n gridleft=%f",grid_left);
        #endif
    // if auto set, update the spinners to reflect the value
    if (ColumnWidthButton.get_active()) ColumnWidthSpinner.set_value(col_width);
    if (RowHeightButton.get_active()) RowHeightSpinner.set_value(row_height);
    // otherwise get the spinner values to use.
    if (!ColumnWidthButton.get_active()) col_width = ColumnWidthSpinner.get_value();
    if (!RowHeightButton.get_active())  row_height = RowHeightSpinner.get_value();

    cnt=0;
    const GSList *items2 = selection->itemList();
    GSList *rev = g_slist_copy((GSList *) items2);
    rev = g_slist_sort(rev, (GCompareFunc) sp_item_repr_compare_position);

#ifdef DEBUG_GRID_ARRANGE
        g_print("\n row_height = %f col_width= %f",row_height,col_width);
        g_print("\n grid_left = %f grid_top= %f",grid_left,grid_top);
#endif

    int noPerRow = NoPerRowSpinner.get_value_as_int();
    for (; rev != NULL; rev = rev->next) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) rev->data);
            SPItem *item=SP_ITEM(rev->data);
            NRRect b;
            sp_item_invoke_bbox(item, &b, sp_item_i2doc_affine(item), TRUE);
            width = b.x1 - b.x0;
            height = b.y1 - b.y0;
            new_x = grid_left + ((col_width - width)/2) + (( col_width + paddingx ) * (cnt % noPerRow));
            new_y = grid_top + ((row_height - height)/2) +(( row_height + paddingy ) * (cnt / noPerRow));
            #ifdef DEBUG_GRID_ARRANGE
            g_print("\n x0 = %f x1= %f new_x= %f",b.x0,b.x1, new_x);
            g_print("\n width = %f ",width);
            #endif
            NR::Point move = NR::Point(new_x-b.x0, b.y0 - new_y);
            NR::Matrix const &affine = NR::Matrix(NR::translate(move));
            sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * affine);
            sp_item_write_transform(item, repr, item->transform,  NULL);
            SP_OBJECT (rev->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            cnt +=1;
    }
    sp_document_done (SP_DT_DOCUMENT (desktop));

}




//#########################################################################
//## E V E N T S
//#########################################################################


/**
 * Default response from the dialog.  Let's intercept it
 */
void TileDialogImpl::responseCallback(int response_id)
{

    if (response_id == GTK_RESPONSE_OK)
        {
        Grid_Arrange();
        }
    else if (response_id == GTK_RESPONSE_CANCEL)
        {
        }
    else
        {
        hide();
        return;
        }
}


/**
 * changed value in # of columns spinbox.
 */
void TileDialogImpl::on_row_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);

    GSList const *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);

    double PerCol = ceil(selcount / NoPerRowSpinner.get_value());
    NoPerColSpinner.set_value(PerCol);
    prefs_set_double_attribute ("dialogs.gridtiler", "NoPerRow", NoPerRowSpinner.get_value());
    updating=false;
}

/**
 * changed value in # of rows spinbox.
 */
void TileDialogImpl::on_col_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);

    GSList const *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);

    double PerRow = ceil(selcount / NoPerColSpinner.get_value());
    NoPerRowSpinner.set_value(PerRow);
    prefs_set_double_attribute ("dialogs.gridtiler", "NoPerRow", PerRow);

    updating=false;
}

/**
 * changed value in x padding spinbox.
 */
void TileDialogImpl::on_xpad_spinbutton_changed()
{

    prefs_set_double_attribute ("dialogs.gridtiler", "XPad", XPadSpinner.get_value());

}

/**
 * changed value in y padding spinbox.
 */
void TileDialogImpl::on_ypad_spinbutton_changed()
{

    prefs_set_double_attribute ("dialogs.gridtiler", "YPad", YPadSpinner.get_value());

}


/**
 * checked/unchecked autosize Rows button.
 */
void TileDialogImpl::on_RowSize_checkbutton_changed()
{

   if (RowHeightButton.get_active()) {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoRowSize", 20);
   } else {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoRowSize", -20);
   }
   RowHeightBox.set_sensitive ( !RowHeightButton.get_active());
}

/**
 * checked/unchecked autosize Rows button.
 */
void TileDialogImpl::on_ColSize_checkbutton_changed()
{

   if (ColumnWidthButton.get_active()) {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoColSize", 20);
   } else {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoColSize", -20);
   }
   ColumnWidthBox.set_sensitive ( !ColumnWidthButton.get_active());

}

/**
 * changed value in columns spinbox.
 */
void TileDialogImpl::on_rowSize_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    prefs_set_double_attribute ("dialogs.gridtiler", "RowHeight", RowHeightSpinner.get_value());
    updating=false;

}

/**
 * changed value in rows spinbox.
 */
void TileDialogImpl::on_colSize_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    prefs_set_double_attribute ("dialogs.gridtiler", "ColWidth", ColumnWidthSpinner.get_value());
    updating=false;

}

/**
 * Desktop selection changed
 */
void TileDialogImpl::updateSelection()
{
    double width, height,col_width, row_height;
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    col_width=0;
    row_height=0;
    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);
    const GSList *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);
    for (; items != NULL; items = items->next) {
        NRRect b;
        SPItem *item=SP_ITEM(items->data);
        sp_item_invoke_bbox(item, &b, sp_item_i2doc_affine(item), TRUE);
        width = fabs (b.x1 - b.x0);
        height = fabs (b.y1 - b.y0);
        if (width > col_width) col_width = width;
        if (height > row_height) row_height = height;
    }

    // Update the number of rows assuming number of columns wanted remains same.
    double PerCol = ceil(selcount / NoPerRowSpinner.get_value());
    NoPerColSpinner.set_value(PerCol);

    // if the selection has less than the number set for one row, reduce it appropriately
    if (selcount<NoPerRowSpinner.get_value()) {
        double PerRow = ceil(selcount / NoPerColSpinner.get_value());
        NoPerRowSpinner.set_value(PerRow);
        prefs_set_double_attribute ("dialogs.gridtiler", "NoPerRow", PerRow);
    }
    updating=false;

}


/*##########################
## Experimental
##########################*/
static void hideCallback(GtkObject *object, gpointer dlgPtr)
{
    TileDialogImpl *dlg = (TileDialogImpl *) dlgPtr;
    dlg->hideF12();
}

static void unhideCallback(GtkObject *object, gpointer dlgPtr)
{
    TileDialogImpl *dlg = (TileDialogImpl *) dlgPtr;
    dlg->showF12();
}

static void updateSelectionCallback(Inkscape::Application *inkscape, Inkscape::Selection *selection, TileDialogImpl *dlg)
{
    TileDialogImpl *tledlg = (TileDialogImpl *) dlg;
    tledlg->updateSelection();
}

//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
TileDialogImpl::TileDialogImpl()
{
    {
    // This block is a much simplified version of the code used in all other dialogs for
    // saving/restoring geometry, transientising, passing events to the aplication, and
    // hiding/unhiding on F12. This code fits badly into gtkmm so it had to be abridged and
    // mutilated somewhat. This block should be removed when the same functionality is made
    // available to all gtkmm dialogs via a base class.
        GtkWidget *dlg = GTK_WIDGET(gobj());

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_SELECTION_GRIDTILE), title);
	    set_title(title);

        gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);

        sp_transientize (dlg);

        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );

        //g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
        //g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

    //  Dont have a clue why the below crashes IS every time you change the selection
        g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (updateSelectionCallback), this);
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (hideCallback), (void *)this );
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (unhideCallback), (void *)this );
    }

    // bool used by spin button callbacks to stop loops where they change each other.
    updating = false;

    Gtk::VBox *mainVBox = get_vbox();

#define MARGIN 4

    //##Set up the panel

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);

    //SelectionContentsLabel(_("objects selected:"));
    GSList const *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);

    /*#### Number of columns ####*/
    NoPerRowSpinner.set_digits(0);
    NoPerRowSpinner.set_increments(1, 5);
    NoPerRowSpinner.set_range(1.0, 100.0);
    double PerRow = prefs_get_double_attribute ("dialogs.gridtiler", "NoPerRow", 3);
    NoPerRowSpinner.set_value(PerRow);
    NoPerRowSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_row_spinbutton_changed));

    NoPerRowBox.pack_end(NoPerRowSpinner, false, false, MARGIN);
    tips.set_tip(NoPerRowSpinner, _("No of columns"));

    NoPerRowLabel.set_label(_("Columns:"));
    NoPerRowBox.pack_end(NoPerRowLabel, false, false, MARGIN);

    TileBox.pack_start(NoPerRowBox, false, false, MARGIN);


    /*#### Number of Rows ####*/

    NoPerColSpinner.set_digits(0);
    NoPerColSpinner.set_increments(1, 5);
    NoPerColSpinner.set_range(1.0, 100.0);
    double PerCol = selcount / PerRow;
    NoPerColSpinner.set_value(PerCol);
    NoPerColSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_col_spinbutton_changed));

    NoPerColBox.pack_end(NoPerColSpinner, false, false, MARGIN);
    tips.set_tip(NoPerColSpinner, _("No of Rows"));

    NoPerColLabel.set_label(_("Rows:"));
    NoPerColBox.pack_end(NoPerColLabel, false, false, MARGIN);

    TileBox.pack_start(NoPerColBox, false, false, MARGIN);


    /*#### X padding ####*/

    XPadSpinner.set_digits(1);
    XPadSpinner.set_increments(0.2, 2);
    XPadSpinner.set_range(0.0, 999.0);
    double XPad = prefs_get_double_attribute ("dialogs.gridtiler", "XPad", 15);
    XPadSpinner.set_value(XPad);
    XPadBox.pack_end(XPadSpinner, false, false, MARGIN);
    tips.set_tip(XPadSpinner, _("Horizontal spacing between columns"));
    XPadSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_xpad_spinbutton_changed));


    XPadLabel.set_label(_("Column spacing:"));
    XPadBox.pack_end(XPadLabel, false, false, MARGIN);

    TileBox.pack_start(XPadBox, false, false, MARGIN);


    /*#### Y Padding ####*/

    YPadSpinner.set_digits(1);
    YPadSpinner.set_increments(0.2, 2);
    YPadSpinner.set_range(0.0, 999.0);
    double YPad = prefs_get_double_attribute ("dialogs.gridtiler", "YPad", 15);
    YPadSpinner.set_value(YPad);
    YPadBox.pack_end(YPadSpinner, false, false, MARGIN);
    tips.set_tip(YPadSpinner, _("Vertical spacing between Rows"));
    YPadSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_ypad_spinbutton_changed));

    YPadLabel.set_label(_("Row spacing:"));
    YPadBox.pack_end(YPadLabel, false, false, MARGIN);

    TileBox.pack_start(YPadBox, false, false, MARGIN);


    /*#### Row Height ####*/
    RowHeightButton.set_label(_("Auto Scale Row Height: "));
    double AutoRow = prefs_get_double_attribute ("dialogs.gridtiler", "AutoRowSize", 15);
    if (AutoRow>0)
         AutoRowSize=true;
    else
         AutoRowSize=false;
    RowHeightButton.set_active(AutoRowSize);

    RowHeightVBox.pack_start(RowHeightButton, false, false, MARGIN);
    tips.set_tip(RowHeightButton, _("Automatically scale Rows to fit selected objects."));
    RowHeightButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::on_RowSize_checkbutton_changed));


    RowHeightSpinner.set_digits(1);
    RowHeightSpinner.set_increments(0.2, 2);
    RowHeightSpinner.set_range(1.0, 9999.0);
    double RowHeight = prefs_get_double_attribute ("dialogs.gridtiler", "RowHeight", 50);
    RowHeightSpinner.set_value(RowHeight);
    RowHeightBox.pack_end(RowHeightSpinner, false, false, MARGIN);
    RowHeightSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_rowSize_spinbutton_changed));
    tips.set_tip(ColumnWidthSpinner, _("Row height."));

    RowHeightLabel.set_label(_("Row height:"));
    RowHeightBox.pack_end(RowHeightLabel, false, false, MARGIN);
    RowHeightBox.set_sensitive (!RowHeightButton.get_active());


    RowHeightVBox.pack_end(RowHeightBox, false, false, MARGIN);
    TileBox.pack_start(RowHeightVBox, false, false, MARGIN);

    /*#### Column Width ####*/


    ColumnWidthButton.set_label(_("Auto Scale Column Width: "));
    double AutoCol = prefs_get_double_attribute ("dialogs.gridtiler", "AutoColSize", 15);
    if (AutoCol>0)
         AutoColSize=true;
    else
         AutoColSize=false;
    ColumnWidthButton.set_active(AutoColSize);

    ColumnWidthVBox.pack_start(ColumnWidthButton, false, false, MARGIN);
    tips.set_tip(ColumnWidthButton, _("Automatically scale Columnss to fit selected objects."));
    ColumnWidthButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::on_ColSize_checkbutton_changed));


    ColumnWidthSpinner.set_digits(1);
    ColumnWidthSpinner.set_increments(0.2, 2);
    ColumnWidthSpinner.set_range(1.0, 9999.0);
    double ColumnWidth = prefs_get_double_attribute ("dialogs.gridtiler", "ColumnWidth", 50);
    ColumnWidthSpinner.set_value(ColumnWidth);
    ColumnWidthBox.pack_end(ColumnWidthSpinner, false, false, MARGIN);
    ColumnWidthSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_colSize_spinbutton_changed));
    tips.set_tip(ColumnWidthSpinner, _("Column width."));

    ColumnWidthLabel.set_label(_("Column width:"));
    ColumnWidthBox.pack_end(ColumnWidthLabel, false, false, MARGIN);
    ColumnWidthBox.set_sensitive ( !ColumnWidthButton.get_active() );

    ColumnWidthVBox.pack_end(ColumnWidthBox, false, false, MARGIN);
    TileBox.pack_start(ColumnWidthVBox, false, false, MARGIN);

    mainVBox->pack_start(TileBox);

    //## The OK button
    TileOkButton     = add_button(Gtk::Stock::OK,   GTK_RESPONSE_OK);
    tips.set_tip((*TileOkButton), _("Arrange Objects"));

    show_all_children();

    //## Connect the signal
    signal_response().connect(
    sigc::mem_fun(*this, &TileDialogImpl::responseCallback) );

}

/**
 * Factory method.  Use this to create a new TileDialog
 */
TileDialog *TileDialog::create()
{
    TileDialog *dialog = new TileDialogImpl();
    return dialog;
}


/**
 * Constructor
 */
TileDialogImpl::~TileDialogImpl()
{


}


/* static instance, to reduce dependencies */
static TileDialog *traceDialogInstance = NULL;

TileDialog *TileDialog::getInstance()
{
    if ( !traceDialogInstance )
        {
        traceDialogInstance = new TileDialogImpl();
        }
    return traceDialogInstance;
}



void TileDialog::showInstance()
{
    TileDialog *traceDialog = getInstance();
    traceDialog->show();
}


//#########################################################################
//## M E T H O D S
//#########################################################################

void TileDialogImpl::show()
{
    userHidden = false;
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();
    Gtk::Dialog::present();
}

void TileDialogImpl::showF12()
{
    if (userHidden)
        return;
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();

}


void TileDialogImpl::hide()
{
    userHidden = true;
    //call super()
    Gtk::Dialog::hide();
}

void TileDialogImpl::hideF12()
{
    //userHidden = true;
    //call super()
    Gtk::Dialog::hide();
}




} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



