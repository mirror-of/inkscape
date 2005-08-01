/**
 * Whiteboard session manager
 * Definitions
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_DEFINES_H__
#define __WHITEBOARD_DEFINES_H__

#include "jabber_whiteboard/message-tags.h"
#include "jabber_whiteboard/internal-constants.h"

namespace Inkscape {

namespace Whiteboard {

// message types
// explicitly numbered to aid protocol description later on
enum MessageType {
    // image and internal data
    CHANGE_NOT_REPEATABLE = 0,
	CHANGE_REPEATABLE = 1,
	DUMMY_CHANGE = 2,
	DOCUMENT_BEGIN = 3,
	DOCUMENT_END = 4,
    // 1-1 connections
    CONNECT_REQUEST_USER = 5,
	CONNECT_REQUEST_RESPONSE_USER = 6, 
    // chat connections
    CONNECT_REQUEST_RESPONSE_CHAT = 7,
	// chatroom document synchronization
	CHATROOM_SYNCHRONIZE_REQUEST = 8, 
	CHATROOM_SYNCHRONIZE_RESPONSE = 9,
    // requests
    DOCUMENT_SENDER_REQUEST = 10, 
	DOCUMENT_SENDER_REQUEST_RESPONSE = 11, 
	DOCUMENT_REQUEST = 12,
    // notifications
    CONNECTED_SIGNAL = 13,
	DISCONNECTED_FROM_USER_SIGNAL = 14,
	// error responses
	CONNECT_REQUEST_REFUSED_BY_PEER = 15, 
	UNSUPPORTED_PROTOCOL_VERSION = 16,
	ALREADY_IN_SESSION = 17,
	
	// error cases, i.e. garbled messages or bad clients.  These should
	// never actually be transmitted
	UNKNOWN = 18
};

// Responses to whiteboard invitations
enum InvitationResponses {
	ACCEPT_INVITATION,
	ACCEPT_INVITATION_IN_NEW_WINDOW,
	DECLINE_INVITATION,
	PEER_ALREADY_IN_SESSION
};

// Message handler modes
enum HandlerMode {
	DEFAULT,
	PRESENCE,
	ERROR
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
