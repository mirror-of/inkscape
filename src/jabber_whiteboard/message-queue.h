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

#ifndef __WHITEBOARD_MESSAGE_QUEUE_H__
#define __WHITEBOARD_MESSAGE_QUEUE_H__

#include <list>
#include <map>

#include "gc-alloc.h"
#include "util/list-container.h"

namespace Inkscape {

namespace Whiteboard {

class SessionManager;
class MessageNode;

typedef std::list< MessageNode* > MessageQueueBuffer;
typedef std::map< std::string, unsigned int > LatestProcessedTracker;

class MessageQueue {
public:
	MessageQueue(SessionManager *sm);
	virtual ~MessageQueue();

	MessageNode* first();
	MessageNode* popFront();
	unsigned int size();
	bool empty();
	void clear();

	virtual void insert(MessageNode* msg) = 0;

protected:
	MessageQueueBuffer _queue;
	SessionManager* _sm;
};

class ReceiveMessageQueue : public MessageQueue {
public:
	ReceiveMessageQueue(SessionManager* sm);
	void insert(MessageNode* msg);
	void setLatestProcessedPacket(std::string sender, unsigned int seq);
private:
	LatestProcessedTracker _tracker;
};

class SendMessageQueue : public MessageQueue {
public:
	SendMessageQueue(SessionManager* sm);
	void insert(MessageNode* msg);
};

}

}

#endif
