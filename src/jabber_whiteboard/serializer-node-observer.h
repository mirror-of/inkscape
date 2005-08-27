/**
 * NodeObserver used for serialization of XML::Events
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_SERIALIZER_NODE_OBSERVER_H__
#define __WHITEBOARD_SERIALIZER_NODE_OBSERVER_H__

#include "xml/node-observer.h"

#include "util/shared-c-string-ptr.h"

#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/node-tracker-observer.h"

#include <map>

namespace Inkscape {

namespace Whiteboard {

class XMLNodeTracker;


/**
 * A SerializerNodeObserver is designed to be called from XML::replay_log_to_observer().

 * 
 * Serialization of XML::Events (currently) also registers XML::Nodes with an XMLNodeTracker.
 * Additionally, there exists information that needs to be extracted from individual nodes
 * that is not directly available in the replay log: children of added objects and their
 * attributes, for instance.  The SerializerNodeObserver maintains internal state to deal 
 * with these additional responsibilities; however, it must be told explicitly when its
 * internal state can be cleared.
 * 
 * The serialized event is stored inside the class, and can be retrieved with three methods:
 *
 * getEventList() - returns a reference to the event list.  Note that if anything re-uses
 * the same SerializerNodeObserver between a call to getEventList() and usage of that 
 * list reference, the integrity of the serialized data cannot be guaranteed.
 *
 * getEventListCopy() - like the above, but returns a copy instead of a reference.  Slower,
 * but useful when you need to maintain a serialized event across a longer period of time.
 *
 * getAndClearEventList() - like the above, but also clears the internal event list.
 *
 * If the event list is to be used immediately after it is serialized, then getEventList()
 * may work best -- it does not involve the overhead of a copy, but it requires the user
 * to ensure that there will be no intervening calls to the same SerializerNodeObserver
 * and clear the list manually when required.
 *
 * Similar retrieval facilities exist for id <-> XML::Node maps constructed during serialization.
 */
class SerializerNodeObserver : public NodeTrackerObserver {
public:
    SerializerNodeObserver(XMLNodeTracker* xnt);
    
    ~SerializerNodeObserver();

    void notifyChildAdded(XML::Node &node, XML::Node &child, XML::Node *prev);

    void notifyChildRemoved(XML::Node &node, XML::Node &child, XML::Node *prev);

    void notifyChildOrderChanged(XML::Node &node, XML::Node &child,
                                         XML::Node *old_prev, XML::Node *new_prev);

    void notifyContentChanged(XML::Node &node,
                                      Util::SharedCStringPtr old_content,
                                      Util::SharedCStringPtr new_content);

    void notifyAttributeChanged(XML::Node &node, GQuark name,
                                        Util::SharedCStringPtr old_value,
                                        Util::SharedCStringPtr new_value);


	SerializedEventList& getEventList()
	{
		return this->_events;
	}

	SerializedEventList getEventListCopy()
	{
		return this->_events;
	}

	SerializedEventList getAndClearEventList()
	{
		SerializedEventList ret = this->_events;
		this->_events.clear();
		return ret;
	}

	void clearEventList()
	{
		this->_events.clear();
	}

	void clearAttributesScannedBuffer()
	{
		this->_attributes_scanned.clear();
	}

	// Convenience method for resetting all stateful aspects of the serializer
	void reset()
	{	
		this->clearEventList();
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Clearing serializer node buffers");
		this->clearNodeBuffers();
		this->clearAttributesScannedBuffer();
	}

private:
	SerializedEventList _events;
	AttributesScannedSet _attributes_scanned;

	void _newObjectEventHelper(XML::Node& parent, XML::Node& child, XML::Node* prev);
	void _recursiveMarkAsRemoved(XML::Node& node);

};

}

}

#endif
