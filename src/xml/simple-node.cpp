/*
 * Inkscape::XML::SimpleNode - simple XML node implementation
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
#include "xml/simple-node.h"
#include "xml/sp-repr-event-vector.h"
#include "xml/repr-get-children.h"
#include "xml/repr.h"

namespace Inkscape {

namespace XML {

using Inkscape::Util::SharedCStringPtr;

SimpleNode::SimpleNode(int code)
: Inkscape::XML::Node(), _name(code), _child_count(0), _cached_positions_valid(false)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = NULL;
    this->_first_child = this->_last_child = NULL;
    this->_attributes = NULL;
    this->_last_listener = this->_listeners = NULL;
}

SimpleNode::SimpleNode(SimpleNode const &node)
: Inkscape::XML::Node(),
  _name(node._name), _content(node._content),
  _child_count(node._child_count),
  _cached_positions_valid(node._cached_positions_valid),
  _cached_position(node._cached_position)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = NULL;
    this->_first_child = this->_last_child = NULL;
    this->_attributes = NULL;
    this->_last_listener = this->_listeners = NULL;

    for ( Inkscape::XML::Node *child = node._first_child ;
          child != NULL ; child = child->next() )
    {
        Inkscape::XML::Node *child_copy=child->duplicate();

        child_copy->_setParent(this);
        if (_last_child) {
            _last_child->_setNext(child_copy);
        } else {
            _first_child = child_copy;
        }
        _last_child = child_copy;

        child_copy->release(); // release to avoid a leak
    }

    Inkscape::XML::AttributeRecord *prev_attr_copy=NULL;
    for ( Inkscape::XML::AttributeRecord *attr = node._attributes ; attr != NULL ; attr = attr->next ) {
        Inkscape::XML::AttributeRecord *attr_copy=new Inkscape::XML::AttributeRecord(*attr);

        if (prev_attr_copy) {
            prev_attr_copy->next = attr_copy;
        } else {
            this->_attributes = attr_copy;
        }

        prev_attr_copy = attr_copy;
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

    for ( Inkscape::XML::AttributeRecord const *attribute = _attributes ;
          attribute ; attribute = attribute->next ) 
    {
        if ( attribute->key == key ) {
            return attribute->value;
        }
    }

    return NULL;
}

unsigned SimpleNode::position() const {
    g_return_val_if_fail(_parent != NULL, 0);
    return _parent->_childPosition(*this);
}

unsigned SimpleNode::_childPosition(Inkscape::XML::Node const &child) const {
    if (!_cached_positions_valid) {
        unsigned position=0;
        for ( Inkscape::XML::Node *sibling = _first_child ;
              sibling ; sibling = sibling->next() )
        {
            sibling->_setCachedPosition(position);
            position++;
        }
        _cached_positions_valid = true;
    }
    return child._cachedPosition();
}

Inkscape::XML::Node *SimpleNode::nthChild(unsigned index) {
    Inkscape::XML::Node *child = _first_child;
    for ( ; index > 0 && child ; child = child->next() ) {
        index--;
    }
    return child;
}

bool SimpleNode::matchAttributeName(gchar const *partial_name) const {
    g_return_val_if_fail(partial_name != NULL, false);

    for ( Inkscape::XML::AttributeRecord const *attribute = _attributes ;
          attribute ; attribute = attribute->next )
    {
        gchar const *name = g_quark_to_string(attribute->key);
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

        for ( Inkscape::XML::NodeListener *listener = _listeners ;
              listener ; listener = listener->next )
        {
            if (listener->vector->content_changed) {
                listener->vector->content_changed(this, old_content, _content, listener->data);
            }
        }
    }
}

void
SimpleNode::setAttribute(gchar const *name, gchar const *value, bool const is_interactive)
{
    g_return_if_fail(name && *name);

    GQuark const key = g_quark_from_string(name);

    Inkscape::XML::AttributeRecord **ref=&_attributes;
    Inkscape::XML::AttributeRecord *attribute;
    for ( attribute = _attributes ;
          attribute ; attribute = attribute->next )
    {
        if ( attribute->key == key ) {
            break;
        }
        ref = &attribute->next;
    }

    SharedCStringPtr old_value=( attribute ? attribute->value : SharedCStringPtr() );

    SharedCStringPtr new_value=SharedCStringPtr();
    if (value) {
        new_value = SharedCStringPtr::copy(value);
        if (!attribute) {
            _attributes = new Inkscape::XML::AttributeRecord(key, new_value, _attributes);
        } else {
            attribute->value = new_value;
        }
    } else {
        if (attribute) {
            // remove attribute
            *ref = attribute->next;
            attribute->next = NULL;
        }
    }

    if ( new_value != old_value ) {
        if (_logger) {
            _logger->notifyAttributeChanged(*this, key, old_value, new_value);
        }

        for ( Inkscape::XML::NodeListener *listener = _listeners ;
              listener ; listener = listener->next )
        {
            if (listener->vector->attr_changed) {
                listener->vector->attr_changed(this, name, old_value, new_value, is_interactive, listener->data);
            }
        }
    }
}

void SimpleNode::addChild(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
    g_assert(child);
    g_assert(!ref || ref->parent() == this);
    g_assert(!child->parent());

    Inkscape::XML::Node *next;
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

    for ( Inkscape::XML::NodeListener *listener = _listeners ;
          listener ; listener = listener->next )
    {
        if (listener->vector->child_added) {
            listener->vector->child_added(this, child, ref, listener->data);
        }
    }
}

void SimpleNode::_bindDocument(Inkscape::XML::Document &document) {
    g_assert(!_document || _document == &document);

    if (!_document) {
        _document = &document;

        for ( Inkscape::XML::Node *child = _first_child ; child != NULL ; child = child->next() ) {
            child->_bindDocument(document);
        }
    }
}

void SimpleNode::_bindLogger(Inkscape::XML::TransactionLogger &logger) {
    g_assert(!_logger || _logger == &logger);

    if (!_logger) {
        _logger = &logger;

        for ( Inkscape::XML::Node *child = _first_child ; child != NULL ; child = child->next() ) {
            child->_bindLogger(logger);
        }
    }
}

void SimpleNode::removeChild(Inkscape::XML::Node *child) {
    g_assert(child);
    g_assert(child->parent() == this);

    Inkscape::XML::Node *ref = ( child != _first_child ? sp_repr_prev(child) : NULL );

    Inkscape::XML::Node *next = child->next();
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

    for ( Inkscape::XML::NodeListener *listener = _listeners ;
          listener ; listener = listener->next )
    {
        if (listener->vector->child_removed) {
            listener->vector->child_removed(this, child, ref, listener->data);
        }
    }
}

void SimpleNode::changeOrder(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
    g_return_if_fail(child);
    g_return_if_fail(child->parent() == this);
    g_return_if_fail(child != ref);
    g_return_if_fail(!ref || ref->parent() == this);

    Inkscape::XML::Node *const prev = sp_repr_prev(child);

    if (prev == ref) { return; }

    Inkscape::XML::Node *next;

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

    for ( Inkscape::XML::NodeListener *listener = _listeners ;
          listener ; listener = listener->next )
    {
        if (listener->vector->order_changed) {
            listener->vector->order_changed(this, child, prev, ref, listener->data);
        }
    }
}

void SimpleNode::setPosition(int pos) {
    g_return_if_fail(_parent != NULL);

    // a position beyond the end of the list means the end of the list;
    // a negative position is the same as an infinitely large position

    Inkscape::XML::Node *ref=NULL;
    for ( Inkscape::XML::Node *sibling = _parent->firstChild() ;
          sibling && pos ; sibling = sibling->next() )
    {
        if ( sibling != this ) {
            ref = sibling;
            pos--;
        }
    }

    _parent->changeOrder(this, ref);
}

void SimpleNode::synthesizeEvents(Inkscape::XML::NodeEventVector const *vector, void *data) {
    if (vector->attr_changed) {
        for (Inkscape::XML::AttributeRecord *attr = this->_attributes ; attr ; attr = attr->next ) {
            vector->attr_changed(this, g_quark_to_string(attr->key), NULL, attr->value, false, data);
        }
    }
    if (vector->child_added) {
        Inkscape::XML::Node *ref = NULL;
        for ( Inkscape::XML::Node *child = this->_first_child ;
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

void SimpleNode::addListener(Inkscape::XML::NodeEventVector const *vector, void *data) {
    g_assert(vector);

    Inkscape::XML::NodeListener *listener = new Inkscape::XML::NodeListener(vector, data);

    if (_last_listener) {
        _last_listener->next = listener;
    } else {
        _listeners = listener;
    }
    _last_listener = listener;
}

void SimpleNode::removeListenerByData(void *data) {
    Inkscape::XML::NodeListener *prev = NULL;
    for (Inkscape::XML::NodeListener *rl = _listeners; rl != NULL; rl = rl->next) {
        if (rl->data == data) {
            if (prev) {
                prev->next = rl->next;
            } else {
                _listeners = rl->next;
            }
            if (!rl->next) {
                _last_listener = prev;
            }
            return;
        }
        prev = rl;
    }
}

Inkscape::XML::Node *SimpleNode::root() {
    Inkscape::XML::Node *parent=this;
    while (parent->parent()) {
        parent = parent->parent();
    }

    if ( parent->type() == Inkscape::XML::DOCUMENT_NODE ) {
        for ( Inkscape::XML::Node *child = _document->firstChild() ;
              child ; child = child->next() )
        {
            if ( child->type() == Inkscape::XML::ELEMENT_NODE ) {
                return child;
            }
        }
        return NULL;
    } else if ( parent->type() == Inkscape::XML::ELEMENT_NODE ) {
        return parent;
    } else {
        return NULL;
    }
}

void SimpleNode::mergeFrom(Inkscape::XML::Node const *src, gchar const *key) {
    g_return_if_fail(src != NULL);
    g_return_if_fail(key != NULL);

    setContent(src->content());

    for ( Inkscape::XML::Node const *child = src->firstChild() ; child != NULL ; child = child->next() )
    {
        gchar const *id = child->attribute(key);
        if (id) {
            Inkscape::XML::Node *rch=sp_repr_lookup_child(this, key, id);
            if (rch) {
                rch->mergeFrom(child, key);
            } else {
                rch = child->duplicate();
                appendChild(rch);
                rch->release();
            }
        } else {
            Inkscape::XML::Node *rch=child->duplicate();
            appendChild(rch);
            rch->release();
        }
    }

    for ( Inkscape::XML::AttributeRecord const *attribute = src->attributeList() ; attribute ; attribute = attribute->next ) {
        setAttribute(g_quark_to_string(attribute->key), attribute->value);
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
