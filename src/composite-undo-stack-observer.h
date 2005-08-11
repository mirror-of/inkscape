/**
 * Aggregates undo stack observers for management and triggering in SPDocument
 *
 * Heavily inspired by Inkscape::XML::CompositeNodeObserver.
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __COMPOSITE_UNDO_COMMIT_OBSERVER_H__
#define __COMPOSITE_UNDO_COMMIT_OBSERVER_H__

#include "undo-stack-observer.h"

#include <list>

namespace Inkscape {

namespace XML {

class Event;

}

class UndoStackObserver;

class CompositeUndoStackObserver {
public:

	// Tracks observers
	struct UndoStackObserverRecord {
	public:
		UndoStackObserverRecord(UndoStackObserver& o) : to_remove(false), _observer(o) { }
		bool to_remove;

		bool operator==(UndoStackObserverRecord const& _x) const
		{
			return &(this->_observer) == &(_x._observer);
		}

		void issueRedo(XML::Event* log)
		{
			this->_observer.notifyRedoEvent(log);
		}

		void issueUndo(XML::Event* log)
		{
			this->_observer.notifyUndoEvent(log);
		}

		void issueUndoCommit(XML::Event* log)
		{
			this->_observer.notifyUndoCommitEvent(log);
		}

	private:
		UndoStackObserver& _observer;
	};

	typedef std::list< UndoStackObserverRecord > UndoObserverRecordList;

	CompositeUndoStackObserver();
	~CompositeUndoStackObserver();

	void add(UndoStackObserver& observer);
	void remove(UndoStackObserver& observer);

	void notifyUndoEvent(XML::Event* log);
	void notifyRedoEvent(XML::Event* log);
	void notifyUndoCommitEvent(XML::Event* log);

private:
	// Remove an observer from a given list
	bool _remove_one(UndoObserverRecordList& list, UndoStackObserver& rec);

	// Mark an observer for removal from a given list
	bool _mark_one(UndoObserverRecordList& list, UndoStackObserver& rec);

	// Keep track of whether or not we are notifying observers
	unsigned int _iterating;

	// Observers in the active list
	UndoObserverRecordList _active;

	// Observers to be added
	UndoObserverRecordList _pending;

	// Prevents the observer vector from modifications during 
	// iteration through the vector
	void _lock() { this->_iterating++; }
	void _unlock();
};

}

#endif
