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

#include <cstring>
#include <glibmm.h>

#include "xml/node-observer.h"
#include "xml/repr.h"

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/node-observer.h"

namespace Inkscape {

namespace Whiteboard {

XMLNodeObserver::XMLNodeObserver(SessionManager* sm) 
{
	this->_session = sm;
}

XMLNodeObserver::~XMLNodeObserver() 
{

}

void
XMLNodeObserver::notifyChildAdded(XML::Node& node, XML::Node& child, XML::Node* prev)
{
	// If the child added listener is locked for this session,
	// then the newly added child isn't ours. 

	if (this->_session->node_tracker()->isLocked(node, CHILD_ADDED)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyChildAdded on node %p (child %p, prev %p) is locked", &node, &child, prev);
		return;
	}
	/*
	if (this->_session->child_added_listener_locked) {
		return;
	}
	*/

	// Otherwise, we created this object; continue.
	Glib::ustring* strbuf = new Glib::ustring(); 
	KeyToNodeMap newids;
	NodeToKeyMap newnodes;
	NewChildObjectMessageList newchildren;

	MessageUtilities::newObjectMessage(strbuf, newids, newnodes, newchildren, this->_session->node_tracker(), &child);

	NodeToKeyMap::iterator i = newnodes.begin();
	for(; i != newnodes.end(); i++) {
		const_cast< XML::Node* >((*i).first)->addObserver(*this);
	}

	this->_session->node_tracker()->put(newids, newnodes);
//	this->_session->node_tracker()->dump();
//	this->_session->sendChange(strbuf, FALSE, this->_session->session_data->status.test(IN_CHATROOM));
	
	NewChildObjectMessageList::iterator j = newchildren.begin();
	for(; j != newchildren.end(); j++) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "New child message: %s", (*j).c_str());
		this->_session->sendChange(&(*j), FALSE, this->_session->session_data->status.test(IN_CHATROOM));
	}

	delete strbuf;
}


void
XMLNodeObserver::notifyChildRemoved(XML::Node& node, XML::Node& child, XML::Node* prev)
{
	if (this->_session->node_tracker()->isLocked(child, CHILD_REMOVED)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyChildRemoved on node %p (child %p) is locked", &node, &child);
		return;
	}

	/*
	if (this->_session->child_removed_listener_locked) {
		return;
	}
	*/

	if (this->_session->node_tracker()->isTracking(child)) {
		// Generate deletion message
		Glib::ustring* strbuf = new Glib::ustring();
		MessageUtilities::objectDeleteMessage(strbuf, this->_session->node_tracker(), node, child, prev);
		this->_session->sendChange(strbuf, FALSE, this->_session->session_data->status.test(IN_CHATROOM));
		child.removeObserver(*this);
		NodeUtilities::recursiveRemoveFromTracker(child, *this->_session->node_observer(), this->_session->node_tracker());
//		this->_session->node_tracker()->remove(child);
		delete strbuf;
	}
}


void
XMLNodeObserver::notifyChildOrderChanged(XML::Node& node, XML::Node& child, XML::Node* old_prev, XML::Node* new_prev)
{
	if (this->_session->node_tracker()->isLocked(child, CHILD_ORDER_CHANGED)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyChildOrderChanged on node %p (child %p, oldprev %p, newprev %p) is locked", &node, &child, old_prev, new_prev);
		return;
	}

	/*
	if (this->_session->child_order_changed_listener_locked) {
		return;
	}
	*/

	if (this->_session->node_tracker()->isTracking(node)) {
		std::string childid = this->_session->node_tracker()->get(child);
		std::string oldprevid = this->_session->node_tracker()->get(*old_prev);
		std::string newprevid = this->_session->node_tracker()->get(*new_prev);

		if (!oldprevid.empty() && !newprevid.empty()) {
			if (oldprevid == newprevid) {
//				g_log(NULL, G_LOG_LEVEL_DEBUG, "Child order has not changed; not transmitting.");
				return;
			} else {
				Glib::ustring strbuf;
				MessageUtilities::childOrderChangeMessage(strbuf, childid, oldprevid, newprevid);
				this->_session->sendChange(&strbuf, true, this->_session->session_data->status.test(IN_CHATROOM));
			}
		}
	}
}

