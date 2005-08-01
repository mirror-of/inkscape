/**
 * Jabber share with user dialog 
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
 * The real connection dialog code is in ui/dialog/whiteboard-sharewithuser.{cpp,h}.
 */
#include <iostream>

#include "dialogs/dockable.h"
#include "dialogs/docker.h"

#include "jabber_whiteboard/typedefs.h"
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

#include "dialogs/whiteboard-sharewithuser-dialog.h"
#include "dialogs/whiteboard-connect-dialog.h"
#include "dialogs/whiteboard-common-dialog.h"


/**
 * Preferences data
 */
static gchar* prefs_path = "dialogs.sharewithuser_connect";


// ??
GtkWidget* share_tf;

GtkWidget * 
sp_whiteboard_sharewithuser_dialog(gchar *msgText)
{
	if (!sharewithuser_dialog) {		// create it if necessary
		Inkscape::Whiteboard::SessionData* active_session = SP_ACTIVE_DESKTOP->whiteboard_session_manager()->session_data;

		g_log(NULL, G_LOG_LEVEL_DEBUG, "Session manager: %x", active_session);

        gchar title[500];
		gint x, y, w, h;

        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_WHITEBOARD_SHAREWITHUSER), title);

        sharewithuser_dialog = sp_window_new (title, TRUE);
		x = prefs_get_int_attribute (prefs_path, "x", 0);
		y = prefs_get_int_attribute (prefs_path, "y", 0);
		w = prefs_get_int_attribute (prefs_path, "w", 0);
		h = prefs_get_int_attribute (prefs_path, "h", 0);

        if (x != 0 || y != 0) {
            gtk_window_move (reinterpret_cast<GtkWindow *>(sharewithuser_dialog), x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(sharewithuser_dialog), GTK_WIN_POS_CENTER);
        }

        if (w != 0 && h != 0) {
            gtk_window_resize (reinterpret_cast<GtkWindow *>(sharewithuser_dialog), w, h);
		}

        sp_transientize (sharewithuser_dialog);
        sharewithuser_wd.win = sharewithuser_dialog;
        sharewithuser_wd.stop = 0;
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &sharewithuser_wd );
                             
        gtk_signal_connect ( GTK_OBJECT (sharewithuser_dialog), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), sharewithuser_dialog);
                             
        gtk_signal_connect ( GTK_OBJECT (sharewithuser_dialog), "destroy", G_CALLBACK (sp_whiteboard_sharewithuser_dialog_destroy), NULL );
        gtk_signal_connect ( GTK_OBJECT (sharewithuser_dialog), "delete_event", G_CALLBACK (sp_whiteboard_sharewithuser_dialog_delete), sharewithuser_dialog);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_whiteboard_sharewithuser_dialog_delete), sharewithuser_dialog);
                             
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), sharewithuser_dialog);
		g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), sharewithuser_dialog);//TODO  : delete this

        GtkTooltips *tt = gtk_tooltips_new ();
        gtk_container_set_border_width (GTK_CONTAINER (sharewithuser_dialog), 4);

        /* Toplevel vbox */
        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (sharewithuser_dialog), vb);
        
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
		
		// Create a scrolled window to pack the user's buddy list into 
		GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

		gtk_box_pack_start(GTK_BOX(vb), scrolled_window, TRUE, TRUE, 0);
		gtk_widget_show (scrolled_window);

		// Create the buddy list
		GtkWidget *buddyList = gtk_clist_new ( 1 );

		// Retrieve and populate the list
		Inkscape::Whiteboard::BuddyList::iterator i = active_session->buddyList.begin();
		for(; i != active_session->buddyList.end(); i++) {
			g_log(NULL, G_LOG_LEVEL_DEBUG, "Adding buddy %s", (*i).c_str());
			char const* tmp = (*i).c_str();
			// this is ugly...
			gtk_clist_append((GtkCList*)buddyList, const_cast< gchar** >(&tmp));
		}

		// Adjust list properties to fit the dialog
		gtk_clist_set_column_width (GTK_CLIST(buddyList), 0, 150);
		
		/* When a selection is made, we want to know about it. The callback
		 * used is selection_made, and its code can be found further down */
		gtk_signal_connect(GTK_OBJECT(buddyList), "select_row", GTK_SIGNAL_FUNC(selection_made), NULL);

		gtk_container_add(GTK_CONTAINER(scrolled_window), buddyList);
		gtk_widget_show(buddyList);

		GtkWidget *hb2 = gtk_hbox_new (FALSE, 0);
		GtkWidget *l = gtk_label_new_with_mnemonic ( _("_Jabber ID: ") );
		gtk_widget_set_usize (l, WHITEBOARDCONNECT_LABELWIDTH, -1);
		gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hb2), l, FALSE, FALSE, 0);

		share_tf = gtk_entry_new ();

		gtk_entry_set_max_length (GTK_ENTRY (share_tf ), 64);
		gtk_box_pack_start (GTK_BOX (hb2), share_tf , TRUE, TRUE, 0);
		gtk_object_set_data (GTK_OBJECT (sharewithuser_dialog), "jid", share_tf );
		gtk_tooltips_set_tip (tt, share_tf , _("User's Root Jabber ID(name@domain)"), NULL);
		
		g_signal_connect ( G_OBJECT (share_tf ), "activate", G_CALLBACK (sp_whiteboard_connect_dialog_do_connect), sharewithuser_dialog );
		gtk_label_set_mnemonic_widget   (GTK_LABEL(l), share_tf );

		gtk_box_pack_start (GTK_BOX (vb), hb2, FALSE, FALSE, 0);

		gtk_widget_show_all (vb);

        {
			GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            gtk_widget_show (hb);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

            sp_whiteboard_connect_new_button (sharewithuser_dialog, hb, _("_Cancel"), tt, _("Cancel"), sp_whiteboard_connect_dialog_Cancel);
            sp_whiteboard_connect_new_button (sharewithuser_dialog, hb, _("_Share"), tt, _("Initialize Direct Sharing"), sp_whiteboard_connect_dialog_sharewithuser);        
        }

		gtk_widget_show((GtkWidget *) sharewithuser_dialog);
		gtk_window_present ((GtkWindow *) sharewithuser_dialog);
	}

	return sharewithuser_dialog;
}


/* Call back to handle the action of the user selecting a row in the list. */
void
selection_made( GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data )
{
    gchar *text;

    gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);
    
    gtk_entry_set_text(GTK_ENTRY (share_tf ), text);

    return;
}


void
sp_whiteboard_connect_dialog_sharewithuser(GObject *object, GObject *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    GtkWidget *widget;
    const gchar *jid;
	Glib::ustring msg("Initializing sharing with <b>");
    
	Inkscape::XML::Node *root = sp_document_repr_root(SP_DT_DOCUMENT (desktop));
    widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "jid"));
    jid = gtk_entry_get_text (GTK_ENTRY(widget));

	msg += jid;
	msg += "</b>";

    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, msg.data());

	// TODO: re-enable after code is included
	std::string s = jid;
	desktop->whiteboard_session_manager()->sendRequestToUser(s);
   // send_connect_request_to_user((char *)jid, "");
    
    /* Once the conection haas been established, set up Inkboard listeners for all the nodes 
    in the document.
	*/

	// TODO: re-enable after code is included
//	SP_DT_DOCUMENT(desktop)->whiteboard_session_manager()->addNodeObservers(root);
   // whiteboard_add_node_listeners(root);

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
