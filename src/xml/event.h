#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_H

#include <glib/gtypes.h>
#include <glib/gquark.h>

#include <iterator>
#include "util/shared-c-string-ptr.h"
#include "util/forward-pointer-iterator.h"
#include "gc-managed.h"
#include "xml/node.h"

namespace Inkscape {
namespace XML {

class Node;

class Event
: public Inkscape::GC::Managed<Inkscape::GC::SCANNED, Inkscape::GC::MANUAL>
{
public:
	Event *next;
	int serial;
	Node *repr;

	struct IteratorStrategy {
		static Event const *next(Event const *action) {
			return action->next;
		}
	};

	typedef Inkscape::Util::ForwardPointerIterator<Event, IteratorStrategy> Iterator;
	typedef Inkscape::Util::ForwardPointerIterator<Event const, IteratorStrategy> ConstIterator;

	Event *optimizeOne() { return _optimizeOne(); }
	void undoOne() const { return _undoOne(); }
	void replayOne() const { return _replayOne(); }

protected:
	Event(Node *r, Event *n)
	: next(n), serial(_next_serial++), repr(r) {}

	virtual Event *_optimizeOne()=0;
	virtual void _undoOne() const=0;
	virtual void _replayOne() const=0;

private:
	static int _next_serial;
};

class EventAdd : public Event {
public:
	EventAdd(Node *repr, Node *c, Node *rr, Event *next)
	: Event(repr, next), child(c), ref(rr) {}

	Node *child;
	Node *ref;

private:

	Event *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class EventDel : public Event {
public:
	EventDel(Node *repr, Node *c, Node *rr, Event *next)
	: Event(repr, next), child(c), ref(rr) {}

	Node *child;
	Node *ref;

private:
	Event *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class EventChgAttr : public Event {
public:
	EventChgAttr(Node *repr, GQuark k,
		     Inkscape::Util::SharedCStringPtr ov,
                     Inkscape::Util::SharedCStringPtr nv,
                     Event *next)
	: Event(repr, next), key(k),
	  oldval(ov), newval(nv) {}

	GQuark key;
	Inkscape::Util::SharedCStringPtr oldval;
	Inkscape::Util::SharedCStringPtr newval;

private:
	Event *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class EventChgContent : public Event {
public:
	EventChgContent(Node *repr,
                        Inkscape::Util::SharedCStringPtr ov,
                        Inkscape::Util::SharedCStringPtr nv,
                        Event *next)
	: Event(repr, next), oldval(ov), newval(nv) {}

	Inkscape::Util::SharedCStringPtr oldval;
	Inkscape::Util::SharedCStringPtr newval;

private:
	Event *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class EventChgOrder : public Event {
public:
	EventChgOrder(Node *repr, Node *c, Node *orr, Node *nrr, Event *next)
	: Event(repr, next), child(c),
	  oldref(orr), newref(nrr) {}

	Node *child;
	Node *oldref, *newref;

private:
	Event *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

}
}

#endif
