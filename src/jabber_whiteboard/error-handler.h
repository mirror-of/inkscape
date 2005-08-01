/**
 * Whiteboard session manager
 * Error handler
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_ERROR_HANDLER_H__
#define __WHITEBOARD_ERROR_HANDLER_H__

#include "jabber_whiteboard/typedefs.h"

// TODO: finish this
namespace Inkscape {

namespace Whiteboard {
class SessionManager;

bool error_handlers_initialized = false;
ErrorHandlerFunctorMap _error_handlers;

class ErrorHandler {
public:
	ErrorHandler(SessionManager* sm);
	~ErrorHandler();
	
	ErrorHandlerFunctor handleError(char const* errcode);
	ErrorHandlerFunctor handleError(unsigned int errcode);

private:

	// noncopyable, nonassignable
	ErrorHandler(ErrorHandler const&);
	ErrorHandler& operator=(ErrorHandler const&);
	
	SessionManager* _sm;
};

}

}

#endif
