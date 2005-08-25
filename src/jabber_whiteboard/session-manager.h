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

#ifndef __SESSION_MANAGER_H__
#define __SESSION_MANAGER_H__

#include <glibmm.h>
#include <set>
#include <bitset>

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/buddy-list-manager.h"

#include "gc-alloc.h"

struct SPDesktop;
struct SPDocument;

namespace Inkscape {
namespace XML {
class Node;
}
}

namespace Inkscape {

namespace Whiteboard {

class ReceiveMessageQueue;
class SendMessageQueue;
class XMLNodeTracker;
class SessionManager;
class MessageHandler;
class ChatMessageHandler;
class Callbacks;
class SessionFile;
class SessionFilePlayer;
class UndoStackObserver;
class SerializerNodeObserver;
class Deserializer;

// Jabber resource name
#define RESOURCE_NAME	"Inkboard"

// connectToServer return values
#define CONNECT_SUCCESS		0
#define FAILED_TO_CONNECT	1
#define INVALID_AUTH		2
#define SSL_INITIALIZATION_ERROR	3

// sendMessage return values
#define SEND_SUCCESS			0
#define CONNECTION_ERROR		1
#define UNKNOWN_OUTGOING_TYPE	2
#define NO_RECIPIENT_JID		3

/**
 * SessionData holds all session data for both 1:1 and chatroom conferences.
 * Access to members should be controlled by first querying the status bitset
 * to see if useful data will actually exist in that member -- i.e. checking
 * status[IN_CHATROOM] to see if the chatters set will contain anything.
 * It usually won't hurt to do a straight query -- there are very few members
 * that remain uninitialized for very long -- but it's a good idea to check.
 */
struct SessionData {
public:
	SessionData(SessionManager *sm);
	~SessionData();

	// Jabber connection tracking

	/**
	 * @brief the JID of the recipient; either another user JID or chatroom.
	 */
	gchar const* recipient;

	/**
	 * @brief pointer to Loudmouth connection structure
	 */
	LmConnection* connection;

	/**
	 * @brief SSL information structure
	 */
	LmSSL* ssl;

	/**
	 * @brief Should we ignore further SSL errors?
	 */
	bool ignoreFurtherSSLErrors;


	// Chatroom tracking
	
	/**
	 * @brief handle in a Jabber chatroom
	 */
	Glib::ustring chat_handle;

	/**
	 * @brief name of the chatroom that a user in a chatroom is connected to
	 */
	Glib::ustring chat_name;

	/**
	 * @brief name of the conference server
	 */
	Glib::ustring chat_server;

	// Message queues
	
	/**
	 * @brief pointer to received message queue
	 */
	//ReceiveMessageQueue* receive_queue;
	RecipientToReceiveQueueMap receive_queues;
	CommitsQueue recipients_committed_queue;

	/**
	 * @brief pointer to queue for messages to be sent
	 */
	SendMessageQueue* send_queue;

	// Message sequence numbers
	
	/**
	 * @brief current sequence number
	 */
	unsigned int sequence_number;

	/**
	 * @brief latest transaction sent
	 */
	//unsigned int latest_sent_transaction;

	/**
	 * @brief latest processed transactions
	 */
	//RecipientToLatestTransactionMap latest_processed_transactions;


	// Status tracking
	/**
	 * @brief session states and status flags
	 */
	std::bitset< NUM_FLAGS > status;
	
	/**
	 * @brief Jabber buddy list data
	 */
	BuddyListManager buddyList;

	/**
	 * @brief list of participants in a Jabber chatroom
	 */
	ChatterList chatters;

	/**
	 * @brief session file filename, blank if no session file is to be
	 * recorded
	 */
	Glib::ustring sessionFile;

private:
	// access to containing class
	SessionManager *_sm;

	// noncopyable, nonassignable
	SessionData(SessionData const&);
	SessionData& operator=(SessionData const&);
};


// TODO: This class is huge.  It might be best to refactor it into smaller,
// more coherent chunks.
//
// TODO: convert to pass-by-reference where appropriate.  In particular, a lot of the
// string buffers passed to methods in the argument list can be made into references
// appropriately and easily.

/**
 * Session management class for Inkboard.
 *
 * By "session management", we refer to the management of all events that an Inkboard
 * session may need to handle: negotiating a connection to a Jabber server, negotiating
 * sessions with users and chatrooms, sending, receiving, and parsing messages, and so
 * forth.
 */
class SessionManager {
public:
	SessionManager(::SPDesktop *desktop);
	~SessionManager();

	// Session tracking data
	
	/** 
	 * @brief pointer to SessionData structure
	 */
	struct SessionData *session_data;

	// Inkscape interface
	
	/**
	 * @brief set the desktop with which this SessionManager is associated
	 *
	 * @param desktop the desktop with which this SessionManager should be associated
	 */
	void setDesktop(::SPDesktop* desktop);
	
	// Session management
	
	/**
	 * @brief connect to a Jabber server
	 *
	 * @param server Jabber server URL
	 * @param username Jabber username
	 * @param pw password for Jabber account
	 * @param usessl use SSL for connection
	 *
	 * @return CONNECT_SUCCESS if connection successful; FAILED_TO_CONNECT if connection failed or INVALID_AUTH
	 * if authentication invalid
	 */
	int connectToServer(Glib::ustring const& server, Glib::ustring const& port, Glib::ustring const& username, Glib::ustring const& pw, bool usessl);

	/**
	 * @brief handle SSL error based on input from user
	 *
	 * @param ssl pointer to LmSSL structure
	 * @param status The error message
	 *
	 * @return LM_SSL_RESPONSE_CONTINUE if user wishes to continue establishing the connection or LM_SSL_RESPONSE_STOP if user wishes to abort connection
	 */
	LmSSLResponse handleSSLError(LmSSL* ssl, LmSSLStatus status);

	/**
	 * @brief disconnect from a Jabber server
	 */
	void disconnectFromServer();

	/**
	 * @brief disconnect from a shared document (connection to the Jabber server remains)
	 */
	void disconnectFromDocument();

	/**
	 * @brief perform session teardown; does <b>not</b> disconnect from a document or server
	 */
	void closeSession();

	/**
	 * @brief set the recipient for all Inkboard messages
	 *
	 * @param recipientJID the recipient's JID
	 */
	void setRecipient(char const* recipientJID);

	// Message sending utilities
	
	/**
	 * @brief put an Inkboard message into the send queue
	 *
	 * @param msg the message to send
	 * @param type the type of message (only CHANGE_* types permitted)
	 * @param chatroom whether or not this message is destined for a chatroom
	 */
	void sendChange(Glib::ustring const* msg, MessageType type, std::string const& recipientJID, bool chatroom);

	/**
	 * @brief send a message to the given recipient
	 *
	 * @param msgtype the type of message to send
	 * @param sequence message sequence number
	 * @param msg the message to send
	 * @param recipientJID the JID of the recipient
	 * @param chatroom whether or not this message is destined for a chatroom
	 *
	 * @return SEND_SUCCESS if successful; otherwise: UNKNOWN_OUTGOING_TYPE if msgtype is not recognized, NO_RECIPIENT_JID if recipientJID is NULL or blank, CONNECTION_ERROR if Jabber connection error occurred
	 */
	int sendMessage(MessageType msgtype, unsigned int sequence, Glib::ustring const* msg, char const* recipientJID, bool chatroom);

	/**
	 * @brief inform user of connection error
	 *
	 * @param errmsg message to display
	 */
	void connectionError(Glib::ustring const& errmsg);

	/**
	 * @brief stream the contents of the document with which this SessionManager is associated with to the given recipient
	 * 
	 * @param recipientJID the JID of the recipient
	 * @param newidsbuf buffer to store IDs of new nodes 
	 * @param newnodesbuf buffer to store address of new nodes 
	 */
	void resendDocument(char const* recipientJID, KeyToNodeMap& newidsbuf, NodeToKeyMap& newnodesbuf);
	
	
	/**
	 * @brief send connection request to user
	 *
	 * @param recipientJID the JID to connect to
	 * @param document document message to send
	 */
	void sendRequestToUser(std::string const& recipientJID);

	/**
	 * @brief send connection request to chatroom
	 * 
	 * @param server server to connect to
	 * @param chatroom name of chatroom
	 * @param handle chatroom handle to use
	 * @param password chatroom password; leave NULL if no password
	 */
	void sendRequestToChatroom(Glib::ustring const& server, Glib::ustring const& chatroom, Glib::ustring const& handle, Glib::ustring const& password);

	/**
	 * @brief send connection request response to a requesting user
	 *
	 * @param requesterJID the JID of the user whom sent us the request
	 * @param accepted_request whether or not we accepted the request
	 */
	void sendConnectRequestResponse(char const* requesterJID, gboolean accepted_request); 

	/**
	 * @brief called when a connection request is received
	 *
	 * @param requesterJID the JID of the user whom sent us the request
	 * @param msg the message associated with this request
	 */
	void receiveConnectRequest(gchar const* requesterJID);

	/**
	 * @brief called when a response to a connection request is received
	 *
	 * @param msg the message associated with this request
	 * @param response the response code
	 * @param sender the JID of the user whom responded to our request
	 */
	void receiveConnectRequestResponse(InvitationResponses response, std::string& sender);

	void receiveConnectRequestResponseChat(gchar const* recipient);

	// Message parsing and passing
	void receiveChange(Glib::ustring const* changemsg);

	// Logging and session file handling
	void startLog(Glib::ustring filename);
	void loadSessionFile(Glib::ustring filename);
	bool isPlayingSessionFile();

	// User event notification
	void userConnectedToWhiteboard(gchar const* JID);
	void userDisconnectedFromWhiteboard(std::string const& JID);

	// Queue dispatching and UI setup
	void startSendQueueDispatch();
	void stopSendQueueDispatch();
	void startReceiveQueueDispatch();
	void stopReceiveQueueDispatch();
	void clearDocument();
	void setupInkscapeInterface();
	void setupCommitListener();

	// Private object retrieval
	::SPDesktop* desktop();
	::SPDocument* document();
	Callbacks* callbacks();
	Whiteboard::UndoStackObserver* undo_stack_observer();
	SerializerNodeObserver* serializer();
	XMLNodeTracker* node_tracker();
	Deserializer* deserializer();
	ChatMessageHandler* chat_handler();
	SessionFilePlayer* session_player();
	SessionFile* session_file();

private:
	// Internal logging methods
	void _log(Glib::ustring const& message);
	void _commitLog();
	void _closeLog();
	void _tryToStartLog();

	::SPDesktop* _myDesktop;
	::SPDocument* _myDoc;
	SerializerNodeObserver* _mySerializer;
	Whiteboard::UndoStackObserver* _myUndoObserver;
	XMLNodeTracker* _myTracker;
	ChatMessageHandler* _myChatHandler;
	Callbacks* _myCallbacks;
	SessionFile* _mySessionFile;
	SessionFilePlayer* _mySessionPlayer;
	MessageHandler* _myMessageHandler;
	Deserializer* _myDeserializer; 

	sigc::connection _send_queue_dispatcher;
	sigc::connection _receive_queue_dispatcher;

	// noncopyable, nonassignable
	SessionManager(SessionManager const&);
	SessionManager& operator=(SessionManager const&);
};

}

}

#endif

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
