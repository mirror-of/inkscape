/*
 * SimpleNode - simple XML node implementation
 *
 * Copyright 2003-2005 MenTaLguY <mental@rydia.net>
 * Copyright 2003 Nathan Hurst
 * Copyright 1999-2003 Lauris Kaplinski
 * Copyright 2000-2002 Ximian Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#include <glib/gquark.h>
#include <glib/gmessages.h>
#include <cstring>
#include "algorithms/find-if-before.h"
#include "xml/simple-node.h"
#include "xml/node-event-vector.h"
#include "xml/node-fns-tree.h"
#include "xml/repr.h"

namespace Inkscape {

namespace XML {

using Inkscape::Util::SharedCStringPtr;
using Inkscape::Util::List;
using Inkscape::Util::MutableList;
using Inkscape::Util::cons;
using Inkscape::Util::rest;
using Inkscape::Util::set_rest;

SimpleNode::SimpleNode(int code)
: Node(), _name(code), _attributes(), _child_count(0),
  _cached_positions_valid(false)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = NULL;
    this->_first_child = this->_last_child = NULL;
}

SimpleNode::SimpleNode(SimpleNode const &node)
: Node(),
  _cached_position(node._cached_position),
  _name(node._name), _attributes(), _content(node._content),
  _child_count(node._child_count),
  _cached_positions_valid(node._cached_positions_valid)
{
    _logger = NULL;
    _document = NULL;
    _parent = _next = NULL;
    _first_child = _last_child = NULL;

    for ( Node *child = node._first_child ;
          child != NULL ; child = child->next() )
    {
        Node *child_copy=child->duplicate();

        child_copy->_setParent(this);
        if (_last_child) {
            _last_child->_setNext(child_copy);
        } else {
            _first_child = child_copy;
        }
        _last_child = child_copy;

        child_copy->release(); // release to avoid a leak
    }

    for ( List<AttributeRecord const> iter = node._attributes ;
          iter ; ++iter )
    {
        _attributes = cons(*iter, _attributes);
    }
}

gchar const *SimpleNode::name() const {
    return g_quark_to_string(_name);
}

gchar const *SimpleNode::content() const {
    return this->_content;
}

gchar const *SimpleNode::attribute(gchar const *name) const {
    g_return_val_if_fail(name != NULL, NULL);

    GQuark const key = g_quark_from_string(name);

    for ( List<AttributeRecord const> iter = _attributes ;
          iter ; ++iter )
    {
        if ( iter->key == key ) {
            return iter->value;
        }
    }

    return NULL;
}

unsigned SimpleNode::position() const {
    g_return_val_if_fail(_parent != NULL, 0);
    return _parent->_childPosition(*this);
}

unsigned SimpleNode::_childPosition(Node const &child) const {
    if (!_cached_positions_valid) {
        unsigned position=0;
        for ( Node *sibling = _first_child ;
              sibling ; sibling = sibling->next() )
        {
            sibling->_setCachedPosition(position);
            position++;
        }
        _cached_positions_valid = true;
    }
    return child._cachedPosition();
}

Node *SimpleNode::nthChild(unsigned index) {
    Node *child = _first_child;
    for ( ; index > 0 && child ; child = child->next() ) {
        index--;
    }
    return child;
}

bool SimpleNode::matchAttributeName(gchar const *partial_name) const {
    g_return_val_if_fail(partial_name != NULL, false);

    for ( List<AttributeRecord const> iter = _attributes ;
          iter ; ++iter )
    {
        gchar const *name = g_quark_to_string(iter->key);
        if (std::strstr(name, partial_name)) {
            return true;
        }
    }

    return false;
}

void SimpleNode::setContent(gchar const *new_content) {
    SharedCStringPtr old_content=_content;
    _content = ( new_content ? SharedCStringPtr::copy(new_content) : SharedCStringPtr() );

    if ( _content != old_content ) {
        if (_logger) {
            _logger->notifyContentChanged(*this, old_content, _content);
        }

        for ( Listeners::iterator iter = _listeners.begin() ; iter ; ++iter ) {
            if (iter->vector.content_changed) {
                iter->vector.content_changed(this, old_content, _content, iter->data);
            }
        }
    }
}

void
SimpleNode::setAttribute(gchar const *name, gchar const *value, bool const is_interactive)
{
    g_return_if_fail(name && *name);

    GQuark const key = g_quark_from_string(name);

    MutableList<AttributeRecord> ref;
    MutableList<AttributeRecord> existing;
    for ( existing = _attributes ; existing ; ++existing ) {
        if ( existing->key == key ) {
            break;
        }
        ref = existing;
    }

    SharedCStringPtr old_value=( existing ? existing->value : SharedCStringPtr() );

    SharedCStringPtr new_value=SharedCStringPtr();
    if (value) {
        new_value = SharedCStringPtr::copy(value);
        if (!existing) {
            _attributes = cons(AttributeRecord(key, new_value), _attributes);
        } else {
            existing->value = new_value;
        }
    } else {
        if (existing) {
            if (ref) {
                set_rest(ref, rest(existing));
            } else {
                _attributes = rest(existing);
            }
            set_rest(existing, MutableList<AttributeRecord>());
        }
    }

    if ( new_value != old_value ) {
        if (_logger) {
            _logger->notifyAttributeChanged(*this, key, old_value, new_value);
        }

        for ( Listeners::iterator iter = _listeners.begin() ; iter ; ++iter ) {
            if (iter->vector.attr_changed) {
                iter->vector.attr_changed(this, name, old_value, new_value, is_interactive, iter->data);
            }
        }
    }
}

void SimpleNode::addChild(Node *child, Node *ref) {
    g_assert(child);
    g_assert(!ref || ref->parent() == this);
    g_assert(!child->parent());

    Node *next;
    if (ref) {
        next = ref->next();
        ref->_setNext(child);
    } else {
        next = _first_child;
        _first_child = child;
    }
    if (!next) { // appending?
        _last_child = child;
        // set cached position if possible when appending
        if (!ref) {
            // if !next && !ref, child is sole child
            child->_setCachedPosition(0);
            _cached_positions_valid = true;
        } else if (_cached_positions_valid) {
            child->_setCachedPosition(ref->_cachedPosition() + 1);
        }
    } else {
        // invalidate cached positions otherwise
        _cached_positions_valid = false;
    }

    child->_setParent(this);
    child->_setNext(next);
    _child_count++;

    if (_document) {
        child->_bindDocument(*_document);
    }
    if (_logger) {
        child->_bindLogger(*_logger);
        _logger->notifyChildAdded(*this, *child, ref);
    }

    for ( Listeners::iterator iter = _listeners.begin() ; iter ; ++iter ) {
        if (iter->vector.child_added) {
            iter->vector.child_added(this, child, ref, iter->data);
        }
    }
}

void SimpleNode::_bindDocument(Document &document) {
    g_assert(!_document || _document == &document);

    if (!_document) {
        _document = &document;

        for ( Node *child = _first_child ; child != NULL ; child = child->next() ) {
            child->_bindDocument(document);
        }
    }
}

void SimpleNode::_bindLogger(TransactionLogger &logger) {
    g_assert(!_logger || _logger == &logger);

    if (!_logger) {
        _logger = &logger;

        for ( Node *child = _first_child ; child != NULL ; child = child->next() ) {
            child->_bindLogger(logger);
        }
    }
}

void SimpleNode::removeChild(Node *child) {
    g_assert(child);
    g_assert(child->parent() == this);

    Node *ref = ( child != _first_child ? sp_repr_prev(child) : NULL );

    Node *next = child->next();
    if (ref) {
        ref->_setNext(next);
    } else {
        _first_child = next;
    }
    if (!next) { // removing the last child?
        _last_child = ref;
    } else {
        // removing any other child invalidates the cached positions
        _cached_positions_valid = false;
    }

    child->_setNext(NULL);
    child->_setParent(NULL);
    _child_count--;

    if (_logger) {
        _logger->notifyChildRemoved(*this, *child, ref);
    }

    for ( Listeners::iterator iter = _listeners.begin() ; iter ; ++iter ) {
        if (iter->vector.child_removed) {
            iter->vector.child_removed(this, child, ref, iter->data);
        }
    }
}

void SimpleNode::changeOrder(Node *child, Node *ref) {
    g_return_if_fail(child);
    g_return_if_fail(child->parent() == this);
    g_return_if_fail(child != ref);
    g_return_if_fail(!ref || ref->parent() == this);

    Node *const prev = sp_repr_prev(child);

    if (prev == ref) { return; }

    Node *next;

    /* Remove from old position. */
    next=child->next();
    if (prev) {
        prev->_setNext(next);
    } else {
        _first_child = next;
    }
    if (!next) {
        _last_child = prev;
    }

    /* Insert at new position. */
    if (ref) {
        next = ref->next();
        ref->_setNext(child);
    } else {
        next = _first_child;
        _first_child = child;
    }
    child->_setNext(next);
    if (!next) {
        _last_child = child;
    }

    _cached_positions_valid = false;

    if (_logger) {
        _logger->notifyChildOrderChanged(*this, *child, prev, ref);
    }

    for ( Listeners::iterator iter = _listeners.begin() ; iter ; ++iter ) {
        if (iter->vector.order_changed) {
            iter->vector.order_changed(this, child, prev, ref, iter->data);
        }
    }
}

