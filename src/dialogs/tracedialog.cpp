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
#include <trace/trace.h>
#include <trace/potrace/inkscape-potrace.h>

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>

#include <gtk/gtk.h>

#include <dialogs/dialog-events.h>
#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include "helper/sp-intl.h"
#include "interface.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "inkscape.h"
#include "macros.h"

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
 * A dialog for adjusting bitmap->vector tracing parameters
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
    void showF12();

    /**
     * Do not show the dialog
     */
    void hide();
    void hideF12();

    /**
     * Callback from OK or Cancel
     */
    void responseCallback(int response_id);

    private:

    bool userHidden;

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
    Gtk::CheckButton     potraceInvertButton;
    Gtk::HBox               potraceInvertBox;
    Gtk::Button           *potraceOkButton;
    Gtk::Button           *potraceCancelButton;

    //brightness
    Gtk::Frame            potraceBrightnessFrame;
    Gtk::VBox             potraceBrightnessVBox;
    Gtk::HBox             potraceBrightnessBox;
    Gtk::RadioButton      potraceBrightnessRadioButton;
    Gtk::Label            potraceBrightnessSpinnerLabel;
    Gtk::SpinButton       potraceBrightnessSpinner;

    //edge detection
    Gtk::Frame            potraceCannyFrame;
    Gtk::HBox             potraceCannyBox;
    Gtk::VBox             potraceCannyVBox;
    Gtk::RadioButton      potraceCannyRadioButton;
    //Gtk::HSeparator     potraceCannySeparator;
    //Gtk::Label          potraceCannyLoSpinnerLabel;
    //Gtk::SpinButton     potraceCannyLoSpinner;
    Gtk::Label            potraceCannyHiSpinnerLabel;
    Gtk::SpinButton       potraceCannyHiSpinner;

    //quantization
    Gtk::Frame            potraceQuantFrame;
    Gtk::HBox             potraceQuantBox;
    Gtk::VBox             potraceQuantVBox;
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
    Gtk::VBox             potraceCreditsVBox;
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
        if (potraceCancelButton)
            potraceCancelButton->set_sensitive(true);
        if (potraceOkButton)
            potraceOkButton->set_sensitive(false);
        tracer.convertImageToPath(&pte);
        if (potraceCancelButton)
            potraceCancelButton->set_sensitive(false);
        if (potraceOkButton)
            potraceOkButton->set_sensitive(true);
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


/*##########################
## Experimental
##########################*/
static void hideCallback(GtkObject *object, gpointer dlgPtr)
{
    TraceDialogImpl *dlg = (TraceDialogImpl *) dlgPtr;
    dlg->hideF12();
}

static void unhideCallback(GtkObject *object, gpointer dlgPtr)
{
    TraceDialogImpl *dlg = (TraceDialogImpl *) dlgPtr;
    dlg->showF12();
}



//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
TraceDialogImpl::TraceDialogImpl()
{
    { 
    // This block is a much simplified version of the code used in all other dialogs for
    // saving/restoring geometry, transientising, passing events to the aplication, and
    // hiding/unhiding on F12. This code fits badly into gtkmm so it had to be abridged and
    // mutilated somewhat. This block should be removed when the same functionality is made
    // available to all gtkmm dialogs via a base class.
        GtkWidget *dlg = GTK_WIDGET(gobj());

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_SELECTION_POTRACE), title);
	set_title(title);

        gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);

        sp_transientize (dlg);
                           
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );
                        
        //g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
        //g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (hideCallback), (void *)this );
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (unhideCallback), (void *)this );
    }

    Gtk::VBox *mainVBox = get_vbox();

