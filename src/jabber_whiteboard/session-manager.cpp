/**
 * Whiteboard session manager
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
#include "inkscape.h"
#include "desktop-handles.h"
*/

#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>

//#include <boost/lexical_cast.hpp>
//#include <boost/function.hpp>

#include "gc-anchored.h"

#include "xml/repr.h"
#include "xml/node-observer.h"

#include "util/ucompose.hpp"

#include "message-context.h"
#include "desktop.h"
#include "document.h"
#include "document-private.h"

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/message-handler.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/jabber-handlers.h"
#include "jabber_whiteboard/callbacks.h"
#include "jabber_whiteboard/chat-handler.h"
#include "jabber_whiteboard/session-file.h"
#include "jabber_whiteboard/session-file-player.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-observer.h"

#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/message-queue.h"

namespace Inkscape {

namespace Whiteboard {

static bool lm_initialize_called = false;

SessionData::SessionData(SessionManager *sm)
{
	g_log(NULL, G_LOG_LEVEL_DEBUG, "SessionData constructor called.");
	this->_sm = sm;
	this->recipient = NULL;
	this->connection = NULL;
	this->receive_queue = new ReceiveMessageQueue(sm);
	this->send_queue = new SendMessageQueue(sm);
	this->sequence_number = 1;
	this->sequence_largest_dropped = 0;
	g_log(NULL, G_LOG_LEVEL_DEBUG, "SessionData construction complete.");
}

SessionData::~SessionData()
{
	if (this->receive_queue) {
		delete this->receive_queue;
	}

	if (this->send_queue) {
		delete this->send_queue;
	}
}

SessionManager::SessionManager(::SPDesktop *desktop)
{
	g_log(NULL, G_LOG_LEVEL_DEBUG, "Constructing SessionManager.");

//	this->attr_changed_listener_locked = false;
//	this->child_added_listener_locked = false;
//	this->child_removed_listener_locked = false;
//	this->child_order_changed_listener_locked = false;
//	this->content_changed_listener_locked = false;

	// Initialize private members to NULL to facilitate deletion in destructor
	this->_myDoc = NULL;
	this->session_data = NULL;
	this->_myObserver = NULL;
	this->_myCallbacks = NULL;
	this->_myTracker = NULL;
	this->_myChatHandler = NULL;
	this->_mySessionFile = NULL;
	this->_mySessionPlayer = NULL;
	this->_myMessageHandler = NULL;

	this->setDesktop(desktop);
	if (desktop->doc == NULL) {
		g_error("Initializing SessionManager on null document object!");
	}

	g_log(NULL, G_LOG_LEVEL_DEBUG, "Completed SessionManager construction.");

    //# lm_initialize() must be called before any network code
    if (!lm_initialize_called) {
        lm_initialize();
        lm_initialize_called = true;
        }
}

SessionManager::~SessionManager()
{
	g_log(NULL, G_LOG_LEVEL_DEBUG, "Destructing SessionManager.");

	if (this->session_data) {
		if (this->session_data->status[IN_WHITEBOARD]) {
			this->disconnectFromDocument();
		}
		this->disconnectFromServer();

		if (this->session_data->status[LOGGED_IN]) {
			// TODO: unref message handlers
		}
	}

	if (this->_mySessionFile) {
		delete this->_mySessionPlayer;
		delete this->_mySessionFile;
	}

	delete this->_myChatHandler;


	// Deletion of _myTracker is done in closeSession;
	// no need to do it here.

	// Deletion is handled separately from session teardown and server disconnection
	// because some teardown methods (e.g. closeSession) require access to members that we will
	// be deleting. Separating deletion from teardown means that we do not have
	// to worry (as much) about proper ordering of the teardown sequence.  (We still need
	// to ensure that destructors in each object being deleted have access to all the
	// members they need, though.)
	delete this->_myObserver;

	// Stop dispatchers
	if (this->_myCallbacks) {
		this->stopSendQueueDispatch();
		this->stopReceiveQueueDispatch();
		delete this->_myCallbacks;
	}

	delete this->_myMessageHandler;

	delete this->session_data;

	Inkscape::GC::release(this->_myDoc);

	g_log(NULL, G_LOG_LEVEL_DEBUG, "SessionManager destruction complete.");
}

void
SessionManager::setDesktop(::SPDesktop* desktop)
{
	this->_myDesktop = desktop;
	if (this->_myDoc != NULL) {
		Inkscape::GC::release(this->_myDoc);
	}
	if (desktop->doc != NULL) {
		this->_myDoc = desktop->doc;
		Inkscape::GC::anchor(this->_myDoc);
	}
}

int
SessionManager::connectToServer(Glib::ustring const& server, Glib::ustring const& username, Glib::ustring const& pw)
{
	GError* error = NULL;
	Glib::ustring jid;

	// JID format is username@server/resource
	jid += username + "@" + server + "/" + RESOURCE_NAME;

	LmMessage* m;
	LmMessageHandler* mh;

	if (!this->session_data) {
		this->session_data = new SessionData(this);
	}

	if (!this->_myMessageHandler) {
		this->_myMessageHandler = new MessageHandler(this);
	}

	// Connect to server
	// We need to check to see if this object already exists, because
	// the user may be reusing an old connection that failed due to e.g.
	// authentication failure.
	if (!this->session_data->connection) {
		this->session_data->connection = lm_connection_new(server.c_str());
	}

	// Send authorization
	lm_connection_set_jid(this->session_data->connection, jid.c_str());

	// 	TODO:
	// 	Asynchronous connection and authentication would be nice,
	// 	but it's a huge mess of mixing C callbacks and C++ method calls.
	// 	I've tried to do it and only managed to severely destabilize the Jabber
	// 	server connection routines.
	//
	// 	This, of course, is an invitation to anyone more capable than me
	// 	to convert this from synchronous to asynchronous Loudmouth calls.
	if (!lm_connection_open_and_block(this->session_data->connection, &error)) {
		g_warning("Failed to open: %s", error->message);
		return FAILED_TO_CONNECT;
	}

	// Authenticate
	if (!lm_connection_authenticate_and_block(this->session_data->connection, username.c_str(), pw.c_str(), RESOURCE_NAME, &error)) {
		g_warning("Failed to authenticate: %s", error->message);
		lm_connection_close(this->session_data->connection, NULL);
		lm_connection_unref(this->session_data->connection);
		this->session_data->connection = NULL;
		return INVALID_AUTH;
	}

	// Register message handler for presence messages
	mh = lm_message_handler_new((LmHandleMessageFunction)presence_handler, reinterpret_cast< gpointer >(this->_myMessageHandler), NULL);
	lm_connection_register_message_handler(this->session_data->connection, mh, LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);

	// Register message handler for stream error messages
	mh = lm_message_handler_new((LmHandleMessageFunction)stream_error_handler, reinterpret_cast< gpointer >(this->_myMessageHandler), NULL);
	lm_connection_register_message_handler(this->session_data->connection, mh, LM_MESSAGE_TYPE_STREAM_ERROR, LM_HANDLER_PRIORITY_NORMAL);

	// Register message handler for chat messages
	mh = lm_message_handler_new((LmHandleMessageFunction)default_handler, reinterpret_cast< gpointer >(this->_myMessageHandler), NULL);
	lm_connection_register_message_handler(this->session_data->connection, mh, LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);

	// Send presence message to server
	m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_NOT_SET);
	if (!lm_connection_send(this->session_data->connection, m, &error)) {
		g_warning("Presence message could not be sent: %s", error->message);
		lm_connection_close(this->session_data->connection, NULL);
		lm_connection_unref(this->session_data->connection);
		this->session_data->connection = NULL;
		return FAILED_TO_CONNECT;
	}

