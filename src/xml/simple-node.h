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

#include "xml/sp-repr.h"
#include "xml/sp-repr-attr.h"
#include "xml/sp-repr-listener.h"
#include "xml/transaction-logger.h"

namespace Inkscape {

namespace XML {

class SimpleNode
: virtual public SPRepr, public Inkscape::GC::Managed<>
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

    SPReprDoc *document() { return _document; }
    SPReprDoc const *document() const {
        return const_cast<SimpleNode *>(this)->document();
    }

    SPRepr *duplicate() const { return _duplicate(); }

    SPRepr *root();
    SPRepr const *root() const {
        return const_cast<SimpleNode *>(this)->root();
    }

    SPRepr *parent() { return _parent; }
    SPRepr const *parent() const { return _parent; }

    SPRepr *next() { return _next; }
    SPRepr const *next() const { return _next; }

    SPRepr *firstChild() { return _first_child; }
    SPRepr const *firstChild() const { return _first_child; }
    SPRepr *lastChild() { return _last_child; }
    SPRepr const *lastChild() const { return _last_child; }

    unsigned childCount() const { return _child_count; }
    SPRepr *nthChild(unsigned index);
    SPRepr const *nthChild(unsigned index) const {
        return const_cast<SimpleNode *>(this)->nthChild(index);
    }

    void addChild(SPRepr *child, SPRepr *ref);
    void appendChild(SPRepr *child) { addChild(child, _last_child); }
    void removeChild(SPRepr *child);
    void changeOrder(SPRepr *child, SPRepr *ref);

    unsigned position() const;
    void setPosition(int pos);

    gchar const *attribute(gchar const *key) const;
    void setAttribute(gchar const *key, gchar const *value, bool is_interactive=false);
    bool matchAttributeName(gchar const *partial_name) const;

    gchar const *content() const;
    void setContent(gchar const *value);

    void mergeFrom(SPRepr const *src, gchar const *key);

    SPReprAttr const *attributeList() const { return _attributes; }

    void synthesizeEvents(SPReprEventVector const *vector, void *data);
    void addListener(SPReprEventVector const *vector, void *data);
    void removeListenerByData(void *data);

protected:
    SimpleNode(int code);
    SimpleNode(SimpleNode const &repr);

    virtual SimpleNode *_duplicate() const=0;

public: // ideally these should be protected somehow...
    void _setParent(SPRepr *parent) { _parent = parent; }
    void _setNext(SPRepr *next) { _next = next; }
    void _bindDocument(SPReprDoc &document);
    void _bindLogger(TransactionLogger &logger);

    unsigned _childPosition(SPRepr const &child) const;
    unsigned _cachedPosition() const { return _cached_position; }
    void _setCachedPosition(unsigned position) const {
        _cached_position = position;
    }

private:
    void operator=(SPRepr const &); // no assign

    int _name;

    SPReprDoc *_document;

    TransactionLogger *_logger;

    SPReprAttr *_attributes;
    Inkscape::Util::SharedCStringPtr _content;
    SPRepr *_parent;
    SPRepr *_first_child;
    SPRepr *_last_child;
    SPRepr *_next;

    unsigned _child_count;
    mutable bool _cached_positions_valid;
    mutable unsigned _cached_position;

    SPReprListener *_listeners;
    SPReprListener *_last_listener;
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