#define MARGIN 4

    //##Set up the Potrace panel

    /*#### brightness ####*/
    potraceBrightnessRadioButton.set_label(_("Brightness"));
    potraceGroup = potraceBrightnessRadioButton.get_group();
    potraceBrightnessBox.pack_start(potraceBrightnessRadioButton, false, false, MARGIN);

    potraceBrightnessSpinner.set_digits(3);
    potraceBrightnessSpinner.set_increments(0.01, 0.1);
    potraceBrightnessSpinner.set_range(0.0, 1.0);
    potraceBrightnessSpinner.set_value(0.45);
    potraceBrightnessBox.pack_end(potraceBrightnessSpinner, false, false, MARGIN);

    potraceBrightnessSpinnerLabel.set_label(_("Threshold:"));
    potraceBrightnessBox.pack_end(potraceBrightnessSpinnerLabel, false, false, MARGIN);

    potraceBrightnessVBox.pack_start(potraceBrightnessBox, false, false, MARGIN);

    potraceBrightnessFrame.set_label(_("Image Brightness"));
    potraceBrightnessFrame.add(potraceBrightnessVBox);
    potraceBox.pack_start(potraceBrightnessFrame, false, false, 0);

    /*#### canny edge detection ####*/
    // TRANSLATORS: "Canny" is the name of the inventor of this edge detection method
    potraceCannyRadioButton.set_label(_("Optimal Edge Detection (Canny)"));
    potraceCannyRadioButton.set_group(potraceGroup);
    potraceCannyBox.pack_start(potraceCannyRadioButton, false, false, MARGIN);
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
    potraceCannyHiSpinner.set_digits(3);
    potraceCannyHiSpinner.set_increments(0.01, 0.1);
    potraceCannyHiSpinner.set_range(0.0, 1.0);
    potraceCannyHiSpinner.set_value(0.65);
    potraceCannyBox.pack_end(potraceCannyHiSpinner, false, false, MARGIN);

    potraceCannyHiSpinnerLabel.set_label(_("Threshold:"));
    potraceCannyBox.pack_end(potraceCannyHiSpinnerLabel, false, false, MARGIN);

    potraceCannyVBox.pack_start(potraceCannyBox, false, false, MARGIN);

    potraceCannyFrame.set_label(_("Edge Detection"));
    potraceCannyFrame.add(potraceCannyVBox);
    potraceBox.pack_start(potraceCannyFrame, false, false, 0);

    /*#### quantization ####*/
    // TRANSLATORS: Color Quantization: the process of reducing the number of colors
    //  in an image by selecting an optimized set of representative colors and then
    //  re-applying this reduced set to the original image.
    potraceQuantRadioButton.set_label(_("Color Quantization"));
    potraceQuantRadioButton.set_group(potraceGroup);
    potraceQuantBox.pack_start(potraceQuantRadioButton, false, false, MARGIN);

    potraceQuantNrColorSpinner.set_digits(2);
    potraceQuantNrColorSpinner.set_increments(1.0, 4.0);
    potraceQuantNrColorSpinner.set_range(2.0, 64.0);
    potraceQuantNrColorSpinner.set_value(8.0);
    potraceQuantBox.pack_end(potraceQuantNrColorSpinner, false, false, MARGIN);

    potraceQuantNrColorLabel.set_label(_("Colors:"));
    potraceQuantBox.pack_end(potraceQuantNrColorLabel, false, false, MARGIN);

    potraceQuantVBox.pack_start(potraceQuantBox, false, false, MARGIN);

    potraceQuantFrame.set_label(_("Quantization / Reduction"));
    potraceQuantFrame.add(potraceQuantVBox);
    potraceBox.pack_start(potraceQuantFrame, false, false, 0);
  
    /*#### Preview ####*/
    potracePreviewButton.set_label(_("Preview"));
    potracePreviewButton.signal_clicked().connect( 
         sigc::mem_fun(*this, &TraceDialogImpl::potracePreviewCallback) );
    potracePreviewBox.pack_end(potracePreviewButton, false, false, 0);//do not expand
    potracePreviewImage.set_size_request(100,100);
    //potracePreviewImage.set_alignment (Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
    potracePreviewBox.pack_start(potracePreviewImage, true, true, 0);
    potracePreviewFrame.set_label(_("Preview")); // I guess it's correct to call the "intermediate bitmap" a preview of the trace
    potracePreviewFrame.add(potracePreviewBox);
    potraceBox.pack_start(potracePreviewFrame, true, true, 0);

    /*#### quantization ####*/
    potraceInvertButton.set_label(_("Invert"));
    potraceInvertButton.set_active(false);
    potraceInvertBox.pack_end(potraceInvertButton, false, false, MARGIN);
    potraceBox.pack_start(potraceInvertBox, false, false, MARGIN);

    /*#### Credits ####*/
    potraceCreditsLabel.set_text(
         "Thanks to Peter Selinger, http://potrace.sourceforge.net"
                         );
    potraceCreditsVBox.pack_start(potraceCreditsLabel, false, false, MARGIN);
    potraceCreditsFrame.set_label(_("Credits"));
    potraceCreditsFrame.add(potraceCreditsVBox);
    potraceBox.pack_start(potraceCreditsFrame, false, false, 0);

    /*done */
    // TRANSLATORS: Potrace is an application for transforming bitmaps into
    //  vector graphics (http://potrace.sourceforge.net/)
    notebook.append_page(potraceBox, _("Potrace"));

    //##Set up the Other panel
    // This may be reenabled when we have another tracer; now an empty tab is confusing so I'm disabling it
    //    notebook.append_page(otherBox, _("Other"));

    //##Put the notebook on the dialog
    mainVBox->pack_start(notebook);

    //## The OK button
    potraceCancelButton = add_button(Gtk::Stock::STOP, GTK_RESPONSE_CANCEL);
    if (potraceCancelButton)
        potraceCancelButton->set_sensitive(false);
    potraceOkButton     = add_button(Gtk::Stock::OK,   GTK_RESPONSE_OK);

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
    userHidden = false;
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();

}

void TraceDialogImpl::showF12()
{
    if (userHidden)
        return;
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();

}


void TraceDialogImpl::hide()
{
    userHidden = true;
    //call super()
    Gtk::Dialog::hide();
}

void TraceDialogImpl::hideF12()
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



