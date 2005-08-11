/**
 * Whiteboard message queue and queue handler functions
 * Node for storing messages in message queues
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_NODE_H__
#define __WHITEBOARD_MESSAGE_NODE_H__

#include <string>
#include <glibmm.h>

#include "gc-managed.h"
#include "gc-anchored.h"
#include "gc-finalized.h"

namespace Inkscape {

namespace Whiteboard {

class MessageNode : public GC::Managed<>, public GC::Anchored, public GC::Finalized {
public:
	MessageNode(unsigned int seq, std::string sender, std::string recip, Glib::ustring const* message_body, MessageType type, bool document, bool chatroom) :
		_seq(seq), _type(type), _document(document), _chatroom(chatroom)
	{
		this->_sender = sender;
		this->_recipient = recip;
	
		if (message_body != NULL) {
			this->_message = new Glib::ustring(*message_body);
		} else {
			this->_message = NULL;
		}
	}

	~MessageNode() 
	{
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "MessageNode destructor");
		if (this->_message) {
			delete this->_message;
		}
	}

	unsigned int sequence()
	{
		return this->_seq;
	}

	MessageType type()
	{
		return this->_type;
	}

	bool chatroom()
	{
		return this->_chatroom;
	}

	bool document()
	{
		return this->_document;
	}

	std::string recipient()
	{
		return this->_recipient;
	}

	std::string sender()
	{
		return this->_sender;
	}

	Glib::ustring* message()
	{
		return this->_message;
	}

private:
	unsigned int _seq;
	std::string _sender;
	std::string _recipient;
	Glib::ustring* _message;
	MessageType _type;
	bool _document;
	bool _chatroom;
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
