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

#include "xml/repr-private.h"
#include "xml/sp-repr-listener.h"
#include "xml/sp-repr-event-vector.h"
#include "xml/sp-repr-attr.h"
#include "xml/sp-repr-action.h"
#include "xml/sp-repr-action-fns.h"

using Inkscape::Util::SharedCString;

static void bind_document(SPReprDoc *doc, SPRepr *repr);

static SPRepr *sp_repr_new_from_code(SPReprType type, int code);

static SPRepr *
sp_repr_new_from_code(SPReprType type, int code)
{
    switch (type) {
    case SP_XML_ELEMENT_NODE:
        return new SPReprElement(code);
    case SP_XML_TEXT_NODE:
        return new SPReprText(code);
    case SP_XML_COMMENT_NODE:
        return new SPReprComment(code);
    case SP_XML_DOCUMENT_NODE:
        return new SPReprDoc(code);
    default:
        g_assert_not_reached();
        return NULL;
    }
}

SPRepr *
sp_repr_new(gchar const *name)
{
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(*name != '\0', NULL);

    return sp_repr_new_from_code(SP_XML_ELEMENT_NODE, g_quark_from_string(name));
}

SPRepr *
sp_repr_new_text(gchar const *content)
{
    g_return_val_if_fail(content != NULL, NULL);
    SPRepr *repr = sp_repr_new_from_code(SP_XML_TEXT_NODE, g_quark_from_static_string("text"));
    repr->content = SharedCString::copy(content);
    return repr;
}

SPRepr *
sp_repr_new_comment(gchar const *comment)
{
    g_return_val_if_fail(comment != NULL, NULL);
    SPRepr *repr = sp_repr_new_from_code(SP_XML_COMMENT_NODE, g_quark_from_static_string("comment"));
    repr->content = SharedCString::copy(comment);
    return repr;
}

SPRepr::SPRepr(SPReprType t, int code) : type(t), name(code) {
    this->doc = NULL;
    this->parent = this->next = this->children = NULL;
    this->attributes = NULL;
    this->last_listener = this->listeners = NULL;
}

SPReprDoc::SPReprDoc(int code) : SPRepr(SP_XML_DOCUMENT_NODE, code) {
    this->doc = this;
    this->log = NULL;
    this->is_logging = false;
}

SPReprDoc::~SPReprDoc() {
    sp_repr_free_log(this->log);
}

SPRepr *sp_repr_ref(SPRepr *repr) {
    return Inkscape::GC::anchor(repr);
}

SPRepr *sp_repr_unref(SPRepr *repr) {
    Inkscape::GC::release(repr);
    return NULL;
}

static SPRepr *
sp_repr_attach(SPRepr *parent, SPRepr *child)
{
    g_assert(parent != NULL);
    g_assert(child != NULL);
    g_assert(child->parent == NULL);
    g_assert(child->doc == NULL && parent->doc == NULL);

    child->parent = parent;

    return child;
}

SPRepr *
sp_repr_duplicate(SPRepr const *repr)
{
    g_assert(repr != NULL);
    return repr->duplicate();
}

SPRepr::SPRepr(SPRepr const &repr)
: Anchored(), type(repr.type), name(repr.name), content(repr.content)
{
    SPRepr *lastchild = NULL;
    for (SPRepr *child = repr.children; child != NULL; child = child->next) {
        if (lastchild) {
            lastchild = lastchild->next = sp_repr_attach(this, sp_repr_duplicate(child));
        } else {
            lastchild = this->children = sp_repr_attach(this, sp_repr_duplicate(child));
        }
    }

    SPReprAttr *lastattr = NULL;
    for (SPReprAttr *attr = repr.attributes; attr != NULL; attr = attr->next) {
        if (lastattr) {
            lastattr = lastattr->next = new SPReprAttr(*attr);
        } else {
            lastattr = this->attributes = new SPReprAttr(*attr);
        }
    }
}

SPReprDoc::SPReprDoc(SPReprDoc const &doc) : SPRepr(doc), Finalized() {
    this->log = NULL;
    this->is_logging = false;
}

