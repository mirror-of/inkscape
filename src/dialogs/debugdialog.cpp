/*
 * A very simple dialog for displaying Inkscape messages. Messages
 * sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "debugdialog.h"

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>



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
class DebugDialogImpl : public DebugDialog, public Gtk::Dialog
{

    public:
    

    /**
     * Constructor
     */
    DebugDialogImpl();

    /**
     * Destructor
     */
    ~DebugDialogImpl();


    /**
     * Show the dialog
     */
    void show();

    /**
     * Do not show the dialog
     */
    void hide();

    /**
     * Clear all information from the dialog
     */
    void clear();

    /**
     * Display a message
     */
    void message(char *msg);
    


    private:


    Gtk::TextView messageText;





};




//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Also a public method.  Remove all text from the dialog
 */
void DebugDialogImpl::clear()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = messageText.get_buffer();
    buffer->erase(buffer->begin(), buffer->end());
}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
DebugDialogImpl::DebugDialogImpl()
{

    Gtk::VBox *vbox = get_vbox();

    Gtk::Button button("Hello World");

    button.signal_clicked().connect(
         sigc::mem_fun(*this, &DebugDialogImpl::clear));

    vbox->pack_start(button);

    //### Set up the text widget
    messageText.set_editable(false);
    vbox->pack_start(messageText);

}

/**
 * Factory method.  Use this to create a new DebugDialog
 */
DebugDialog *DebugDialog::create()
{
    DebugDialog *dialog = new DebugDialogImpl();
    return dialog;
}


/**
 * Constructor
 */
DebugDialogImpl::~DebugDialogImpl()
{


}


//#########################################################################
//## M E T H O D S
//#########################################################################

void DebugDialogImpl::show()
{

}



void DebugDialogImpl::hide()
{

}



void DebugDialogImpl::message(char *msg)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = messageText.get_buffer();
    Glib::ustring uMsg = msg;
    buffer->insert (buffer->end(), uMsg);
}





}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