void
XMLNodeObserver::notifyContentChanged(XML::Node& node, Util::SharedCStringPtr old_content, Util::SharedCStringPtr new_content)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyContentChanged listener triggered");

	if (this->_session->node_tracker()->isLocked(node, CONTENT_CHANGED)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyContentChanged on node %p is locked", &node);
		return;
	}

	/*
	if (this->_session->content_changed_listener_locked) {
		g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyContentChanged listener locked");
		return;
	}
	*/

	if (old_content.cString() != NULL && new_content.cString() != NULL && strcmp(old_content.cString(), new_content.cString()) != 0) {
		if (this->_session->node_tracker()->isTracking(node)) {
			Glib::ustring strbuf;
			MessageUtilities::contentChangeMessage(strbuf, this->_session->node_tracker()->get(node), old_content, new_content);
			this->_session->sendChange(&strbuf, TRUE, this->_session->session_data->status.test(IN_CHATROOM));
			return;
		}
	} else {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Node content has not changed; not transmitting.");
		return;
	}
}

void
XMLNodeObserver::notifyAttributeChanged(XML::Node& node, GQuark name, Util::SharedCStringPtr old_value, Util::SharedCStringPtr new_value)
{

//	g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyAttributeChanged");

	if (this->_session->node_tracker()->isLocked(node, ATTR_CHANGED)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "notifyAttributeChanged on node %p is locked", &node);
		return;
	}

	/*
	if (this->_session->attr_changed_listener_locked) {
		return;
	}
	*/

	if (old_value.cString() != NULL && new_value.cString() != NULL) {
		if ((strcmp(old_value.cString(), new_value.cString()) == 0)) {
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "old == new, not transmitting change");
			return;
		}

		// FIXME: Sometimes, when using the text tool, changes will be made 
		// (usually in x/y coordinate position) that differ in a string
		// comparison but are really the same sans significant digits
		// (i.e. 100 vs 100.00000 or 567.2313 vs 567.23130).
		//
		// These changes tend to be relayed back and forth between Inkboard
		// clients, which results in an infinite loop of messages.
		//
		// Detecting these similarities and discarding them _seems_ to 
		// eliminate the loop, but this really shouldn't be necessary in
		// the first place...
		if (this->isNumeric(old_value)) {
			double oldv = Glib::Ascii::strtod(old_value.cString());
			double newv = Glib::Ascii::strtod(new_value.cString());
			if (oldv == newv) {
				return;
			}
		}
	}

	if (this->_session->node_tracker()->isTracking(node)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "Sending change (made from %s): node=%s oldval=%s newval=%s", lm_connection_get_jid(this->_session->session_data->connection), this->_session->node_tracker()->get(node).c_str(), old_value.cString(), new_value.cString());

		Glib::ustring *strbuf = new Glib::ustring();

		MessageUtilities::objectChangeMessage(strbuf, this->_session->node_tracker(), this->_session->node_tracker()->get(node), g_quark_to_string(name), old_value.cString(), new_value.cString(), false);

		this->_session->sendChange(strbuf, TRUE, this->_session->session_data->status.test(IN_CHATROOM));

		delete strbuf;
	}
}

bool
XMLNodeObserver::isNumeric(Util::SharedCStringPtr& str)
{
	bool found_dec = false;
	size_t i = 0;

	while(str[i] != '\0') {
		char c = str[i];
		if (c == '.' && found_dec == false) {
			found_dec = true;
		} else if (c == '.' && found_dec == true) {
			return false;
		} else if (!this->isDigit(c)) {
			return false;
		}
		i++;
	}
	if (i < 1) {
		return false;
	} else {
		return true;
	}
}

inline bool
XMLNodeObserver::isDigit(char const d)
{
	switch(d) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return true;
		default:
			return false;
	}
}

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
