/**
 * Whiteboard session manager
 * Message dispatch devices and timeout triggers
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_CALLBACKS_H__
#define __WHITEBOARD_CALLBACKS_H__

#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

class SessionManager;
class SessionData;

class Callbacks {
public:
	Callbacks(SessionManager* sm);
	~Callbacks();

	bool dispatchSendQueue();
	bool dispatchReceiveQueue();

private:
	SessionManager* _sm;
	SessionData* _sd;
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
