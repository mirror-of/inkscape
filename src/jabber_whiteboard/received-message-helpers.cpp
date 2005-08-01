/**
 * Whiteboard session manager
 * Received message helpers
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <set>
#include <bitset>
#include <iostream>

#include <glibmm.h>
#include <glibmm/i18n.h>


extern "C" {
#include <loudmouth/loudmouth.h>
}

#include "inkscape.h"
#include "desktop-handles.h"
#include "document.h"
#include "sp-object.h"

#include "xml/node.h"
#include "xml/repr.h"
#include "xml/node-observer.h"

#include "dialogs/whiteboard-connect-dialog.h"
#include "dialogs/whiteboard-sharewithuser-dialog.h"

#include "jabber_whiteboard/session-file.h"
#include "jabber_whiteboard/node-tracker.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/node-utilities.h"
#include "jabber_whiteboard/node-observer.h"
#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/defines.h"


namespace Inkscape {

namespace Whiteboard {

void
SessionManager::receivedChangeHelper(Glib::ustring* msg)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receivedChangeHelper");
	std::string id;
	Glib::ustring key, oldval, newval, repeatable;

	if (MessageUtilities::extractChangeData(msg, id, &key, &oldval, &newval, &repeatable)) {
		if (this->_myTracker->isTracking(id)) {
			XML::Node& target = this->_myTracker->get(id);

			if (this->session_data->connection) {
//				g_log(NULL, G_LOG_LEVEL_DEBUG, "(%s) Setting attribute %s of node %s to %s", lm_connection_get_jid(this->session_data->connection), key.data(), target.attribute("id"), newval.data());
			}

			this->_myTracker->lock(target, ATTR_CHANGED);
			this->_myTracker->lock(target, CONTENT_CHANGED);

		//	this->attr_changed_listener_locked = true;
		//	this->content_changed_listener_locked = true;

			target.setAttribute(key.data(), newval.data(), FALSE);

			// need this to properly force update of text
			// (and other objects with dependents)
			// TODO: Is this right place to issue updateRepr method call? Investigate.
			SPObject* updated = this->_myDoc->getObjectByRepr(&target);
			if (updated) {
				updated->updateRepr();
			}

			this->_myTracker->unlock(target, CONTENT_CHANGED);
			this->_myTracker->unlock(target, ATTR_CHANGED);

//			this->content_changed_listener_locked = false;
//			this->attr_changed_listener_locked = false;
		} else {

		}
	} else {
		return;
	}
}

void
SessionManager::receivedContentChangeHelper(Glib::ustring const& msg) 
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receivedContentChangeHelper");
	std::string id;
	Glib::ustring oldval, newval;
	Node buf;

	// Required attributes
	buf.tag = MESSAGE_ID;
	if (!MessageUtilities::findTag(&buf, &msg)) {
		return;
	} else {
		id = buf.data.data();
	}

	buf.tag = MESSAGE_OLDVAL;
	if (!MessageUtilities::findTag(&buf, &msg)) {
		return;
	} else {
		oldval = buf.data;
	}

	buf.tag = MESSAGE_NEWVAL;
	if (!MessageUtilities::findTag(&buf, &msg)) {
		return;
	} else {
		newval = buf.data;
	}

	// Lookup node
	if (this->_myTracker->isTracking(id)) {
		XML::Node& node = this->_myTracker->get(id);
		if (oldval != newval) {
			// Lock listeners

			this->_myTracker->lock(node, CONTENT_CHANGED);
			this->_myTracker->lock(node, ATTR_CHANGED);

		//	this->content_changed_listener_locked = true;
		//	this->attr_changed_listener_locked = true;

			node.setContent(newval.data());
			SPObject* updated = this->_myDoc->getObjectByRepr(&node);
			if (updated) {
				updated->updateRepr();
			}

			// Unlock
			this->_myTracker->unlock(node, ATTR_CHANGED);
			this->_myTracker->unlock(node, CONTENT_CHANGED);

//			this->content_changed_listener_locked = false;
//			this->attr_changed_listener_locked = false;
		}
	} else {
		g_warning("Received content change message for unknown node %s; ignoring.", id.c_str());
	}
}

void
SessionManager::receivedChildOrderChangeHelper(Glib::ustring const& msg)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receivedChildOrderChangeHelper");
	std::string id, oldprevid, newprevid;

	Node buf;

	// Required attributes
	buf.tag = MESSAGE_CHILD;
	if (!MessageUtilities::findTag(&buf, &msg)) {
		return;
	} else {
		id = buf.data.data();
	}

	/*
	buf.tag = MESSAGE_OLDVAL;
	if (!MessageUtilities::findTag(&buf, &msg)) {
		return;
	} else {
		oldprevid = buf.data;
	}*/

	buf.tag = MESSAGE_NEWVAL;
	if (!MessageUtilities::findTag(&buf, &msg)) {
		return;
	} else {
		newprevid = buf.data;
	}

	// Look up node
	if (this->_myTracker->isTracking(id)) {
		XML::Node* child = &this->_myTracker->get(id);
		XML::Node* parent = child->parent();
		if (parent != NULL) {
			if (this->_myTracker->isTracking(newprevid)) {
				XML::Node* newprev = &this->_myTracker->get(newprevid);
//				g_log(NULL, G_LOG_LEVEL_DEBUG, "newprev parent: %p, node parent: %p", newprev->parent(), parent);

				this->_myTracker->lock(child, CHILD_ORDER_CHANGED);

			//	this->child_order_changed_listener_locked = true;
				parent->changeOrder(child, newprev);

				this->_myTracker->unlock(child, CHILD_ORDER_CHANGED);
			//	this->child_order_changed_listener_locked = false;
			} else {
				g_warning(_("Reference for node %p (key %s) not found in tracker; order change will not be made."), child, id.c_str());
			}
		} else {
			g_warning(_("Parent node for child node %p (key %s) not found in tracker; order change will not be made."), child, id.c_str());
		}
	} else {
		g_warning(_("Node identified by key %s not found in tracker; order change will not be made."), id.c_str());
	}
}

