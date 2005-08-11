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

#ifndef __WHITEBOARD_TRACKER_ACTION_OBSERVER_H__
#define __WHITEBOARD_TRACKER_ACTION_OBSERVER_H__

#include "jabber_whiteboard/node-tracker-observer.h"

namespace Inkscape {

namespace Whiteboard {

// TODO: put an instance of this inside a SerializerNodeObserver to avoid 
// code duplication
class TrackerActionObserver : public NodeTrackerObserver {
public:
	TrackerActionObserver(XMLNodeTracker* xnt) : NodeTrackerObserver(xnt) { }
	~TrackerActionObserver() { }

	void notifyChildAdded(XML::Node& node, XML::Node& child, XML::Node* prev);
	void notifyChildRemoved(XML::Node& node, XML::Node& child, XML::Node* prev);

	// don't need to handle these
    void notifyChildOrderChanged(XML::Node &node, XML::Node &child,
                                         XML::Node *old_prev, XML::Node *new_prev) {}

    void notifyContentChanged(XML::Node &node,
                                      Util::SharedCStringPtr old_content,
                                      Util::SharedCStringPtr new_content) { }

    void notifyAttributeChanged(XML::Node &node, GQuark name,
                                        Util::SharedCStringPtr old_value,
                                        Util::SharedCStringPtr new_value) { }

private:
	void _markObjectHelper(XML::Node& parent, XML::Node& child, XML::Node* prev, NodeTrackerAction action);
};

}

}

#endif
