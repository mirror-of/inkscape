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
: SPRepr(), _name(code), _child_count(0), _cached_positions_valid(false)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = this->_children = NULL;
    this->_attributes = NULL;
    this->_last_listener = this->_listeners = NULL;
}

SimpleNode::SimpleNode(SimpleNode const &node)
: SPRepr(),
  _name(node._name), _content(node._content),
  _child_count(node._child_count),
  _cached_positions_valid(node._cached_positions_valid),
  _cached_position(node._cached_position)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = this->_children = NULL;
    this->_attributes = NULL;
    this->_last_listener = this->_listeners = NULL;

    SPRepr *prev_child_copy=NULL;
    for ( SPRepr *child = node._children ; child != NULL ; child = child->next() ) {
        SPRepr *child_copy=child->duplicate();

        child_copy->_setParent(this);
        if (prev_child_copy) {
            prev_child_copy->_setNext(child_copy);
        } else {
            this->_children = child_copy;
        }
        child_copy->release(); // release here to avoid a leak

        prev_child_copy = child_copy;
    }

    SPReprAttr *prev_attr_copy=NULL;
    for ( SPReprAttr *attr = node._attributes ; attr != NULL ; attr = attr->next ) {
        SPReprAttr *attr_copy=new SPReprAttr(*attr);

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

    for ( SPReprAttr const *attribute = _attributes ;
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

unsigned SimpleNode::_childPosition(SPRepr const &child) const {
    if (!_cached_positions_valid) {
        unsigned position=0;
        for ( SPRepr *sibling = _children ;
              sibling ; sibling = sibling->next() )
        {
            sibling->_setCachedPosition(position);
            position++;
        }
        _cached_positions_valid = true;
    }
    return child._cachedPosition();
}

SPRepr *SimpleNode::nthChild(unsigned index) {
    SPRepr *child = _children;
    for ( ; index > 0 && child ; child = child->next() ) {
        index--;
    }
    return child;
}

SPRepr *SimpleNode::lastChild() {
    SPRepr *child = _children;
    if (child) {
        while ( child->next() ) {
            child = child->next();
        }
    }
    return child;
}

bool SimpleNode::matchAttributeName(gchar const *partial_name) const {
    g_return_val_if_fail(partial_name != NULL, false);

    for ( SPReprAttr const *attribute = _attributes ;
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

        for ( SPReprListener *listener = _listeners ;
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

    SPReprAttr **ref=&_attributes;
    SPReprAttr *attribute;
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
            _attributes = new SPReprAttr(key, new_value, _attributes);
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

        for ( SPReprListener *listener = _listeners ;
              listener ; listener = listener->next )
        {
            if (listener->vector->attr_changed) {
                listener->vector->attr_changed(this, name, old_value, new_value, is_interactive, listener->data);
            }
        }
    }
}

void SimpleNode::addChild(SPRepr *child, SPRepr *ref) {
    g_assert(child);
    g_assert(!ref || ref->parent() == this);
    g_assert(!child->parent());

    SPRepr *next;
    if (ref) {
        next = ref->next();
        ref->_setNext(child);
    } else {
        next = _children;
        _children = child;
    }
    if (!next) {
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

    for ( SPReprListener *listener = _listeners ;
          listener ; listener = listener->next )
    {
        if (listener->vector->child_added) {
            listener->vector->child_added(this, child, ref, listener->data);
        }
    }
}

void SimpleNode::_bindDocument(SPReprDoc &document) {
    g_assert(!_document || _document == &document);

    if (!_document) {
        _document = &document;

        for ( SPRepr *child = _children ; child != NULL ; child = child->next() ) {
            child->_bindDocument(document);
        }
    }
}

void SimpleNode::_bindLogger(Inkscape::XML::TransactionLogger &logger) {
    g_assert(!_logger || _logger == &logger);

    if (!_logger) {
        _logger = &logger;

        for ( SPRepr *child = _children ; child != NULL ; child = child->next() ) {
            child->_bindLogger(logger);
        }
    }
}

void SimpleNode::removeChild(SPRepr *child) {
    g_assert(child);
    g_assert(child->parent() == this);

    SPRepr *ref = ( child != _children ? sp_repr_prev(child) : NULL );

    SPRepr *next = child->next();
    if (ref) {
        ref->_setNext(next);
    } else {
        _children = next;
    }
    if (next) {
        // removing any child, save the last, invalidates
        // the cached positions
        _cached_positions_valid = false;
    }

    child->_setNext(NULL);
    child->_setParent(NULL);
    _child_count--;

    if (_logger) {
        _logger->notifyChildRemoved(*this, *child, ref);
    }

    for ( SPReprListener *listener = _listeners ;
          listener ; listener = listener->next )
    {
        if (listener->vector->child_removed) {
            listener->vector->child_removed(this, child, ref, listener->data);
        }
    }
}

void SimpleNode::changeOrder(SPRepr *child, SPRepr *ref) {
    g_return_if_fail(child);
    g_return_if_fail(child->parent() == this);
    g_return_if_fail(child != ref);
    g_return_if_fail(!ref || ref->parent() == this);

    SPRepr *const prev = sp_repr_prev(child);

    if (prev == ref) { return; }

    /* Remove from old position. */
    if (prev) {
        prev->_setNext(child->next());
    } else {
        _children = child->next();
    }
    /* Insert at new position. */
    if (ref) {
        child->_setNext(ref->next());
        ref->_setNext(child);
    } else {
        child->_setNext(_children);
        _children = child;
    }

    _cached_positions_valid = false;

    if (_logger) {
        _logger->notifyChildOrderChanged(*this, *child, prev, ref);
    }

    for ( SPReprListener *listener = _listeners ;
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

    SPRepr *ref=NULL;
    for ( SPRepr *sibling = _parent->firstChild() ;
          sibling && pos ; sibling = sibling->next() )
    {
        if ( sibling != this ) {
            ref = sibling;
            pos--;
        }
    }

    _parent->changeOrder(this, ref);
}

void SimpleNode::synthesizeEvents(SPReprEventVector const *vector, void *data) {
    if (vector->attr_changed) {
        for (SPReprAttr *attr = this->_attributes ; attr ; attr = attr->next ) {
            vector->attr_changed(this, g_quark_to_string(attr->key), NULL, attr->value, false, data);
        }
    }
    if (vector->child_added) {
        SPRepr *ref = NULL;
        SPRepr *child = this->_children;
        for ( ; child ; ref = child, child = child->next() ) {
            vector->child_added(this, child, ref, data);
        }
    }
    if (vector->content_changed) {
        vector->content_changed(this, NULL, this->_content, data);
    }
}

void SimpleNode::addListener(SPReprEventVector const *vector, void *data) {
    g_assert(vector);

    SPReprListener *listener = new SPReprListener(vector, data);

    if (_last_listener) {
        _last_listener->next = listener;
    } else {
        _listeners = listener;
    }
    _last_listener = listener;
}

void SimpleNode::removeListenerByData(void *data) {
    SPReprListener *prev = NULL;
    for (SPReprListener *rl = _listeners; rl != NULL; rl = rl->next) {
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

SPRepr *SimpleNode::root() {
    SPRepr *parent=this;
    while (parent->parent()) {
        parent = parent->parent();
    }

    if ( parent->type() == SP_XML_DOCUMENT_NODE ) {
        for ( SPRepr *child = _document->firstChild() ;
              child ; child = child->next() )
        {
            if ( child->type() == SP_XML_ELEMENT_NODE ) {
                return child;
            }
        }
        return NULL;
    } else if ( parent->type() == SP_XML_ELEMENT_NODE ) {
        return parent;
    } else {
        return NULL;
    }
}

void SimpleNode::mergeFrom(SPRepr const *src, gchar const *key) {
    g_return_if_fail(src != NULL);
    g_return_if_fail(key != NULL);

    setContent(src->content());

    for ( SPRepr const *child = src->firstChild() ; child != NULL ; child = child->next() )
    {
        gchar const *id = child->attribute(key);
        if (id) {
            SPRepr *rch=sp_repr_lookup_child(this, key, id);
            if (rch) {
                rch->mergeFrom(child, key);
            } else {
                rch = child->duplicate();
                appendChild(rch);
                rch->release();
            }
        } else {
            SPRepr *rch=child->duplicate();
            appendChild(rch);
            rch->release();
        }
    }

    for ( SPReprAttr const *attribute = src->attributeList() ; attribute ; attribute = attribute->next ) {
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
