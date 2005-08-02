/**
 * Whiteboard session manager
 * Typedef declarations and template specializations
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_TYPEDEFS_H__
#define __WHITEBOARD_TYPEDEFS_H__

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <algorithm>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <bitset>

#include <glibmm.h>
#include <sigc++/sigc++.h>

#include <boost/function.hpp>

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/tracker-node.h"

#include "gc-alloc.h"
//#include <gc/gc_allocator.h>

namespace Inkscape {

namespace XML {

class Node;

}

namespace Util {

template< typename T >
class ListContainer;

}

}

// Various specializations of std::less for XMLNodeTracker maps.
namespace std {
struct less< Inkscape::Whiteboard::TrackerNode* > : public binary_function < Inkscape::Whiteboard::TrackerNode*, Inkscape::Whiteboard::TrackerNode*, bool >
{
	bool operator()(Inkscape::Whiteboard::TrackerNode* _x, Inkscape::Whiteboard::TrackerNode* _y) const
	{
		return _x->_node < _y->_node;
	}

};
}

namespace Inkscape {

namespace Whiteboard {
// I am assuming that std::string (which will not properly represent Unicode data) will
// suffice for associating (integer, Jabber ID) identifiers with nodes. 
// We do not need to preserve all semantics handled by Unicode; we just need to have 
// the byte representation.  std::string is good enough for that.
//
// The reason for this is that comparisons with std::string are much faster than 
// comparisons with Glib::ustring (simply because the latter is using significantly
// more complex text-handling algorithms), and we need speed here.  We _could_ use
// Glib::ustring::collate_key() here and therefore get the best of both worlds,
// but collation keys are rather big.
//
// XML node tracker maps
//

/*
typedef std::map< std::string, TrackerNode*, std::less< std::string >, traceable_allocator< std::pair< std::string, TrackerNode* > > > KeyToTrackerNodeMap;
typedef std::map< TrackerNode*, std::string, std::less< TrackerNode* >, traceable_allocator< std::pair< TrackerNode*, std::string > > > TrackerNodeToKeyMap;
*/

// FIXME: GC::Alloc doesn't seem to work with std::map
typedef std::map< std::string, TrackerNode*, std::less< std::string >, GC::Alloc< std::pair < std::string, TrackerNode* >, GC::MANUAL > > KeyToTrackerNodeMap;
typedef std::map< TrackerNode*, std::string, std::less< TrackerNode* >, GC::Alloc< std::pair< TrackerNode*, std::string >, GC::MANUAL > > TrackerNodeToKeyMap;

// Temporary storage of new object messages and new nodes in said messages
typedef std::list< Glib::ustring > NewChildObjectMessageList;
typedef std::map< std::string, XML::Node const* > KeyToNodeMap;
typedef std::map< XML::Node const*, std::string > NodeToKeyMap;

// Buddy list management
typedef std::set< std::string > BuddyList;
typedef sigc::signal< void, std::string const& > BuddyListSignal;
typedef sigc::slot< void, std::string const& > BuddyListListener;

// Chatroom list participants
typedef std::set< char const* > ChatterList;

// Message context verification and processing
class SessionManager;
struct ProcessorShell;
struct JabberMessage;

typedef std::map< MessageType, std::bitset< NUM_FLAGS > > MessageContextMap;
typedef std::map< MessageType, ProcessorShell*, std::less< MessageType >, GC::Alloc< std::pair< MessageType, ProcessorShell* >, GC::MANUAL > > MessageProcessorMap;
//typedef std::map< MessageType, ProcessorShell*, std::less< MessageType >, traceable_allocator< std::pair< MessageType, ProcessorShell* > > > MessageProcessorMap;

// Error handling -- someday
// TODO: finish and integrate this
typedef boost::function< LmHandlerResult (unsigned int code) > ErrorHandlerFunctor;
typedef std::map< unsigned int, ErrorHandlerFunctor > ErrorHandlerFunctorMap;
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
