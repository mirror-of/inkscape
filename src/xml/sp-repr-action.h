#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_H

#include <glib/gtypes.h>
#include <glib/gquark.h>

#include "xml/xml-forward.h"
#include "util/shared-c-string.h"
#include "gc-managed.h"

class SPReprAction
: public Inkscape::GC::Managed<Inkscape::GC::SCANNED, Inkscape::GC::MANUAL>
{
public:
	SPReprAction *next;
	int serial;
	SPRepr *repr;

	SPReprAction *optimizeOne() { return _optimizeOne(); }
	void undoOne() const { return _undoOne(); }
	void replayOne() const { return _replayOne(); }

protected:
	SPReprAction(SPRepr *r, SPReprAction *n=NULL)
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
			SPReprAction *next=NULL)
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
			SPReprAction *next=NULL)
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
                            SPReprAction *next=NULL)
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
                               SPReprAction *next=NULL)
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
			     SPReprAction *next=NULL)
	: SPReprAction(repr, next), child(c),
	  oldref(orr), newref(nrr) {}

	SPRepr *child;
	SPRepr *oldref, *newref;

private:
	SPReprAction *_optimizeOne();
	void _undoOne() const;
	void _replayOne() const;
};

namespace Inkscape {

namespace Traits {

template <typename T> struct List;

template <>
struct List<SPReprAction *> {
	typedef SPReprAction *ListType;
	typedef SPReprAction *Data;

	static bool is_null(SPReprAction *as) { return as == NULL; }
	static ListType null() { return NULL; }

	static SPReprAction *first(SPReprAction *as) { return as; }
	static SPReprAction *rest(SPReprAction *as) { return as->next; }
};

template <>
struct List<SPReprAction const *> {
	typedef SPReprAction const *ListType;
	typedef SPReprAction const *Data;

	static bool is_null(SPReprAction const *as) { return as == NULL; }
	static ListType null() { return NULL; }

	static SPReprAction const *first(SPReprAction const *as) { return as; }
	static SPReprAction const *rest(SPReprAction const *as) { return as->next; }
};

}

}

#endif
