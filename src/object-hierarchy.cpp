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

#include <glib/gmessages.h>
#include "sp-object.h"
#include "object-hierarchy.h"

namespace Inkscape {

ObjectHierarchy::ObjectHierarchy(SPObject *top) {
    if (top) {
        _addBottom(top, top);
    }
}

ObjectHierarchy::~ObjectHierarchy() {
    _clear();
}

void ObjectHierarchy::setTop(SPObject *object) {
    g_return_if_fail(object != NULL);

    if ( object == top() ) {
        return;
    }

    if (!top()) {
        _addBottom(object, object);
    } else if (object->isAncestorOf(top())) {
        _addTop(object, SP_OBJECT_PARENT(top()));
    } else if (object->isAncestorOf(bottom())) {
        _trimAbove(object);
    } else {
        _clear();
        _addBottom(object, object);
    }
}

void ObjectHierarchy::_addTop(SPObject *senior, SPObject *junior) {
    g_assert(junior != NULL);
    g_assert(senior != NULL);

    SPObject *object=junior;
    while ( object != senior ) {
        g_assert(object != NULL);
        _hierarchy.push_back(_attach(object));
        object = SP_OBJECT_PARENT(object);
    }
}

void ObjectHierarchy::_trimAbove(SPObject *limit) {
    while ( !_hierarchy.empty() && _hierarchy.back().object != limit ) {
        _detach(_hierarchy.back());
        _hierarchy.pop_back();
    }
}

void ObjectHierarchy::setBottom(SPObject *object) {
    g_return_if_fail(object != NULL);

    if ( object == bottom() ) {
        return;
    }

    if (!top()) {
        _addBottom(object, object);
    } else if (bottom()->isAncestorOf(object)) {
        _addBottom(bottom(), object);
    } else if (top()->isAncestorOf(object)) {
        _trimBelow(object);
    } else {
        _clear();
        _addBottom(object, object);
    }
}

void ObjectHierarchy::_trimBelow(SPObject *limit) {
    while ( !_hierarchy.empty() && _hierarchy.front().object != limit ) {
        _detach(_hierarchy.front());
        _hierarchy.pop_front();
    }
}

void ObjectHierarchy::_addBottom(SPObject *senior, SPObject *junior) {
    g_assert(junior != NULL);
    g_assert(senior != NULL);

    if ( junior != senior ) {
        _addBottom(senior, SP_OBJECT_PARENT(junior));
    }

    _hierarchy.push_front(_attach(junior));
}

void ObjectHierarchy::_trim_for_release(SPObject *object, ObjectHierarchy *hier)
{
    hier->_trimBelow(object);
    g_assert(!hier->_hierarchy.empty());
    g_assert(hier->_hierarchy.front().object == object);
    hier->_detach(hier->_hierarchy.front());
    hier->_hierarchy.pop_front();
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
