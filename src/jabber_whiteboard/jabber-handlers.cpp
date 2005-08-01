/**
 * Whiteboard session manager
 * C-style Loudmouth callbacks
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/jabber-handlers.h"
#include "jabber_whiteboard/message-handler.h"

namespace Inkscape {

namespace Whiteboard {

LmHandlerResult
default_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data)
{
	MessageHandler* mh = reinterpret_cast< MessageHandler* >(user_data);
	return mh->handle(message, DEFAULT);
}


LmHandlerResult
presence_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data)
{
	MessageHandler* mh = reinterpret_cast< MessageHandler* >(user_data);
	return mh->handle(message, PRESENCE);
}


LmHandlerResult
stream_error_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data)
{
	MessageHandler* mh = reinterpret_cast< MessageHandler* >(user_data);
	return mh->handle(message, ERROR);
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
