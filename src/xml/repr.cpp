#define __SP_REPR_C__

/** \file
 * Fuzzy DOM-like tree implementation
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 * g++ port Copyright (C) 2003 Nathan Hurst
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noREPR_VERBOSE

#include "config.h"

#include <string.h>

#if HAVE_STDDEF_H
#include <stddef.h>
#endif

#include <glib.h>

#include "xml/repr-get-children.h"
#include "xml/repr-private.h"
#include "xml/sp-repr-listener.h"
#include "xml/sp-repr-event-vector.h"
#include "xml/sp-repr-attr.h"
#include "xml/sp-repr-action.h"
#include "xml/sp-repr-action-fns.h"
#include "xml/simple-session.h"

using Inkscape::Util::SharedCStringPtr;

SPRepr *
sp_repr_new(gchar const *name)
{
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(*name != '\0', NULL);

    return new SPReprElement(g_quark_from_string(name));
}

SPRepr *
sp_repr_new_text(gchar const *content)
{
    g_return_val_if_fail(content != NULL, NULL);
    return new SPReprText(SharedCStringPtr::copy(content));
}

SPRepr *
sp_repr_new_comment(gchar const *comment)
{
    g_return_val_if_fail(comment != NULL, NULL);
    return new SPReprComment(SharedCStringPtr::copy(comment));
}

SPRepr::SPRepr(SPReprType t, int code)
: _name(code), _type(t), _child_count(0),
  _cached_positions_valid(false)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = this->_children = NULL;
    this->_attributes = NULL;
    this->_last_listener = this->_listeners = NULL;
}

SPReprDoc::SPReprDoc(int code) : SPRepr(SP_XML_DOCUMENT_NODE, code) {
    _bindDocument(*this);
    _bindLogger(*(new Inkscape::XML::SimpleSession()));
}

SPRepr *sp_repr_ref(SPRepr *repr) {
    return Inkscape::GC::anchor(repr);
}

SPRepr *sp_repr_unref(SPRepr *repr) {
    Inkscape::GC::release(repr);
    return NULL;
}

SPRepr *
sp_repr_duplicate(SPRepr const *repr)
{
    g_assert(repr != NULL);
    return repr->duplicate();
}

SPRepr::SPRepr(SPRepr const &repr)
: Anchored(), _name(repr._name),
  _type(repr._type), _content(repr._content),
  _child_count(repr._child_count),
  _cached_positions_valid(repr._cached_positions_valid),
  _cached_position(repr._cached_position)
{
    this->_logger = NULL;
    this->_document = NULL;
    this->_parent = this->_next = this->_children = NULL;
    this->_attributes = NULL;
    this->_last_listener = this->_listeners = NULL;

    SPRepr *prev_child_copy=NULL;
    for ( SPRepr *child = repr._children ; child != NULL ; child = child->next() ) {
        SPRepr *child_copy=child->duplicate();

        child_copy->_setParent(this);
        if (prev_child_copy) {
            prev_child_copy->_setNext(child_copy);
        } else {
            this->_children = child_copy;
        }
        sp_repr_unref(child_copy); // even duplicates are created with a
                                   // refcount of one; unref here to avoid a
                                   // leak; the ref from the child list
                                   // suffices

        prev_child_copy = child_copy;
    }

    SPReprAttr *prev_attr_copy=NULL;
    for ( SPReprAttr *attr = repr._attributes ; attr != NULL ; attr = attr->next ) {
        SPReprAttr *attr_copy=new SPReprAttr(*attr);

        if (prev_attr_copy) {
            prev_attr_copy->next = attr_copy;
        } else {
            this->_attributes = attr_copy;
        }

        prev_attr_copy = attr_copy;
    }
}

SPReprDoc::SPReprDoc(SPReprDoc const &doc) : SPRepr(doc) {
    _bindDocument(*this);
    _bindLogger(*(new Inkscape::XML::SimpleSession()));
}

gchar const *SPRepr::name() const {
    return g_quark_to_string(_name);
}

gchar const *
sp_repr_name(SPRepr const *repr)
{
    g_return_val_if_fail(repr != NULL, NULL);
    return repr->name();
}

gchar const *
SPRepr::content() const {
    return this->_content;
}

gchar const *
sp_repr_content(SPRepr const *repr)
{
    g_assert(repr != NULL);
    return repr->content();
}

gchar const *
SPRepr::attribute(gchar const *key) const {
    g_return_val_if_fail(key != NULL, NULL);

    /* retrieve an int identifier specific to this string */
    GQuark const q = g_quark_from_string(key);

    for (SPReprAttr *ra = this->_attributes; ra != NULL; ra = ra->next) {
        if ( ra->key == q ) {
            return ra->value;
        }
    }

    return NULL;
}

