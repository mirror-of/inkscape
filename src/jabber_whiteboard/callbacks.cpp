/**
 * Whiteboard session manager
 * Message dispatch devices and timeout triggers
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

#include "jabber_whiteboard/jabber-handlers.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-queue.h"
#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/callbacks.h"

namespace Inkscape {

namespace Whiteboard {

Callbacks::Callbacks(SessionManager* sm) : _sm(sm) 
{
	this->_sd = this->_sm->session_data;
}

Callbacks::~Callbacks() 
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Callbacks destructor called.");
}

bool
Callbacks::dispatchSendQueue()
{
	// If we're not in a whiteboard session, don't dispatch anything
	if (!(this->_sd->status[IN_WHITEBOARD])) {
		g_log(NULL, G_LOG_LEVEL_DEBUG, "not in whiteboard; not dispatching");
		return false;
	}


	// If the connection is not open, inform the user that an error has occurred
	// and stop the queue
	if (lm_connection_get_state(this->_sd->connection) != LM_CONNECTION_STATE_OPEN) {
		// TODO: re-enable
		// error_connection("Connection lost");
		g_log(NULL, G_LOG_LEVEL_DEBUG, "error: connection lost");
		this->_sm->desktop()->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Jabber connection lost."));
		return false;
	}

	// If there's nothing to send, don't do anything
	if (this->_sd->send_queue->empty()) {
		return true;
	}

	// otherwise, send out the first change
	MessageNode* first = this->_sd->send_queue->popFront();
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "sending first change in queue %p to recipient %s", first, first->recipient().c_str());
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Queue size: %u", this->_sd->send_queue->size());

	this->_sm->desktop()->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, _("Sending message; %u messages remaining in send queue."), this->_sd->send_queue->size());

	if (this->_sd->send_queue->empty()) {
		this->_sm->desktop()->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Receive queue empty."));
	}

	if (first->repeatable()) {
		this->_sm->sendMessage(CHANGE_REPEATABLE, first->sequence(), first->message(), first->recipient().c_str(), first->chatroom());
	} else {
		this->_sm->sendMessage(CHANGE_NOT_REPEATABLE, first->sequence(), first->message(), first->recipient().c_str(), first->chatroom());
	}

	delete first;
//	Inkscape::GC::release(first);

	// TODO: is this used to facilitate resending?
	// this->_sd->send_queue->insert(first);

	return true;
}

bool
Callbacks::dispatchReceiveQueue()
{
	ReceiveMessageQueue *rq = this->_sd->receive_queue;

	// If we're not in a whiteboard session, don't dispatch anything,
	// but return true (to signify we're still alive)
	if (!this->_sd->status[IN_WHITEBOARD]) {
		return true;
	}

	// If there's nothing in the receive queue, return
	if (rq->empty()) {
		return true;
	}

	// Otherwise, retrieve the first change
	MessageNode* msg = rq->popFront();
	rq->setLatestProcessedPacket(msg->sender(), msg->sequence());
	this->_sm->desktop()->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, _("Receiving change; %u changes left to process."), rq->size());

	if (rq->empty()) {
		this->_sm->desktop()->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Receive queue empty."));
	}

	this->_sm->receiveChange(msg->message());
	delete msg;
	return true;
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