// receivedNewObjectHelper is the only message helper routine that may
// digest multiple messages, since it may have to set various attributes on a
// XML::Node before adding it to the document.  Therefore, it will "consume"
// more of the message buffer than just one message.  The return value from
// this helper is how many additional characters the helper parser routine 
// has consumed, which is passed back to the message parser loop so it can avoid
// unnecessary processing.
unsigned int
SessionManager::receivedNewObjectHelper(Glib::ustring* msg, Glib::ustring* rest)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receivedNewObjectHelper");

	std::string parent, child, ref;
	Glib::ustring name, type;

	// Flag used to determine if we should actually add the received node,
	// or just process its child objects and attributes.  Normally
	// this should be true, but it must be false in the case of "special"
	// nodes, described below.
	bool add_received_node = true;


	Inkscape::XML::Node* root = sp_document_repr_root(this->_myDoc);

	struct Node buf;
	unsigned int consumed = 0;

	// Required parameters -- return zero (characters consumed)
	// if we don't find these
	buf.tag = MESSAGE_PARENT;
	if (!MessageUtilities::findTag(&buf, msg)) {
		return consumed;
	}
	parent = buf.data.data();

	buf.tag = MESSAGE_CHILD;
	if (!MessageUtilities::findTag(&buf, msg)) {
		return consumed;
	}
	child = buf.data.data();

	buf.tag = MESSAGE_NAME;
	if (!MessageUtilities::findTag(&buf, msg)) {
		return consumed;
	} else {
		name = buf.data;
	}

	buf.tag = MESSAGE_NODETYPE;
	if (!MessageUtilities::findTag(&buf, msg)) {
		return consumed;
	} else {
		type = buf.data;
	}

	// Optional parameters
	buf.tag = MESSAGE_REF;
	if (MessageUtilities::findTag(&buf, msg)) {
		ref = buf.data.data();
	}
	
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Adding object %s", name.data());

//	std::string const* objkey = new std::string(child);

	// Check if the received node is a special node.
	// If it is, and we already have it, then we only should set
	// attributes of the special node, and add child nodes
	// of that special node.  We should _not_ add the received node itself.
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Checking if %s is a special node...", name.data());
	if (this->_myTracker->isSpecialNode(name)) {
//		g_log(NULL, G_LOG_LEVEL_DEBUG, "%s is a special node", name.data());
		if (this->_myTracker->isTracking(this->_myTracker->getSpecialNodeKeyFromName(name))) {
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "%s is already being tracked as %s", name.data(), this->_myTracker->getSpecialNodeKeyFromName(name).data());
			add_received_node = false;
		} else {
			// TODO: keep going as usual, except reset the key to what it should be
		}
	}

	XML::Node* childRepr = NULL;

	if (add_received_node) {
		switch (NodeUtilities::stringToNodeType(type)) {
			case XML::TEXT_NODE:
				buf.tag = MESSAGE_CONTENT;
				if (!MessageUtilities::findTag(&buf, msg)) {
					childRepr = sp_repr_new_text("");
				} else {
					childRepr = sp_repr_new_text(buf.data.data()); // a little awkward notation here...
				}
				break;
			case XML::DOCUMENT_NODE:
				// TODO
			case XML::COMMENT_NODE:
				// also TODO
			case XML::ELEMENT_NODE: 
			default:
				childRepr = sp_repr_new(name.data());
				break;
		}
			
		if (childRepr != NULL) {
			this->_myTracker->put(child, *childRepr);
		}
//		this->_myTracker->dump();
	} else {
		childRepr = &(this->_myTracker->get(this->_myTracker->getSpecialNodeKeyFromName(name)));
	}

