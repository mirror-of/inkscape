/*
 * Inkscape::ObjectHierarchy - tracks a hierarchy of active SPObjects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>
#include <glib/gmessages.h>
#include "sp-object.h"
#include "object-hierarchy.h"

namespace Inkscape {

ObjectHierarchy::ObjectHierarchy(SPObject *top) {
    if (top) {
        _addBottom(top);
    }
}

ObjectHierarchy::~ObjectHierarchy() {
    _clear();
}

bool ObjectHierarchy::contains(SPObject *object) {
    std::list<Record>::iterator iter;
    for ( iter = _hierarchy.begin() ; iter != _hierarchy.end() ; ++iter ) {
        if ( (*iter).object == object ) {
            return true;
        }
    }
    return false;
}

void ObjectHierarchy::clear() {
    _clear();
    _changed_signal.emit(NULL, NULL);
}

void ObjectHierarchy::setTop(SPObject *object) {
    g_return_if_fail(object != NULL);

    if ( top() == object ) {
        return;
    }

    if (!top()) {
        _addTop(object);
    } else if (object->isAncestorOf(top())) {
        _addTop(object, top());
    } else if ( object == bottom() || object->isAncestorOf(bottom()) ) {
        _trimAbove(object);
    } else {
        _clear();
        _addTop(object);
    }

    _changed_signal.emit(top(), bottom());
}

void ObjectHierarchy::_addTop(SPObject *senior, SPObject *junior) {
    g_assert(junior != NULL);
    g_assert(senior != NULL);

    SPObject *object=SP_OBJECT_PARENT(junior);
    do {
        _addTop(object);
        object = SP_OBJECT_PARENT(object);
    } while ( object != senior );
}

void ObjectHierarchy::_addTop(SPObject *object) {
    g_assert(object != NULL);
    _hierarchy.push_back(_attach(object));
    _added_signal.emit(object);
}

void ObjectHierarchy::_trimAbove(SPObject *limit) {
    while ( !_hierarchy.empty() && _hierarchy.back().object != limit ) {
        SPObject *object=_hierarchy.back().object;

        sp_object_ref(object, NULL);
        _detach(_hierarchy.back());
        _hierarchy.pop_back();
        _removed_signal.emit(object);
        sp_object_unref(object, NULL);
    }
}

void ObjectHierarchy::setBottom(SPObject *object) {
    g_return_if_fail(object != NULL);

    if ( bottom() == object ) {
        return;
    }

    if (!top()) {
        _addBottom(object);
    } else if (bottom()->isAncestorOf(object)) {
        _addBottom(bottom(), object);
    } else if ( top() == object ) {
        _trimBelow(top());
    } else if (top()->isAncestorOf(object)) {
        if (object->isAncestorOf(bottom())) {
            _trimBelow(object);
        } else { // object is a sibling or cousin of bottom()
            SPObject *saved_top=top();
            sp_object_ref(saved_top, NULL);
            _clear();
            _addBottom(saved_top);
            _addBottom(saved_top, object);
            sp_object_unref(saved_top, NULL);
        }
    } else {
        _clear();
        _addBottom(object);
    }

    _changed_signal.emit(top(), bottom());
}

void ObjectHierarchy::_trimBelow(SPObject *limit) {
    while ( !_hierarchy.empty() && _hierarchy.front().object != limit ) {
        SPObject *object=_hierarchy.front().object;
        sp_object_ref(object, NULL);
        _detach(_hierarchy.front());
        _hierarchy.pop_front();
        _removed_signal.emit(object);
        sp_object_unref(object, NULL);
    }
}

void ObjectHierarchy::_addBottom(SPObject *senior, SPObject *junior) {
    g_assert(junior != NULL);
    g_assert(senior != NULL);

    if ( junior != senior ) {
        _addBottom(senior, SP_OBJECT_PARENT(junior));
        _addBottom(junior);
    }
}

void ObjectHierarchy::_addBottom(SPObject *object) {
    g_assert(object != NULL);
    _hierarchy.push_front(_attach(object));
    _added_signal.emit(object);
}

void ObjectHierarchy::_trim_for_release(SPObject *object, ObjectHierarchy *hier)
{
    hier->_trimBelow(object);
    g_assert(!hier->_hierarchy.empty());
    g_assert(hier->_hierarchy.front().object == object);

    sp_object_ref(object, NULL);
    hier->_detach(hier->_hierarchy.front());
    hier->_hierarchy.pop_front();
    hier->_removed_signal.emit(object);
    sp_object_unref(object, NULL);

    hier->_changed_signal.emit(hier->top(), hier->bottom());
}

ObjectHierarchy::Record ObjectHierarchy::_attach(SPObject *object) {
    sp_object_ref(object, NULL);
    gulong id = g_signal_connect(G_OBJECT(object), "release", GCallback(&ObjectHierarchy::_trim_for_release), this);
    return Record(object, id);
}

void ObjectHierarchy::_detach(ObjectHierarchy::Record const &rec) {
    g_signal_handler_disconnect(G_OBJECT(rec.object), rec.handler_id);
    sp_object_unref(rec.object, NULL);
}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