gchar const *
sp_repr_name(SPRepr const *repr)
{
    g_return_val_if_fail(repr != NULL, NULL);

    return SP_REPR_NAME(repr);
}

gchar const *
sp_repr_content(SPRepr const *repr)
{
    g_assert(repr != NULL);

    return SP_REPR_CONTENT(repr);
}

/**
 * \brief Retrieves the first attribute in the XML representation with
 * the given key 'key'
 */
gchar const *
sp_repr_attr(SPRepr const *repr, gchar const *key)
{
    g_return_val_if_fail(repr != NULL, NULL);
    g_return_val_if_fail(key != NULL, NULL);

    /* retrieve an int identifier specific to this string */
    GQuark const q = g_quark_from_string(key);

    for (SPReprAttr *ra = repr->attributes; ra != NULL; ra = ra->next) {
        if ( ra->key == q ) {
            return ra->value;
        }
    }

    return NULL;
}

/**
 * Returns true if the XML representation has attribute with
 * the given name or including the given name as part
 */
bool
sp_repr_has_attr(SPRepr const *repr, gchar const *partial_name)
{
    g_return_val_if_fail(repr != NULL, false);
    g_return_val_if_fail(partial_name != NULL, false);

    for (SPReprAttr *ra = repr->attributes; ra != NULL; ra = ra->next) {
        gchar const *attr_name = g_quark_to_string(ra->key);
        if (strstr(attr_name, partial_name) != NULL) {
            return true;
        }
    }

    return false;
}

unsigned
sp_repr_set_content(SPRepr *repr, gchar const *newcontent)
{
    g_return_val_if_fail(repr != NULL, FALSE);

    unsigned allowed = TRUE;
    for (SPReprListener *rl = repr->listeners; rl && allowed; rl = rl->next) {
        if (rl->vector->change_content) {
            allowed = (* rl->vector->change_content)(repr, repr->content, newcontent, rl->data);
        }
    }

    if (allowed) {
        SharedCString oldcontent = repr->content;

        if (newcontent) {
            repr->content = SharedCString::copy(newcontent);
        } else {
            repr->content = SharedCString();
        }
        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = (new SPReprActionChgContent(repr, oldcontent, repr->content, repr->doc->log))->optimizeOne();
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->content_changed) {
                (* rl->vector->content_changed)(repr, oldcontent, repr->content, rl->data);
            }
        }
    }

    return allowed;
}

static unsigned
sp_repr_del_attr(SPRepr *repr, gchar const *key, bool is_interactive)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(*key != '\0', FALSE);

    GQuark const q = g_quark_from_string(key);
    SPReprAttr *prev = NULL;
    SPReprAttr *attr;
    for (attr = repr->attributes; attr && (attr->key != q); attr = attr->next) {
        prev = attr;
    }

    unsigned allowed = TRUE;

    if (attr) {
        for (SPReprListener *rl = repr->listeners; rl && allowed; rl = rl->next) {
            if (rl->vector->change_attr) {
                allowed = (* rl->vector->change_attr)(repr, key, attr->value, NULL, rl->data);
            }
        }

        if (allowed) {
            if (prev) {
                prev->next = attr->next;
            } else {
                repr->attributes = attr->next;
            }
            if ( repr->doc && repr->doc->is_logging ) {
                repr->doc->log = (new SPReprActionChgAttr(repr, q, attr->value, SharedCString(), repr->doc->log))->optimizeOne();
            }

            for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
                if (rl->vector->attr_changed) {
                    (* rl->vector->attr_changed)(repr, key, attr->value, NULL, is_interactive, rl->data);
                }
            }
        }
    }

    return allowed;
}

