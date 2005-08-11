/**
 * Aggregates individual serialized XML::Events into larger packages 
 * for more efficient delivery
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_AGGREGATOR_H__
#define __WHITEBOARD_MESSAGE_AGGREGATOR_H__

#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

class MessageAggregator {
public:
	// Maximum size of aggregates in kilobytes; UINT_MAX = no limit.
	// TODO: This should be user-configurable; perhaps an option in Inkscape Preferences...
	static unsigned int const MAX_SIZE = 16384;

	MessageAggregator() { }
	~MessageAggregator() { }

	static MessageAggregator& instance()
	{
		static MessageAggregator singleton;
		return singleton;
	}

	// Aggregates using a user-provided buffer.  Returns true if more messages can be
	// added to the buffer; false otherwise.
	bool addOne(Glib::ustring const& msg, Glib::ustring& buf);

	// Aggregates using the internal buffer.  
	// Note that since this class is designed to be a singleton class, usage of the internal
	// buffer is not thread-safe.  Use the above method if this matters to you
	// (it currently shouldn't matter, but in future...)
	//
	// Also note that usage of the internal buffer means that you will have to manually
	// clear the internal buffer; use reset() for that.
	bool addOne(Glib::ustring const& msg);

	Glib::ustring const& getAggregate()
	{
		return this->_buf;
	}

	Glib::ustring const getAggregateCopy()
	{
		return this->_buf;
	}

	Glib::ustring const detachAggregate()
	{
		Glib::ustring ret = this->_buf;
		this->_buf.clear();
		return ret;
	}

	void reset()
	{
		this->_buf.clear();
	}

private:
	Glib::ustring _buf;
};

}

}

#endif
