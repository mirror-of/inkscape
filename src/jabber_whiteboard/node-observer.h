/**
 * Whiteboard session manager
 * XML node observer
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifndef __WHITEBOARD_NODE_OBSERVER_H__
#define __WHITEBOARD_NODE_OBSERVER_H__

#include <glib/gquark.h>
#include <glibmm.h>

namespace Inkscape {
namespace XML {
class Node;
class NodeObserver;
}
}

namespace Inkscape {

namespace Whiteboard {

class SessionManager;

class XMLNodeObserver : public XML::NodeObserver {
public:
	XMLNodeObserver(SessionManager* sm);
	~XMLNodeObserver();

	void notifyChildAdded(XML::Node& node, XML::Node& child, XML::Node* prev);
	void notifyChildRemoved(XML::Node& node, XML::Node& child, XML::Node* prev);
	void notifyChildOrderChanged(XML::Node& node, XML::Node& child, XML::Node* old_prev, XML::Node* new_prev);
	void notifyContentChanged(XML::Node& node, Util::SharedCStringPtr old_content, Util::SharedCStringPtr new_content);
	void notifyAttributeChanged(XML::Node& node, GQuark name, Util::SharedCStringPtr old_value, Util::SharedCStringPtr new_value);

private:
	bool isNumeric(Util::SharedCStringPtr& str);
	inline bool isDigit(char const d);

	SessionManager* _session;

	// noncopyable, nonassignable
	XMLNodeObserver(XMLNodeObserver const&);
	XMLNodeObserver& operator=(XMLNodeObserver const&);
};

}

}


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
#endif
