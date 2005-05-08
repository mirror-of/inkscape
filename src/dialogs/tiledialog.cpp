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
//#define DEBUG_GRID_ARRANGE 0

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

/*
 *    0    positions are equivalent
 *    1    first object's position is greater than the second
 *   -1    first object's position is less than the second
 */
int
sp_compare_x_position(SPItem *first, SPItem *second)
{
    NRRect a;
    sp_item_invoke_bbox(first, &a, sp_item_i2doc_affine(first), TRUE);
    NRRect b;
    sp_item_invoke_bbox(second, &b, sp_item_i2doc_affine(second), TRUE);

    if (a.x0 > b.x0) return 1;
    if (a.x0 > b.x0) return -1;
    return 0;
}

/*
 *    0    positions are equivalent
 *    1    first object's position is greater than the second
 *   -1    first object's position is less than the second
 */
int
sp_compare_y_position(SPItem *first, SPItem *second)
{
    NRRect a;
    sp_item_invoke_bbox(first, &a, sp_item_i2doc_affine(first), TRUE);
    NRRect b;
    sp_item_invoke_bbox(second, &b, sp_item_i2doc_affine(second), TRUE);

    if (a.y0 > b.y0) return 1;
    if (a.y0 > b.y0) return -1;
    return 0;
}

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
    void Spacing_button_changed();
    void VertAlign_changed();
    void HorizAlign_changed();
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


    Gtk::HBox             AlignHBox;
    Gtk::HBox             SpinsHBox;
    Gtk::HBox             SizesHBox;

    // Number per Row
    Gtk::VBox             NoOfColsBox;
    Gtk::Label            NoOfColsLabel;
    Gtk::SpinButton       NoOfColsSpinner;
    bool AutoRowSize;
    Gtk::CheckButton      RowHeightButton;

    Gtk::VBox             XByYLabelVBox;
    Gtk::Label            padXByYLabel;
    Gtk::Label            XByYLabel;

    // Number per Column
    Gtk::VBox             NoOfRowsBox;
    Gtk::Label            NoOfRowsLabel;
    Gtk::SpinButton       NoOfRowsSpinner;
    bool AutoColSize;
    Gtk::CheckButton      ColumnWidthButton;

    // Vertical align
    Gtk::Label            VertAlignLabel;
    Gtk::HBox             VertAlignHBox;
    Gtk::VBox             VertAlignVBox;
    Gtk::RadioButtonGroup VertAlignGroup;
    Gtk::RadioButton      VertCentreRadioButton;
    Gtk::RadioButton      VertTopRadioButton;
    Gtk::RadioButton      VertBotRadioButton;
    double VertAlign;

    // Horizontal align
    Gtk::Label            HorizAlignLabel;
    Gtk::VBox             HorizAlignVBox;
    Gtk::HBox             HorizAlignHBox;
    Gtk::RadioButtonGroup HorizAlignGroup;
    Gtk::RadioButton      HorizCentreRadioButton;
    Gtk::RadioButton      HorizLeftRadioButton;
    Gtk::RadioButton      HorizRightRadioButton;
    double HorizAlign;

    // padding in x
    Gtk::VBox             XPadBox;
    Gtk::Label            XPadLabel;
    Gtk::SpinButton       XPadSpinner;

    // padding in y
    Gtk::VBox             YPadBox;
    Gtk::Label            YPadLabel;
    Gtk::SpinButton       YPadSpinner;

    // BBox or manual spacing
    Gtk::VBox             SpacingVBox;
    Gtk::RadioButtonGroup SpacingGroup;
    Gtk::RadioButton      SpaceByBBoxRadioButton;
    Gtk::RadioButton      SpaceManualRadioButton;
    bool ManualSpacing;



    // Row height
    Gtk::VBox             RowHeightVBox;
    Gtk::HBox             RowHeightBox;
    Gtk::Label            RowHeightLabel;
    Gtk::SpinButton       RowHeightSpinner;

    // Column width
    Gtk::VBox             ColumnWidthVBox;
    Gtk::HBox             ColumnWidthBox;
    Gtk::Label            ColumnWidthLabel;
    Gtk::SpinButton       ColumnWidthSpinner;

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

    int cnt,row_cnt,col_cnt,a,row,col;
    double grid_left,grid_top,col_width,row_height,paddingx,paddingy,width, height, new_x, new_y,cx,cy;
    double total_col_width,total_row_height;
    col_width = 0;
    row_height = 0;
    total_col_width=0;
    total_row_height=0;


    // set padding to manual values
    paddingx = XPadSpinner.get_value();
    paddingy = YPadSpinner.get_value();

    std::vector<double> row_heights;
    std::vector<double> col_widths;
    std::vector<double> row_ys;
    std::vector<double> col_xs;

    int NoOfCols = NoOfColsSpinner.get_value_as_int();
    int NoOfRows = NoOfRowsSpinner.get_value_as_int();

    width = 0;
    for (a=0;a<NoOfCols; a++){
        col_widths.push_back(width);
    }

    height = 0;
    for (a=0;a<NoOfRows; a++){
        row_heights.push_back(height);
    }
    grid_left = 99999;
    grid_top = 99999;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);
    const GSList *items = selection->itemList();
    cnt=0;
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

    // require the sorting done before we can calculate row heights etc.

    const GSList *items2 = selection->itemList();
    GSList *rev = g_slist_copy((GSList *) items2);
    GSList *sorted = NULL;
    rev = g_slist_sort(rev, (GCompareFunc) sp_compare_y_position);

    for (row_cnt=0; ((rev != NULL) && (row_cnt<NoOfRows)); row_cnt++) {

             GSList *sort_current_row = NULL;
             for (col_cnt = 0; ((rev != NULL) && (col_cnt<NoOfCols)); col_cnt++) {
                 sort_current_row = g_slist_append (sort_current_row, rev->data);
                 rev = rev->next;
             }
             sort_current_row = g_slist_sort(sort_current_row, (GCompareFunc) sp_compare_x_position);
             sorted = g_slist_concat(sorted, sort_current_row);
         }

    // Calculate individual Row and Column sizes if necessary

    if ((!ColumnWidthButton.get_active()) || (!RowHeightButton.get_active())){
        cnt=0;
        const GSList *items3 = sorted;
        GSList *sizes = g_slist_copy((GSList *) items3);
        for (; sizes != NULL; sizes = sizes->next) {
            NRRect b;
            SPItem *item=SP_ITEM(sizes->data);
            sp_item_invoke_bbox(item, &b, sp_item_i2doc_affine(item), TRUE);
            width = fabs (b.x1 - b.x0);
            height = fabs (b.y1 - b.y0);
            if (width > col_widths[(cnt % NoOfCols)]) col_widths[(cnt % NoOfCols)] = width;
            if (height > row_heights[(cnt / NoOfCols)]) row_heights[(cnt / NoOfCols)] = height;
            cnt++;
        }
    }

    // Calculate total widths and heights, allowing for columns and rows non uniformly sized.

    if (ColumnWidthButton.get_active()){
        total_col_width = col_width * NoOfCols;
        col_widths.clear();
        for (a=0;a<NoOfCols; a++){
            col_widths.push_back(col_width);
        }
    } else {
        for (a = 0; a < col_widths.size(); a++)
        {
          total_col_width += col_widths[a] ;
        }
    }

    if (RowHeightButton.get_active()){
        total_row_height = row_height * NoOfRows;
        row_heights.clear();
        for (a=0;a<NoOfRows; a++){
            row_heights.push_back(row_height);
        }
    } else {
        for (a = 0; a < row_heights.size(); a++)
        {
          total_row_height += row_heights[a] ;
        }
    }


    // Fit to bbox, calculate padding between rows accordingly.
    if (!SpaceManualRadioButton.get_active()){
        NRRect b;
        selection->bounds(&b);
        g_print("\n row = %f     col = %f", total_row_height,total_col_width);
        paddingx = (fabs (b.x1 - b.x0) - total_col_width) / (NoOfCols - 1);
        paddingy = (fabs (b.y1 - b.y0) - total_row_height) / (NoOfRows - 1);
    }

    // Calculate row and column x and y coords required to allow for columns and rows which are non uniformly sized.

    for (a=0;a<NoOfCols; a++){
        if (a<1) col_xs.push_back(0);
        else col_xs.push_back(col_widths[a-1]+paddingx+col_xs[a-1]);
    }


    for (a=0;a<NoOfRows; a++){
        if (a<1) row_ys.push_back(0);
        else row_ys.push_back(row_heights[a-1]+paddingy+row_ys[a-1]);
    }

    cnt=0;
  for (row_cnt=0; ((sorted != NULL) && (row_cnt<NoOfRows)); row_cnt++) {

             GSList *current_row = NULL;
             for (col_cnt = 0; ((sorted != NULL) && (col_cnt<NoOfCols)); col_cnt++) {
                 current_row = g_slist_append (current_row, sorted->data);
                 sorted = sorted->next;
             }

             for (; current_row != NULL; current_row = current_row->next) {
                 SPItem *item=SP_ITEM(current_row->data);
                 Inkscape::XML::Node *repr = SP_OBJECT_REPR(item);
                 NRRect b;
                 sp_item_invoke_bbox(item, &b, sp_item_i2doc_affine(item), TRUE);
                 width = b.x1 - b.x0;
                 height = b.y1 - b.y0;
                 row = cnt / NoOfCols;
                 col = cnt % NoOfCols;


                // original before I started fecking about with it.
                // new_x = grid_left + (((col_width - width)/2)*HorizAlign) + (( col_width + paddingx ) * (cnt % NoOfCols));
                // new_y = grid_top + (((row_height - height)/2)*VertAlign) +(( row_height + paddingy ) * (cnt / NoOfCols));

                 new_x = grid_left + (((col_widths[col] - width)/2)*HorizAlign) + col_xs[col];
                 new_y = grid_top + (((row_heights[row] - height)/2)*VertAlign) + row_ys[row];

                 NR::Point move = NR::Point(new_x-b.x0, b.y0 - new_y);
                 NR::Matrix const &affine = NR::Matrix(NR::translate(move));
                 sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * affine);
                 sp_item_write_transform(item, repr, item->transform,  NULL);
                 SP_OBJECT (current_row->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                 cnt +=1;
             }
             g_slist_free (current_row);
    }

    NRRect b;
            selection->bounds(&b);
            //g_print("\n NEW b.x0 = %f b.x1= %f b.y0 = %f b.y1= %f",b.x0,b.x1,b.y0,b.y1);


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

    double PerCol = ceil(selcount / NoOfColsSpinner.get_value());
    NoOfRowsSpinner.set_value(PerCol);
    prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", NoOfColsSpinner.get_value());
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

    double PerRow = ceil(selcount / NoOfRowsSpinner.get_value());
    NoOfColsSpinner.set_value(PerRow);
    prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", PerRow);

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
 * changed Radio button in Spacing group.
 */