	this->session_data->status.set(LOGGED_IN, 1);

	this->_myCallbacks = new Callbacks(this);

	lm_message_unref(m);

	return CONNECT_SUCCESS;
}

void
SessionManager::disconnectFromServer()
{
	if (this->session_data->connection) {
		GError* error = NULL;

		LmMessage *m;
		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
		if (!lm_connection_send(this->session_data->connection, m, &error)) {
			g_warning("Could not send unavailable presence message: %s", error->message);
		}

		lm_message_unref(m);
		lm_connection_close(this->session_data->connection, NULL);
		lm_connection_unref(this->session_data->connection);
		this->session_data->connection = NULL;
	}
}

void
SessionManager::disconnectFromDocument()
{
	if (this->session_data->status[IN_WHITEBOARD]) {
		this->sendMessage(DISCONNECTED_FROM_USER_SIGNAL, 0, NULL, this->session_data->recipient, this->session_data->status[IN_CHATROOM]);
	}
	this->closeSession();
}

void
SessionManager::closeSession()
{
	g_log(NULL, G_LOG_LEVEL_DEBUG, "Closing whiteboard session.");
	XML::Node* root = sp_document_repr_root(this->_myDoc);

	if (this->session_data->status[IN_WHITEBOARD]) {
		this->session_data->status.set(IN_WHITEBOARD, 0);
		this->session_data->receive_queue->clear();
		this->session_data->send_queue->clear();
		this->stopSendQueueDispatch();
		this->stopReceiveQueueDispatch();
	}

	this->removeNodeObservers(root);

	if (this->_myTracker) {
		delete this->_myTracker;
		this->_myTracker = NULL;
	}

	this->setRecipient(NULL);
	g_log(NULL, G_LOG_LEVEL_DEBUG, "Whiteboard session closed.");
}

void
SessionManager::setRecipient(char const* recipientJID)
{
	if (this->session_data->recipient) {
		free(const_cast< gchar* >(this->session_data->recipient));
	}

	if (recipientJID == NULL) {
		this->session_data->recipient = NULL;
	} else {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Setting recipient to %s", recipientJID);
		this->session_data->recipient = g_strdup(recipientJID);
	}
}

void
SessionManager::addNodeObservers(XML::Node* node)
{
	if (node == NULL) {
		return;
	}

	if (this->_myObserver == NULL) {
		return;
	}

	node->addObserver(*_myObserver);
	for(XML::Node* child = node->firstChild(); child != NULL; child = child->next()) {
		this->addNodeObservers(child);
	}
}

void
SessionManager::removeNodeObservers(XML::Node* node)
{
	if (node == NULL) {
		return;
	}

	if (this->_myObserver == NULL) {
		return;
	}

	node->removeObserver(*_myObserver);
	for(XML::Node* child = node->firstChild(); child != NULL; child = child->next()) {
		this->removeNodeObservers(child);
	}
}

void
SessionManager::sendChange(Glib::ustring const* msg, gboolean is_repeatable, bool chatroom)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Inserting change: from %s to %s, message: %s", lm_connection_get_jid(this->session_data->connection), this->session_data->recipient, msg->c_str());

