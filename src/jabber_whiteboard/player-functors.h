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


#ifndef __WHITEBOARD_SESSION_FILE_PLAYER_FUNCTORS_H__
#define __WHITEBOARD_SESSION_FILE_PLAYER_FUNCTORS_H__

#include <list>
#include "gc-alloc.h"

namespace Inkscape {

namespace Whiteboard {

class SessionMessageNode;
class SessionFile;

typedef std::list< SessionMessageNode*, std::allocator< SessionMessageNode* > > nodelist;

class NewMessageToFront {
public:
	NewMessageToFront(SessionFile* sf);
	bool operator()(nodelist& buffer, SessionMessageNode* nextItem) const;
	bool hasMore();

private:
	SessionFile* _activefile;
};

class NewMessageToBack {
public:
	NewMessageToBack(SessionFile* sf);
	~NewMessageToBack();
	bool operator()(nodelist& buffer, SessionMessageNode* prevItem) const;
	bool hasMore();

private:
	SessionFile* _activefile;
};

class SessionBufferInitializer {
public:
	SessionBufferInitializer(SessionFile* sf);
	bool operator()(nodelist& buffer) const;

private:
	SessionFile* _activefile;
};

}

}

#endif