/**
 * \brief Retrieves the first attribute in the XML representation with
 * the given key 'key'
 */
gchar const *
sp_repr_attr(SPRepr const *repr, gchar const *key)
{
    g_return_val_if_fail(repr != NULL, NULL);
    return repr->attribute(key);
}

bool
SPRepr::matchAttributeName(gchar const *partial_name) const {
    g_return_val_if_fail(partial_name != NULL, false);

    for (SPReprAttr *ra = this->_attributes; ra != NULL; ra = ra->next) {
        gchar const *attr_name = g_quark_to_string(ra->key);
        if (strstr(attr_name, partial_name) != NULL) {
            return true;
        }
    }

    return false;
}

/**
 * Returns true if the XML representation has attribute with
 * the given name or including the given name as part
 */
bool
sp_repr_has_attr(SPRepr const *repr, gchar const *partial_name)
{
    g_return_val_if_fail(repr != NULL, false);
    return repr->matchAttributeName(partial_name);
}

void
SPRepr::setContent(gchar const *newcontent)
{
    SharedCStringPtr oldcontent = this->_content;

    if (newcontent) {
        this->_content = SharedCStringPtr::copy(newcontent);
    } else {
        this->_content = SharedCStringPtr();
    }
    if (_logger) {
        _logger->notifyContentChanged(*this, oldcontent, this->_content);
    }

    for (SPReprListener *rl = this->_listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->content_changed) {
            (* rl->vector->content_changed)(this, oldcontent, this->_content, rl->data);
        }
    }
}

unsigned
sp_repr_set_content(SPRepr *repr, gchar const *newcontent) {
    g_return_val_if_fail(repr != NULL, FALSE);
    repr->setContent(newcontent);
    return true;
}

void
SPRepr::_deleteAttribute(gchar const *key, bool is_interactive)
{
    g_return_if_fail(key != NULL);
    g_return_if_fail(*key != '\0');

    GQuark const q = g_quark_from_string(key);
    SPReprAttr *prev = NULL;
    SPReprAttr *attr;
    for (attr = this->_attributes; attr && (attr->key != q); attr = attr->next) {
        prev = attr;
    }

    if (attr) {
        if (prev) {
            prev->next = attr->next;
        } else {
            this->_attributes = attr->next;
        }
        if (_logger) {
            _logger->notifyAttributeChanged(*this, q, attr->value, SharedCStringPtr());
        }

        for (SPReprListener *rl = this->_listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->attr_changed) {
                (* rl->vector->attr_changed)(this, key, attr->value, NULL, is_interactive, rl->data);
            }
        }
    }
}

