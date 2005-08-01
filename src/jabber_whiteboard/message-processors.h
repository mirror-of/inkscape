/**
 * Whiteboard session manager
 * Jabber received message processors
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_PROCESSORS_H__
#define __WHITEBOARD_MESSAGE_PROCESSORS_H__

#include "jabber_whiteboard/typedefs.h"

#include "gc-managed.h"

namespace Inkscape {

namespace Whiteboard {

class SessionManager;

// Processor forward declarations
struct ChangeHandler;
struct DocumentSignalHandler;
struct ConnectRequestHandler;
struct ConnectErrorHandler;
struct ChatSynchronizeHandler;

struct JabberMessage {
public:
	JabberMessage()
	{

	}

	~JabberMessage() 
	{

	}

	// pointer to original Loudmouth message
	LmMessage* message;
	
	// sequence number
	unsigned int sequence;
	
	// sender JID
	std::string sender;

	// message body
	Glib::ustring body;

private:
	// noncopyable, nonassignable (for now, anyway...)
//	JabberMessage(JabberMessage const&);
//	JabberMessage& operator=(JabberMessage const&);
};

struct MessageProcessor : public GC::Managed<>, public GC::Finalized {
public:
	virtual ~MessageProcessor() 
	{
		g_log(NULL, G_LOG_LEVEL_DEBUG, "~MessageProcessor()");
	}

	virtual LmHandlerResult operator()(MessageType mode, JabberMessage& m) = 0;

	MessageProcessor(SessionManager* sm) : _sm(sm) { }
protected:
	SessionManager *_sm;

private:
	// noncopyable, nonassignable
	MessageProcessor(MessageProcessor const&);
	MessageProcessor& operator=(MessageProcessor const&);
};

/** 
 * boost::bind does not work for function objects with virtual
 * methods.  This is a class with no virtual methods that wraps
 * a MessageProcessor for binding.
 */
struct ProcessorShell : public GC::Managed<> {
public:
	ProcessorShell(MessageProcessor* mpm) : _mpm(mpm) { }

	LmHandlerResult operator()(MessageType type, JabberMessage& msg)
	{
		return (*this->_mpm)(type, msg);
	}
private:
	MessageProcessor* _mpm;
};

void initialize_received_message_processors(SessionManager* sm, MessageProcessorMap& mpm);

void destroy_received_message_processors(MessageProcessorMap& mpm);

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
