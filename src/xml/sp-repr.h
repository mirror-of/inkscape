/*
 * SPRepr - interface for XML nodes
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SP_REPR_H
#define SEEN_INKSCAPE_XML_SP_REPR_H

#include <glib/gtypes.h>
#include "gc-anchored.h"
#include "util/shared-c-string-ptr.h"

class SPReprEventVector;
class SPReprAttr;
class SPReprDoc;

namespace Inkscape {
namespace XML {

class Session;
class TransactionLogger;

}
}

enum SPReprType {
	SP_XML_DOCUMENT_NODE,
	SP_XML_ELEMENT_NODE,
	SP_XML_TEXT_NODE,
	SP_XML_COMMENT_NODE
};

// careful; GC::Anchored should only appear once in the inheritance
// hierarcy; else there will be leaks
class SPRepr : public Inkscape::GC::Anchored {
public:
    SPRepr() {}

    virtual SPReprType type() const=0;

    virtual Inkscape::XML::Session *session()=0;

    virtual gchar const *name() const=0;
    virtual int code() const=0;
    virtual void setCodeUnsafe(int code)=0;

    virtual SPReprDoc *document()=0;
    virtual SPReprDoc const *document() const=0;

    virtual SPRepr *duplicate() const=0;

    virtual SPRepr *root()=0;
    virtual SPRepr const *root() const=0;

    virtual SPRepr *parent()=0;
    virtual SPRepr const *parent() const=0;

    virtual SPRepr *next()=0;
    virtual SPRepr const *next() const=0;

    virtual SPRepr *firstChild()=0;
    virtual SPRepr const *firstChild() const=0;
    virtual SPRepr *lastChild()=0;
    virtual SPRepr const *lastChild() const=0;

    virtual unsigned childCount() const=0;
    virtual SPRepr *nthChild(unsigned index)=0;
    virtual SPRepr const *nthChild(unsigned index) const=0;

    virtual void addChild(SPRepr *child, SPRepr *ref)=0;
    virtual void appendChild(SPRepr *child)=0;
    virtual void removeChild(SPRepr *child)=0;
    virtual void changeOrder(SPRepr *child, SPRepr *ref)=0;

    virtual unsigned position() const=0;
    virtual void setPosition(int pos)=0;

    virtual gchar const *attribute(gchar const *key) const=0;
    virtual void setAttribute(gchar const *key, gchar const *value, bool is_interactive=false)=0;
    virtual bool matchAttributeName(gchar const *partial_name) const=0;

    virtual gchar const *content() const=0;
    virtual void setContent(gchar const *value)=0;

    virtual void mergeFrom(SPRepr const *src, gchar const *key)=0;

    virtual SPReprAttr const *attributeList() const=0;

    virtual void synthesizeEvents(SPReprEventVector const *vector, void *data)=0;
    virtual void addListener(SPReprEventVector const *vector, void *data)=0;
    virtual void removeListenerByData(void *data)=0;

protected:
    SPRepr(SPRepr const &) : Anchored() {}

public: // ideally these should be protected too somehow...
    virtual void _setParent(SPRepr *parent)=0;
    virtual void _setNext(SPRepr *next)=0;
    virtual void _bindDocument(SPReprDoc &document)=0;
    virtual void _bindLogger(Inkscape::XML::TransactionLogger &logger)=0;

    virtual unsigned _childPosition(SPRepr const &child) const=0;
    virtual unsigned _cachedPosition() const=0;
    virtual void _setCachedPosition(unsigned position) const=0;
};

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