void
SPRepr::_changeAttribute(gchar const *key, gchar const *value, bool is_interactive)
{
    g_return_if_fail(key != NULL);
    g_return_if_fail(*key != '\0');

    GQuark const q = g_quark_from_string(key);
    SPReprAttr *prev = NULL;
    SPReprAttr *attr;
    for (attr = this->_attributes; attr && (attr->key != q); attr = attr->next) {
        prev = attr;
    }

    if ( attr && !strcmp(attr->value, value) ) {
        return;
    }

    SharedCStringPtr oldval = ( attr ? attr->value : SharedCStringPtr() );

    if (attr) {
        attr->value = SharedCStringPtr::copy(value);
    } else {
        attr = new SPReprAttr(q, SharedCStringPtr::copy(value));
        if (prev) {
            prev->next = attr;
        } else {
            this->_attributes = attr;
        }
    }
    if (_logger) {
        _logger->notifyAttributeChanged(*this, q, oldval, attr->value);
    }

    for (SPReprListener *rl = this->_listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->attr_changed) {
            (* rl->vector->attr_changed)(this, key, oldval, attr->value, is_interactive, rl->data);
        }
    }
}

void
SPRepr::setAttribute(gchar const *key, gchar const *value, bool const is_interactive)
{
    g_return_if_fail(key != NULL);
    g_return_if_fail(*key != '\0');

    if (!value) {
        _deleteAttribute(key, is_interactive);
    } else {
        _changeAttribute(key, value, is_interactive);
    }
}

unsigned
sp_repr_set_attr(SPRepr *repr, gchar const *key, gchar const *value, bool const is_interactive)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    repr->setAttribute(key, value, is_interactive);
    return true;
}

SPRepr *
sp_repr_parent(SPRepr const *repr)
{
    g_assert(repr != NULL);
    return const_cast<SPRepr *>(repr->parent());
}

void
SPRepr::addChild(SPRepr *child, SPRepr *ref)
{
    g_assert(child != NULL);
    g_assert(!ref || ref->parent() == this);
    g_assert(child->parent() == NULL);
    g_assert(child->document() == NULL || child->document() == _document);

    SPRepr *next;
    if (ref) {
        next = ref->next();
        ref->_setNext(child);
    } else {
        next = this->_children;
        this->_children = child;
    }
    if (!next) {
        // update cached positions if appending
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

    for (SPReprListener *rl = this->_listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->child_added) {
            (* rl->vector->child_added)(this, child, ref, rl->data);
        }
    }
}

/** Make \a child a child of \a repr, inserting after \a ref if non-null, or as the first
 *  child if \a ref is null.
 *
 *  \pre See block of g_asserts in the definition.
 *  \pre ( ( child->doc == _document )
 *         || all of child's children (recursively) have null doc ).
 */
unsigned
sp_repr_add_child(SPRepr *repr, SPRepr *child, SPRepr *ref)
{
    g_assert(repr != NULL);
    repr->addChild(child, ref);
    return true;
}

void SPRepr::_bindDocument(SPReprDoc &document) {
    g_assert(!_document || _document == &document);
    if (!_document) {
        _document = &document;

        for ( SPRepr *child = _children ; child != NULL ; child = child->next() ) {
            child->_bindDocument(document);
        }
    }
}

void SPRepr::_bindLogger(Inkscape::XML::TransactionLogger &logger) {
    g_assert(!_logger || _logger == &logger);

    if (!_logger) {
        _logger = &logger;

        for ( SPRepr *child = _children ; child != NULL ; child = child->next() ) {
            child->_bindLogger(logger);
        }
    }
}

void SPRepr::removeChild(SPRepr *child) {
    g_assert(child != NULL);
    g_assert(child->parent() == this);

    SPRepr *ref = NULL;
    if (child != this->_children) {
        ref = this->_children;
        while (ref->next() != child) {
            ref = ref->next();
        }
    }

    SPRepr *next = child->next();
    if (ref) {
        ref->_setNext(next);
    } else {
        this->_children = next;
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

    for (SPReprListener *rl = this->_listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->child_removed) {
            (* rl->vector->child_removed)(this, child, ref, rl->data);
        }
    }
}

unsigned
sp_repr_remove_child(SPRepr *repr, SPRepr *child)
{
    g_assert(repr != NULL);
    repr->removeChild(child);
    return true;
}

