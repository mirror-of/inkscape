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
		this->_log = NULL;
	}

	KeyToNodeActionMap& getNodeActionMap()
	{
		return this->_newnodes;
	}

	KeyToNodeActionMap getNodeActionMapCopy()
	{
		return this->_newnodes;
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
		this->_newnodes.clear();
		this->_newkeys.clear();
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
		KeyToNodeActionMap::iterator i = this->_newnodes.find(id);
		if (i != this->_newnodes.end()) {
			return const_cast< XML::Node* >((*i).second.second);
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

	XML::LogBuilder _builder;

	KeyToNodeActionMap _newnodes;
	NodeToKeyMap _newkeys;
	AttributesUpdatedSet _updated;

	XMLNodeTracker* _xnt;

	XML::Event* _log;
};

}

}

#endif
