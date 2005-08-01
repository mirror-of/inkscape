/**
 * Whiteboard session manager
 * Message tags
 * 
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_TAGS_H__
#define __WHITEBOARD_MESSAGE_TAGS_H__

namespace Inkscape {

namespace Whiteboard {

// TODO: breaking these up into namespaces would be nice, but it's too much typing
// for now
extern char const* MESSAGE_CHANGE;
extern char const* MESSAGE_NEWOBJ;
extern char const* MESSAGE_DELETE;
extern char const* MESSAGE_DOCUMENT;
extern char const* MESSAGE_NODECONTENT;
extern char const* MESSAGE_ORDERCHANGE;
extern char const* MESSAGE_OBJKEY;
extern char const* MESSAGE_ID;
extern char const* MESSAGE_KEY;
extern char const* MESSAGE_OLDVAL;
extern char const* MESSAGE_NEWVAL;
extern char const* MESSAGE_NAME;
extern char const* MESSAGE_ISINTERACTIVE;
extern char const* MESSAGE_DATA;
extern char const* MESSAGE_PARENT;
extern char const* MESSAGE_CHILD;
extern char const* MESSAGE_REF;
extern char const* MESSAGE_CONTENT;
extern char const* MESSAGE_REPEATABLE;
extern char const* MESSAGE_CHATROOM;

extern char const* MESSAGE_TYPE;
extern char const* MESSAGE_NODETYPE;
extern char const* MESSAGE_FROM;
extern char const* MESSAGE_TO;
extern char const* MESSAGE_BODY;
extern char const* MESSAGE_QUEUE;
extern char const* MESSAGE_SEQNUM;
extern char const* MESSAGE_PROTOCOL_VER;

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