void SPRepr::changeOrder(SPRepr *child, SPRepr *ref) {
    g_return_if_fail( child != NULL );
    g_return_if_fail( child->parent() == this );
    g_return_if_fail( child != ref );
    g_return_if_fail( !ref || ref->parent() == this );

    SPRepr *const prev = sp_repr_prev(child);

    if (prev == ref) { return; }

    /* Remove from old position. */
    if (prev) {
        prev->_setNext(child->next());
    } else {
        this->_children = child->next();
    }
    /* Insert at new position. */
    if (ref) {
        child->_setNext(ref->next());
        ref->_setNext(child);
    } else {
        child->_setNext(this->_children);
        this->_children = child;
    }

    this->_cached_positions_valid = false;

    if (_logger) {
        _logger->notifyChildOrderChanged(*this, *child, prev, ref);
    }

    for (SPReprListener *rl = this->_listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->order_changed) {
            (* rl->vector->order_changed)(this, child, prev, ref, rl->data);
        }
    }
}

unsigned
sp_repr_change_order(SPRepr *const repr, SPRepr *const child, SPRepr *const ref)
{
    g_assert( repr != NULL );
    repr->changeOrder(child, ref);
    return true;
}

void SPRepr::setPosition(int pos) {
    SPRepr *parent = this->_parent;

    if (pos < 0) {
        pos = 0x7fffffff;
        /* fixme: Would INT__MAX be better?  Better yet, should pos be unsigned?  Perhaps
           call g_warning?  -- pjrm. */
    }

    /* Find the child before child pos of parent (or NULL if pos==0). */
    SPRepr *ref = NULL;
    SPRepr *cur = parent->firstChild();
    while (pos > 0 && cur) {
        ref = cur;
        cur = cur->next();
        pos -= 1;
    }

    if (ref == this) {
        /* FIXME: I think this test should be moved into the above loop, i.e.  we want prev to be
           number (pos-1) of the children other than repr.  I.e. only decrement pos if ref != repr.
           Consider the case of repr==parent->_children && pos==2. */
        ref = this->_next;
        if (!ref) {
            return;
        }
    }

    parent->changeOrder(this, ref);
}

/** Note: Many if not all existing callers would be better off calling sp_repr_prev in
 *  place of sp_repr_position, and passing that prev sibling to sp_repr_add_child or
 *  sp_repr_change_order.  The main thing to watch for is if that prev sibling gets removed between
 *  the sp_repr_prev / sp_repr_position call and the sp_repr_add_child /
 *  sp_repr_change_order call; though of course this presumably already needs handling for the
 *  sp_repr_position-based code (decrementing the position number), unless that removed prev
 *  sibling were replaced with a different SPRepr object.
 */
void
sp_repr_set_position_absolute(SPRepr *repr, int pos)
{
    repr->setPosition(pos);
}

