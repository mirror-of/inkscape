#ifndef __SP_SELECTION_H__
#define __SP_SELECTION_H__

/*
 * Per-desktop selection container
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/sigc++.h>
#include "forward.h"
#include "xml/repr.h"
#include "libnr/nr-rect.h"

struct SPSelection {
public:
	SPSelection(SPDesktop *desktop);
	~SPSelection();

	SPDesktop *desktop() { return _desktop; }

	SPSelection &reference() {
		++_refcount;
		return *this;
	}
	SPSelection const &reference() const {
		++_refcount;
		return *this;
	}
	void unreference() const {
		if (!--_refcount) {
			delete const_cast<SPSelection *>(this);
		}
	}

	void addItem(SPItem *item);
	void addRepr(SPRepr *repr);
	void setItem(SPItem *item);
	void setRepr(SPRepr *repr);
	void removeItem(SPItem *item);
	void removeRepr(SPRepr *repr);
	void setItemList(GSList const *items);
	void setReprList(GSList const *reprs);
	void clear();

	bool isEmpty() const { return _items == NULL; }
	bool includesItem(SPItem *item) const;
	bool includesRepr(SPRepr *repr) const;

	SPItem *singleItem();
	SPRepr *singleRepr();
	GSList const *itemList();
	GSList const *reprList();

	NRRect *bounds(NRRect *dest) const;
	NR::Rect bounds() const;

	NRRect *boundsInDocument(NRRect *dest) const;
	NR::Rect boundsInDocument() const;

	int getSnapPoints(NR::Point points[], int max_points) const;

	void updateStatusbar();
	void invokeChanged();

	static SPSelection *create(SPDesktop *desktop);

	SigC::Connection connectChanged(SigC::Slot1<void, SPSelection *> slot) {
		return _changed_signal.connect(slot);
	}
	SigC::Connection connectModified(SigC::Slot2<void, SPSelection *, guint> slot) {
		return _modified_signal.connect(slot);
	}

private:
	SPSelection(SPSelection const &);
	void operator=(SPSelection const &);

	static gboolean _idle_handler(SPSelection *selection);
	static void _item_modified(SPItem *item, guint flags, SPSelection *selection);
	static void _release_item(SPItem *item, SPSelection *selection);

	void _removeItemChildren(SPItem *item);
	void _clear();
	SPItem *_itemForRepr(SPRepr *repr);

	GSList *_reprs;
	GSList *_items;
	SPDesktop *_desktop;
	guint _flags;
	guint _idle;

	mutable unsigned _refcount;

	SigC::Signal1<void, SPSelection *> _changed_signal;
	SigC::Signal2<void, SPSelection *, guint> _modified_signal;
};

struct SPSelectionClass {
	GObjectClass parent_class;
};

/* Constructor */

inline SPSelection *sp_selection_new(SPDesktop *desktop) {
	return new SPSelection(desktop);
}

/* This are private methods & will be removed from this file */

inline void sp_selection_changed(SPSelection *selection) {
	selection->invokeChanged();
}
inline void sp_selection_update_statusbar(SPSelection *selection) {
	selection->updateStatusbar();
}

inline gboolean sp_selection_item_selected(SPSelection const *selection,
		                           SPItem *item)
{
	return selection->includesItem(item);
}
inline gboolean sp_selection_repr_selected(SPSelection const *selection,
                                           SPRepr *repr)
{
	return selection->includesRepr(repr);
}
inline gboolean sp_selection_is_empty(SPSelection const *selection) {
	return selection->isEmpty();
}

inline void sp_selection_add_item(SPSelection *selection, SPItem *item) {
	selection->addItem(item);
}
inline void sp_selection_add_repr(SPSelection *selection, SPRepr *repr) {
	selection->addRepr(repr);
}
inline void sp_selection_set_item(SPSelection *selection, SPItem *item) {
	selection->setItem(item);
}
inline void sp_selection_set_repr(SPSelection *selection, SPRepr *repr) {
	selection->setRepr(repr);
}
inline void sp_selection_remove_item(SPSelection *selection, SPItem *item) {
	selection->removeItem(item);
}
inline void sp_selection_remove_repr(SPSelection *selection, SPRepr *repr) {
	selection->removeRepr(repr);
}
inline void sp_selection_set_item_list(SPSelection *selection, GSList const *list) {
	selection->setItemList(list);
}
inline void sp_selection_set_repr_list(SPSelection *selection, GSList const *list) {
	selection->setReprList(list);
}
inline void sp_selection_empty(SPSelection *selection) {
	selection->clear();
}
inline GSList const *sp_selection_item_list (SPSelection *selection) {
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->itemList();
}
inline GSList const *sp_selection_repr_list (SPSelection *selection) {
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->reprList();
}
inline SPItem *sp_selection_item (SPSelection *selection) {
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->singleItem();
}
inline SPRepr *sp_selection_repr (SPSelection *selection) {
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->singleRepr();
}

inline NRRect *sp_selection_bbox (SPSelection *selection, NRRect *bbox) {
	return selection->bounds(bbox);
}
inline NR::Rect sp_selection_bbox (SPSelection *selection) {
	return selection->bounds();
}
inline NRRect *sp_selection_bbox_document (SPSelection *selection, NRRect *bbox) {
	return selection->boundsInDocument(bbox);
}
inline NR::Rect sp_selection_bbox_document (SPSelection *selection) {
	return selection->boundsInDocument();
}

/* Returns number of points used */
inline int sp_selection_snappoints(SPSelection *selection,
                                   NR::Point points[], int size)
{
	return selection->getSnapPoints(points, size);
}


#endif /* !__SP_SELECTION_H__ */
