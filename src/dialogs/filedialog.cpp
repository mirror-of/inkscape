
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
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

#include "interface.h"
#include "filedialog.h"
#include "helper/sp-intl.h"
#include "prefs-utils.h"

#include <dialogs/dialog-events.h>
#include <extension/db.h>

#define IO_STORAGE_NAME "IOExtension"
#define EXTENSION_VAR   "VarExtension"
#define FILE_EXT_CHECK  "FileExtensionCheckbox"

namespace Inkscape
{
namespace UI
{
namespace Dialogs
{

/*#################################
# U T I L I T Y
#################################*/
static int
num_bytes_rec (const gchar * string, int count) {
	if (string[0] == '\0')
		return count;
	return num_bytes_rec(string++, count++);
}

static int
num_bytes (const gchar * string) {
	return num_bytes_rec(string, 0);
}

static void
save_menu_switch (GtkWidget * widget, GtkWidget * filesel)
{
	Inkscape::Extension::DB::IOExtensionDescription * desc;
	Inkscape::Extension::Extension ** extension;
	GtkWidget * checkbox;
	const gchar * old_extension = NULL;
	const gchar * filename;

	extension = reinterpret_cast<Inkscape::Extension::Extension **>(g_object_get_data(G_OBJECT(filesel), EXTENSION_VAR));
	checkbox = reinterpret_cast<GtkWidget *>(g_object_get_data(G_OBJECT(filesel), FILE_EXT_CHECK));

	if (extension != NULL && *extension != NULL) {
		Inkscape::Extension::Output * oext;

		oext = dynamic_cast<Inkscape::Extension::Output *>(*extension);
		old_extension = oext->get_extension();
	} else {
		old_extension = ".svg";
	}

	desc = reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(g_object_get_data(G_OBJECT(widget), IO_STORAGE_NAME));
	// printf("The menu has changed to: %s\n", desc->name);
	*extension = desc->extension;

	if (desc->extension == NULL) {
		gtk_widget_set_sensitive(checkbox, FALSE);
	} else {
		gtk_widget_set_sensitive(checkbox, TRUE);
	}
	
	/* Change file name here */
	filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel));
	if (extension != NULL &&
			*extension != NULL &&
			old_extension != NULL &&
			g_str_has_suffix(filename, old_extension)) {
		gchar * work0_filename;
		gchar * work1_filename;
		gchar * work2_filename;
		gchar * work3_filename;
		Inkscape::Extension::Output * oext;

		oext = dynamic_cast<Inkscape::Extension::Output *>(*extension);

		work0_filename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
		work1_filename = g_strndup(work0_filename, num_bytes(work0_filename) - num_bytes(old_extension));
		work2_filename = g_strconcat(work1_filename, oext->get_extension(), NULL);
		work3_filename = g_filename_from_utf8(work2_filename, -1, NULL, NULL, NULL);

		gtk_file_selection_set_filename(GTK_FILE_SELECTION(filesel), work3_filename);

		g_free(work0_filename);
		g_free(work1_filename);
		g_free(work2_filename);
		g_free(work3_filename);
	}

	return;
}

static void
open_menu_switch (GtkWidget * widget, GtkWidget * filesel)
{
	Inkscape::Extension::DB::IOExtensionDescription * desc;
	const Inkscape::Extension::Extension ** extension;

	extension = reinterpret_cast<const Inkscape::Extension::Extension **>(g_object_get_data(G_OBJECT(filesel), EXTENSION_VAR));

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

	/* Setting the extension variable */
	g_object_set_data(G_OBJECT(dlg), EXTENSION_VAR, (gpointer)&extension);

	/* Set the pwd and/or the filename */
    if (dir != NULL)
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
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(open_menu_switch), (gpointer)(dlg));
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
		Inkscape::Extension::DB::IOExtensionDescription * ioext = reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(current_item->data);

		item = gtk_menu_item_new_with_label(ioext->name);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(open_menu_switch), (gpointer)(dlg));
		g_object_set_data(G_OBJECT(item), IO_STORAGE_NAME, current_item->data);
		gtk_widget_set_sensitive(item, ioext->sensitive);
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
    sp_transientize (dlg);
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

	/* Setting the extension variable */
	g_object_set_data(G_OBJECT(dlg), EXTENSION_VAR, (gpointer)&extension);

	/* Set the pwd and/or the filename */
    if (dir != NULL)
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
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(save_menu_switch), (gpointer)(dlg));
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
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(save_menu_switch), (gpointer)(dlg));
		g_object_set_data(G_OBJECT(item), IO_STORAGE_NAME, current_item->data);
		gtk_widget_show(item);
		gtk_widget_set_sensitive(item, currentIO->sensitive);
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

	checkbox = gtk_check_button_new_with_label(_("Append filename extension automatically"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), nativeData->append_extension);
	g_signal_connect(G_OBJECT(checkbox), "toggled", G_CALLBACK(checkbox_toggle), (gpointer)&nativeData->append_extension);
	gtk_widget_show(checkbox);
    gtk_box_pack_end (GTK_BOX (GTK_FILE_SELECTION(dlg)->main_vbox),
        checkbox, FALSE, FALSE, 0);

	if (extension == NULL) {
		gtk_widget_set_sensitive(checkbox, FALSE);
	}

	g_object_set_data(G_OBJECT(dlg), FILE_EXT_CHECK, (gpointer)checkbox);

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
    sp_transientize (dlg);
    gint b = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_hide(GTK_WIDGET(dlg));

    if (b == GTK_RESPONSE_OK) {
        gchar * dialog_filename =
            (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION(dlg));

		if (g_file_test(dialog_filename, G_FILE_TEST_IS_DIR)) {
			sp_ui_error_dialog(_("No filename specified.  Unable to save."));
			return FALSE;
		}

		g_free(filename);
	    filename = g_strdup(dialog_filename);

		prefs_set_int_attribute("dialogs.save_as", "append_extension", (gint)nativeData->append_extension);
		if (extension != NULL)
			prefs_set_string_attribute("dialogs.save_as", "default", extension->get_id());
		else
			prefs_set_string_attribute("dialogs.save_as", "default", "");

		return TRUE;
	} else {
	    return FALSE;
	}
}



}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape

