
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
#include "prefs-utils.h"

#include <extension/db.h>

#define IO_STORAGE_NAME "IOExtension"

namespace Inkscape
{
namespace UI
{
namespace Dialogs
{

/*#################################
# U T I L I T Y
#################################*/


static void
menu_switch (GtkWidget * widget, const Inkscape::Extension::Extension ** extension)
{
	Inkscape::Extension::DB::IOExtensionDescription * desc;

	desc = reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(g_object_get_data(G_OBJECT(widget), IO_STORAGE_NAME));
	// printf("The menu has changed to: %s\n", desc->name);
	*extension = desc->extension;
	return;
}

static void
checkbox_toggle (GtkWidget * widget, bool * append_extension)
{
	*append_extension = !*append_extension;
	//printf("Toggled Checkbox\n");
	return;
}


/*#################################
# F I L E    O P E N
#################################*/

struct FileOpenNativeData_def {
    GtkWidget  *dlg;
};

FileOpenDialog::FileOpenDialog(const char *dir,
           FileDialogType fileTypes, const char *title) {
	GSList *    extension_list;
	GSList *    current_item;
    GtkWidget * menu;
    GtkWidget * item;

    nativeData = (FileOpenNativeData *) g_malloc( sizeof(FileOpenNativeData));
    if ( !nativeData ) {
        //do we want an exception?
        return;
    }

	/* Initalize to Autodetect */
	extension = NULL;
	/* No filename to start out with */
	filename  = NULL;

	extension_list = Inkscape::Extension::db.get_input_list();
	if (extension_list == NULL) {
		// Another exception
		printf("Ted, you messed up somewhere\n");
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

    /*
    Add the selection type menu (svg/with extensions/etc) to the dialog
    */
    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION(dlg)->main_vbox),
        hb, FALSE, FALSE, 0);
    GtkWidget *om = gtk_option_menu_new ();
    gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, 0);

    menu = gtk_menu_new();
    item = gtk_menu_item_new_with_label(_("Autodetect"));
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_switch), (gpointer)(&extension));
	g_object_set_data(G_OBJECT(item), IO_STORAGE_NAME, extension_list->data);
    gtk_menu_append(GTK_MENU(menu), item);

	/* Seperator for looks */
	item = gtk_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), item);

	/* First one gets skipped, it is the autodetect one that is handled
	   above. */
	for (current_item = g_slist_next(extension_list);
            current_item != NULL;
            current_item = g_slist_next(current_item)) {
		item = gtk_menu_item_new_with_label((reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(current_item->data))->name);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_switch), (gpointer)(&extension));
		g_object_set_data(G_OBJECT(item), IO_STORAGE_NAME, current_item->data);
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(menu), item);
	}

    gtk_option_menu_set_menu (GTK_OPTION_MENU (om), menu);
    GtkWidget *l = gtk_label_new (_("File type:"));
    gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    gtk_widget_show_all (hb);

    nativeData->dlg      = dlg;
	Inkscape::Extension::db.free_list(extension_list);

	return;
}

FileOpenDialog::~FileOpenDialog()
{
    if ( nativeData ) {
        gtk_widget_destroy( nativeData->dlg );
        g_free( nativeData );
    }
	if (filename != NULL)
		g_free(filename);
}


bool
FileOpenDialog::show()
{

    if ( !nativeData ) {
        //do we want an exception?
        return FALSE;
    }
   
    GtkWidget *dlg = nativeData->dlg;

    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
    gint b = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_hide(GTK_WIDGET(dlg));

    if (b == GTK_RESPONSE_OK)
        {
        filename = g_strdup (
            gtk_file_selection_get_filename (GTK_FILE_SELECTION(dlg)));
        return TRUE;
        }
    else
        {
	    return FALSE;
	    }
}




/*#################################
# F I L E    S A V E
#################################*/

struct FileSaveNativeData_def {
    GtkWidget *dlg;
	bool append_extension;
};

FileSaveDialog::FileSaveDialog(
    const char *dir, FileDialogType fileTypes, const char *title, const char * default_key)

