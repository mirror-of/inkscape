/*
 * A simple dialog for previewing icon representation.
 *
 * Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "iconpreview.h"

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/toolbutton.h>

#include <gtk/gtk.h>

#include <dialogs/dialog-events.h>
#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <glibmm/i18n.h>
#include "interface.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "inkscape.h"
#include "macros.h"
#include "document.h"
#include "desktop-handles.h"
#include "selection.h"
#include "display/nr-arena.h"

#include <glib.h>

extern "C" {
// takes doc, root, icon, and icon name to produce pixels
guchar *
sp_icon_doc_icon( SPDocument *doc, NRArenaItem *root,
                  const gchar *name, unsigned int psize );
}

namespace Inkscape {
namespace UI {
namespace Dialogs {



//#########################################################################
//## I M P L E M E N T A T I O N
//#########################################################################

/**
 * A simple dialog for previewing icon representation.
 */
class IconPreviewImpl : public IconPreview, public Gtk::Dialog
{
public:


    /**
     * Constructor
     */
    IconPreviewImpl();

    /**
     * Destructor
     */
    ~IconPreviewImpl();


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

    void refreshPreview();
    void renderPreview( SPObject* obj );

private:
    IconPreviewImpl(IconPreviewImpl const &); // no copy
    IconPreviewImpl &operator=(IconPreviewImpl const &); // no assign

    void on_button_clicked(int which);
    void updateMagnify();

    //void _setDesktop(SPDesktop *desktop);

    bool userHidden;

    //SPDesktop *_desktop;

    Gtk::Notebook   notebook;
    Gtk::Tooltips   tips;
    Gtk::VBox       iconBox;
    Gtk::HPaned     splitter;

    int hot;
    int numEntries;
    int* sizes;

    Gtk::Button           *refreshButton;

    guchar** pixMem;
    Gtk::Image** images;
    Gtk::Image* magnified;
    Glib::ustring** labels;
};




//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Default response from the dialog.  Let's intercept it
 */
void IconPreviewImpl::responseCallback(int response_id)
{

    if (response_id == GTK_RESPONSE_OK)
        {
        int panelNr = notebook.get_current_page();
        //g_message("selected panel:%d\n", panelNr);

        if (panelNr == 0)
            {
//             potraceProcess(true);
            }
        }
    else if ( response_id == GTK_RESPONSE_APPLY ) {
        // do it.
        refreshPreview();
    }
    else if (response_id == GTK_RESPONSE_CANCEL)
        {
//         abort();
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
    (void)object;

    IconPreviewImpl *dlg = (IconPreviewImpl *) dlgPtr;
    dlg->hideF12();
}

static void unhideCallback(GtkObject *object, gpointer dlgPtr)
{
    (void)object;

    IconPreviewImpl *dlg = (IconPreviewImpl *) dlgPtr;
    dlg->showF12();
}



//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
IconPreviewImpl::IconPreviewImpl() :
    hot(1),
    refreshButton(0),
    magnified(0)
{
    numEntries = 4;
    sizes = new int[numEntries];
    pixMem = new guchar*[numEntries];
    images = new Gtk::Image*[numEntries];
    labels = new Glib::ustring*[numEntries];

    sizes[0] = 16;
    labels[0] = new Glib::ustring("16x16");
    sizes[1] = 24;
    labels[1] = new Glib::ustring("24x24");
    sizes[2] = 32;
    labels[2] = new Glib::ustring("32x32");
    sizes[3] = 48;
    labels[3] = new Glib::ustring("48x48");

    for ( int i = 0; i < numEntries; i++ ) {
        pixMem[i] = 0;
        images[i] = 0;
    }

    {
    // This block is a much simplified version of the code used in all other dialogs for
    // saving/restoring geometry, transientising, passing events to the aplication, and
    // hiding/unhiding on F12. This code fits badly into gtkmm so it had to be abridged and
    // mutilated somewhat. This block should be removed when the same functionality is made
    // available to all gtkmm dialogs via a base class.
        GtkWidget *dlg = GTK_WIDGET(gobj());

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_VIEW_ICON_PREVIEW), title);
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

    /*done */
    notebook.append_page(iconBox, _("Icon"));

    //##Put the notebook on the dialog
    mainVBox->pack_start(notebook);


    Gtk::VBox* magBox = new Gtk::VBox();
    magnified = new Gtk::Image();
    Gtk::Alignment* align = new Gtk::Alignment();
    align->add( *magnified );
    magBox->add( *align );

    Gtk::VBox * verts = new Gtk::VBox();
    for ( int i = 0; i < numEntries; i++ ) {
        pixMem[i] = new guchar[4 * sizes[i] * sizes[i]];
        memset( pixMem[i], 0x00, 4 *  sizes[i] * sizes[i] );

        GdkPixbuf *pb = gdk_pixbuf_new_from_data( pixMem[i], GDK_COLORSPACE_RGB, TRUE, 8, sizes[i], sizes[i], sizes[i] * 4, /*(GdkPixbufDestroyNotify)nr_free*/NULL, NULL );
        GtkImage* img = GTK_IMAGE( gtk_image_new_from_pixbuf( pb ) );
        images[i] = Glib::wrap(img);
        Glib::ustring label(*labels[i]);
        Gtk::ToolButton* btn = new Gtk::ToolButton(label);
        btn->set_icon_widget(*images[i]);


        btn->signal_clicked().connect( sigc::bind<int>( sigc::mem_fun(*this, &IconPreviewImpl::on_button_clicked), i) );


        verts->add(*btn);
    }

    iconBox.pack_start(splitter);
    splitter.pack1( *magBox, true, true );
    splitter.pack2( *verts, false, false );


    //## The Refresh button

    refreshButton = add_button(Gtk::Stock::REFRESH, GTK_RESPONSE_APPLY);
    tips.set_tip((*refreshButton), _("Refresh the icons"));


    refreshPreview();

    show_all_children();

    //## Connect the signal
    signal_response().connect(
         sigc::mem_fun(*this, &IconPreviewImpl::responseCallback) );
}