void
SPRepr::synthesizeEvents(SPReprEventVector const *vector, void *data) {
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

void
sp_repr_synthesize_events(SPRepr *repr, SPReprEventVector const *vector, void *data)
{
    repr->synthesizeEvents(vector, data);
}

void SPRepr::addListener(SPReprEventVector const *vector, void *data) {
    g_assert(vector != NULL);

    SPReprListener *rl = new SPReprListener(vector, data);

    if (_last_listener) {
        _last_listener->next = rl;
    } else {
        _listeners = rl;
    }
    _last_listener = rl;
}

void
sp_repr_add_listener(SPRepr *repr, SPReprEventVector const *vector, void *data)
{
    g_assert(repr != NULL);
    repr->addListener(vector, data);
}

void SPRepr::removeListenerByData(void *data) {
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

void
sp_repr_remove_listener_by_data(SPRepr *repr, void *data)
{
    g_return_if_fail(repr != NULL);
    repr->removeListenerByData(data);
}


/* Documents - 1st step in migrating to real XML */
/* fixme: Do this somewhere, somehow The Right Way (TM) */

SPReprDoc *
sp_repr_document_new(char const *rootname)
{
    SPReprDoc *doc = new SPReprDoc(g_quark_from_static_string("xml"));
    if (!strcmp(rootname, "svg:svg")) {
        sp_repr_set_attr(doc, "version", "1.0");
        sp_repr_set_attr(doc, "standalone", "no");
        SPRepr *comment = sp_repr_new_comment(" Created with Inkscape (http://www.inkscape.org/) ");
        sp_repr_append_child(doc, comment);
        sp_repr_unref(comment);
    }

    SPRepr *root = sp_repr_new(rootname);
    sp_repr_append_child(doc, root);
    sp_repr_unref(root);

    return doc;
}

void
sp_repr_document_ref(SPReprDoc *doc)
{
    sp_repr_ref((SPRepr *) doc);
}

void
sp_repr_document_unref(SPReprDoc *doc)
{
    sp_repr_unref((SPRepr *) doc);
}

SPReprDoc *
sp_repr_document_new_list(GSList *reprs)
{
    g_assert(reprs != NULL);

    SPReprDoc *doc = sp_repr_document_new("void");
    doc->removeChild(doc->firstChild());

    for ( GSList *iter = reprs ; iter ; iter = iter->next ) {
        SPRepr *repr = (SPRepr *) iter->data;
        doc->appendChild(repr);
    }

    g_assert(sp_repr_document_root(doc) != NULL);

    return doc;
}

SPRepr *
sp_repr_document_first_child(SPReprDoc const *doc)
{
    return const_cast<SPRepr *>(doc->firstChild());
}

SPReprDoc *
sp_repr_document(SPRepr const *repr)
{
    return const_cast<SPReprDoc *>(repr->document());
}

SPRepr *SPRepr::root() {
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

SPRepr *sp_repr_document_root(SPReprDoc const *doc)
{
    g_assert( doc != NULL );
    return const_cast<SPRepr *>(doc->root());
}

/*
 * Duplicate all attributes and children from src into doc
 * Does NOT erase original attributes and children
 */

unsigned
sp_repr_document_merge(SPReprDoc *doc, SPReprDoc const *src, gchar const *key)
{
    g_return_val_if_fail(doc != NULL, FALSE);
    g_return_val_if_fail(src != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    SPRepr *rdoc = sp_repr_document_root(doc);
    SPRepr const *rsrc = sp_repr_document_root(src);

    return sp_repr_merge(rdoc, rsrc, key);
}

void
SPRepr::mergeFrom(SPRepr const *src, gchar const *key) {
    g_return_if_fail(src != NULL);
    g_return_if_fail(key != NULL);

    this->_content = src->_content;

    for ( SPRepr const *child = src->firstChild() ; child != NULL ; child = child->next() )
    {
        gchar const *id = sp_repr_attr(child, key);
        if (id) {
            SPRepr *rch = sp_repr_lookup_child(this, key, id);
            if (rch) {
                rch->mergeFrom(child, key);
            } else {
                rch = sp_repr_duplicate(child);
                sp_repr_append_child(this, rch);
                sp_repr_unref(rch);
            }
        } else {
            SPRepr *rch = sp_repr_duplicate(child);
            sp_repr_append_child(this, rch);
            sp_repr_unref(rch);
        }
    }

    for (SPReprAttr *attr = src->_attributes; attr != NULL; attr = attr->next) {
        this->setAttribute(SP_REPR_ATTRIBUTE_KEY(attr), SP_REPR_ATTRIBUTE_VALUE(attr));
    }
}

/**
 * Duplicates all attributes and children from src into doc.
 * Does NOT erase original attributes and children.
 */
unsigned
sp_repr_merge(SPRepr *repr, SPRepr const *src, gchar const *key)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    repr->mergeFrom(src, key);
    return TRUE;
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
