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

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

#include <glibmm.h>
#include <glibmm/i18n.h>

#include "jabber_whiteboard/error-handler-setup.h"
#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/session-manager.h"
#include "jabber_whiteboard/error-handler.h"

namespace Inkscape {

namespace Whiteboard {

ErrorHandler::ErrorHandler(SessionManager* sm) : _sm(sm)
{
	if (error_handlers_initialized == false) {
		initialize_error_handlers(sm, _error_handlers);
		error_handlers_initialized = true;
	}
}

ErrorHandler::~ErrorHandler()
{

}

ErrorHandlerFunctor
ErrorHandler::handleError(char const* errcode)
{
	try {
		return this->handleError(boost::lexical_cast< unsigned int >(errcode));
	} catch (boost::bad_lexical_cast&) {
		return UnknownErrorHandler();
	}
}

ErrorHandlerFunctor
ErrorHandler::handleError(unsigned int errcode)
{
	ErrorHandlerFunctorMap::iterator i;
	i = _error_handlers.find(errcode);
	if (i != _error_handlers.end()) {
		return (*i).second;
	} else {
		return UnknownErrorHandler();
	}
}

}

}
