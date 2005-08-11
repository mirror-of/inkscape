/**
 * Jabber whiteboard connection dialog 
 *
 * Authors:
 * 	Jason Segal, Jonas Collaros, Stephen Montgomery, Brandi Soggs, Matthew Weinstock
 * 	(original authors)
 * 	David Yip <yipdw@rose-hulman.edu> (cleanup, Inkscape integration)
 *
 * 	Released under GNU GPL, read the file 'COPYING' for more information
 */


/**
 * IMPORTANT NOTE:  THIS CODE IS DEPRECATED.
 *
 * It is only included to satisfy function dependencies of other C/Gtk-style whiteboard dialogs,
 * which will soon be converted to C++/Gtkmm anyway.
 *
 * The real connection dialog code is in ui/dialog/whiteboard-connect.{cpp,h}.
 */


#include "util/ucompose.hpp"

#include <glibmm/i18n.h>

#include "jabber_whiteboard/session-manager.h"

#include "dialogs/whiteboard-connect-dialog.h"
#include "dialogs/whiteboard-common-dialog.h"
#include "dialogs/dockable.h"
#include "dialogs/docker.h"

#include "widgets/icon.h"

#include "helper/window.h"
#include "helper/unit-menu.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "interface.h"
#include "prefs-utils.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "verbs.h"

/**
 * Preferences data
 */
static gchar *prefs_path = "dialogs.whiteboard_connect";


/**
 * Creates a connection dialog.
 * \param msgText The text displayed by the dialog.
 */
GtkWidget *
sp_whiteboard_connect_dialog(gchar *msgText)
{
	if (!connect_dialog) {
		gint x, y, w, h;

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_CONNECT), title);

		connect_dialog = sp_window_new(title, TRUE);

		// position dialog
		x = prefs_get_int_attribute (prefs_path, "x", 0);
		y = prefs_get_int_attribute (prefs_path, "y", 0);
		w = prefs_get_int_attribute (prefs_path, "w", 0);
		h = prefs_get_int_attribute (prefs_path, "h", 0);
		
		if (x != 0 || y != 0) {
			gtk_window_move(reinterpret_cast<GtkWindow *>(connect_dialog), x, y);
		} else {
			gtk_window_set_position(GTK_WINDOW(connect_dialog), GTK_WIN_POS_CENTER);
		}

		if (w != 0 && h != 0) {
            gtk_window_resize (reinterpret_cast<GtkWindow *>(connect_dialog), w, h);
		}

		// set dialog transience
        sp_transientize (connect_dialog);
        connect_dialog_wd.win = connect_dialog;
        connect_dialog_wd.stop = 0;

		// signal setup
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &connect_dialog_wd );
                             
        gtk_signal_connect ( GTK_OBJECT (connect_dialog), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), connect_dialog);
                             
        gtk_signal_connect ( GTK_OBJECT (connect_dialog), "destroy", G_CALLBACK (sp_whiteboard_connect_dialog_destroy), NULL );
        gtk_signal_connect ( GTK_OBJECT (connect_dialog), "delete_event", G_CALLBACK (sp_whiteboard_connect_dialog_delete), connect_dialog);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_whiteboard_connect_dialog_delete), connect_dialog);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), connect_dialog);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), connect_dialog); //TODO  : delete this

		GtkWidget * sp_find_dialog_old (void);

		// tooltip setup
        GtkTooltips *tt = gtk_tooltips_new ();
        gtk_container_set_border_width (GTK_CONTAINER (connect_dialog), 4);

        // Toplevel vbox 
        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (connect_dialog), vb);

		// Display msgText
		if (msgText != NULL)
		{      
	   		GtkWidget *l = gtk_label_new (msgText);			
			gtk_label_set_line_wrap(GTK_LABEL(l), true);
    		gtk_box_pack_start (GTK_BOX(vb), l, FALSE, FALSE, 0);
			
			//spacer
			GtkWidget *spacer = gtk_label_new ("");			
			gtk_label_set_line_wrap(GTK_LABEL(spacer), true);
    		gtk_box_pack_start (GTK_BOX(vb), spacer, FALSE, FALSE, 0);
		}
        
        sp_whiteboard_connect_new_searchfield (connect_dialog, vb, _("_Server: "), "server", tt, _("Jabber Server Domain"));
        sp_whiteboard_connect_new_searchfield (connect_dialog, vb, _("_User Name: "), "username", tt, _("Jabber User Name"));
        sp_whiteboard_connect_new_searchfield (connect_dialog, vb, _("_Password: "), "password", tt, _("Jabber User Password")); 

        gtk_widget_show_all (vb);

        {
			GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            gtk_widget_show (hb);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

            sp_whiteboard_connect_new_button (connect_dialog, hb, _("_Cancel"), tt, _("Cancel"), sp_whiteboard_connect_dialog_Cancel);
            sp_whiteboard_connect_new_button (connect_dialog, hb, _("_Connect"), tt, _("Connect to Jabber Server"), sp_whiteboard_connect_dialog_do_connect);        
        }

		gtk_widget_show((GtkWidget *) connect_dialog);
		gtk_window_present ((GtkWindow *) connect_dialog);
		sp_whiteboard_connect_dialog_reset (NULL, G_OBJECT (connect_dialog));
	}

	return connect_dialog;
}

