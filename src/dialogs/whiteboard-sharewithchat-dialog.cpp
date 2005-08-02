/**
 * Jabber share with chatroom
 *
 * Authors:
 * 	David Yip <yipdw@rose-hulman.edu> 
 *
 * 	Released under GNU GPL, read the file 'COPYING' for more information
 */


/**
 * NOTE:
 * This dialog, like the C/Gtk connection and user-to-user dialogs,
 * is slated for replacement.
 */
#include "dialogs/whiteboard-sharewithchat-dialog.h"
#include "dialogs/whiteboard-connect-dialog.h"
#include "dialogs/whiteboard-common-dialog.h"

#include "dialogs/dockable.h"
#include "dialogs/docker.h"

#include "util/ucompose.hpp"

#include "jabber_whiteboard/session-manager.h"

#include "helper/window.h"
#include "helper/unit-menu.h"

#include "widgets/icon.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "interface.h"
#include "prefs-utils.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "verbs.h"

#include <gtkmm/messagedialog.h>

#include <iostream>
#include <glibmm.h>

/**
 * Preferences data
 */
static gchar* prefs_path = "dialogs.sharewithchat_connect";


// Text fields
GtkWidget* chatroom_tf;
GtkWidget* server_tf;
GtkWidget* handle_tf;
GtkWidget* passwd_tf;

using Inkscape::Whiteboard::LOGGED_IN;

GtkWidget * 
sp_whiteboard_sharewithchat_dialog(gchar *msgText)
{
	Inkscape::Whiteboard::SessionManager* sm = SP_ACTIVE_DESKTOP->whiteboard_session_manager();
	if (!(sm->session_data && sm->session_data->status[LOGGED_IN])) {
		Gtk::MessageDialog dlg(_("You need to connect to a Jabber server before sharing a document with a chatroom."), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE);
		dlg.run();
		return NULL;
	} 

	if (!sharewithchat_dialog) {		// create it if necessary
		Inkscape::Whiteboard::SessionData* active_session = SP_ACTIVE_DESKTOP->whiteboard_session_manager()->session_data;

		g_log(NULL, G_LOG_LEVEL_DEBUG, "Session manager: %x", active_session);

        gchar title[500];
		gint x, y, w, h;

        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHCHAT), title);

        sharewithchat_dialog = sp_window_new (title, TRUE);
		x = prefs_get_int_attribute (prefs_path, "x", 0);
		y = prefs_get_int_attribute (prefs_path, "y", 0);
		w = prefs_get_int_attribute (prefs_path, "w", 0);
		h = prefs_get_int_attribute (prefs_path, "h", 0);

        if (x != 0 || y != 0) {
            gtk_window_move (reinterpret_cast<GtkWindow *>(sharewithchat_dialog), x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(sharewithchat_dialog), GTK_WIN_POS_CENTER);
        }

        if (w != 0 && h != 0) {
            gtk_window_resize (reinterpret_cast<GtkWindow *>(sharewithchat_dialog), w, h);
		}

        sp_transientize (sharewithchat_dialog);
        sharewithchat_wd.win = sharewithchat_dialog;
        sharewithchat_wd.stop = 0;
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &sharewithchat_wd );
                             
        gtk_signal_connect ( GTK_OBJECT (sharewithchat_dialog), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), sharewithchat_dialog);
                             
        gtk_signal_connect ( GTK_OBJECT (sharewithchat_dialog), "destroy", G_CALLBACK (sp_whiteboard_sharewithchat_dialog_destroy), NULL );
        gtk_signal_connect ( GTK_OBJECT (sharewithchat_dialog), "delete_event", G_CALLBACK (sp_whiteboard_sharewithchat_dialog_delete), sharewithchat_dialog);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_whiteboard_sharewithchat_dialog_delete), sharewithchat_dialog);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), sharewithchat_dialog);
		g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), sharewithchat_dialog);//TODO  : delete this

        GtkTooltips *tt = gtk_tooltips_new ();
        gtk_container_set_border_width (GTK_CONTAINER (sharewithchat_dialog), 4);

        /* Toplevel vbox */
        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (sharewithchat_dialog), vb);
        
        //Display any error message to the user
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

		GtkWidget *hb = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vb), hb, TRUE, TRUE, 0);

		GtkWidget *crbox = gtk_hbox_new (FALSE, 0);

		// Chatroom name label and text entry
		GtkWidget *crlabel = gtk_label_new_with_mnemonic ( _("_Chatroom name: ") );
