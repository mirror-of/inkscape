/*
 * Inkscape::XML::SimpleNode - generic XML node implementation
 *
 * Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SIMPLE_NODE_H
#define SEEN_INKSCAPE_XML_SIMPLE_NODE_H

#include "xml/node.h"
#include "xml/attribute-record.h"
#include "xml/node-listener.h"
#include "xml/transaction-logger.h"

namespace Inkscape {

namespace XML {

class SimpleNode
: virtual public Inkscape::XML::Node, public Inkscape::GC::Managed<>
{
public:
    Session *session() {
        return ( _logger ? &_logger->session() : NULL );
    }

    gchar const *name() const;
    int code() const { return _name; }
    void setCodeUnsafe(int code) {
        g_assert(!_logger && !_listeners);
        _name = code;
    }

    Inkscape::XML::Document *document() { return _document; }
    Inkscape::XML::Document const *document() const {
        return const_cast<SimpleNode *>(this)->document();
    }

    Inkscape::XML::Node *duplicate() const { return _duplicate(); }

    Inkscape::XML::Node *root();
    Inkscape::XML::Node const *root() const {
        return const_cast<SimpleNode *>(this)->root();
    }

    Inkscape::XML::Node *parent() { return _parent; }
    Inkscape::XML::Node const *parent() const { return _parent; }

    Inkscape::XML::Node *next() { return _next; }
    Inkscape::XML::Node const *next() const { return _next; }

    Inkscape::XML::Node *firstChild() { return _first_child; }
    Inkscape::XML::Node const *firstChild() const { return _first_child; }
    Inkscape::XML::Node *lastChild() { return _last_child; }
    Inkscape::XML::Node const *lastChild() const { return _last_child; }

    unsigned childCount() const { return _child_count; }
    Inkscape::XML::Node *nthChild(unsigned index);
    Inkscape::XML::Node const *nthChild(unsigned index) const {
        return const_cast<SimpleNode *>(this)->nthChild(index);
    }

    void addChild(Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
    void appendChild(Inkscape::XML::Node *child) {
        SimpleNode::addChild(child, _last_child);
    }
    void removeChild(Inkscape::XML::Node *child);
    void changeOrder(Inkscape::XML::Node *child, Inkscape::XML::Node *ref);

    unsigned position() const;
    void setPosition(int pos);

    gchar const *attribute(gchar const *key) const;
    void setAttribute(gchar const *key, gchar const *value, bool is_interactive=false);
    bool matchAttributeName(gchar const *partial_name) const;

    gchar const *content() const;
    void setContent(gchar const *value);

    void mergeFrom(Inkscape::XML::Node const *src, gchar const *key);

    Inkscape::XML::AttributeRecord const *attributeList() const { return _attributes; }

    void synthesizeEvents(Inkscape::XML::NodeEventVector const *vector, void *data);
    void addListener(Inkscape::XML::NodeEventVector const *vector, void *data);
    void removeListenerByData(void *data);

protected:
    SimpleNode(int code);
    SimpleNode(SimpleNode const &repr);

    virtual SimpleNode *_duplicate() const=0;

public: // ideally these should be protected somehow...
    void _setParent(Inkscape::XML::Node *parent) { _parent = parent; }
    void _setNext(Inkscape::XML::Node *next) { _next = next; }
    void _bindDocument(Inkscape::XML::Document &document);
    void _bindLogger(TransactionLogger &logger);

    unsigned _childPosition(Inkscape::XML::Node const &child) const;
    unsigned _cachedPosition() const { return _cached_position; }
    void _setCachedPosition(unsigned position) const {
        _cached_position = position;
    }

private:
    void operator=(Inkscape::XML::Node const &); // no assign

    int _name;

    Inkscape::XML::Document *_document;

    TransactionLogger *_logger;

    Inkscape::XML::AttributeRecord *_attributes;
    Inkscape::Util::SharedCStringPtr _content;
    Inkscape::XML::Node *_parent;
    Inkscape::XML::Node *_first_child;
    Inkscape::XML::Node *_last_child;
    Inkscape::XML::Node *_next;

    unsigned _child_count;
    mutable bool _cached_positions_valid;
    mutable unsigned _cached_position;

    Inkscape::XML::NodeListener *_listeners;
    Inkscape::XML::NodeListener *_last_listener;
};

}

}

#endif
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
