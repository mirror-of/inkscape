/*
 * A simple dialog for setting the parameters for autotracing a
 * bitmap <image> into an svg <path>
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tracedialog.h"
#include <trace.h>
#include <potrace/inkscape-potrace.h>

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>

#include <dialogs/dialog-events.h>
#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include "helper/sp-intl.h"

#include <glib.h>

namespace Inkscape
{
namespace UI
{
namespace Dialogs
{


//#########################################################################
//## I M P L E M E N T A T I O N
//#########################################################################

/**
 * A dialog that displays log messages
 */
class TraceDialogImpl : public TraceDialog, public Gtk::Dialog
{

    public:
    

    /**
     * Constructor
     */
    TraceDialogImpl();

    /**
     * Destructor
     */
    ~TraceDialogImpl();


    /**
     * Show the dialog
     */
    void show();

    /**
     * Do not show the dialog
     */
    void hide();

    /**
     * Callback from OK or Cancel
     */
    void responseCallback(int response_id);


    private:


    Gtk::Notebook   notebook;

    // Potrace items
    Gtk::VBox       potraceBox;
    Gtk::RadioButtonGroup potraceGroup;

    Gtk::HBox        potraceBrightnessBox;
    Gtk::RadioButton potraceBrightnessRadioButton;
    Gtk::SpinButton  potraceBrightnessSpinner;
    Gtk::HBox        potraceCannyBox;
    Gtk::RadioButton potraceCannyRadioButton;
    Gtk::Frame       potracePreviewFrame;
    Gtk::Image       potracePreviewImage;

    // Other items
    Gtk::VBox       otherBox;



};




//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Default response from the dialog.  Let's intercept it
 */
void TraceDialogImpl::responseCallback(int response_id)
{
    if (response_id != GTK_RESPONSE_OK)
        {
        hide();
        return;
        }

    int panelNr = notebook.get_current_page();
    //g_message("selected panel:%d\n", panelNr);

    if (panelNr == 0)
        {
        Inkscape::Trace tracer;
        Inkscape::Potrace::PotraceTracingEngine pte;
        pte.setUseBrightness(potraceBrightnessRadioButton.get_active());
        pte.setUseCanny(potraceCannyRadioButton.get_active());
        double threshold = potraceBrightnessSpinner.get_value();
        pte.setBrightnessThreshold(threshold);
        tracer.convertImageToPath(&pte);
        }


}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
TraceDialogImpl::TraceDialogImpl()
{

    set_title(_("Bitmap Tracing"));
    set_size_request(250, 150);

    Gtk::VBox *mainVBox = get_vbox();


    //##Set up the Potrace panel
    /* brightness */
    potraceBrightnessRadioButton.set_label(_("Brightness Threshold"));
    potraceGroup = potraceBrightnessRadioButton.get_group();
    potraceBrightnessBox.pack_start(potraceBrightnessRadioButton);
    potraceBrightnessSpinner.set_digits(5);
    potraceBrightnessSpinner.set_increments(0.01, 0.1);
    potraceBrightnessSpinner.set_range(0.0, 1.0);
    potraceBrightnessSpinner.set_value(0.5);
    potraceBrightnessBox.pack_start(potraceBrightnessSpinner);
    potraceBox.pack_start(potraceBrightnessBox);

    /* canny edge detection */
    potraceCannyRadioButton.set_label(_("Canny Edge Detection"));
    potraceCannyRadioButton.set_group(potraceGroup);
    potraceCannyBox.pack_start(potraceCannyRadioButton);
    potraceBox.pack_start(potraceCannyBox);

    /*done */
    notebook.append_page(potraceBox, _("Potrace"));

    //##Set up the Other panel
    notebook.append_page(otherBox, _("Other"));

    //##Put the notebook on the dialog
    mainVBox->pack_start(notebook);

    //## The OK button
    add_button(Gtk::Stock::OK,     GTK_RESPONSE_OK);

    show_all_children();

    //## Connect the signal
    signal_response().connect( 
         sigc::mem_fun(*this, &TraceDialogImpl::responseCallback) );
}

/**
 * Factory method.  Use this to create a new TraceDialog
 */
TraceDialog *TraceDialog::create()
{
    TraceDialog *dialog = new TraceDialogImpl();
    return dialog;
}


/**
 * Constructor
 */
TraceDialogImpl::~TraceDialogImpl()
{


}


/* static instance, to reduce dependencies */
static TraceDialog *traceDialogInstance = NULL;

TraceDialog *TraceDialog::getInstance()
{
    if ( !traceDialogInstance )
        {
        traceDialogInstance = new TraceDialogImpl();
        }
    return traceDialogInstance;
}



void TraceDialog::showInstance()
{
    TraceDialog *traceDialog = getInstance();
    traceDialog->show();
}


//#########################################################################
//## M E T H O D S
//#########################################################################

void TraceDialogImpl::show()
{
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();

}



void TraceDialogImpl::hide()
{
    //call super()
    Gtk::Dialog::hide();
}




} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



