/**
 * Stripped-down subclass of SerializerNodeObserver that simply watches and records
 * additions and deletions to an XMLNodeTracker
 * (used in recording undo/redo events in which we don't want to actually serialize
 * anything, just extract additions and deletions)
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/node.h"

#include "jabber_whiteboard/tracker-action-observer.h"

namespace Inkscape {

namespace Whiteboard {

void
TrackerActionObserver::notifyChildAdded(XML::Node& node, XML::Node& child, XML::Node* prev)
{
	this->_markObjectHelper(node, child, prev, NODE_ADD);
}

void
TrackerActionObserver::notifyChildRemoved(XML::Node& node, XML::Node& child, XML::Node* prev)
{
	this->_markObjectHelper(node, child, prev, NODE_REMOVE);
}

void
TrackerActionObserver::_markObjectHelper(XML::Node& parent, XML::Node& child, XML::Node* prev, NodeTrackerAction action)
{
	std::string childid = this->_findOrGenerateNodeID(child);
	this->newnodes[childid] = SerializedEventNodeAction(action, &child);
	this->newkeys[&child] = childid;

	if (child.childCount() > 0) {
		XML::Node* prev = child.firstChild();
		for(XML::Node* ch = child.firstChild(); ch; ch = ch->next()) {
			if (ch == child.firstChild()) {
				// No prev node in this case.
				this->_markObjectHelper(child, *ch, NULL, action);
			} else {
				this->_markObjectHelper(child, *ch, prev, action);
				prev = ch;
			}
		}
	}
	
	return;
}

}

}
