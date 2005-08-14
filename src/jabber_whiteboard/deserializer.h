/**
 * Inkboard message -> XML::Event* deserializer
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_DESERIALIZER_H__
#define __WHITEBOARD_MESSAGE_DESERIALIZER_H__

#include "xml/log-builder.h"
#include "xml/event.h"

#include "jabber_whiteboard/node-tracker-event-tracker.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/typedefs.h"

#include <functional>
#include <algorithm>
#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

/**
 * A stateful deserializer, meant to deserialize XML::Events serialized by 
 * Inkscape::Whiteboard::SerializerNodeObserver or a serializer that serializes
 * XML::Events into the same format.
 *
 * This deserializer provides facilities similar to SerializerNodeObserver.
 */
class Deserializer {
public:
	Deserializer(XMLNodeTracker* xnt) : _xnt(xnt)
	{
		this->clearEventLog();
	}

	~Deserializer() { }

	void deserializeEventAdd(Glib::ustring const& msg);
	void deserializeEventDel(Glib::ustring const& msg);
	void deserializeEventChgOrder(Glib::ustring const& msg);
	void deserializeEventChgContent(Glib::ustring const& msg);
	void deserializeEventChgAttr(Glib::ustring const& msg);

	XML::Event* getEventLog()
	{
		return this->_log;
	}

	XML::Event* detachEventLog()
	{
		XML::Event* ret = this->_log;
		this->clearEventLog();
		return ret;
	}

	void clearEventLog()
	{
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Clearing event log");
		this->_log = NULL;
	}

	KeyToNodeActionList& getNodeTrackerActions()
	{
		return this->_actions;
	}

	KeyToNodeActionList getNodeTrackerActionsCopy()
	{
		return this->_actions;
	}

	AttributesUpdatedSet& getUpdatedAttributeNodeSet()
	{
		return this->_updated;
	}

	AttributesUpdatedSet getUpdatedAttributeNodeSetCopy()
	{
		return this->_updated;
	}

	void clearNodeBuffers()
	{
		g_log(NULL, G_LOG_LEVEL_DEBUG, "Clearing deserializer node buffers");
		this->_newnodes.clear();
		this->_actions.clear();
		this->_newkeys.clear();
		this->_parent_child_map.clear();
		this->_updated.clear();
	}

	void reset() 
	{
		this->clearEventLog();
		this->clearNodeBuffers();
	}

private:
	XML::Node* _getNodeByID(std::string const& id)
	{
		KeyToNodeMap::iterator i = this->_newnodes.find(id);
		if (i != this->_newnodes.end()) {
			return const_cast< XML::Node* >(i->second);
		} else {
			if (this->_xnt->isTracking(id)) {
				return this->_xnt->get(id);
			} else {
				return NULL;
			}
		}
	}

	void _addOneEvent(XML::Event* event)
	{
		if (this->_log == NULL) {
			this->_log = event;
		} else {
			event->next = this->_log;
			this->_log = event;
		}
	}

	void _recursiveMarkForRemoval(XML::Node* node);

	// internal states with accessors:
	
	// node tracker actions (add node, remove node)
	KeyToNodeActionList _actions;

	// nodes that have had their attributes updated
	AttributesUpdatedSet _updated;

	// the deserialized event log
	XML::Event* _log;


	// for internal use:
	
	// These maps only store information on a single node.  That's fine, though;
	// all we care about is the ability to do key <-> node association.  The NodeTrackerEventTracker
	// and KeyToNodeActionList keep track of the actual actions we need to perform 
	// on the node tracker.
	NodeToKeyMap _newkeys;
	KeyToNodeMap _newnodes;
	NodeTrackerEventTracker _node_action_tracker;

	typedef std::map< XML::Node*, XML::Node* > _pc_map_type;
	_pc_map_type _parent_child_map;

	XMLNodeTracker* _xnt;

	XML::LogBuilder _builder;
};

}

}

#endif
