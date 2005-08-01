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

#ifndef __WHITEBOARD_ERROR_HANDLER_SETUP_H__
#define __WHITEBOARD_ERROR_HANDLER_SETUP_H__

// TODO: finish and integrate this
namespace Inkscape {

namespace Whiteboard {

struct BaseErrorHandler {
public:
	BaseErrorHandler(SessionManager* sm);
	virtual ~BaseErrorHandler();

	virtual LmHandlerResult operator()(unsigned int code) = 0;
};

void initialize_error_handlers(SessionManager* sm, ErrorHandlerFunctorMap& ehfm);

}

}

#endif