static unsigned
sp_repr_chg_attr(SPRepr *repr, gchar const *key, gchar const *value, bool is_interactive)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(*key != '\0', FALSE);

    GQuark const q = g_quark_from_string(key);
    SPReprAttr *prev = NULL;
    SPReprAttr *attr;
    for (attr = repr->attributes; attr && (attr->key != q); attr = attr->next) {
        prev = attr;
    }

    if ( attr && !strcmp(attr->value, value) ) {
        return TRUE;
    }

    unsigned allowed = TRUE;
    for (SPReprListener *rl = repr->listeners; rl && allowed; rl = rl->next) {
        if (rl->vector->change_attr) {
            allowed = (* rl->vector->change_attr)(repr, key, ( attr ? attr->value : SharedCString() ), value, rl->data);
        }
    }

    if (allowed) {
        SharedCString oldval = ( attr ? attr->value : SharedCString() );

        if (attr) {
            attr->value = SharedCString::copy(value);
        } else {
            attr = new SPReprAttr(q, SharedCString::copy(value));
            if (prev) {
                prev->next = attr;
            } else {
                repr->attributes = attr;
            }
        }
        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = (new SPReprActionChgAttr(repr, q, oldval, attr->value, repr->doc->log))->optimizeOne();
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->attr_changed) {
                (* rl->vector->attr_changed)(repr, key, oldval, attr->value, is_interactive, rl->data);
            }
        }
    }

    return allowed;
}

unsigned
sp_repr_set_attr(SPRepr *repr, gchar const *key, gchar const *value, bool const is_interactive)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(*key != '\0', FALSE);

    if (!value) {
        return sp_repr_del_attr(repr, key, is_interactive);
    } else {
        return sp_repr_chg_attr(repr, key, value, is_interactive);
    }
}

SPRepr *
sp_repr_parent(SPRepr const *repr)
{
    g_assert(repr != NULL);

    return repr->parent;
}

/** Make \a child a child of \a repr, inserting after \a ref if non-null, or as the first
 *  child if \a ref is null.
 *
 *  \pre See block of g_asserts in the definition.
 *  \pre ( ( child->doc == repr->doc )
 *         || all of child's children (recursively) have null doc ).
 */
unsigned
sp_repr_add_child(SPRepr *repr, SPRepr *child, SPRepr *ref)
{
    g_assert(repr != NULL);
    g_assert(child != NULL);
    g_assert(!ref || ref->parent == repr);
    g_assert(child->parent == NULL);
    g_assert(child->doc == NULL || child->doc == repr->doc);

    unsigned allowed = TRUE;
    for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->add_child) {
            allowed = (* rl->vector->add_child)(repr, child, ref, rl->data);
            /* FIXME: I'd guess that `&& allowed' should be added to the loop condition, like the
               other `allowed' loops, so that we don't overwrite false.  Alternatively, given that
               this probable-bug has occurred, perhaps use explicit `break' statement when false is
               encountered, so that no easy-to-miss `&& allowed' test is needed in the `for'
               condition.  Similarly elsewhere in repr.cpp.  -- pjrm */
        }
    }

    if (allowed) {
        if (ref != NULL) {
            child->next = ref->next;
            ref->next = child;
        } else {
            child->next = repr->children;
            repr->children = child;
        }

        child->parent = repr;

        if (child->doc == NULL) bind_document(repr->doc, child);

        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = (new SPReprActionAdd(repr, child, ref, repr->doc->log))->optimizeOne();
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->child_added) {
                (* rl->vector->child_added)(repr, child, ref, rl->data);
            }
        }
    }

    return allowed;
}

static void
bind_document(SPReprDoc *doc, SPRepr *repr)
{
    g_assert(repr->doc == NULL);

    repr->doc = doc;

    for ( SPRepr *child = repr->children ; child != NULL ; child = child->next ) {
        bind_document(doc, child);
    }
}