//	g_log(NULL, G_LOG_LEVEL_DEBUG, "Adding observer to node %p", childRepr);
	if (this->_myObserver) {
		childRepr->addObserver(*(dynamic_cast< XML::NodeObserver* >(this->_myObserver)));
	}

	while(MessageUtilities::getFirstMessageTag(&buf, rest) != false) {
		if (buf.tag == MESSAGE_CHANGE) {
			this->applyChangesToNode(childRepr, &(buf.data));
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "Applying changes to %s", name.data());
			consumed += buf.next_pos;
			rest->erase(0, buf.next_pos);
		} else {
			break;
		}
	}

	if (add_received_node) {
		if (root == NULL) {
			this->_myDoc->rroot->appendChild(childRepr);
		} else {
			XML::Node* parentRepr = NULL;
			XML::Node* refRepr = NULL;
		
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "Looking up parent node identified by %s", parent.c_str());
			if (this->_myTracker->isTracking(parent)) {
				parentRepr = &(this->_myTracker->get(parent));
			}

//			g_log(NULL, G_LOG_LEVEL_DEBUG, "parentRepr lookup result: %s -> %p", parent.c_str(), parentRepr);

			if (!ref.empty()) {
				refRepr = &(this->_myTracker->get(ref));
				if (refRepr != NULL) {
//					g_log(NULL, G_LOG_LEVEL_DEBUG, "refRepr=%p refRepr->parent()=%p", refRepr, refRepr->parent());
				}
			}

			if (parentRepr != NULL) {

				this->_myTracker->lock(parentRepr, CHILD_ADDED);
				this->_myTracker->lock(childRepr, CHILD_ADDED);
				this->_myTracker->lock(parentRepr, ATTR_CHANGED);
				this->_myTracker->lock(childRepr, ATTR_CHANGED);

//				this->child_added_listener_locked = true;
//				this->attr_changed_listener_locked = true; 

				if (refRepr != NULL) {
//					g_log(NULL, G_LOG_LEVEL_DEBUG, "parentRepr=%p refRepr=%p refRepr->parent()=%p", parentRepr, refRepr, refRepr->parent());
				}

				parentRepr->addChild(childRepr, refRepr);

				Inkscape::GC::release(childRepr);

				this->_myTracker->unlock(parentRepr, CHILD_ADDED);
				this->_myTracker->unlock(childRepr, CHILD_ADDED);
				this->_myTracker->unlock(parentRepr, ATTR_CHANGED);
				this->_myTracker->unlock(childRepr, ATTR_CHANGED);

//				this->child_added_listener_locked = false;
//				this->attr_changed_listener_locked = false;

			} else {
				this->node_tracker()->dump();
				g_error(_("parentRepr identified by %s was NOT found in local tracker; unable to maintain referential integrity.  Aborting!\nPLEASE report this situation as an Inkboard bug and attach a copy of your document to your bug report if possible."), parent.c_str());
			}
		}
	}

	return consumed;
}

void
SessionManager::receivedDeleteHelper(Glib::ustring* msg)
{
//	g_log(NULL, G_LOG_LEVEL_DEBUG, "receivedDeleteHelper");
	std::string parent, child, ref;

	struct Node buf;

	buf.tag = MESSAGE_PARENT;
	if (!MessageUtilities::findTag(&buf, msg)) {
		return;
	}
	parent = buf.data.data();

	buf.tag = MESSAGE_CHILD;
	if (!MessageUtilities::findTag(&buf, msg)) {
		return;
	}
	child = buf.data.data();

	buf.tag = MESSAGE_REF;
	if (!MessageUtilities::findTag(&buf, msg)) {

	} else {
		ref = buf.data.data();
	}

	if (this->_myTracker->isTracking(parent) && this->_myTracker->isTracking(child)) {
		XML::Node& parentRepr = this->_myTracker->get(parent);
		XML::Node& childRepr = this->_myTracker->get(child);
	
		this->_myTracker->lock(parentRepr, CHILD_REMOVED);
		this->_myTracker->lock(childRepr, CHILD_REMOVED);

//		this->child_removed_listener_locked = true;

		parentRepr.removeChild(&childRepr);
		NodeUtilities::recursiveRemoveFromTracker(childRepr, *this->_myObserver, this->_myTracker);
	
		this->_myTracker->unlock(parentRepr, CHILD_REMOVED);

		// we don't need to unlock the child, because it's been removed

//		this->child_removed_listener_locked = false;
	}
}

void
SessionManager::applyChangesToNode(Inkscape::XML::Node* node, Glib::ustring* msg)
{
	std::string id;
	Glib::ustring key, oldval, newval, boolean;

	if (node) {
		if (MessageUtilities::extractChangeData(msg, id, &key, &oldval, &newval, &boolean)) {
			this->_myTracker->lock(node, ATTR_CHANGED);
//			this->attr_changed_listener_locked = true;
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "Setting %s on node %p to %s", key.data(), node, newval.data());

			node->setAttribute(key.data(), newval.data(), FALSE);

//			this->attr_changed_listener_locked = false;
			this->_myTracker->unlock(node, ATTR_CHANGED);
		} else {
//			g_log(NULL, G_LOG_LEVEL_DEBUG, "Failed to extract change data");
		}
	} else {
	}

	return;

}

void
SessionManager::startLog(Glib::ustring filename)
{
	try {
		this->_mySessionFile = new SessionFile(filename, false, false);
	} catch (Glib::FileError e) {
		g_warning(_("Caught I/O error %s while attemping to open file %s for session recording."), e.what().c_str(), filename.c_str());
		throw;
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
