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
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "debugdialog.h"

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>

#include <dialogs/dialog-events.h>
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
    
    /**
     * Redirect g_log() messages to this widget
     */
    void captureLogMessages();

    /**
     * Return g_log() messages to normal handling
     */
    void releaseLogMessages();



    private:


    Gtk::MenuBar menuBar;

    Gtk::Menu   fileMenu;

    Gtk::ScrolledWindow textScroll;

    Gtk::TextView messageText;

    guint dialogHandler;






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
    set_title(_("Messages"));
    set_size_request(300, 400);

    Gtk::VBox *mainVBox = get_vbox();

    //## Add a menu for clear()
    menuBar.items().push_back( Gtk::Menu_Helpers::MenuElem("_File", fileMenu) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem("_Clear",
           sigc::mem_fun(*this, &DebugDialogImpl::clear) ) );
    mainVBox->pack_start(menuBar, Gtk::PACK_SHRINK);
    

    //### Set up the text widget
    messageText.set_editable(false);
    textScroll.add(messageText);
    textScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    mainVBox->pack_start(textScroll);

    show_all_children();

    dialogHandler = 0;
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
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();

}



void DebugDialogImpl::hide()
{
    //call super()
    Gtk::Dialog::hide();
}



void DebugDialogImpl::message(char *msg)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = messageText.get_buffer();
    Glib::ustring uMsg = msg;
    buffer->insert (buffer->end(), uMsg);
}


/* static instance, to reduce dependencies */
static DebugDialog *debugDialogInstance = NULL;

DebugDialog *DebugDialog::getInstance()
{
    if ( !debugDialogInstance )
        {
        debugDialogInstance = new DebugDialogImpl();
        }
    return debugDialogInstance;
}



void DebugDialog::showInstance()
{
    DebugDialog *debugDialog = getInstance();
    debugDialog->show();
}


void dialogLoggingFunction(const gchar *log_domain,
                           GLogLevelFlags log_level,
                           const gchar *messageText,
                           gpointer user_data)
{
    DebugDialogImpl *dlg = (DebugDialogImpl *)user_data;

    dlg->message((char *)messageText);

}


void DebugDialogImpl::captureLogMessages()
{
    if ( !dialogHandler )
        {
        dialogHandler = g_log_set_handler(NULL,
            (GLogLevelFlags)(G_LOG_LEVEL_ERROR   | G_LOG_LEVEL_CRITICAL |
                             G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE  |
                             G_LOG_LEVEL_INFO    | G_LOG_LEVEL_DEBUG),
              dialogLoggingFunction,
              (gpointer)this);
        }
}

void DebugDialogImpl::releaseLogMessages()
{
    if ( dialogHandler )
        {
        g_log_remove_handler(NULL, dialogHandler);
        dialogHandler = 0;
        }
}



}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



