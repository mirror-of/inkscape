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

    /**
     * This is the big almighty McGuffin
     */
    Inkscape::Trace tracer;

    /**
     * This does potrace processing
     * Only preview if do_i_trace is false
     */
    void potraceProcess(bool do_i_trace);

    /**
     * Abort processing
     */
    void abort();

    void potracePreviewCallback();

    Gtk::Notebook   notebook;

    //########## Potrace items
    Gtk::VBox             potraceBox;
    Gtk::RadioButtonGroup potraceGroup;
    Gtk::ToggleButton     potraceInvertButton;

    //brightness
    Gtk::Frame            potraceBrightnessFrame;
    Gtk::HBox             potraceBrightnessBox;
    Gtk::RadioButton      potraceBrightnessRadioButton;
    Gtk::Label            potraceBrightnessSpinnerLabel;
    Gtk::SpinButton       potraceBrightnessSpinner;

    //edge detection
    Gtk::Frame            potraceCannyFrame;
    Gtk::HBox             potraceCannyBox;
    Gtk::RadioButton      potraceCannyRadioButton;
    //Gtk::HSeparator     potraceCannySeparator;
    //Gtk::Label          potraceCannyLoSpinnerLabel;
    //Gtk::SpinButton     potraceCannyLoSpinner;
    Gtk::Label            potraceCannyHiSpinnerLabel;
    Gtk::SpinButton       potraceCannyHiSpinner;

    //quantization
    Gtk::Frame            potraceQuantFrame;
    Gtk::HBox             potraceQuantBox;
    Gtk::RadioButton      potraceQuantRadioButton;
    Gtk::Label            potraceQuantNrColorLabel;
    Gtk::SpinButton       potraceQuantNrColorSpinner;

    //preview
    Gtk::Frame            potracePreviewFrame;
    Gtk::HBox             potracePreviewBox;
    Gtk::Button           potracePreviewButton;
    Gtk::Image            potracePreviewImage;

    //credits
    Gtk::Frame            potraceCreditsFrame;
    Gtk::Label            potraceCreditsLabel;

    //########## Other items
    Gtk::VBox             otherBox;



};




//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * This does potrace processing
 * Only preview if do_i_trace is false
 */
void TraceDialogImpl::potraceProcess(bool do_i_trace)
{
    //##### Get the tracer and engine
    Inkscape::Potrace::PotraceTracingEngine pte;

    //##### Get the settings
    /* which one? */
    pte.setUseBrightness(potraceBrightnessRadioButton.get_active());
    pte.setUseCanny(potraceCannyRadioButton.get_active());
    pte.setUseQuantization(potraceQuantRadioButton.get_active());

    /* brightness */
    double brightnessThreshold = potraceBrightnessSpinner.get_value();
    pte.setBrightnessThreshold(brightnessThreshold);

    /* canny */
    //double cannyLowThreshold = potraceCannyLoSpinner.get_value();
    //pte.setCannyLowThreshold(cannyLowThreshold);
    double cannyHighThreshold = potraceCannyHiSpinner.get_value();
    pte.setCannyHighThreshold(cannyHighThreshold);

    /* quantization */
    int quantNrColors = potraceQuantNrColorSpinner.get_value_as_int();
    pte.setQuantizationNrColors(quantNrColors);

    /* inversion */
    bool invert = potraceInvertButton.get_active();
    pte.setInvert(invert);

    //##### Get intermediate bitmap image
    GdkPixbuf *pixbuf = tracer.getSelectedImage();
    if (pixbuf)
         {
         GdkPixbuf *preview = pte.preview(pixbuf);
         if (preview)
             {
             Glib::RefPtr<Gdk::Pixbuf> thePreview = Glib::wrap(preview);
             int width  = thePreview->get_width();
             int height = thePreview->get_height();
             double scaleFactor = 100.0 / (double)height;
             int newWidth  = (int) (((double)width)  * scaleFactor);
             int newHeight = (int) (((double)height) * scaleFactor);
             Glib::RefPtr<Gdk::Pixbuf> scaledPreview =
                    thePreview->scale_simple(newWidth, newHeight,
                       Gdk::INTERP_NEAREST);
             //g_object_unref(preview);
             potracePreviewImage.set(scaledPreview);
             }
         }

    //##### Convert
    if (do_i_trace)
        {
        tracer.convertImageToPath(&pte);
        }

}


/**
 * Abort processing
 */
void TraceDialogImpl::abort()
{
    //### Do some GUI thing
 
    //### Make the abort() call to the tracer
    tracer.abort();
    
}



//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Callback from the Preview button.  Can be called from elsewhere.
 */
void TraceDialogImpl::potracePreviewCallback()
{
    potraceProcess(false);
}

/**
 * Default response from the dialog.  Let's intercept it
 */
