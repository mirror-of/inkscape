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

/**
 * @brief The set of selected SPItems for a given desktop.
 *
 * This class represents the set of selected SPItems for a given
 * SPDesktop.
 *
 * An SPItem and its parent cannot be simultaneously selected;
 * selecting an SPItem has the side-effect of unselecting any of its
 * children which might have been selected.
 *
 */
struct SPSelection {
public:
	/**
	 * Constructs an selection object, bound to a particular
	 * SPDesktop
	 *
	 * @param desktop the desktop in question
	 */
	SPSelection(SPDesktop *desktop);
	~SPSelection();

	/**
	 * @brief Returns the desktop the seoection is bound to
	 *
	 * @return the desktop the selection is bound to
	 */
	SPDesktop *desktop() { return _desktop; }

	/**
	 * Increment the selection's reference count
	 *
	 * @return the selection's own self
	 */
	SPSelection *reference() {
		++_refcount;
		return this;
	}
	/**
	 * Increment the selection's reference count
	 *
	 * @return the selection's own self
	 */
	SPSelection const *reference() const {
		++_refcount;
		return this;
	}
	/**
	 * Decrement the selection's reference count
	 */
	void unreference() const {
		if (!--_refcount) {
			delete const_cast<SPSelection *>(this);
		}
	}

	/**
	 * @brief Add an SPItem to the set of selected items.
	 *
	 * @param item the item to add
	 */
	void addItem(SPItem *item);
	/**
	 * @brief Add an SPRepr to the set of selected items
	 *
	 * @param the xml node of the item to add
	 */
	void addRepr(SPRepr *repr);
	/**
	 * @brief Clear the existing set of items and select a new one
	 *
	 * @param item the item to select
	 */
	void setItem(SPItem *item);
	/**
	 * @brief Clear the existing set of items and select a new one
	 *
	 * @param repr the xml node of the item to select
	 */
	void setRepr(SPRepr *repr);
	/**
	 * @brief Removes an item from the set of selected items
	 *
	 * @param item the item to unselect
	 */
	void removeItem(SPItem *item);
	/**
	 * @brief Removes an item from the set of selected items
	 *
	 * @param repr the xml node of the item to remove
	 */
	void removeRepr(SPRepr *repr);
	/**
	 * @brief Clears the selection and selects the specified items
	 *
	 * @param items a list of items to select
	 */
	void setItemList(GSList const *items);
	/**
	 * @brief Clears the selection and selects the specified items
	 *
	 * @param repr a list of xml nodes for the items to select
	 */
	void setReprList(GSList const *reprs);
	/**
	 * @brief Unselects all selected items.
	 */
	void clear();

	/**
	 * @brief Returns true if no items are selected
	 */
	bool isEmpty() const { return _items == NULL; }
	/**
	 * @brief Returns true if the given item is selected
	 */
	bool includesItem(SPItem *item) const;
	/**
	 * @brief Returns true if the given item is selected
	 */
	bool includesRepr(SPRepr *repr) const;

	/**
	 * @brief Returns a single selected item
	 *
	 * @return NULL unless exactly one item is selected
	 */
	SPItem *singleItem();
	/**
	 * @brief Returns a single selected item's xml node
	 *
	 * @return NULL unless exactly one item is selected
	 */
	SPRepr *singleRepr();
	/** @brief Returns the list of selected items */
	GSList const *itemList();
	/** @brief Returns a list of the xml nodes of all selected items */
	GSList const *reprList();

	/** @brief Returns the bounding rectangle of the selection */
	NRRect *bounds(NRRect *dest) const;
	/** @brief Returns the bounding rectangle of the selection */
	NR::Rect bounds() const;

	/**
	 * @brief Returns the bounding rectangle of the selection
	 *
	 * TODO: how is this different from bounds()?
	 */ 
	NRRect *boundsInDocument(NRRect *dest) const;
	/**
	 * @brief Returns the bounding rectangle of the selection
	 *
	 * TODO: how is this different from bounds()?
	 */
	NR::Rect boundsInDocument() const;

	/**
	 * @brief Gets the selection's snap points.
	 *
	 * This method populates the given NR::Point array with the
	 * selection's snap points.
	 *
	 * @param points the array to populate
	 * @param max_points the size of the array
	 *
	 * @return the number of snap points placed in the array
	 */
	int getSnapPoints(NR::Point points[], int max_points) const;

