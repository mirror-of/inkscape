#define __SP_SELECTION_C__

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

#include <gtk/gtkmain.h>
#include "macros.h"
#include "helper/sp-marshal.h"
#include "helper/sp-intl.h"
#include "inkscape-private.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "sp-item.h"
#include "selection.h"
#include "tools-switch.h"
#include "sp-item-group.h"
#include <algorithm>

#define SP_SELECTION_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE + 1)

SPSelection::SPSelection(SPDesktop *desktop)
: _reprs(NULL), _items(NULL), _desktop(desktop), _flags(0), _idle(0), _refcount(1)
{
}

SPSelection::~SPSelection() {
	_clear();
	_desktop = NULL;
	if (_idle) {
		gtk_idle_remove(_idle);
		_idle = 0;
	}
}

void
SPSelection::_release_item(SPItem *item, SPSelection *selection)
{
	selection->removeItem(item);
}

/* Handler for selected objects "modified" signal */

void
SPSelection::_schedule_modified(SPItem *item, guint flags, SPSelection *selection)
{
	if (!selection->_idle) {
		/* Request handling to be run in _idle loop */
		selection->_idle = gtk_idle_add_priority (SP_SELECTION_UPDATE_PRIORITY, (GtkFunction)&SPSelection::_emit_modified, selection);
	}

	/* Collect all flags */
	selection->_flags |= flags;
}

gboolean
SPSelection::_emit_modified(SPSelection *selection)
{
	/* force new handler to be created if requested before we return */
	selection->_idle = 0;
	guint flags = selection->_flags;
	selection->_flags = 0;

	selection->_modified_signal.emit(selection, flags);

	/* Request "selection_modified" signal on Inkscape::Application */
	inkscape_selection_modified (selection, flags);

	/* drop this handler */
	return FALSE;
}

void SPSelection::_clear()
{
	g_slist_free(_reprs);
	_reprs = NULL;

	while (_items) {
		SPItem *item=SP_ITEM(_items->data);
		sp_signal_disconnect_by_data(item, this);
		_items = g_slist_remove(_items, item);
	}
}

bool SPSelection::includesItem(SPItem *item) const {
	g_return_val_if_fail (item != NULL, FALSE);

	if (!SP_IS_ITEM(item)) {
		return FALSE;
	} else {
		return ( g_slist_find(_items, item) != NULL );
	}
}

bool SPSelection::includesRepr(SPRepr *repr) const {
	g_return_val_if_fail (repr != NULL, FALSE);

	for (GSList *l = _items; l != NULL; l = l->next) {
		if (((SPObject *)l->data)->repr == repr) return TRUE;
	}

	return FALSE;
}

void SPSelection::addItem(SPItem *item) {
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_ITEM (item));
	g_return_if_fail (!includesItem(item));

	g_slist_free (_reprs);
	_reprs = NULL;

	_items = g_slist_prepend (_items, item);
	g_signal_connect (G_OBJECT (item), "release",
			  G_CALLBACK(&SPSelection::_release_item), this);
	g_signal_connect (G_OBJECT (item), "modified",
			  G_CALLBACK(&SPSelection::_schedule_modified), this);

	// unselect any of the item's children which may be selected
	// (to prevent double-selection)
	_removeItemChildren(item);

	_changed_signal.emit(this);
}

void SPSelection::addRepr(SPRepr *repr) {
	addItem(_itemForRepr(repr));
}

void SPSelection::setItem(SPItem *item) {
	_clear();
	addItem(item);
}

void SPSelection::setRepr(SPRepr *repr) {
	setItem(_itemForRepr(repr));
}

void SPSelection::removeItem(SPItem *item) {
	g_return_if_fail (item != NULL);
	g_return_if_fail (SP_IS_ITEM (item));
	g_return_if_fail (includesItem(item));

	g_slist_free(_reprs);
	_reprs = NULL;

 	sp_signal_disconnect_by_data (item, this);
	_items = g_slist_remove (_items, item);

	_changed_signal.emit(this);
}

void SPSelection::removeRepr(SPRepr *repr) {
	removeItem(_itemForRepr(repr));
}

void SPSelection::setItemList(GSList const *list) {
	_clear();

	if (list != NULL) {
		for (GSList const *l = list; l != NULL; l = l->next) {
			SPItem *i = (SPItem *) l->data;
			if (!SP_IS_ITEM (i)) break;
			_items = g_slist_prepend (_items, i);
			g_signal_connect (G_OBJECT (i), "release",
					  G_CALLBACK (&SPSelection::_release_item), this);
			g_signal_connect (G_OBJECT (i), "modified",
					  G_CALLBACK (&SPSelection::_schedule_modified), this);
		}
	}

	_changed_signal.emit(this);
}

