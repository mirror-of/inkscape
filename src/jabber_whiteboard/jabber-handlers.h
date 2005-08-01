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

#ifndef __WHITEBOARD_LOUDMOUTH_CALLBACKS__
#define __WHITEBOARD_LOUDMOUTH_CALLBACKS__

extern "C" {
#include <loudmouth/loudmouth.h>
}

#include <glib.h>

namespace Inkscape {

namespace Whiteboard {

LmHandlerResult presence_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data);
LmHandlerResult default_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data);
LmHandlerResult stream_error_handler(LmMessageHandler* handler, LmConnection* connection, LmMessage* message, gpointer user_data);

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
