/**
 * Whiteboard message queue 
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "desktop.h"

#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/message-node.h"
#include "jabber_whiteboard/message-queue.h"

namespace Inkscape {

namespace Whiteboard {

MessageQueue::MessageQueue(SessionManager* sm) : _sm(sm)
{

}

MessageQueue::~MessageQueue()
{

}

MessageNode*
MessageQueue::first()
{
	return this->_queue.front();
}

MessageNode*
MessageQueue::popFront()
{
	MessageNode* mn = this->first();
	this->_queue.pop_front();
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Removed element, queue size (for %s): %u", lm_connection_get_jid(this->_sm->session_data->connection), this->_queue.size());
	return mn;
}

unsigned int
MessageQueue::size()
{
	return this->_queue.size();
}

bool
MessageQueue::empty()
{
	return this->_queue.empty();
}

void
MessageQueue::clear()
{
	this->_queue.clear();
}

ReceiveMessageQueue::ReceiveMessageQueue(SessionManager* sm) : MessageQueue(sm)
{

}

void
ReceiveMessageQueue::insert(MessageNode* msg)
{
	// Check to see if the incoming message has a sequence number
	// lower than the sequence number of the latest message processed
	// by this message's sender.  If it does, drop the message and produce
	// a warning.
	unsigned int latest = this->_tracker[msg->sender()];
	if (msg->sequence() < latest) {
		g_warning(_("Received late message (message sequence number is %u, but latest processed message had sequence number %u).  Discarding message; session may be desynchronized."), msg->sequence(), latest);
	}

	// Otherwise, it is safe to insert this message.
//	Inkscape::GC::anchor(msg);
	this->_queue.push_back(msg);
	this->_sm->desktop()->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, _("%u changes queued in receive queue."), this->_queue.size());
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Receive queue size (for %s): %u", lm_connection_get_jid(this->_sm->session_data->connection), this->_queue.size());
}

void
ReceiveMessageQueue::setLatestProcessedPacket(std::string sender, unsigned int seq)
{
	this->_tracker[sender] = seq;
}

SendMessageQueue::SendMessageQueue(SessionManager* sm) : MessageQueue(sm)
{

}

void
SendMessageQueue::insert(MessageNode* msg)
{
//	Inkscape::GC::anchor(msg);
	this->_queue.push_back(msg);
	this->_sm->desktop()->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, _("%u changes queued in send queue."), this->_queue.size());
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Send queue size (for %s): %u",  lm_connection_get_jid(this->_sm->session_data->connection), this->_queue.size());
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
