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

    Gtk::Label      spinlabel;

    Gtk::SpinButton spinner;



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

    Inkscape::Trace tracer;
    Inkscape::Potrace::PotraceTracingEngine pte;
    double threshold = spinner.get_value();
    pte.setBrightnessThreshold(threshold);
    tracer.setTracingEngine(&pte);
    tracer.convertImageToPath();


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
    set_size_request(250, 100);

    Gtk::VBox *mainVBox = get_vbox();

    //##Set up the spinbutton
    spinlabel.set_text(_("Brightness Threshold"));
    mainVBox->pack_start(spinlabel);
    spinner.set_digits(5);
    spinner.set_increments(0.05, 0.25);
    spinner.set_range(0.0, 1.0);
    spinner.set_value(0.5);
    mainVBox->pack_start(spinner);

    add_button(Gtk::Stock::CANCEL, GTK_RESPONSE_CANCEL);
    add_button(Gtk::Stock::OK,     GTK_RESPONSE_OK);

    signal_response().connect( 
         sigc::mem_fun(*this, &TraceDialogImpl::responseCallback) );

    show_all_children();

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