unsigned
sp_repr_remove_child(SPRepr *repr, SPRepr *child)
{
    g_assert(repr != NULL);
    g_assert(child != NULL);
    g_assert(child->parent == repr);

    SPRepr *ref = NULL;
    if (child != repr->children) {
        ref = repr->children;
        while (ref->next != child) {
            ref = ref->next;
        }
    }

    unsigned allowed = TRUE;
    for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
        if (rl->vector->remove_child) {
            /* TODO:  This is a quick & nasty hack to prevent
               a crash in the XML editor when deleting the
               'namedview' node.  There is a better solution but
               it's much more involved.  See Inkscape Bug #850971 */
            // added a check that the namedview is top-level (child of svg),
            // otherwise this freezes when trying to ungroup imported group   --bb
            if ( !strcmp(sp_repr_name(child), "sodipodi:namedview")  &&
                 !strcmp(sp_repr_name(sp_repr_parent(child)), "svg") )
            {
                allowed = FALSE;
            } else {
                allowed = (* rl->vector->remove_child)(repr, child, ref, rl->data);
            }
            /* FIXME: I'd guess that `&& allowed' should be added to the loop condition, like the
               other `allowed' loops, so that we don't overwrite false.  Alternatively, given that
               this probable-bug has occurred, perhaps use explicit `break' statement when false is
               encountered, so that no easy-to-miss `&& allowed' test is needed in the `for'
               condition.  Similarly elsewhere in repr.cpp.  -- pjrm */
        }
    }

    if (allowed) {
        if (ref) {
            ref->next = child->next;
        } else {
            repr->children = child->next;
        }
        child->parent = NULL;
        child->next = NULL;

        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = (new SPReprActionDel(repr, child, ref, repr->doc->log))->optimizeOne();
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->child_removed) {
                (* rl->vector->child_removed)(repr, child, ref, rl->data);
            }
        }
    }

    return allowed;
}

unsigned
sp_repr_change_order(SPRepr *repr, SPRepr *child, SPRepr *ref)
{
    SPRepr *prev = NULL;
    if (child != repr->children) {
        for (SPRepr *cur = repr->children; cur != child; cur = cur->next) {
            prev = cur;
        }
    }

    if (prev == ref) {
        return TRUE;
    }

    unsigned allowed = TRUE;
    for (SPReprListener *rl = repr->listeners; rl && allowed; rl = rl->next) {
        if (rl->vector->change_order) {
            allowed = (* rl->vector->change_order)(repr, child, prev, ref, rl->data);
        }
    }

    if (allowed) {
        /* Remove from old position. */
        if (prev) {
            prev->next = child->next;
        } else {
            repr->children = child->next;
        }
        /* Insert at new position. */
        if (ref) {
            child->next = ref->next;
            ref->next = child;
        } else {
            child->next = repr->children;
            repr->children = child;
        }

        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = (new SPReprActionChgOrder(repr, child, prev, ref, repr->doc->log))->optimizeOne();
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->order_changed) {
                (* rl->vector->order_changed)(repr, child, prev, ref, rl->data);
            }
        }
    }

    return allowed;
}

/** Note: Many if not all existing callers would be better off calling sp_repr_get_prev_sibling in
 *  place of sp_repr_position, and passing that prev sibling to sp_repr_add_child or
 *  sp_repr_change_order.  The main thing to watch for is if that prev sibling gets removed between
 *  the sp_repr_get_prev_sibling / sp_repr_position call and the sp_repr_add_child /
 *  sp_repr_change_order call; though of course this presumably already needs handling for the
 *  sp_repr_position-based code (decrementing the position number), unless that removed prev
 *  sibling were replaced with a different SPRepr object.
 */
void
sp_repr_set_position_absolute(SPRepr *repr, int pos)
{
    SPRepr *parent = repr->parent;

    if (pos < 0) {
        pos = 0x7fffffff;
        /* fixme: Would INT__MAX be better?  Better yet, should pos be unsigned?  Perhaps
           call g_warning?  -- pjrm. */
    }

    /* Find the child before child pos of parent (or NULL if pos==0). */
    SPRepr *ref = NULL;
    {
        SPRepr *cur = parent->children;
        while (pos > 0 && cur) {
            ref = cur;
            cur = cur->next;
            pos -= 1;
        }
    }

    if (ref == repr) {
        /* FIXME: I think this test should be moved into the above loop, i.e.  we want prev to be
           number (pos-1) of the children other than repr.  I.e. only decrement pos if ref != repr.
           Consider the case of repr==parent->children && pos==2. */
        ref = repr->next;
        if (!ref) {
            return;
        }
    }

    sp_repr_change_order(parent, repr, ref);
}