	/**
	 * @brief Connects a slot to be notified of selection changes
	 *
	 * This method connects the given slot such that it will
	 * be called upon any change in the set of selected items.
	 *
	 * @param slot the slot to connect
	 *
	 * @return the resulting connection
	 */
	SigC::Connection connectChanged(SigC::Slot1<void, SPSelection *> slot) {
		return _changed_signal.connect(slot);
	}
	/**
	 * @brief Connects a slot to be notified of selected 
	 *        object modifications 
	 *
	 * This method connects the given slot such that it will
	 * receive notifications whenever any selected item is
	 * modified.
	 *
	 * @param slot the slot to connect
	 *
	 * @return the resulting connection
	 *
	 */
	SigC::Connection connectModified(SigC::Slot2<void, SPSelection *, guint> slot) {
		return _modified_signal.connect(slot);
	}

private:
	/** @brief no copy */
	SPSelection(SPSelection const &);
	/** @brief no assign */
	void operator=(SPSelection const &);

	/** @brief Issues modification notification signals */
	static gboolean _emit_modified(SPSelection *selection);
	/** @brief Schedules an item modification signal to be sent */
	static void _schedule_modified(SPItem *item, guint flags, SPSelection *selection);
	/** @brief Releases a selected item that is being removed */
	static void _release_item(SPItem *item, SPSelection *selection);

	/** @brief unselect all children of the given item */
	void _removeItemChildren(SPItem *item);
	/** @brief clears the selection (without issuing a notification) */
	void _clear();
	/** @brief returns the SPItem corresponding to an xml node (if any) */
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


/* these wrappers are all obsolete and should be replaced with their
 * contents at their call sites */

inline __attribute__ ((deprecated))
gboolean sp_selection_item_selected(SPSelection const *selection,
		                    SPItem *item)
{
	return selection->includesItem(item);
}
inline __attribute__ ((deprecated))
gboolean sp_selection_repr_selected(SPSelection const *selection,
                                    SPRepr *repr)
{
	return selection->includesRepr(repr);
}

inline __attribute__ ((deprecated))
gboolean sp_selection_is_empty(SPSelection const *selection)
{
	return selection->isEmpty();
}

inline __attribute__ ((deprecated))
void sp_selection_set_item(SPSelection *selection, SPItem *item)
{
	selection->setItem(item);
}
inline __attribute__ ((deprecated))
void sp_selection_set_repr(SPSelection *selection, SPRepr *repr)
{
	selection->setRepr(repr);
}
inline __attribute__ ((deprecated))
void sp_selection_set_item_list(SPSelection *selection, GSList const *list)
{
	selection->setItemList(list);
}
inline __attribute__ ((deprecated))
void sp_selection_set_repr_list(SPSelection *selection, GSList const *list)
{
	selection->setReprList(list);
}
inline __attribute__ ((deprecated))
void sp_selection_empty(SPSelection *selection)
{
	selection->clear();
}
inline __attribute__ ((deprecated))
GSList const *sp_selection_item_list (SPSelection *selection)
{
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->itemList();
}
inline __attribute__ ((deprecated))
GSList const *sp_selection_repr_list (SPSelection *selection)
{
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->reprList();
}
inline __attribute__ ((deprecated))
SPItem *sp_selection_item (SPSelection *selection)
{
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->singleItem();
}
inline __attribute__ ((deprecated))
SPRepr *sp_selection_repr (SPSelection *selection)
{
	g_return_val_if_fail(selection != NULL, NULL);
	return selection->singleRepr();
}

inline __attribute__ ((deprecated))
NRRect *sp_selection_bbox (SPSelection *selection, NRRect *bbox)
{
	return selection->bounds(bbox);
}
inline __attribute__ ((deprecated))
NR::Rect sp_selection_bbox (SPSelection *selection)
{
	return selection->bounds();
}
inline __attribute__ ((deprecated))
NRRect *sp_selection_bbox_document (SPSelection *selection,
                                           NRRect *bbox)
{
	return selection->boundsInDocument(bbox);
}
inline __attribute__ ((deprecated))
NR::Rect sp_selection_bbox_document (SPSelection *selection)
{
	return selection->boundsInDocument();
}

inline __attribute__ ((deprecated))
int sp_selection_snappoints(SPSelection *selection,
                            NR::Point points[], int size)
{
	return selection->getSnapPoints(points, size);
}


#endif /* !__SP_SELECTION_H__ */