	MessageNode *newNode = new MessageNode(this->session_data->sequence_number++, lm_connection_get_jid(this->session_data->connection), this->session_data->recipient, msg, is_repeatable, false, chatroom);
	this->session_data->send_queue->insert(newNode);
//	Inkscape::GC::release(newNode);
}

int
SessionManager::sendMessage(MessageType msgtype, unsigned int sequence, Glib::ustring const* msg, char const* recipientJID, bool chatroom)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "sendMessage, message type %s", MessageHandler::ink_type_to_string(msgtype));
	LmMessage* m;
	GError* error = NULL;
	char* type, * seq;

	if (recipientJID == NULL || recipientJID == "") {
		g_warning("Null recipient JID specified; not sending message.");
		return NO_RECIPIENT_JID;
	} else {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Sending to %s", recipientJID);
	}

	// create message
	m = lm_message_new(recipientJID, LM_MESSAGE_TYPE_MESSAGE);

	// add sender
	lm_message_node_set_attribute(m->node, "from", lm_connection_get_jid(this->session_data->connection));

	// set message subtype according to whether or not this is
	// destined for a chatroom
	if (chatroom) {
		lm_message_node_set_attribute(m->node, "type", "groupchat");
	} else {
		lm_message_node_set_attribute(m->node, "type", "chat");
	}

	// set protocol version;
	// we are currently fixed at version 1
	lm_message_node_add_child(m->node, MESSAGE_PROTOCOL_VER, MESSAGE_PROTOCOL_V1);

	// add message type
	type = (char *)calloc(TYPE_FIELD_SIZE, sizeof(char));
	snprintf(type, TYPE_FIELD_SIZE, "%i", msgtype);
	lm_message_node_add_child(m->node, MESSAGE_TYPE, type);
	free(type);

	// add message body
	if (msg != NULL) {
		lm_message_node_add_child(m->node, MESSAGE_BODY, msg->data());
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Sending message from %s to %s: %s", lm_connection_get_jid(this->session_data->connection), recipientJID, msg->data());
	}

	// add sequence number
	switch(msgtype) {
		case CHANGE_REPEATABLE:
		case CHANGE_NOT_REPEATABLE:
		case DUMMY_CHANGE:
		case CONNECT_REQUEST_RESPONSE_CHAT:
		case CONNECT_REQUEST_RESPONSE_USER:
		case CHATROOM_SYNCHRONIZE_RESPONSE:
			seq = (char* )calloc(SEQNUM_FIELD_SIZE, sizeof(char));
			sprintf(seq, "%u", sequence);
			lm_message_node_add_child(m->node, MESSAGE_SEQNUM, seq);
			free(seq);
			break;

		case DOCUMENT_BEGIN:
		case DOCUMENT_END:
		case CONNECT_REQUEST_USER:
		case CONNECTED_SIGNAL:
		case DISCONNECTED_FROM_USER_SIGNAL:
			break;

		// Error messages and synchronization requests do not need a sequence number
		case CHATROOM_SYNCHRONIZE_REQUEST:
		case CONNECT_REQUEST_REFUSED_BY_PEER:
		case UNSUPPORTED_PROTOCOL_VERSION:
		case ALREADY_IN_SESSION:
			break;

		default:
			g_warning("Outgoing message type not recognized; not sending.");
			lm_message_unref(m);
			return UNKNOWN_OUTGOING_TYPE;
	}

	// We want to log messages even if they were not successfully sent,
	// since the user may opt to re-synchronize a session using the session
	// file record.
	if (msg != NULL) {
		this->_log(*msg);
		this->_commitLog();
	}

	// send message

	if (!lm_connection_send(this->session_data->connection, m, &error)) {
		g_error("Send failed: %s", error->message);
		lm_message_unref(m);
		return CONNECTION_ERROR;
	}