//		gtk_widget_set_usize (crlabel, WHITEBOARDCONNECT_LABELWIDTH, -1);

		gtk_misc_set_alignment (GTK_MISC (crlabel), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (crbox), crlabel, FALSE, FALSE, 0);

		chatroom_tf = gtk_entry_new ();

		// TODO: remove after debugging is done!
		gtk_entry_set_text(GTK_ENTRY(chatroom_tf), "inkboard");

		gtk_entry_set_max_length (GTK_ENTRY (chatroom_tf), 64);
		gtk_box_pack_start (GTK_BOX (crbox), chatroom_tf , TRUE, TRUE, 0);
		gtk_object_set_data (GTK_OBJECT (sharewithchat_dialog), "chatroom", chatroom_tf );
		gtk_tooltips_set_tip (tt, chatroom_tf, _("Enter the chatroom name here"), NULL);

		gtk_box_pack_start (GTK_BOX (vb), crbox, TRUE, TRUE, 0);

		GtkWidget *pwbox = gtk_hbox_new (FALSE, 0);

		// Password label and text entry
		GtkWidget *pwlabel = gtk_label_new_with_mnemonic ( _("Chatroom _password: ") );
//		gtk_widget_set_usize (pwlabel, WHITEBOARDCONNECT_LABELWIDTH, -1);
		gtk_misc_set_alignment (GTK_MISC (pwlabel), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (pwbox), pwlabel, FALSE, FALSE, 0);

		passwd_tf = gtk_entry_new ();

		gtk_entry_set_max_length (GTK_ENTRY (passwd_tf), 64);
		gtk_box_pack_start (GTK_BOX (pwbox), passwd_tf , TRUE, TRUE, 0);
		gtk_object_set_data (GTK_OBJECT (sharewithchat_dialog), "password", passwd_tf );
		gtk_tooltips_set_tip (tt, passwd_tf , _("If a password is required, enter the password here.\n If no password is required, leave this field blank."), NULL);

		gtk_box_pack_start (GTK_BOX (vb), pwbox, TRUE, TRUE, 0);

		GtkWidget *serverbox = gtk_hbox_new (FALSE, 0);
		// Chatroom server label and text entry
		GtkWidget *snlabel = gtk_label_new_with_mnemonic ( _("_Server name: ") );
//		gtk_widget_set_usize (snlabel, WHITEBOARDCONNECT_LABELWIDTH, -1);
		gtk_misc_set_alignment (GTK_MISC (snlabel), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (serverbox), snlabel, FALSE, FALSE, 0);

		server_tf = gtk_entry_new ();
		// TODO: remove after debugging is done!
//		gtk_entry_set_text(GTK_ENTRY(server_tf), "lothlann-conf");

		gtk_entry_set_max_length (GTK_ENTRY (server_tf), 64);
		gtk_box_pack_start (GTK_BOX (serverbox), server_tf , TRUE, TRUE, 0);
		gtk_object_set_data (GTK_OBJECT (sharewithchat_dialog), "server", server_tf );
		gtk_tooltips_set_tip (tt, server_tf , _("Enter the Jabber server name here"), NULL);

		gtk_box_pack_start (GTK_BOX (vb), serverbox, TRUE, TRUE, 0);

		GtkWidget *hnbox = gtk_hbox_new (FALSE, 0);
		// Handle label and text entry
		GtkWidget *hnlabel = gtk_label_new_with_mnemonic ( _("_Handle: ") );