void
sp_whiteboard_connect_new_button (GtkWidget *dlg, GtkWidget *hb, const gchar *label, GtkTooltips *tt, const gchar *tip, void (*function) (GObject *, GObject *))
{
    GtkWidget *b = gtk_button_new_with_mnemonic (label);
    gtk_tooltips_set_tip (tt, b, tip, NULL);
    gtk_box_pack_start (GTK_BOX (hb), b, TRUE, TRUE, 0);
    g_signal_connect ( G_OBJECT (b), "clicked", G_CALLBACK (function), dlg );
    gtk_widget_show (b);
}

void
sp_whiteboard_connect_new_searchfield (GtkWidget *dlg, GtkWidget *vb, const gchar *label, const gchar *id, GtkTooltips *tt, const gchar *tip)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, 0);
    GtkWidget *l = gtk_label_new_with_mnemonic (label);
    gtk_widget_set_usize (l, WHITEBOARDCONNECT_LABELWIDTH, -1);
    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

    GtkWidget *tf = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (tf), 64);
    gtk_box_pack_start (GTK_BOX (hb), tf, TRUE, TRUE, 0);
    gtk_object_set_data (GTK_OBJECT (dlg), id, tf);
    gtk_tooltips_set_tip (tt, tf, tip, NULL);
    g_signal_connect ( G_OBJECT (tf), "activate", G_CALLBACK (sp_whiteboard_connect_dialog_do_connect), dlg );
    gtk_label_set_mnemonic_widget (GTK_LABEL(l), tf);

    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
}

/**
 * Callback for sp_whiteboard_connect_new_searchfield.
 */
void
sp_whiteboard_connect_dialog_do_connect (GObject *object, GObject *dlg)
{
	SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    GtkWidget *widget;
    const gchar *server, *username, *password;

    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "server"));
    server = gtk_entry_get_text (GTK_ENTRY(widget));

    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "username"));
    username = gtk_entry_get_text (GTK_ENTRY(widget));

    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "password"));
    password = gtk_entry_get_text (GTK_ENTRY(widget));

	// TRANSLATORS: %1 is the Jabber server name, %2 is the Jabber user name
	Glib::ustring msg = String::ucompose(_("Connecting and authenticating to Jabber server <b>%1</b> as user <b>%2</b>"), server, username); 

    desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, msg.data());

	/*
    switch (SP_ACTIVE_DESKTOP->whiteboard_session_manager()->connectToServer((char *)server, (char *)username, (char *)password)) {
		case CONNECT_SUCCESS:
			// TRANSLATORS: %1 is the Jabber server name, %2 is the Jabber user name
			msg = String::ucompose(_("Connection established to Jabber server <b>%1</b> as user <b>%2</b>"), server, username);
			break;
		case INVALID_AUTH:
			// TRANSLATORS: %1 is the Jabber server name, %2 is the user name
			msg = String::ucompose(_("Failed to authenticate as user <b>%1</b> to Jabber server <b>%2</b>"), username, server);
			break;
		case FAILED_TO_CONNECT:
			// TRANSLATORS: %1 is the Jabber server name
			msg = String::ucompose(_("Failed to establish connection to Jabber server <b>%1</b>"), server);
			break;
	}

	desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, msg.data());
    close_dialog(GTK_WIDGET(dlg));    
	*/
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