//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Message sent");
	lm_message_unref(m);
	return SEND_SUCCESS;
}

void
SessionManager::timeoutError(char const* errmsg)
{
	Gtk::MessageDialog dlg(errmsg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
	dlg.run();
//	sp_whiteboard_sharewithuser_dialog(const_cast< gchar* >(errmsg));
}

void
SessionManager::connectionError(Glib::ustring const& errmsg)
{
	Gtk::MessageDialog dlg(errmsg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
	dlg.run();
//	sp_whiteboard_connect_dialog(const_cast< gchar* >(errmsg));
}

void
SessionManager::resendDocument(char const* recipientJID, KeyToNodeMap& newidsbuf, NodeToKeyMap& newnodesbuf)
{
	this->sendMessage(DOCUMENT_BEGIN, 0, NULL, recipientJID, false);

	Glib::ustring *buf = new Glib::ustring();

	Inkscape::XML::Node* root = sp_document_repr_root(this->_myDoc);

	if(root == NULL) {
		return;
    }

	NewChildObjectMessageList newchildren;

	MessageUtilities::newObjectMessage(buf, newidsbuf, newnodesbuf, newchildren, this->_myTracker, root, false, false);

    for ( Inkscape::XML::Node *child = root->firstChild() ; child != NULL ; child = child->next() ) {
		MessageUtilities::newObjectMessage(buf, newidsbuf, newnodesbuf, newchildren, this->_myTracker, child);

//		this->sendMessage(CHANGE_REPEATABLE, this->session_data->sequence_number++, buf, recipientJID, false);

		NewChildObjectMessageList::iterator j = newchildren.begin();
		for(; j != newchildren.end(); j++) {
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "New child message: %s", (*j).c_str());
			this->sendMessage(CHANGE_REPEATABLE, this->session_data->sequence_number++, &(*j), recipientJID, false);
		}

		newchildren.clear();

		buf->clear();
    }

	delete buf;

	this->sendMessage(DOCUMENT_END, this->session_data->sequence_number++, NULL, recipientJID, false);
}

void
SessionManager::receiveChange(Glib::ustring const* changemsg)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receiveChange operating on %s", changemsg->data());

	struct Node* part = new Node;

	// TODO: there's no real reason to make a copy anymore; this is a holdover
	// from the old Inkboard code
	Glib::ustring* msgcopy = new Glib::ustring(changemsg->data());
	this->_log(*changemsg);

	bool validmsg = true;
	while(MessageUtilities::getFirstMessageTag(part, msgcopy) != false) {
		validmsg = true;

		if (part->tag == MESSAGE_CHANGE) {
			this->receivedChangeHelper(&(part->data));
			msgcopy->erase(0, part->next_pos);
		} else if (part->tag == MESSAGE_NEWOBJ) {
			unsigned int consumed;
			Glib::ustring *restcopy = new Glib::ustring(msgcopy->substr(part->next_pos, msgcopy->length() - part->next_pos));
			consumed = this->receivedNewObjectHelper(&(part->data), restcopy);
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "consumed %u characters", consumed);
			msgcopy->erase(0, part->next_pos + consumed);
			delete restcopy;
		} else if (part->tag == MESSAGE_DELETE) {
			this->receivedDeleteHelper(&(part->data));
			msgcopy->erase(0, part->next_pos);
		} else if (part->tag == MESSAGE_DOCUMENT) {
			// no special handler, just keep going with the rest of the message
			msgcopy->erase(0, part->next_pos);
		} else if (part->tag == MESSAGE_NODECONTENT) {
			this->receivedContentChangeHelper(part->data);
			msgcopy->erase(0, part->next_pos);
		} else if (part->tag == MESSAGE_ORDERCHANGE) {
			this->receivedChildOrderChangeHelper(part->data);
			msgcopy->erase(0, part->next_pos);
		} else {
			validmsg = false;
			msgcopy->erase(0, part->next_pos);
		}
	}

	this->_commitLog();

	delete part;
	delete msgcopy;
}

