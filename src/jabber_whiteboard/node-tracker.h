/**
 * Whiteboard session manager
 * XML node tracking facility
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_XML_NODE_TRACKER_H__
#define __WHITEBOARD_XML_NODE_TRACKER_H__

#include "jabber_whiteboard/tracker-node.h"
#include "jabber_whiteboard/typedefs.h"

#include <bitset>
#include <cstring>
#include <map>
#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard { 

class SessionManager;

struct strcmpless : public std::binary_function< char const*, char const*, bool >
{
	bool operator()(char const* _x, char const* _y) const
	{
		return (strcmp(_x, _y) < 0);
	}
};


// TODO: This is a pretty heinous mess of methods that accept
// both pointers and references -- a lot of it has to do with
// XML::Node& in the node observer and XML::Node* elsewhere,
// although some of it (like Glib::ustring const& vs. 
// Glib::ustring const*) is completely mea culpa. When possible
// it'd be good to thin this class out.
class XMLNodeTracker  {
public:
	XMLNodeTracker(SessionManager* doc);
	~XMLNodeTracker();

	void put(std::string key, XML::Node& node);
	void put(KeyToNodeMap& newids, NodeToKeyMap& newnodes);

	XML::Node& get(std::string& key);
	XML::Node& get(std::string const& key);
	std::string const get(XML::Node const& node);

	void remove(std::string& key);
	void remove(XML::Node& node);

	bool isTracking(std::string& key);
	bool isTracking(std::string const& key);
	bool isTracking(XML::Node const& node);

	void lock(XML::Node const* node, ListenerType listener);
	void lock(XML::Node const& node, ListenerType listener);
	void unlock(XML::Node const* node, ListenerType listener);
	void unlock(XML::Node const& node, ListenerType listener);
	bool isLocked(XML::Node const& node, ListenerType listener);
	bool isLocked(std::string& key, ListenerType listener);

	bool isSpecialNode(char const* name);
	bool isSpecialNode(std::string const& name);
	std::string const getSpecialNodeKeyFromName(Glib::ustring const& name);
	std::string const getSpecialNodeKeyFromName(Glib::ustring const* name);

	bool isRootNode(XML::Node& node);

	std::string generateKey(gchar const* JID);
	std::string generateKey();

	// TODO: remove debugging function
	void dump();
	void reset();

private:
	void createSpecialNodeTables();
	void _clear();
	
	unsigned int _counter;
	SessionManager* _sm;

	// defined in typedefs.h
	KeyToTrackerNodeMap _keyToNode;
	TrackerNodeToKeyMap _nodeToKey;

	std::map< char const*, char const*, strcmpless > _specialnodes;

	// Keys for special nodes
	std::string _rootKey;
	std::string _defsKey;
	std::string _namedviewKey;
	std::string _metadataKey;

	// noncopyable, nonassignable
	XMLNodeTracker(XMLNodeTracker const&);
	XMLNodeTracker& operator=(XMLNodeTracker const&);
};

}

}


#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