//		gtk_widget_set_usize (hnlabel, WHITEBOARDCONNECT_LABELWIDTH, -1);
		gtk_misc_set_alignment (GTK_MISC (hnlabel), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hnbox), hnlabel, FALSE, FALSE, 0);

		handle_tf = gtk_entry_new ();
		// TODO: remove after debugging is done!
		Glib::ustring jid = lm_connection_get_jid(active_session->connection);
		Glib::ustring nick = jid.substr(0, jid.find_first_of('@'));
		gtk_entry_set_text(GTK_ENTRY(handle_tf), nick.data());

		gtk_entry_set_max_length (GTK_ENTRY (handle_tf), 64);
		gtk_box_pack_start (GTK_BOX (hnbox), handle_tf , TRUE, TRUE, 0);
		gtk_object_set_data (GTK_OBJECT (sharewithchat_dialog), "handle", handle_tf );
		gtk_tooltips_set_tip (tt, handle_tf , _("Enter your Jabber handle here"), NULL);

		gtk_box_pack_start (GTK_BOX (vb), hnbox, TRUE, TRUE, 0);
		
//		g_signal_connect ( G_OBJECT (share_tf ), "activate", G_CALLBACK (sp_whiteboard_connect_dialog_do_connect), sharewithchat_dialog );
		gtk_label_set_mnemonic_widget   (GTK_LABEL(crlabel), chatroom_tf );
		gtk_label_set_mnemonic_widget   (GTK_LABEL(snlabel), server_tf );
		gtk_label_set_mnemonic_widget   (GTK_LABEL(hnlabel), handle_tf );
		gtk_label_set_mnemonic_widget   (GTK_LABEL(pwlabel), passwd_tf );

//		gtk_box_pack_start (GTK_BOX (vb), hb2, FALSE, FALSE, 0);

		gtk_widget_show_all (vb);

        {
			GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            gtk_widget_show (hb);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

            // TRANSLATORS: "Clear" is a verb here
            sp_whiteboard_connect_new_button (sharewithchat_dialog, hb, _("_Cancel"), tt, _("Cancel"), sp_whiteboard_connect_dialog_Cancel);
            sp_whiteboard_connect_new_button (sharewithchat_dialog, hb, _("_Share"), tt, _("Initialize Direct Sharing"), sp_whiteboard_connect_dialog_sharewithchat);        
        }

		gtk_widget_show((GtkWidget *) sharewithchat_dialog);
		gtk_window_present ((GtkWindow *) sharewithchat_dialog);
	}

	return sharewithchat_dialog;
}

void
sp_whiteboard_connect_dialog_sharewithchat(GObject *object, GObject *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    GtkWidget *widget;
    const gchar* chatroom, * server, * handle, * password;

//	Glib::ustring msg("Initializing sharing with chatroom <b>");
    
	Inkscape::XML::Node *root = sp_document_repr_root(SP_DT_DOCUMENT (desktop));

	// Extract data
    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "chatroom"));
  	chatroom = gtk_entry_get_text (GTK_ENTRY(widget));

    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "password"));
  	password = gtk_entry_get_text (GTK_ENTRY(widget));

    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "server"));
  	server = gtk_entry_get_text (GTK_ENTRY(widget));

    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "handle"));
  	handle = gtk_entry_get_text (GTK_ENTRY(widget));

//	msg = msg + chatroom + "@" + server + " as user " + handle + "</b>";
//
	Glib::ustring msg = String::ucompose("Synchronizing with chatroom <b>%1@%2</b> using the handle <b>%3</b>", chatroom, server, handle);

    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, msg.data());

	desktop->whiteboard_session_manager()->sendRequestToChatroom(server, chatroom, handle, password, NULL);

    close_dialog(GTK_WIDGET(dlg));    
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