void
SessionManager::receiveDocument(Glib::ustring const* documentmsg)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receiveDocument");
	this->receiveChange(documentmsg);
}

bool
SessionManager::isPlayingSessionFile()
{
	return this->session_data->status[PLAYING_SESSION_FILE];
}

void
SessionManager::loadSessionFile(Glib::ustring filename)
{
	if (!this->session_data || !this->session_data->status[IN_WHITEBOARD]) {
		try {
			if (this->_mySessionFile) {
				delete this->_mySessionFile;
			}
			this->_mySessionFile = new SessionFile(filename, true, false);
			g_log(NULL, G_LOG_LEVEL_DEBUG, "Loaded session file %s (%p)", filename.data(), this->_mySessionFile);

			// Initialize objects needed for session playback
			if (this->_mySessionPlayer == NULL) {
				this->_mySessionPlayer = new SessionFilePlayer(16, this);
			} else {
				this->_mySessionPlayer->load(this->_mySessionFile);
			}

			if (this->_myTracker == NULL) {
				this->_myTracker = new XMLNodeTracker(this);
			} else {
				this->_myTracker->reset();
			}

			if (!this->session_data) {
				this->session_data = new SessionData(this);
			}

			this->session_data->status.set(PLAYING_SESSION_FILE, 1);


		} catch (Glib::FileError e) {
			g_warning("Could not load session file: %s", e.what().data());
		}
	}

}

void
SessionManager::userConnectedToWhiteboard(gchar const* JID)
{
	this->_myDesktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("Established whiteboard session with <b>%s</b>."), JID);
}


void
SessionManager::userDisconnectedFromWhiteboard(std::string const& JID)
{

//	g_log(NULL, G_LOG_LEVEL_DEBUG, "%s has left the whiteboard session.", JID.c_str());
	this->_myDesktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("<b>%s</b> has <b>left</b> the whiteboard session."), JID.c_str());

	// Inform the user
	// TRANSLATORS: %1 is the name of the user that disconnected, %2 is the name of the user whom the disconnected user disconnected from.
	// This message is not used in a chatroom context.
	Glib::ustring primary = String::ucompose(_("<span weight=\"bold\" size=\"larger\">The user <b>%1</b> has left the whiteboard session.</span>\n\n"), JID);
	Glib::ustring secondary = String::ucompose(_("You are still connected to a Jabber server as <b>%2</b>, and may establish a new session to <b>%1</b> or a different user."), JID, lm_connection_get_jid(this->session_data->connection));

	// TODO: parent this dialog to the active desktop
	Gtk::MessageDialog dialog(primary + secondary, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false);
	/*
	dialog.set_message(primary, true);
	dialog.set_secondary_text(secondary, true);
	*/
	dialog.run();
}

void
SessionManager::userSessionDisconnect()
{
	this->_myDesktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("User <b>%s</b> has <b>disconnected</b> from the whiteboard session."), this->session_data->recipient);
}

void
SessionManager::startSendQueueDispatch()
{
	this->_send_queue_dispatcher = Glib::signal_timeout().connect(sigc::mem_fun(*(this->_myCallbacks), &Callbacks::dispatchSendQueue), SEND_TIMEOUT);
}

void
SessionManager::stopSendQueueDispatch()
{
	if (this->_send_queue_dispatcher) {
		this->_send_queue_dispatcher.disconnect();
	}
}

void
SessionManager::startReceiveQueueDispatch()
{
	this->_receive_queue_dispatcher = Glib::signal_timeout().connect(sigc::mem_fun(*(this->_myCallbacks), &Callbacks::dispatchReceiveQueue), SEND_TIMEOUT);
}

void
SessionManager::stopReceiveQueueDispatch()
{
	if (this->_receive_queue_dispatcher) {
		this->_receive_queue_dispatcher.disconnect();
	}
}