void IconPreviewImpl::on_button_clicked(int which)
{
    hot = which;
    updateMagnify();
    queue_draw();
}


/**
 * Factory method.  Use this to create a new IconPreview
 */
IconPreview *IconPreview::create()
{
    IconPreview *dialog = new IconPreviewImpl();
    return dialog;
}


/**
 * Destructor
 */
IconPreviewImpl::~IconPreviewImpl()
{


}


/* static instance, to reduce dependencies */
static IconPreview *iconPreviewInstance = NULL;

IconPreview *IconPreview::getInstance()
{
    if ( !iconPreviewInstance )
        {
        iconPreviewInstance = new IconPreviewImpl();
        }
    return iconPreviewInstance;
}



void IconPreview::showInstance()
{
    IconPreview *iconPreviewDialog = getInstance();
    iconPreviewDialog->show();
}


//#########################################################################
//## M E T H O D S
//#########################################################################

void IconPreviewImpl::show()
{
    userHidden = false;
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();
    Gtk::Dialog::present();
}

void IconPreviewImpl::showF12()
{
    if (userHidden)
        return;
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();

}


void IconPreviewImpl::hide()
{
    userHidden = true;
    //call super()
    Gtk::Dialog::hide();
}

void IconPreviewImpl::hideF12()
{
    //userHidden = true;
    //call super()
    Gtk::Dialog::hide();
}

void IconPreviewImpl::refreshPreview()
{
    //g_message("refresh the icons");

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if ( desktop ) {
        g_assert( SP_IS_DESKTOP( desktop) );

        SPSelection * sel = SP_DT_SELECTION(desktop);
        if ( sel ) {
            //g_message("found a selection to play with");

            GSList const *items = sel->itemList();
            SPObject *target = 0;
            while ( items && !target ) {
                SPItem* item = SP_ITEM( items->data );
                SPObject * obj = SP_OBJECT(item);
                gchar const *id = SP_OBJECT_ID( obj );
                if ( id ) {
                    target = obj;
                }

                items = g_slist_next(items);
            }
            if ( target ) {
                renderPreview(target);
            }
        }
    }
}


void IconPreviewImpl::renderPreview( SPObject* obj )
{
    SPDocument * doc = SP_OBJECT_DOCUMENT(obj);
    gchar * id = SP_OBJECT_ID(obj);

//     g_message(" setting up to render '%s' as the icon", id );

    NRArenaItem *root = NULL;

    /* Create new arena */
    NRArena *arena = NRArena::create();

    /* Create ArenaItem and set transform */
    unsigned int visionkey = sp_item_display_key_new (1);

    /* fixme: Memory manage root if needed (Lauris) */
    root = sp_item_invoke_show ( SP_ITEM (SP_DOCUMENT_ROOT (doc)),
                                 arena, visionkey, SP_ITEM_SHOW_DISPLAY );

    for ( int i = 0; i < numEntries; i++ ) {
        guchar * px = sp_icon_doc_icon( doc, root, id, sizes[i] );
//         g_message( " size %d %s", sizes[i], (px ? "worked" : "failed") );
        if ( px ) {
            memcpy( pixMem[i], px, sizes[i] * sizes[i] * 4 );
            g_free( px );
            px = 0;
        } else {
            memset( pixMem[i], 0, sizes[i] * sizes[i] * 4 );
        }
        images[i]->queue_draw();
    }
    updateMagnify();
}

void IconPreviewImpl::updateMagnify()
{
    Glib::RefPtr<Gdk::Pixbuf> buf = images[hot]->get_pixbuf()->scale_simple( 128, 128, Gdk::INTERP_NEAREST );
    magnified->set( buf );
    magnified->queue_draw();
    magnified->get_parent()->queue_draw();
}

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################
