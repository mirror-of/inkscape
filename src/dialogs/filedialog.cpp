
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkfilesel.h>

#include <string.h>

#include "filedialog.h"
#include "helper/sp-intl.h"


namespace Inkscape
{
namespace UI
{
namespace Dialogs
{


/*#################################
# F I L E    O P E N
#################################*/

struct FileOpenNativeData_def {
    GtkWidget  *dlg;
};


FileOpenDialog::FileOpenDialog(const char *dir,
           FileDialogType fileTypes, const char *title) {

    nativeData = (FileOpenNativeData *) g_malloc( sizeof(FileOpenNativeData));
    if ( !nativeData ) {
        //do we want an exception?
        return;
    }
    /*
    We will want to use FileChooserDialog here, but that will
    have to wait until Gtk2.4 is on all platforms
    */
    GtkWidget *dlg = gtk_file_selection_new (title);
    if ( !dir )
        dir = "";
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg), dir);


    nativeData->dlg      = dlg;
}

FileOpenDialog::~FileOpenDialog()
{
    if ( nativeData ) {
        gtk_widget_destroy( nativeData->dlg );
        g_free( nativeData );
    }
}


char *
FileOpenDialog::show()
{

    if ( !nativeData ) {
        //do we want an exception?
        return NULL;
    }
   
    GtkWidget *dlg = nativeData->dlg;

    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
    gint b = gtk_dialog_run (GTK_DIALOG (dlg));

    if (b == GTK_RESPONSE_OK)
        {
        gchar *filename = g_strdup (
            gtk_file_selection_get_filename (GTK_FILE_SELECTION(dlg)));
        return g_strdup(filename);
        }
    else
        {
	    return NULL;
	    }
}




/*#################################
# F I L E    S A V E
#################################*/

struct FileSaveNativeData_def {
    GtkWidget *dlg;
};

FileSaveDialog::FileSaveDialog(
    const char *dir, FileDialogType fileTypes, const char *title)

{

    nativeData = (FileSaveNativeData *) g_malloc( sizeof(FileSaveNativeData));
    if ( !nativeData ) {
        //do we want an exception?
        return;
    }

    /*
    We will want to use FileChooserDialog here, but that will
    have to wait until Gtk2.4 is on all platforms
    */
    GtkWidget *dlg = gtk_file_selection_new (title);
    if ( !dir )
        dir = "*.svg";
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg), dir);

    /*
    Add the selection type menu (svg/with extensions/etc) to the dialog
    */
    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION(dlg)->main_vbox),
        hb, FALSE, FALSE, 0);
    GtkWidget *om = gtk_option_menu_new ();
    gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, 0);

    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item = gtk_menu_item_new_with_label(_("Autodetect"));
    gtk_menu_append(GTK_MENU(menu), item);
    item = gtk_menu_item_new_with_label(
        _("Use the extension of the file to choose a filetype"));
    gtk_menu_append(GTK_MENU(menu), item);


    gtk_option_menu_set_menu (GTK_OPTION_MENU (om), menu);
    GtkWidget *l = gtk_label_new (_("File type:"));
    gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    gtk_widget_show_all (hb);

    nativeData->dlg      = dlg;

}

FileSaveDialog::~FileSaveDialog()
{
    if ( nativeData ) {
        gtk_widget_destroy( nativeData->dlg );
        g_free( nativeData );
    }
}

char *
FileSaveDialog::show() {

    if ( !nativeData ) {
        //do we want an exception?
        return NULL;
    }
   
    GtkWidget *dlg = nativeData->dlg;

    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
    gint b = gtk_dialog_run (GTK_DIALOG (dlg));

    if (b == GTK_RESPONSE_OK)
        {
        gchar *filename =
            (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION(dlg));
        selectionType = SVG_NAMESPACE_WITH_EXTENSIONS;
        //if ( strcmp(key, "xxx") == 0 )
        //    selectionType = SVG_NAMESPACE;
	    return g_strdup(filename);
        }
    else
        {
	    return NULL;
	    }

}






}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape

