/**
 * Whiteboard session manager
 * Jabber received message processors
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <glibmm/i18n.h>

#include "desktop.h"
#include "message-stack.h"

#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/message-processors.h"
#include "jabber_whiteboard/typedefs.h"

namespace Inkscape {

namespace Whiteboard {

// Message processors are here!

// TODO: Remove unnecessary status checks from processors --
// we do all of that in MessageHandler::_hasValidReceiveContext

// *********************************************************************
// ChangeHandler begin
// *********************************************************************
struct ChangeHandler : public MessageProcessor {
public:
	~ChangeHandler()
	{

	}

	ChangeHandler(SessionManager* sm) : MessageProcessor(sm)
	{

	}

	LmHandlerResult
	operator()(MessageType mode, JabberMessage& p) {
		MessageNode* msgNode;
		bool chatroom = this->_sm->session_data->status[IN_CHATROOM];
		ReceiveMessageQueue* rmq = this->_sm->session_data->receive_queue;

		switch (mode) {
			case CHANGE_REPEATABLE:
				msgNode = new MessageNode(p.sequence, p.sender, "", &p.body, true, false, chatroom);
				rmq->insert(msgNode);
				break;
			case CHANGE_NOT_REPEATABLE:
				msgNode = new MessageNode(p.sequence, p.sender, "", &p.body, false, false, chatroom);
				rmq->insert(msgNode);
				break;
			case DUMMY_CHANGE:
			default:
				break;
		}
		
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ChangeHandler end
// *********************************************************************




// *********************************************************************
// DocumentSignalHandler begin
// *********************************************************************
struct DocumentSignalHandler : public MessageProcessor {
public:
	~DocumentSignalHandler() 
	{

	}

	DocumentSignalHandler(SessionManager* sm) : MessageProcessor(sm)
	{

	}
	LmHandlerResult
	operator()(MessageType mode, JabberMessage& m)
	{
		std::bitset< NUM_FLAGS >& status = this->_sm->session_data->status;
		switch(mode) {
			case DOCUMENT_BEGIN: 
			{
				if (status[WAITING_TO_SYNC_TO_CHAT]) {
					status.set(WAITING_TO_SYNC_TO_CHAT, 0);
					status.set(SYNCHRONIZING_WITH_CHAT, 1);
				}
				break;
			}
			case DOCUMENT_END:
				if (status[SYNCHRONIZING_WITH_CHAT]) {
					this->_sm->sendMessage(CONNECTED_SIGNAL, 0, NULL, this->_sm->session_data->recipient, true);
					status.set(SYNCHRONIZING_WITH_CHAT, 0);
					status.set(IN_CHATROOM, 1);
				} else {
					this->_sm->sendMessage(CONNECTED_SIGNAL, 0, NULL, m.sender.c_str(), false);
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
private:
};

// *********************************************************************
// DocumentSignalHandler end
// *********************************************************************




// *********************************************************************
// ConnectRequestHandler begin
// *********************************************************************
struct ConnectRequestHandler : public MessageProcessor {
public:
	~ConnectRequestHandler()
	{

	}

	ConnectRequestHandler(SessionManager* sm) : MessageProcessor(sm) 
	{

	}

	LmHandlerResult 
	operator()(MessageType mode, JabberMessage& m)
	{
		std::bitset< NUM_FLAGS >& status = this->_sm->session_data->status;
		switch(mode) {
			case CONNECT_REQUEST_USER:
				this->_sm->receiveConnectRequest(m.sender.c_str());
				break;
			case CONNECT_REQUEST_RESPONSE_USER:
				if (m.sequence == 0) {
					this->_sm->receiveConnectRequestResponse(DECLINE_INVITATION);
				} else { // FIXME: this has got to be buggy...
					this->_sm->setRecipient(m.sender.c_str());
					this->_sm->receiveConnectRequestResponse(ACCEPT_INVITATION);
				}
				break;
			case Inkscape::Whiteboard::CONNECTED_SIGNAL:
				if (!status[IN_CHATROOM] && !status[CONNECTING_TO_CHAT] && !status[SYNCHRONIZING_WITH_CHAT] && !status[WAITING_TO_SYNC_TO_CHAT]) {
					this->_sm->userConnectedToWhiteboard(m.sender.c_str());
					this->_sm->setRecipient(m.sender.c_str());
				} else {
					this->_sm->desktop()->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("<b>%s</b> has joined the chatroom."), m.sender.c_str());
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ConnectRequestHandler end
// *********************************************************************




// *********************************************************************
// ConnectErrorHandler begin
// *********************************************************************
struct ConnectErrorHandler : public MessageProcessor {
public:
	~ConnectErrorHandler()
	{

	}

	ConnectErrorHandler(SessionManager* sm) : MessageProcessor(sm) 
	{

	}

	LmHandlerResult 
	operator()(MessageType mode, JabberMessage& m)
	{
		switch(mode) {
			case CONNECT_REQUEST_REFUSED_BY_PEER:
				if (this->_sm->session_data->status[WAITING_FOR_INVITE_RESPONSE]) {
					this->_sm->receiveConnectRequestResponse(DECLINE_INVITATION, m.sender.c_str());
				}
				break;
			case Inkscape::Whiteboard::ALREADY_IN_SESSION:
				if (this->_sm->session_data->status[WAITING_FOR_INVITE_RESPONSE]) {
					this->_sm->receiveConnectRequestResponse(PEER_ALREADY_IN_SESSION, m.sender.c_str());
				}
				break;
			case Inkscape::Whiteboard::DISCONNECTED_FROM_USER_SIGNAL:
				if (!this->_sm->session_data->status[IN_CHATROOM]) {
					this->_sm->closeSession();
					this->_sm->userDisconnectedFromWhiteboard(m.sender.c_str());
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ConnectErrorHandler end
// *********************************************************************




// *********************************************************************
// ChatSynchronizeHandler begin
// *********************************************************************
struct ChatSynchronizeHandler : public MessageProcessor {
public:
	~ChatSynchronizeHandler()
	{

	}

	ChatSynchronizeHandler(SessionManager* sm) : MessageProcessor(sm) 
	{

	}

	LmHandlerResult 
	operator()(MessageType mode, JabberMessage& m)
	{
		switch(mode) {
			case CONNECT_REQUEST_RESPONSE_CHAT:
				this->_sm->receiveConnectRequestResponseChat(m.sender.c_str());
				break;
			case CHATROOM_SYNCHRONIZE_REQUEST:
				if (this->_sm->session_data->status[IN_CHATROOM] && this->_sm->session_data->status[IN_WHITEBOARD]) {
//					g_log(NULL, G_LOG_LEVEL_DEBUG, "%s received synchronization request from %s", lm_connection_get_jid(this->_sm->session_data->connection), m->sender.c_str());
					// Send response.  Everyone in the chatroom will do this,
					// but the client will accept only one response.
					// The response is sent privately to the client
					// <http://www.jabber.org/jeps/jep-0045.html#privatemessage>
					this->_sm->sendMessage(CHATROOM_SYNCHRONIZE_RESPONSE, this->_sm->session_data->sequence_number, NULL, m.sender.c_str(), false);
				}
				break;
			case CHATROOM_SYNCHRONIZE_RESPONSE:
//				g_log(NULL, G_LOG_LEVEL_DEBUG, "%s received synchronization response from %s", lm_connection_get_jid(this->_sm->session_data->connection), m->sender.c_str());
				if (m.sequence != 0) {
					// Set sequence number
					this->_sm->session_data->sequence_number = m.sequence;

					// Set status flags
					this->_sm->session_data->status.set(WAITING_TO_SYNC_TO_CHAT, 0);
					this->_sm->session_data->status.set(SYNCHRONIZING_WITH_CHAT, 1);

					// Send document synchronization request
					this->_sm->clearDocument();
					this->_sm->setupInkscapeInterface();
					this->_sm->sendMessage(CONNECT_REQUEST_RESPONSE_CHAT, m.sequence, NULL, m.sender.c_str(), false);
				} else {
//					g_log(NULL, G_LOG_LEVEL_DEBUG, "Sequence number from synchronization response was zero; dropping response and trying again.");
					this->_sm->sendMessage(CHATROOM_SYNCHRONIZE_REQUEST, 0, NULL, this->_sm->session_data->recipient, true);
				}
				break;
			default:
				break;
		}
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
};
// *********************************************************************
// ChatSynchronizeHandler end
// *********************************************************************




// *********************************************************************
// Initializer
// *********************************************************************
void
initialize_received_message_processors(SessionManager* sm, MessageProcessorMap& mpm)
{
	ProcessorShell* ch = new ProcessorShell(new ChangeHandler(sm));
	ProcessorShell* dsh = new ProcessorShell(new DocumentSignalHandler(sm));
	ProcessorShell* crh = new ProcessorShell(new ConnectRequestHandler(sm));
	ProcessorShell* ceh = new ProcessorShell(new ConnectErrorHandler(sm));
	ProcessorShell* csh = new ProcessorShell(new ChatSynchronizeHandler(sm));

	mpm[CHANGE_REPEATABLE] = ch;
	mpm[CHANGE_NOT_REPEATABLE] = ch;
	mpm[DUMMY_CHANGE] = ch;

	mpm[DOCUMENT_BEGIN] = dsh;
	mpm[DOCUMENT_END] = dsh;

	mpm[CONNECT_REQUEST_USER] = crh;
	mpm[CONNECT_REQUEST_RESPONSE_USER] = crh;
	mpm[CONNECTED_SIGNAL] = crh;

	mpm[CONNECT_REQUEST_REFUSED_BY_PEER] = ceh;
	mpm[ALREADY_IN_SESSION] = ceh;
	mpm[DISCONNECTED_FROM_USER_SIGNAL] = ceh;

	mpm[CONNECT_REQUEST_RESPONSE_CHAT] = csh;
	mpm[CHATROOM_SYNCHRONIZE_REQUEST] = csh;
	mpm[CHATROOM_SYNCHRONIZE_RESPONSE] = csh;
}

/**
 * This function is provided strictly for convenience and style.  You can, of course,
 * delete every MessageProcessor in the map with your own loop.
 */
void
destroy_received_message_processors(MessageProcessorMap& mpm)
{
	mpm.clear();
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