{
	GSList *    extension_list;
	GSList *    current_item;
    GtkWidget * menu;
    GtkWidget * item;
	GtkWidget * checkbox;
	guint       default_item;
	guint       i;

    nativeData = (FileSaveNativeData *) g_malloc( sizeof(FileSaveNativeData));
    if ( !nativeData ) {
        //do we want an exception?
        return;
    }
	nativeData->append_extension = (bool)prefs_get_int_attribute("dialogs.save_as", "append_extension", 1);

	/* Initalize to Autodetect */
	extension = NULL;
	/* No filename to start out with */
	filename  = NULL;

	extension_list = Inkscape::Extension::db.get_output_list();
	if (extension_list == NULL) {
		// Another exception
		printf("Ted, you messed up somewhere\n");
		return;
	}

    /*
    We will want to use FileChooserDialog here, but that will
    have to wait until Gtk2.4 is on all platforms
    */
    GtkWidget *dlg = gtk_file_selection_new (title);

	/* This doesn't seem to do anything useful, so I'm dropping it */
	/*
    if (dir != NULL)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg), dir);
	*/

    /*
    Add the selection type menu (svg/with extensions/etc) to the dialog
    */
    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION(dlg)->main_vbox),
        hb, FALSE, FALSE, 0);
    GtkWidget *om = gtk_option_menu_new ();
    gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, 0);

    menu = gtk_menu_new();
    item = gtk_menu_item_new_with_label(_("Autodetect"));
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_switch), (gpointer)(&extension));
	g_object_set_data(G_OBJECT(item), IO_STORAGE_NAME, extension_list->data);
	extension = NULL;
	default_item = 0;
    gtk_menu_append(GTK_MENU(menu), item);

	/* Seperator for looks */
	item = gtk_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), item);

	/* First one gets skipped, it is the autodetect one that is handled
	   above. */
	for (current_item = g_slist_next(extension_list), i = 2;
            current_item != NULL;
            current_item = g_slist_next(current_item), i++) {
        Inkscape::Extension::DB::IOExtensionDescription * currentIO;

		currentIO = reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(current_item->data);

		item = gtk_menu_item_new_with_label(currentIO->name);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_switch), (gpointer)(&extension));
		g_object_set_data(G_OBJECT(item), IO_STORAGE_NAME, current_item->data);
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(menu), item);

		if (default_key != NULL &&
			currentIO->extension->get_id() != NULL &&
			!strcmp(default_key, currentIO->extension->get_id())) {
			default_item = i;
			/* gtk_menu_item_select(GTK_MENU_ITEM(item)); */
			extension = currentIO->extension;
		}
	}


    gtk_option_menu_set_menu (GTK_OPTION_MENU (om), menu);
	gtk_option_menu_set_history(GTK_OPTION_MENU (om), default_item);
    GtkWidget *l = gtk_label_new (_("File type:"));
    gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    gtk_widget_show_all (hb);

	checkbox = gtk_check_button_new_with_label(_("Automatically append filename extension"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), nativeData->append_extension);
	g_signal_connect(G_OBJECT(checkbox), "toggled", G_CALLBACK(checkbox_toggle), (gpointer)&nativeData->append_extension);
	gtk_widget_show(checkbox);
    gtk_box_pack_end (GTK_BOX (GTK_FILE_SELECTION(dlg)->main_vbox),
        checkbox, FALSE, FALSE, 0);

    nativeData->dlg      = dlg;
	Inkscape::Extension::db.free_list(extension_list);

	return;
}

FileSaveDialog::~FileSaveDialog()
{
    if ( nativeData ) {
        gtk_widget_destroy( nativeData->dlg );
        g_free( nativeData );
    }
	if (filename != NULL)
		g_free(filename);
}

bool
FileSaveDialog::show() {

    if ( !nativeData ) {
        //do we want an exception?
        return FALSE;
    }
   
    GtkWidget *dlg = nativeData->dlg;

	/* Reset the default back to autodetect */
    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
    gint b = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_hide(GTK_WIDGET(dlg));

    if (b == GTK_RESPONSE_OK) {
        gchar * dialog_filename =
            (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION(dlg));
		g_free(filename);
	    filename = g_strdup(dialog_filename);

		if (nativeData->append_extension == TRUE && extension != NULL) {
			Inkscape::Extension::Output * omod = dynamic_cast<Inkscape::Extension::Output *>(extension);
			if (!g_str_has_suffix(filename, omod->get_extension())) {
				gchar * newfilename;
				newfilename = g_strdup_printf("%s%s", filename, omod->get_extension());
				g_free(filename);
				filename = newfilename;
			}
		}

		prefs_set_int_attribute("dialogs.save_as", "append_extension", (gint)nativeData->append_extension);
		if (extension != NULL)
			prefs_set_string_attribute("dialogs.save_as", "default", extension->get_id());
		else
			prefs_set_string_attribute("dialogs.save_as", "default", NULL);

		return TRUE;
	} else {
	    return FALSE;
	}
}



}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape

