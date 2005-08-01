/**
 * Whiteboard session manager
 * Buddy list management facility
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_BUDDYLIST_MANAGER_H__
#define __WHITEBOARD_BUDDYLIST_MANAGER_H__

#include <string>
#include <sigc++/sigc++.h>
#include "jabber_whiteboard/typedefs.h"

namespace Inkscape {

namespace Whiteboard {

class BuddyListManager {
public:
	BuddyListManager();
	~BuddyListManager();
	void insert(std::string& jid);
	void erase(std::string& jid);
	BuddyList::iterator begin();
	BuddyList::iterator end();

	void addInsertListener(BuddyListListener listener);
	void addEraseListener(BuddyListListener listener);

	BuddyList& getList();
private:
	BuddyList _bl;
	BuddyListSignal _sig_insert;
	BuddyListSignal _sig_erase;

};

}

}

#endif
