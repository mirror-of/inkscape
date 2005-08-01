/**
 * Whiteboard session file playback mechanism
 * Insertion functors for sliding buffer
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>

#include "jabber_whiteboard/player-functors.h"
#include "jabber_whiteboard/session-file.h"
#include "jabber_whiteboard/session-file-player.h"

namespace Inkscape {

namespace Whiteboard {

// Buffer "push front" functor
NewMessageToFront::NewMessageToFront(SessionFile* sf) : _activefile(sf) { }

bool
NewMessageToFront::operator()(nodelist& buffer, SessionMessageNode* next) const
{
	SessionMessageNode* smn = new SessionMessageNode;
	smn->prevmsgbytecount = 0;
	gint64 from = next->nextmsgstart - next->msg.bytes() - next->prevmsgbytecount;
	smn->nextmsgstart = this->_activefile->nextMessageFrom(from, smn->msg);
	g_log(NULL, G_LOG_LEVEL_DEBUG, "nmf: from: %llu, next: %llu, msg: %s", from, smn->nextmsgstart, smn->msg.c_str());
	if (smn->nextmsgstart != from) {
		buffer.push_front(smn);
		return true;
	} else {
		delete smn;
		return false;
	}
}

bool
NewMessageToFront::hasMore()
{
	return true;
}

// Buffer "push back" functor
NewMessageToBack::NewMessageToBack(SessionFile* sf) : _activefile(sf) {
	g_log(NULL, G_LOG_LEVEL_DEBUG, "nmb ctor: activefile: %p", this->_activefile);
}

NewMessageToBack::~NewMessageToBack() {
	g_log(NULL, G_LOG_LEVEL_DEBUG, "nmb destructor");
}

bool
NewMessageToBack::operator()(nodelist& buffer, SessionMessageNode* prev) const
{
	SessionMessageNode* smn = new SessionMessageNode;
	smn->prevmsgbytecount = prev->msg.bytes();
	smn->nextmsgstart = this->_activefile->nextMessageFrom(prev->nextmsgstart, smn->msg);
	if (smn->nextmsgstart != prev->nextmsgstart) {
		buffer.push_back(smn);
		return true;
	} else {
		delete smn;
		return false;
	}
}

bool
NewMessageToBack::hasMore()
{
	return !(this->_activefile->eof());
}


// Buffer initialization functor
SessionBufferInitializer::SessionBufferInitializer(SessionFile* sf) : _activefile(sf) { }

bool
SessionBufferInitializer::operator()(nodelist& buffer) const
{
	// Read the first message from the session file
	SessionMessageNode* smn = new SessionMessageNode;
	smn->prevmsgbytecount = 0;
	smn->nextmsgstart = this->_activefile->nextMessageFrom(0, smn->msg);
	buffer.push_front(smn);
	return true;
}

}

}
