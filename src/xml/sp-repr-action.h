#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_H

#include <glib/gtypes.h>
#include <glib/gquark.h>

#include <iterator>
#include "xml/xml-forward.h"
#include "util/shared-c-string.h"
#include "util/forward-pointer-iterator.h"
#include "gc-managed.h"

class SPReprAction
: public Inkscape::GC::Managed<Inkscape::GC::SCANNED, Inkscape::GC::MANUAL>
{
public:
	SPReprAction *next;
	int serial;
	SPRepr *repr;

	struct IteratorStrategy {
		static SPReprAction const *next(SPReprAction const *action) {
			return action->next;
		}
	};

	typedef Inkscape::Util::ForwardPointerIterator<SPReprAction, IteratorStrategy> Iterator;
	typedef Inkscape::Util::ForwardPointerIterator<SPReprAction const, IteratorStrategy> ConstIterator;

	SPReprAction *optimizeOne() { return _optimizeOne(); }
	void undoOne() const { return _undoOne(); }
	void replayOne() const { return _replayOne(); }

protected:
	SPReprAction(SPRepr *r, SPReprAction *n)
	: next(n), serial(_next_serial++), repr(r) {}

	virtual SPReprAction *_optimizeOne()=0;
	virtual void _undoOne() const=0;
	virtual void _replayOne() const=0;

private:
	static int _next_serial;
};

class SPReprActionAdd : public SPReprAction {
public:
	SPReprActionAdd(SPRepr *repr,
			SPRepr *c, SPRepr *rr,
			SPReprAction *next)
	: SPReprAction(repr, next), child(c), ref(rr) {}

	SPRepr *child;
	SPRepr *ref;

private:

	SPReprAction *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class SPReprActionDel : public SPReprAction {
public:
	SPReprActionDel(SPRepr *repr,
			SPRepr *c, SPRepr *rr,
			SPReprAction *next)
	: SPReprAction(repr, next), child(c), ref(rr) {}

	SPRepr *child;
	SPRepr *ref;

private:
	SPReprAction *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class SPReprActionChgAttr : public SPReprAction {
public:
	SPReprActionChgAttr(SPRepr *repr, GQuark k,
			    Inkscape::Util::SharedCString ov,
                            Inkscape::Util::SharedCString nv,
                            SPReprAction *next)
	: SPReprAction(repr, next), key(k),
	  oldval(ov), newval(nv) {}

	GQuark key;
	Inkscape::Util::SharedCString oldval;
	Inkscape::Util::SharedCString newval;

private:
	SPReprAction *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class SPReprActionChgContent : public SPReprAction {
public:
	SPReprActionChgContent(SPRepr *repr,
                               Inkscape::Util::SharedCString ov,
                               Inkscape::Util::SharedCString nv,
                               SPReprAction *next)
	: SPReprAction(repr, next), oldval(ov), newval(nv) {}

	Inkscape::Util::SharedCString oldval;
	Inkscape::Util::SharedCString newval;

private:
	SPReprAction *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

class SPReprActionChgOrder : public SPReprAction {
public:
	SPReprActionChgOrder(SPRepr *repr, SPRepr *c,
			     SPRepr *orr, SPRepr *nrr,
			     SPReprAction *next)
	: SPReprAction(repr, next), child(c),
	  oldref(orr), newref(nrr) {}

	SPRepr *child;
	SPRepr *oldref, *newref;

private:
	SPReprAction *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

#endif