void TileDialogImpl::Spacing_button_changed()
{
   if (SpaceManualRadioButton.get_active()) {
       prefs_set_double_attribute ("dialogs.gridtiler", "SpacingType", 20);
   } else {
       prefs_set_double_attribute ("dialogs.gridtiler", "SpacingType", -20);
   }

   SizesHBox.set_sensitive ( SpaceManualRadioButton.get_active());
}

/**
 * changed Radio button in Vertical Align group.
 */
void TileDialogImpl::VertAlign_changed()
{
   if (VertTopRadioButton.get_active()) {
       VertAlign = 0;
       prefs_set_double_attribute ("dialogs.gridtiler", "VertAlign", 0);
   } else if (VertBotRadioButton.get_active()){
       VertAlign = 2;
       prefs_set_double_attribute ("dialogs.gridtiler", "VertAlign", 2);
   } else if (VertCentreRadioButton.get_active()){
       VertAlign = 1;
       prefs_set_double_attribute ("dialogs.gridtiler", "VertAlign", 1);
   }

}

/**
 * changed Radio button in Vertical Align group.
 */
void TileDialogImpl::HorizAlign_changed()
{
   if (HorizLeftRadioButton.get_active()) {
       HorizAlign = 0;
       prefs_set_double_attribute ("dialogs.gridtiler", "HorizAlign", 0);
   } else if (HorizRightRadioButton.get_active()){
       prefs_set_double_attribute ("dialogs.gridtiler", "HorizAlign", 2);
       HorizAlign = 2;
   } else if (HorizCentreRadioButton.get_active()){
       HorizAlign = 1;
       prefs_set_double_attribute ("dialogs.gridtiler", "HorizAlign", 1);
   }

}