void
SessionManager::clearDocument()
{
	// clear all layers, definitions, and metadata
	XML::Node* rroot = this->_myDoc->rroot;

	// remove observers
	this->removeNodeObservers(rroot);

	// clear definitions
	XML::Node* defs = SP_OBJECT_REPR((SPDefs*)SP_DOCUMENT_DEFS(this->_myDoc));
	g_assert(SP_ROOT(this->_myDoc->root)->defs);

	for(XML::Node* child = defs->firstChild(); child; child = child->next()) {
		defs->removeChild(child);
	}

	// clear layers
	for(XML::Node* child = rroot->firstChild(); child; child = child->next()) {
		if (strcmp(child->name(), "svg:g") == 0) {
			rroot->removeChild(child);
		}
	}

	// clear metadata
	for(XML::Node* child = rroot->firstChild(); child; child = child->next()) {
		if (strcmp(child->name(), "svg:metadata") == 0) {
			rroot->removeChild(child);
		}
	}

//	sp_document_done(this->_myDoc);
}

void
SessionManager::setupInkscapeInterface()
{
	XML::Node* rroot = this->_myDoc->rroot;
	this->session_data->status.set(IN_WHITEBOARD, 1);
	this->startSendQueueDispatch();
	this->startReceiveQueueDispatch();
	if (!this->_myTracker) {
		this->_myTracker = new XMLNodeTracker(this);
	}
	this->_myObserver = new XMLNodeObserver(this);
	this->addNodeObservers(rroot);
}

::SPDesktop*
SessionManager::desktop()
{
	return this->_myDesktop;
}

::SPDocument*
SessionManager::document()
{
	return this->_myDoc;
}

Callbacks*
SessionManager::callbacks()
{
	return this->_myCallbacks;
}

XMLNodeObserver*
SessionManager::node_observer()
{
	return this->_myObserver;
}

XMLNodeTracker*
SessionManager::node_tracker()
{
	return this->_myTracker;
}


ChatMessageHandler*
SessionManager::chat_handler()
{
	return this->_myChatHandler;
}

SessionFilePlayer*
SessionManager::session_player()
{
	return this->_mySessionPlayer;
}

SessionFile*
SessionManager::session_file()
{
	return this->_mySessionFile;
}

void
SessionManager::_log(Glib::ustring const& message)
{
	if (this->_mySessionFile && !this->_mySessionFile->isReadOnly()) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Logging message in session file %s: %s", this->_mySessionFile->filename().c_str(), message.c_str());
		this->_mySessionFile->addMessage(message);
	}
}

void
SessionManager::_commitLog()
{
	if (this->_mySessionFile && !this->_mySessionFile->isReadOnly()) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Committing changes to session file %s", this->_mySessionFile->filename().c_str());
		this->_mySessionFile->commit();
	}
}

void
SessionManager::_closeLog()
{
	if (this->_mySessionFile) {
		this->_mySessionFile->close();
	}
}

void
SessionManager::_tryToStartLog()
{
	if (this->session_data) {
		if (!this->session_data->sessionFile.empty()) {
			bool undecided = true;
			while(undecided) {
				try {
					this->startLog(this->session_data->sessionFile);
					undecided = false;
				} catch (Glib::FileError e) {
					undecided = true;
					Glib::ustring msg = String::ucompose(_("Could not open file %1 for session recording.\nThe error encountered was: %2.\n\nYou may select a different location to record the session, or you may opt to not record this session."), this->session_data->sessionFile, e.what());
					Gtk::MessageDialog dlg(msg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, false);
					dlg.add_button(_("Choose a different location"), 0);
					dlg.add_button(_("Skip session recording"), 1);
					switch (dlg.run()) {
						case 0:
						{
							Gtk::FileChooserDialog sessionfiledlg(_("Select a location and filename"), Gtk::FILE_CHOOSER_ACTION_SAVE);
							sessionfiledlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
							sessionfiledlg.add_button(_("Set filename"), Gtk::RESPONSE_OK);
							int result = sessionfiledlg.run();
							switch (result) {
								case Gtk::RESPONSE_OK:
								{
									this->session_data->sessionFile = sessionfiledlg.get_filename();
									break;
								}
								case Gtk::RESPONSE_CANCEL:
								default:
									undecided = false;
									break;
							}
							break;
						}
						case 1:
						default:
							undecided = false;
							break;
					}
				}
			}
		}
	}
}

}

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
