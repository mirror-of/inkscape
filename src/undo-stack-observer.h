/**
 * Undo stack observer interface
 *
 * Observes undo, redo, and undo log commit events.
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __UNDO_COMMIT_OBSERVER_H__
#define __UNDO_COMMIT_OBSERVER_H__

namespace Inkscape {

namespace XML {

class Event;

}

class UndoStackObserver {
public:
	UndoStackObserver() { }
	virtual ~UndoStackObserver() { }

	// Triggered when the user issues an undo command.
	virtual void notifyUndoEvent(XML::Event* log) = 0;

	// Triggered when the user issues a redo command.
	virtual void notifyRedoEvent(XML::Event* log) = 0;

	// Triggered when a commit is made to the undo log.
	virtual void notifyUndoCommitEvent(XML::Event* log) = 0;
};

}

#endif