/**
 * Desktop selection changed
 */
void TileDialogImpl::updateSelection()
{
    double col_width, row_height;
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

    if (NoOfColsSpinner.get_value()>1){
        // Update the number of rows assuming number of columns wanted remains same.
        double NoOfRows = ceil(selcount / NoOfColsSpinner.get_value());
        NoOfRowsSpinner.set_value(NoOfRows);

        // if the selection has less than the number set for one row, reduce it appropriately
        if (selcount<NoOfColsSpinner.get_value()) {
            double NoOfCols = ceil(selcount / NoOfRowsSpinner.get_value());
            NoOfColsSpinner.set_value(NoOfCols);
            prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", NoOfCols);
        }
    } else {
        NoOfColsSpinner.set_value(selcount);
        prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", selcount);

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


    /*#### Number of Rows ####*/

    double PerRow = prefs_get_double_attribute ("dialogs.gridtiler", "NoOfCols", 3);
    double PerCol = selcount / PerRow;


    NoOfRowsLabel.set_label(_("Rows:"));
    NoOfRowsBox.pack_start(NoOfRowsLabel, false, false, MARGIN);

    NoOfRowsSpinner.set_digits(0);
    NoOfRowsSpinner.set_increments(1, 5);
    NoOfRowsSpinner.set_range(1.0, 100.0);
    NoOfRowsSpinner.set_value(PerCol);
    NoOfRowsSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_col_spinbutton_changed));

    NoOfRowsBox.pack_start(NoOfRowsSpinner, false, false, MARGIN);
    tips.set_tip(NoOfRowsSpinner, _("No of Rows"));


    RowHeightButton.set_label(_("Equal Height: "));
    double AutoRow = prefs_get_double_attribute ("dialogs.gridtiler", "AutoRowSize", 15);
    if (AutoRow>0)
         AutoRowSize=true;
    else
         AutoRowSize=false;
    RowHeightButton.set_active(AutoRowSize);

    NoOfRowsBox.pack_start(RowHeightButton, false, false, MARGIN);
    tips.set_tip(RowHeightButton, _("Automatically scale Rows to fit selected objects."));
    RowHeightButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::on_RowSize_checkbutton_changed));

 {

        /*#### Radio buttons to control vertical alignment ####*/

        VertAlignLabel.set_label(_("Align:"));
        VertAlignHBox.pack_start(VertAlignLabel, false, false, MARGIN);

        VertTopRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::VertAlign_changed));
        VertAlignGroup = VertTopRadioButton.get_group();
        VertAlignVBox.pack_start(VertTopRadioButton, false, false, MARGIN);

        VertCentreRadioButton.set_group(VertAlignGroup);
        VertCentreRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::VertAlign_changed));
        VertAlignVBox.pack_start(VertCentreRadioButton, false, false, MARGIN);

        VertBotRadioButton.set_group(VertAlignGroup);
        VertBotRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::VertAlign_changed));
        VertAlignVBox.pack_start(VertBotRadioButton, false, false, MARGIN);

        VertAlign = prefs_get_double_attribute ("dialogs.gridtiler", "VertAlign", 1);
        if (VertAlign == 0) {
            VertTopRadioButton.set_active(TRUE);
        }
        else if (VertAlign == 2) {
            VertCentreRadioButton.set_active(TRUE);
        }
        else if (VertAlign == 1){
            VertBotRadioButton.set_active(TRUE);
        }
        VertAlignHBox.pack_start(VertAlignVBox, false, false, MARGIN);
        NoOfRowsBox.pack_start(VertAlignHBox, false, false, MARGIN);
    }

    SpinsHBox.pack_start(NoOfRowsBox, false, false, MARGIN);


    /*#### Label for X ####*/
    padXByYLabel.set_label(_("   "));
    XByYLabelVBox.pack_start(padXByYLabel, false, false, MARGIN);
    XByYLabel.set_label(_(" X "));
    XByYLabelVBox.pack_start(XByYLabel, false, false, MARGIN);
    SpinsHBox.pack_start(XByYLabelVBox, false, false, MARGIN);

    /*#### Number of columns ####*/

    NoOfColsLabel.set_label(_("Columns:"));
    NoOfColsBox.pack_start(NoOfColsLabel, false, false, MARGIN);

    NoOfColsSpinner.set_digits(0);
    NoOfColsSpinner.set_increments(1, 5);
    NoOfColsSpinner.set_range(1.0, 100.0);
    NoOfColsSpinner.set_value(PerRow);
    NoOfColsSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_row_spinbutton_changed));

    NoOfColsBox.pack_start(NoOfColsSpinner, false, false, MARGIN);
    tips.set_tip(NoOfColsSpinner, _("No of columns"));

    ColumnWidthButton.set_label(_("Equal Width: "));
    double AutoCol = prefs_get_double_attribute ("dialogs.gridtiler", "AutoColSize", 15);
    if (AutoCol>0)
         AutoColSize=true;
    else
         AutoColSize=false;
    ColumnWidthButton.set_active(AutoColSize);

    NoOfColsBox.pack_start(ColumnWidthButton, false, false, MARGIN);
    tips.set_tip(ColumnWidthButton, _("Automatically scale Columns to fit selected objects."));
    ColumnWidthButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::on_ColSize_checkbutton_changed));



    {
        /*#### Radio buttons to control horizontal alignment ####*/

        HorizAlignLabel.set_label(_("Align:"));
        HorizAlignVBox.pack_start(HorizAlignLabel, false, false, MARGIN);

        HorizLeftRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::HorizAlign_changed));
        HorizAlignGroup = HorizLeftRadioButton.get_group();
        HorizAlignHBox.pack_start(HorizLeftRadioButton, false, false, MARGIN);

        HorizCentreRadioButton.set_group(HorizAlignGroup);
        HorizCentreRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::HorizAlign_changed));
        HorizAlignHBox.pack_start(HorizCentreRadioButton, false, false, MARGIN);

        HorizRightRadioButton.set_group(HorizAlignGroup);
        HorizRightRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::HorizAlign_changed));
        HorizAlignHBox.pack_start(HorizRightRadioButton, false, false, MARGIN);

        HorizAlign = prefs_get_double_attribute ("dialogs.gridtiler", "HorizAlign", 1);
        if (HorizAlign == 0) {
            HorizLeftRadioButton.set_active(TRUE);
        }
        else if (HorizAlign == 2) {
            HorizCentreRadioButton.set_active(TRUE);
        }
        else if (HorizAlign == 2) {
            HorizRightRadioButton.set_active(TRUE);
        }
        HorizAlignVBox.pack_start(HorizAlignHBox, false, false, MARGIN);

        NoOfColsBox.pack_start(HorizAlignVBox, false, false, MARGIN);

    }

    SpinsHBox.pack_start(NoOfColsBox, false, false, MARGIN);

    TileBox.pack_start(SpinsHBox, false, false, MARGIN);




    {
        /*#### Radio buttons to control spacing manually or to fit selection bbox ####*/
        SpaceByBBoxRadioButton.set_label(_("Fit into Selection BBox "));
        SpaceByBBoxRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::Spacing_button_changed));
        SpacingGroup = SpaceByBBoxRadioButton.get_group();


        SpacingVBox.pack_start(SpaceByBBoxRadioButton, false, false, MARGIN);

        SpaceManualRadioButton.set_label(_("Set Spacing: "));
        SpaceManualRadioButton.set_group(SpacingGroup);
        SpaceManualRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialogImpl::Spacing_button_changed));
        SpacingVBox.pack_start(SpaceManualRadioButton, false, false, MARGIN);

        double SpacingType = prefs_get_double_attribute ("dialogs.gridtiler", "SpacingType", 15);
        if (SpacingType>0) {
            ManualSpacing=true;
            SpaceManualRadioButton.set_active(ManualSpacing);
            SpaceByBBoxRadioButton.set_active(!ManualSpacing);
        }
        else {
            ManualSpacing=false;
            SpaceManualRadioButton.set_active(ManualSpacing);
            SpaceByBBoxRadioButton.set_active(!ManualSpacing);
        }


        TileBox.pack_start(SpacingVBox, false, false, MARGIN);

    }


    /*#### X padding ####*/


    XPadLabel.set_label(_("Column spacing:"));
    XPadBox.pack_start(XPadLabel, false, false, MARGIN);

    XPadSpinner.set_digits(1);
    XPadSpinner.set_increments(0.2, 2);
    XPadSpinner.set_range(0.0, 999.0);
    double XPad = prefs_get_double_attribute ("dialogs.gridtiler", "XPad", 15);
    XPadSpinner.set_value(XPad);
    XPadBox.pack_start(XPadSpinner, false, false, MARGIN);
    tips.set_tip(XPadSpinner, _("Horizontal spacing between columns"));
    XPadSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_xpad_spinbutton_changed));

    SizesHBox.pack_start(XPadBox, false, false, MARGIN);

    /*#### Y Padding ####*/

    YPadLabel.set_label(_("Row spacing:"));
    YPadBox.pack_start(YPadLabel, false, false, MARGIN);

    YPadSpinner.set_digits(1);
    YPadSpinner.set_increments(0.2, 2);
    YPadSpinner.set_range(0.0, 999.0);
    double YPad = prefs_get_double_attribute ("dialogs.gridtiler", "YPad", 15);
    YPadSpinner.set_value(YPad);
    YPadBox.pack_start(YPadSpinner, false, false, MARGIN);
    tips.set_tip(YPadSpinner, _("Vertical spacing between Rows"));
    YPadSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialogImpl::on_ypad_spinbutton_changed));


    SizesHBox.pack_start(YPadBox, false, false, MARGIN);

    TileBox.pack_start(SizesHBox, false, false, MARGIN);



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