void
sp_repr_synthesize_events(SPRepr *repr, SPReprEventVector const *vector, void *data)
{
    if (vector->attr_changed) {
        for (SPReprAttr *attr = repr->attributes ; attr ; attr = attr->next ) {
            vector->attr_changed(repr, g_quark_to_string(attr->key), NULL, attr->value, false, data);
        }
    }
    if (vector->child_added) {
        SPRepr *ref = NULL;
        SPRepr *child = repr->children;
        for ( ; child ; ref = child, child = child->next ) {
            vector->child_added(repr, child, ref, data);
        }
    }
    if (vector->content_changed) {
        vector->content_changed(repr, NULL, repr->content, data);
    }
}

void
sp_repr_add_listener(SPRepr *repr, SPReprEventVector const *vector, void *data)
{
    g_assert(repr != NULL);
    g_assert(vector != NULL);

    SPReprListener *rl = new SPReprListener(vector, data);

    if (repr->last_listener) {
        repr->last_listener->next = rl;
    } else {
        repr->listeners = rl;
    }
    repr->last_listener = rl;
}

void
sp_repr_remove_listener_by_data(SPRepr *repr, void *data)
{
    g_return_if_fail(repr != NULL);

    SPReprListener *prev = NULL;
    for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
        if (rl->data == data) {
            if (prev) {
                prev->next = rl->next;
            } else {
                repr->listeners = rl->next;
            }
            if (!rl->next) {
                repr->last_listener = prev;
            }
            return;
        }
        prev = rl;
    }
}


/* Documents - 1st step in migrating to real XML */
/* fixme: Do this somewhere, somehow The Right Way (TM) */

SPReprDoc *
sp_repr_document_new(char const *rootname)
{
    SPReprDoc *doc = (SPReprDoc *) sp_repr_new_from_code(SP_XML_DOCUMENT_NODE, g_quark_from_static_string("xml"));
    if (!strcmp(rootname, "svg")) {
        sp_repr_set_attr(doc, "version", "1.0");
        sp_repr_set_attr(doc, "standalone", "no");
        sp_repr_set_attr(doc, "doctype",
                         "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n"
                         "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");

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
    sp_repr_remove_child(doc, doc->children);

    for ( GSList *iter = reprs ; iter ; iter = iter->next ) {
        SPRepr *repr = (SPRepr *) iter->data;
        sp_repr_append_child(doc, repr);
    }

    g_assert(sp_repr_document_root(doc) != NULL);

    return doc;
}

SPRepr *
sp_repr_document_first_child(SPReprDoc const *doc)
{
    return doc->children;
}

SPReprDoc *
sp_repr_document(SPRepr const *repr)
{
    return repr->doc;
}

SPRepr *sp_repr_document_root(SPReprDoc const *doc)
{
    g_assert( doc != NULL );

    /* We can have comments before the root node. */
    for (SPRepr *repr = doc->children ; repr ; repr = repr->next ) {
        if ( repr->type == SP_XML_ELEMENT_NODE ) {
            return repr;
        }
    }
    return NULL;
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

/**
 * Duplicates all attributes and children from src into doc.
 * Does NOT erase original attributes and children.
 */
unsigned
sp_repr_merge(SPRepr *repr, SPRepr const *src, gchar const *key)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(src != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    repr->content = src->content;

    for (SPRepr *child = src->children; child != NULL; child = child->next) {
        gchar const *id = sp_repr_attr(child, key);
        if (id) {
            SPRepr *rch = sp_repr_lookup_child(repr, key, id);
            if (rch) {
                sp_repr_merge(rch, child, key);
            } else {
                rch = sp_repr_duplicate(child);
                sp_repr_append_child(repr, rch);
                sp_repr_unref(rch);
            }
        } else {
            SPRepr *rch = sp_repr_duplicate(child);
            sp_repr_append_child(repr, rch);
            sp_repr_unref(rch);
        }
    }

    for (SPReprAttr *attr = src->attributes; attr != NULL; attr = attr->next) {
        sp_repr_set_attr(repr, SP_REPR_ATTRIBUTE_KEY(attr), SP_REPR_ATTRIBUTE_VALUE(attr));
    }

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