void SimpleNode::setPosition(int pos) {
    g_return_if_fail(_parent != NULL);

    // a position beyond the end of the list means the end of the list;
    // a negative position is the same as an infinitely large position

    Node *ref=NULL;
    for ( Node *sibling = _parent->firstChild() ;
          sibling && pos ; sibling = sibling->next() )
    {
        if ( sibling != this ) {
            ref = sibling;
            pos--;
        }
    }

    _parent->changeOrder(this, ref);
}

void SimpleNode::synthesizeEvents(NodeEventVector const *vector, void *data) {
    if (vector->attr_changed) {
        for ( List<AttributeRecord const> iter = _attributes ;
              iter ; ++iter )
        {
            vector->attr_changed(this, g_quark_to_string(iter->key), NULL, iter->value, false, data);
        }
    }
    if (vector->child_added) {
        Node *ref = NULL;
        for ( Node *child = this->_first_child ;
              child ; child = child->next() )
        {
            vector->child_added(this, child, ref, data);
            ref = child;
        }
    }
    if (vector->content_changed) {
        vector->content_changed(this, NULL, this->_content, data);
    }
}

void SimpleNode::addListener(NodeEventVector const *vector, void *data) {
    g_assert(vector);
    _listeners.push_back(Listener(*vector, data));
}

