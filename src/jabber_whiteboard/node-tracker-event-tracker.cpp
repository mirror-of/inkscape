/**
 * Tracks node add/remove events to an XMLNodeTracker, and eliminates cases such as
 * consecutive add/remove.
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/node.h"

#include "jabber_whiteboard/node-tracker-event-tracker.h"

namespace Inkscape {

namespace Whiteboard {

bool
NodeTrackerEventTracker::tryToTrack(XML::Node* node, NodeTrackerAction action)
{
	// 1.  Check if node is being tracked.
	NodeActionMap::iterator i = this->_actions.find(node);
	if (i != this->_actions.end()) {
		// 2a.  Check the action.  If it is the same as the action we are registering,
		// return false.  Otherwise, register the action with the actions map
		// and return true.
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Found event for node %p: action=%u, node action=%u", node, action, i->second);
		if (i->second == action) {
			return false;
		} else {
			this->_actions[node] = action;
			return true;
		}
	} else {
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Registering node %p: node action=%u", node, action);
		// 2b.  If we aren't tracking this node, insert it with the given action.
		this->_actions[node] = action;
		return true;
	}
}

}

}