void SPSelection::setReprList(GSList const *list) {
	GSList *itemlist=NULL;

	for (GSList const *l = list; l != NULL; l = l->next) {
		SPItem *item=_itemForRepr((SPRepr *)l->data);
		if (item) {
			itemlist = g_slist_prepend(itemlist, item);
		}
	}

	itemlist = g_slist_reverse(itemlist);

	setItemList(itemlist);

	g_slist_free (itemlist);
}

void SPSelection::clear() {
	_clear();
	_changed_signal.emit(this);
}

GSList const *SPSelection::itemList() {
	return _items;
}

GSList const *SPSelection::reprList() {
	if (_reprs) { return _reprs; }

	for (GSList *l = _items; l != NULL; l = l->next) {
		SPItem *i = (SPItem *) l->data;
		_reprs = g_slist_prepend(_reprs, SP_OBJECT_REPR(SP_OBJECT(i)));
	}

	return _reprs;
}

SPItem *SPSelection::singleItem() {
	if ( _items == NULL || _items->next != NULL ) return NULL;
	return SP_ITEM(_items->data);
}

SPRepr *SPSelection::singleRepr() {
	SPItem *item=this->singleItem();
	return item ? SP_OBJECT_REPR(item) : NULL;
}

NRRect *SPSelection::bounds(NRRect *bbox) const {
	g_return_val_if_fail (bbox != NULL, NULL);

	if (isEmpty()) {
		bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
		return bbox;
	}

	bbox->x0 = bbox->y0 = 1e18;
	bbox->x1 = bbox->y1 = -1e18;

	for (GSList *l = _items; l != NULL; l = l-> next) {
		SPItem *item = SP_ITEM (l->data);
		NRRect b;
		sp_item_bbox_desktop (item, &b);
		if (b.x0 < bbox->x0) bbox->x0 = b.x0;
		if (b.y0 < bbox->y0) bbox->y0 = b.y0;
		if (b.x1 > bbox->x1) bbox->x1 = b.x1;
		if (b.y1 > bbox->y1) bbox->y1 = b.y1;
	}

	return bbox;
}

NR::Rect SPSelection::bounds() const {
	NRRect r;
	return NR::Rect(*bounds(&r));
}

NRRect *SPSelection::boundsInDocument(NRRect *bbox) const {
	g_return_val_if_fail (bbox != NULL, NULL);

	if (isEmpty()) {
		bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
		return bbox;
	}

	bbox->x0 = bbox->y0 = 1e18;
	bbox->x1 = bbox->y1 = -1e18;

	for (GSList *l = _items; l != NULL; l = l-> next) {
		NRMatrix i2doc;
		sp_item_i2doc_affine(SP_ITEM(l->data), &i2doc);
		sp_item_invoke_bbox(SP_ITEM(l->data), bbox, &i2doc, FALSE);
	}

	return bbox;
}

NR::Rect SPSelection::boundsInDocument() const {
	NRRect r;
	return NR::Rect(*boundsInDocument(&r));
}

/**
 * Compute the list of points in the selection that are to be considered for snapping.
 */
int SPSelection::getSnapPoints(NR::Point points[], int size) const {
	if (!_items) { // no items
		return 0;
	} else if (!_items->next) { // one item
		/* selection has only one item -> take snappoints of item */
		return sp_item_snappoints (SP_ITEM (_items->data), points, size);
	} else { // multiple items
		/* selection has more than one item -> take corners of selection */
		/* Just a pair of opposite corners of the bounding box suffices given that we don't
		   yet support angled guide lines. */
		NR::Rect bbox = bounds();
		int pos = 0;
		if ( pos < size ) {
			points[pos++] = bbox.min();
		}
		if ( pos < size ) {
			points[pos++] = bbox.max();
		}
		return pos;
	}
}

void SPSelection::_removeItemChildren(SPItem *item) {
	SPObject *object=SP_OBJECT(item);
	GSList *iter, *next;
	for ( iter = _items ; iter ; iter = next ) {
		next = iter->next;
		SPItem *sel_item=(SPItem *)iter->data;
		SPObject *parent=SP_OBJECT_PARENT(SP_OBJECT(sel_item));
		while (parent) {
			if ( parent == object ) {
				removeItem(item);
				break;
			}
			parent = SP_OBJECT_PARENT(parent);
		}
	}
}

SPItem *SPSelection::_itemForRepr(SPRepr *repr) {
	g_return_val_if_fail(repr != NULL, NULL);
	gchar const *id = sp_repr_attr(repr, "id");
	g_return_val_if_fail(id != NULL, NULL);
	SPObject *object = sp_document_lookup_id(SP_DT_DOCUMENT(_desktop), id);
	g_return_val_if_fail(object != NULL, NULL);
	g_return_val_if_fail(SP_IS_ITEM(object), NULL);
	return SP_ITEM(object);
}