namespace {

struct listener_data_matches {
    listener_data_matches(void *d) : data(d) {}
    bool operator()(SimpleNode::Listener const &listener) {
        return listener.data == data;
    }
    void * const data;
};

}

void SimpleNode::removeListenerByData(void *data) {
    using Inkscape::Algorithms::find_if_before;

    if (_listeners.empty()) {
        return;
    }

    if ( _listeners.front().data == data ) {
        _listeners.pop_front();
    } else {
        Listeners::iterator pos=find_if_before(_listeners.begin(), _listeners.end(), listener_data_matches(data));
        if (pos) {
            _listeners.erase_after(pos);
        }
    }
}

Node *SimpleNode::root() {
    Node *parent=this;
    while (parent->parent()) {
        parent = parent->parent();
    }

    if ( parent->type() == DOCUMENT_NODE ) {
        for ( Node *child = _document->firstChild() ;
              child ; child = child->next() )
        {
            if ( child->type() == ELEMENT_NODE ) {
                return child;
            }
        }
        return NULL;
    } else if ( parent->type() == ELEMENT_NODE ) {
        return parent;
    } else {
        return NULL;
    }
}

void SimpleNode::mergeFrom(Node const *src, gchar const *key) {
    g_return_if_fail(src != NULL);
    g_return_if_fail(key != NULL);

    setContent(src->content());

    for ( Node const *child = src->firstChild() ; child != NULL ; child = child->next() )
    {
        gchar const *id = child->attribute(key);
        if (id) {
            Node *rch=sp_repr_lookup_child(this, key, id);
            if (rch) {
                rch->mergeFrom(child, key);
            } else {
                rch = child->duplicate();
                appendChild(rch);
                rch->release();
            }
        } else {
            Node *rch=child->duplicate();
            appendChild(rch);
            rch->release();
        }
    }

    for ( List<AttributeRecord const> iter = src->attributeList() ;
          iter ; ++iter )
    {
        setAttribute(g_quark_to_string(iter->key), iter->value);
    }
}

}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