void TraceDialogImpl::responseCallback(int response_id)
{
    
    if (response_id == GTK_RESPONSE_OK)
        {
        int panelNr = notebook.get_current_page();
        //g_message("selected panel:%d\n", panelNr);

        if (panelNr == 0)
            {
            potraceProcess(true);
            }
        }
    else if (response_id == GTK_RESPONSE_CANCEL)
        {
        abort();
        }
    else
        {
        hide();
        return;
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
    set_size_request(380, 400);

    Gtk::VBox *mainVBox = get_vbox();


    //##Set up the Potrace panel

    /*#### brightness ####*/
    potraceBrightnessRadioButton.set_label(_("Brightness"));
    potraceGroup = potraceBrightnessRadioButton.get_group();
    potraceBrightnessBox.pack_start(potraceBrightnessRadioButton);
    potraceBrightnessSpinnerLabel.set_label(_("Threshold"));
    potraceBrightnessBox.pack_start(potraceBrightnessSpinnerLabel);
    potraceBrightnessSpinner.set_digits(5);
    potraceBrightnessSpinner.set_increments(0.01, 0.1);
    potraceBrightnessSpinner.set_range(0.0, 1.0);
    potraceBrightnessSpinner.set_value(0.5);
    potraceBrightnessBox.pack_start(potraceBrightnessSpinner);
    potraceBrightnessFrame.set_label(_("Image Brightness"));
    potraceBrightnessFrame.add(potraceBrightnessBox);
    potraceBox.pack_start(potraceBrightnessFrame);

    /*#### canny edge detection ####*/
    potraceCannyRadioButton.set_label(_("Optimal Edge Detection (Canny)"));
    potraceCannyRadioButton.set_group(potraceGroup);
    potraceCannyBox.pack_start(potraceCannyRadioButton);
    /*
    potraceCannyBox.pack_start(potraceCannySeparator);
    potraceCannyLoSpinnerLabel.set_label(_("Low"));
    potraceCannyBox.pack_start(potraceCannyLoSpinnerLabel);
    potraceCannyLoSpinner.set_digits(5);
    potraceCannyLoSpinner.set_increments(0.01, 0.1);
    potraceCannyLoSpinner.set_range(0.0, 1.0);
    potraceCannyLoSpinner.set_value(0.1);
    potraceCannyBox.pack_start(potraceCannyLoSpinner);
    */
    potraceCannyHiSpinnerLabel.set_label(_("Threshold"));
    potraceCannyBox.pack_start(potraceCannyHiSpinnerLabel);
    potraceCannyHiSpinner.set_digits(5);
    potraceCannyHiSpinner.set_increments(0.01, 0.1);
    potraceCannyHiSpinner.set_range(0.0, 1.0);
    potraceCannyHiSpinner.set_value(0.65);
    potraceCannyBox.pack_start(potraceCannyHiSpinner);
    potraceCannyFrame.set_label(_("Edge Detection"));
    potraceCannyFrame.add(potraceCannyBox);
    potraceBox.pack_start(potraceCannyFrame);

    /*#### quantization ####*/
    potraceQuantRadioButton.set_label(_("Color Quantization"));
    potraceQuantRadioButton.set_group(potraceGroup);
    potraceQuantBox.pack_start(potraceQuantRadioButton);
    potraceQuantNrColorLabel.set_label(_("Colors"));
    potraceQuantBox.pack_start(potraceQuantNrColorLabel);
    potraceQuantNrColorSpinner.set_digits(2);
    potraceQuantNrColorSpinner.set_increments(1.0, 4.0);
    potraceQuantNrColorSpinner.set_range(4.0, 64.0);
    potraceQuantNrColorSpinner.set_value(8.0);
    potraceQuantBox.pack_start(potraceQuantNrColorSpinner);
    potraceQuantFrame.set_label(_("Quantization / Reduction"));
    potraceQuantFrame.add(potraceQuantBox);
    potraceBox.pack_start(potraceQuantFrame);

    /*#### quantization ####*/
    potraceInvertButton.set_label(_("Invert"));
    potraceInvertButton.set_active(false);
    potraceBox.pack_start(potraceInvertButton);

    
    /*#### Preview ####*/
    potracePreviewButton.set_label(_("Preview"));
    potracePreviewButton.signal_clicked().connect( 
         sigc::mem_fun(*this, &TraceDialogImpl::potracePreviewCallback) );
    potracePreviewBox.pack_start(potracePreviewButton, false, true, 0);//do not expand
    potracePreviewImage.set_size_request(100,100);
    //potracePreviewImage.set_alignment (Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
    potracePreviewBox.pack_start(potracePreviewImage);
    potracePreviewFrame.set_label(_("Intermediate Bitmap"));
    potracePreviewFrame.add(potracePreviewBox);
    potraceBox.pack_start(potracePreviewFrame);

    /*#### Credits ####*/
    potraceCreditsLabel.set_text(
         "Thanks to Peter Selinger, http://potrace.sourceforge.net"
                         );
    potraceCreditsFrame.set_label(_("Credits"));
    potraceCreditsFrame.add(potraceCreditsLabel);
    potraceBox.pack_start(potraceCreditsFrame);

    /*done */
    notebook.append_page(potraceBox, _("Potrace"));

    //##Set up the Other panel
    notebook.append_page(otherBox, _("Other"));

    //##Put the notebook on the dialog
    mainVBox->pack_start(notebook);

    //## The OK button
    //add_button(Gtk::Stock::STOP, GTK_RESPONSE_CANCEL);
    add_button(Gtk::Stock::OK,   GTK_RESPONSE_OK);

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



