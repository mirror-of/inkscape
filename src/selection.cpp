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

#include <glib/gmain.h>
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
#include "inkscape.h"
#include <algorithm>

#define SP_SELECTION_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE + 1)

SPSelection::SPSelection(SPDesktop *desktop)
: _objs(NULL), _reprs(NULL), _items(NULL), _desktop(desktop),
  _flags(0), _idle(0), _refcount(1)
{
}

SPSelection::~SPSelection() {
	_clear();
	_desktop = NULL;
	if (_idle) {
		g_source_remove(_idle);
		_idle = 0;
	}
}

void
SPSelection::_release(SPObject *obj, SPSelection *selection)
{
	selection->remove(obj);
}

/* Handler for selected objects "modified" signal */

void
SPSelection::_schedule_modified(SPObject *obj, guint flags, SPSelection *selection)
{
	if (!selection->_idle) {
		/* Request handling to be run in _idle loop */
		selection->_idle = g_idle_add_full(SP_SELECTION_UPDATE_PRIORITY, GSourceFunc(&SPSelection::_emit_modified), selection, NULL);
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

        selection->_emitModified(flags);

	/* drop this handler */
	return FALSE;
}

void SPSelection::_emitModified(guint flags) {
    inkscape_selection_modified(this, flags);
    _modified_signal.emit(this, flags);
}

void SPSelection::_emitChanged() {
    inkscape_selection_changed(this);
    _changed_signal.emit(this);
}

void SPSelection::_invalidateCachedLists() {
    g_slist_free(_items);
    _items = NULL;

    g_slist_free(_reprs);
    _reprs = NULL;
}

void SPSelection::_clear()
{
    _invalidateCachedLists();
    while (_objs) {
        SPObject *obj=reinterpret_cast<SPObject *>(_objs->data);
        sp_signal_disconnect_by_data(obj, this);
        _objs = g_slist_remove(_objs, obj);
    }
}

bool SPSelection::includes(SPObject *obj) const {
    g_return_val_if_fail (obj != NULL, FALSE);
    g_return_val_if_fail(SP_IS_OBJECT(obj), FALSE);

    return ( g_slist_find(_objs, obj) != NULL );
}

void SPSelection::add(SPObject *obj) {
    g_return_if_fail(obj != NULL);
    g_return_if_fail(SP_IS_OBJECT(obj));
    g_return_if_fail(!includes(obj));

    _invalidateCachedLists();
    _add(obj);
    _emitChanged();
}

void SPSelection::_add(SPObject *obj) {
    // unselect any of the item's children which may be selected
    // (to prevent double-selection)
    // TODO: what if we are adding the child of an already selected object?
    _removeObjectChildren(obj);

    _objs = g_slist_prepend(_objs, obj);
    g_signal_connect(G_OBJECT(obj), "release",
                     G_CALLBACK(&SPSelection::_release), this);
    g_signal_connect(G_OBJECT(obj), "modified",
                     G_CALLBACK(&SPSelection::_schedule_modified), this);
}

void SPSelection::set(SPObject *object) {
    _clear();
    add(object);
}

void SPSelection::remove(SPObject *obj) {
    g_return_if_fail(obj != NULL);
    g_return_if_fail(SP_IS_OBJECT(obj));
    g_return_if_fail(includes(obj));

    _invalidateCachedLists();
    _remove(obj);
    _emitChanged();
}

void SPSelection::_remove(SPObject *obj) {
    sp_signal_disconnect_by_data(obj, this);
    _objs = g_slist_remove(_objs, obj);
}

void SPSelection::setList(GSList const *list) {
    _clear();

    if ( list != NULL ) {
        for ( GSList const *iter = list ; iter != NULL ; iter = iter->next ) {
            _add(reinterpret_cast<SPObject *>(iter->data));
        }
    }

    _emitChanged();
}

void SPSelection::setReprList(GSList const *list) {
    _clear();

    for ( GSList const *iter = list ; iter != NULL ; iter = iter->next ) {
        SPObject *obj=_objectForRepr(reinterpret_cast<SPRepr *>(iter->data));
        if (obj) {
            _add(obj);
        }
    }

    _emitChanged();
}

void SPSelection::clear() {
    _clear();
    _emitChanged();
}

GSList const *SPSelection::list() {
    return _objs;
}

GSList const *SPSelection::itemList() {
    if (_items) {
        return _items;
    }

    for ( GSList *iter=_objs ; iter != NULL ; iter = iter->next ) {
        SPObject *obj = reinterpret_cast<SPObject *>(iter->data);
        if (SP_IS_ITEM(obj)) {
            _items = g_slist_prepend(_items, SP_ITEM(obj));
        }
    }
    _items = g_slist_reverse(_items);

    return _items;
}

GSList const *SPSelection::reprList() {
    if (_reprs) { return _reprs; }

    for ( GSList *iter = _items ; iter != NULL ; iter = iter->next ) {
        SPObject *obj = reinterpret_cast<SPObject *>(iter->data);
        _reprs = g_slist_prepend(_reprs, SP_OBJECT_REPR(obj));
    }
    _reprs = g_slist_reverse(_reprs);

    return _reprs;
}

SPObject *SPSelection::single() {
    if ( _objs != NULL && _objs->next == NULL ) {
        return reinterpret_cast<SPObject *>(_objs->data);
    } else {
        return NULL;
    }
}

SPItem *SPSelection::singleItem() {
    GSList const *items=itemList();
    if ( items != NULL && items->next == NULL ) {
        return reinterpret_cast<SPItem *>(items->data);
    } else {
        return NULL;
    }
}

SPRepr *SPSelection::singleRepr() {
    SPObject *obj=single();
    return obj ? SP_OBJECT_REPR(obj) : NULL;
}

NRRect *SPSelection::bounds(NRRect *bbox) const {
    g_return_val_if_fail (bbox != NULL, NULL);

    GSList const *items=const_cast<SPSelection *>(this)->itemList();
    if (!items) {
        bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
        return bbox;
    }

    bbox->x0 = bbox->y0 = 1e18;
    bbox->x1 = bbox->y1 = -1e18;

    for ( GSList const *iter=items ; iter != NULL ; iter = iter->next ) {
        SPItem *item=SP_ITEM(iter->data);
        NRRect b;
        sp_item_bbox_desktop(item, &b);
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

    GSList const *items=const_cast<SPSelection *>(this)->itemList();
    if (!items) {
        bbox->x0 = bbox->y0 = bbox->x1 = bbox->y1 = 0.0;
        return bbox;
    }

    bbox->x0 = bbox->y0 = 1e18;
    bbox->x1 = bbox->y1 = -1e18;

    for ( GSList const *iter=items ; iter != NULL ; iter = iter->next ) {
        SPItem *item=SP_ITEM(iter->data);
        NR::Matrix const i2doc(sp_item_i2doc_affine(item));
        sp_item_invoke_bbox(item, bbox, i2doc, FALSE);
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
std::vector<NR::Point> SPSelection::getSnapPoints() const {
    GSList const *items = const_cast<SPSelection *>(this)->itemList();
    std::vector<NR::Point> p;
    if (!items->next) { // one item
        /* selection has only one item -> take snappoints of item */
        p = sp_item_snappoints(const_cast<SPSelection *>(this)->singleItem());
    } else { // multiple items
        /* selection has more than one item -> take corners of selection */
        /* Just a pair of opposite corners of the bounding box suffices given that we don't
           yet support angled guide lines. */
        NR::Rect bbox = bounds();
	p.push_back(bbox.min());
	p.push_back(bbox.max());
    }

    return p;
}

void SPSelection::_removeObjectChildren(SPObject *obj) {
    GSList *iter, *next;
    for ( iter = _objs ; iter ; iter = next ) {
        next = iter->next;
        SPObject *sel_obj=reinterpret_cast<SPObject *>(iter->data);
        SPObject *parent=SP_OBJECT_PARENT(sel_obj);
        while (parent) {
            if ( parent == obj ) {
                _remove(sel_obj);
                break;
            }
            parent = SP_OBJECT_PARENT(parent);
        }
    }
}

SPObject *SPSelection::_objectForRepr(SPRepr *repr) const {
    g_return_val_if_fail(repr != NULL, NULL);
    gchar const *id = sp_repr_attr(repr, "id");
    g_return_val_if_fail(id != NULL, NULL);
    SPObject *object=SP_DT_DOCUMENT(_desktop)->getObjectById(id);
    g_return_val_if_fail(object != NULL, NULL);
    return object;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  c-basic-offset:8
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
