#define __SP_REPR_C__

/*
 * Fuzzy DOM-like tree implementation
 *
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

#include "repr-private.h"

typedef struct SPReprListener SPListener;

static void repr_init(SPRepr *repr);
static void repr_doc_init(SPRepr *repr);

static void repr_copy(SPRepr *to, SPRepr const *from);
static void repr_doc_copy(SPRepr *to, SPRepr const *from);

static void repr_finalize(SPRepr *repr);
static void repr_doc_finalize(SPRepr *repr);

static void bind_document(SPReprDoc *doc, SPRepr *repr);

SPReprClass _sp_repr_xml_document_class = {
    sizeof(SPReprDoc),
    NULL,
    repr_doc_init,
    repr_doc_copy,
    repr_doc_finalize
};

SPReprClass _sp_repr_xml_element_class = {
    sizeof(SPRepr),
    NULL,
    repr_init,
    repr_copy,
    repr_finalize
};

SPReprClass _sp_repr_xml_text_class = {
    sizeof(SPRepr),
    NULL,
    repr_init,
    repr_copy,
    repr_finalize
};

SPReprClass _sp_repr_xml_comment_class = {
    sizeof(SPRepr),
    NULL,
    repr_init,
    repr_copy,
    repr_finalize
};

static SPRepr *sp_repr_new_from_code(SPReprClass *type, int code);
static void sp_repr_remove_attribute(SPRepr *repr, SPReprAttr *attr);
static void sp_repr_remove_listener(SPRepr *repr, SPListener *listener);

static SPReprAttr *sp_attribute_duplicate(SPReprAttr const *attr);
static SPReprAttr *sp_attribute_new_from_code(int key, gchar const *value);

static SPRepr *sp_repr_alloc(SPReprClass *type);
static void sp_repr_free(SPRepr *repr);
static SPReprAttr *sp_attribute_alloc(void);
static void sp_attribute_free(SPReprAttr *attribute);
static SPListener *sp_listener_alloc(void);
static void sp_listener_free(SPListener *listener);

static SPRepr *
sp_repr_new_from_code(SPReprClass *type, int code)
{
    SPRepr *repr = sp_repr_alloc(type);
    repr->name = code;
    repr->type->init(repr);

    return repr;
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
    repr->content = g_strdup(content);
    return repr;
}

SPRepr *
sp_repr_new_comment(gchar const *comment)
{
    g_return_val_if_fail(comment != NULL, NULL);
    SPRepr *repr = sp_repr_new_from_code(SP_XML_COMMENT_NODE, g_quark_from_static_string("comment"));
    repr->content = g_strdup(comment);
    return repr;
}

static void
repr_init(SPRepr *repr)
{
    repr->refcount = 1;
    repr->doc = NULL;
    repr->parent = repr->next = repr->children = NULL;
    repr->attributes = NULL;
    repr->last_listener = repr->listeners = NULL;
    repr->content = NULL;
}

static void
repr_doc_init(SPRepr *repr)
{
    SPReprDoc *doc = (SPReprDoc *) repr;

    repr_init(repr);

    repr->doc = doc;
    doc->log = NULL;
    doc->is_logging = false;
}

SPRepr *
sp_repr_ref(SPRepr *repr)
{
    g_return_val_if_fail(repr != NULL, NULL);
    g_return_val_if_fail(repr->refcount > 0, NULL);

    repr->refcount += 1;

    return repr;
}

SPRepr *
sp_repr_unref(SPRepr *repr)
{
    g_return_val_if_fail(repr != NULL, NULL);
    g_return_val_if_fail(repr->refcount > 0, NULL);

    repr->refcount -= 1;

    if (repr->refcount < 1) {
        repr->type->finalize(repr);
        sp_repr_free(repr);
    }

    return NULL;
}

static void
repr_finalize(SPRepr *repr)
{
    SPReprListener *rl;
    /* Parents reference children */
    g_assert(repr->parent == NULL);
    g_assert(repr->next == NULL);
    repr->doc = NULL;

    for (rl = repr->listeners; rl; rl = rl->next) {
        if (rl->vector->destroy) (* rl->vector->destroy)(repr, rl->data);
    }
    while (repr->children) sp_repr_remove_child(repr, repr->children);
    while (repr->attributes) sp_repr_remove_attribute(repr, repr->attributes);
    g_free(repr->content);
    while (repr->listeners) sp_repr_remove_listener(repr, repr->listeners);
}

static void
repr_doc_finalize(SPRepr *repr)
{
    SPReprDoc *doc = (SPReprDoc *) repr;
    if (doc->log) {
        sp_repr_free_log(doc->log);
        doc->log = NULL;
    }
    repr_finalize(repr);
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
    SPRepr *new_repr = sp_repr_new_from_code(repr->type, repr->name);
    repr->type->copy(new_repr, repr);
    return new_repr;
}

void
repr_copy(SPRepr *to, SPRepr const *from)
{
    g_return_if_fail(from != NULL);

    if (from->content != NULL) {
        to->content = g_strdup(from->content);
    }

    SPRepr *lastchild = NULL;
    for (SPRepr *child = from->children; child != NULL; child = child->next) {
        if (lastchild) {
            lastchild = lastchild->next = sp_repr_attach(to, sp_repr_duplicate(child));
        } else {
            lastchild = to->children = sp_repr_attach(to, sp_repr_duplicate(child));
        }
    }

    SPReprAttr *lastattr = NULL;
    for (SPReprAttr *attr = from->attributes; attr != NULL; attr = attr->next) {
        if (lastattr) {
            lastattr = lastattr->next = sp_attribute_duplicate(attr);
        } else {
            lastattr = to->attributes = sp_attribute_duplicate(attr);
        }
    }
}

void
repr_doc_copy(SPRepr *to, SPRepr const *from)
{
    SPReprDoc *to_doc = (SPReprDoc *) to;
    repr_copy(to, from);
    to_doc->log = NULL;
    to_doc->is_logging = false;
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
 * Retrieves the first attribute in the XML representation with
 * the given key 'key'
 */
gchar const *
sp_repr_attr(SPRepr const *repr, gchar const *key)
{
    g_return_val_if_fail(repr != NULL, NULL);
    g_return_val_if_fail(key != NULL, NULL);

    /* retrieve an int identifier specific to this string */
    unsigned const q = g_quark_from_string(key);

    for (SPReprAttr *ra = repr->attributes; ra != NULL; ra = ra->next) {
        if ( ra->key == static_cast<int>(q) ) {
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
        gchar *oldcontent = repr->content;

        if (newcontent) {
            repr->content = g_strdup(newcontent);
        } else {
            repr->content = NULL;
        }
        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = sp_repr_log_chgcontent(repr->doc->log, repr, oldcontent, newcontent);
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->content_changed) {
                (* rl->vector->content_changed)(repr, oldcontent, newcontent, rl->data);
            }
        }

        g_free(oldcontent);
    }

    return allowed;
}

static unsigned
sp_repr_del_attr(SPRepr *repr, gchar const *key, bool is_interactive)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(*key != '\0', FALSE);

    unsigned const q = g_quark_from_string(key);
    SPReprAttr *prev = NULL;
    SPReprAttr *attr;
    for (attr = repr->attributes; attr && (attr->key != static_cast<int>(q)); attr = attr->next) {
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
                repr->doc->log = sp_repr_log_chgattr(repr->doc->log, repr, q, attr->value, NULL);
            }

            for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
                if (rl->vector->attr_changed) {
                    (* rl->vector->attr_changed)(repr, key, attr->value, NULL, is_interactive, rl->data);
                }
            }

            sp_attribute_free(attr);
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

    unsigned const q = g_quark_from_string(key);
    SPReprAttr *prev = NULL;
    SPReprAttr *attr;
    for (attr = repr->attributes; attr && (attr->key != static_cast<int>(q)); attr = attr->next) {
        prev = attr;
    }

    if ( attr && !strcmp(attr->value, value) ) {
        return TRUE;
    }

    unsigned allowed = TRUE;
    for (SPReprListener *rl = repr->listeners; rl && allowed; rl = rl->next) {
        if (rl->vector->change_attr) {
            allowed = (* rl->vector->change_attr)(repr, key, ( attr ? attr->value : NULL ), value, rl->data);
        }
    }

    if (allowed) {
        gchar *oldval = ( attr ? attr->value : NULL );

        if (attr) {
            attr->value = g_strdup(value);
        } else {
            attr = sp_attribute_new_from_code(q, value);
            if (prev) {
                prev->next = attr;
            } else {
                repr->attributes = attr;
            }
        }
        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = sp_repr_log_chgattr(repr->doc->log, repr, q, oldval, value);
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->attr_changed) {
                (* rl->vector->attr_changed)(repr, key, oldval, value, is_interactive, rl->data);
            }
        }

        g_free(oldval);
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


static void
sp_repr_remove_attribute(SPRepr *repr, SPReprAttr *attr)
{
    g_assert(repr != NULL);
    g_assert(attr != NULL);

    if (attr == repr->attributes) {
        repr->attributes = attr->next;
    } else {
        SPReprAttr *prev = repr->attributes;
        while (prev->next != attr) {
            prev = prev->next;
            g_assert(prev != NULL);
        }
        prev->next = attr->next;
    }

    sp_attribute_free(attr);
}

SPRepr *
sp_repr_parent(SPRepr *repr)
{
    g_assert(repr != NULL);

    return repr->parent;
}

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
        sp_repr_ref(child);

        if (child->doc == NULL) bind_document(repr->doc, child);

        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = sp_repr_log_add(repr->doc->log, repr, child, ref);
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
            repr->doc->log = sp_repr_log_remove(repr->doc->log, repr, child, ref);
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->child_removed) {
                (* rl->vector->child_removed)(repr, child, ref, rl->data);
            }
        }

#ifdef REPR_VERBOSE
        if (child->refcount > 1) {
            g_warning("Detaching repr with refcount > 1");
        }
#endif
        sp_repr_unref(child);
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
        if (prev) {
            prev->next = child->next;
        } else {
            repr->children = child->next;
        }
        if (ref) {
            child->next = ref->next;
            ref->next = child;
        } else {
            child->next = repr->children;
            repr->children = child;
        }

        if ( repr->doc && repr->doc->is_logging ) {
            repr->doc->log = sp_repr_log_chgorder(repr->doc->log, repr, child, prev, ref);
        }

        for (SPReprListener *rl = repr->listeners; rl != NULL; rl = rl->next) {
            if (rl->vector->order_changed) {
                (* rl->vector->order_changed)(repr, child, prev, ref, rl->data);
            }
        }
    }

    return allowed;
}

void
sp_repr_set_position_absolute(SPRepr *repr, int pos)
{
    SPRepr *parent = repr->parent;

    if (pos < 0) {
        pos = 0x7fffffff;
        /* fixme: Would INT__MAX be better?  Better yet, should pos be unsigned?  -- pjrm. */
    }

    SPRepr *ref = NULL;
    SPRepr *cur = parent->children;
    while (pos > 0 && cur) {
        ref = cur;
        cur = cur->next;
        pos -= 1;
    }

    if (ref == repr) {
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

    SPReprListener *rl = sp_listener_alloc();
    rl->next = NULL;
    rl->vector = vector;
    rl->data = data;

    if (repr->last_listener) {
        repr->last_listener->next = rl;
    } else {
        repr->listeners = rl;
    }
    repr->last_listener = rl;
}

static void
sp_repr_remove_listener(SPRepr *repr, SPListener *listener)
{
    SPReprListener *prev = NULL;
    for ( SPReprListener *iter = repr->listeners ; iter ; iter = iter->next ) {
        if ( iter == listener ) {
            if (prev) {
                prev->next = listener->next;
            } else {
                repr->listeners = listener->next;
            }
            if (!listener->next) {
                repr->last_listener = prev;
            }
            break;
        }
        prev = iter;
    }

    sp_listener_free(listener);
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
            sp_listener_free(rl);
            return;
        }
        prev = rl;
    }
}

SPRepr *
sp_repr_nth_child(SPRepr const *repr, int n)
{
    SPRepr *child = repr->children;

    while (n > 0 && child) {
        child = child->next;
        n -= 1;
    }

    return child;
}

/* Documents - 1st step in migrating to real XML */
/* fixme: Do this somewhere, somehow The Right Way (TM) */

SPReprDoc *
sp_repr_document_new(char const *rootname)
{
    SPReprDoc *doc = (SPReprDoc *) sp_repr_new_from_code(SP_XML_DOCUMENT_NODE, g_quark_from_static_string("xml"));
    if (!strcmp(rootname, "svg")) {
        sp_repr_set_attr(&doc->repr, "version", "1.0");
        sp_repr_set_attr(&doc->repr, "standalone", "no");
        sp_repr_set_attr(&doc->repr, "doctype",
                         "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n"
                         "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n");

        SPRepr *comment = sp_repr_new_comment(" Created with Inkscape (http://www.inkscape.org/) ");
        sp_repr_append_child(&doc->repr, comment);
        sp_repr_unref(comment);
    }

    SPRepr *root = sp_repr_new(rootname);
    sp_repr_append_child(&doc->repr, root);
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
    sp_repr_remove_child(&doc->repr, doc->repr.children);

    for ( GSList *iter = reprs ; iter ; iter = iter->next ) {
        SPRepr *repr = (SPRepr *) iter->data;
        sp_repr_append_child(&doc->repr, repr);
    }

    g_assert(sp_repr_document_root(doc) != NULL);

    return doc;
}

SPRepr *
sp_repr_document_first_child(SPReprDoc const *doc)
{
    return doc->repr.children;
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
    for (SPRepr *repr = doc->repr.children ; repr ; repr = repr->next ) {
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

/*
 * Duplicate all attributes and children from src into doc
 * Does NOT erase original attributes and children
 */

unsigned
sp_repr_merge(SPRepr *repr, SPRepr const *src, gchar const *key)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(src != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    g_free(repr->content);
    repr->content = ( src->content
                      ? g_strdup(src->content)
                      : NULL );

    for (SPRepr *child = src->children; child != NULL; child = child->next) {
        gchar const *id = sp_repr_attr(child, key);
        if (id) {
            SPRepr *rch = sp_repr_lookup_child(repr, key, id);
            if (rch) {
                sp_repr_merge(rch, child, key);
            } else {
                rch = sp_repr_duplicate(child);
                sp_repr_append_child(repr, rch);
            }
        } else {
            SPRepr *rch = sp_repr_duplicate(child);
            sp_repr_append_child(repr, rch);
        }
    }

    for (SPReprAttr *attr = src->attributes; attr != NULL; attr = attr->next) {
        sp_repr_set_attr(repr, SP_REPR_ATTRIBUTE_KEY(attr), SP_REPR_ATTRIBUTE_VALUE(attr));
    }

    return TRUE;
}

static SPReprAttr *
sp_attribute_duplicate(SPReprAttr const *attr)
{
    SPReprAttr *new_attr = sp_attribute_alloc();
    new_attr->next = NULL;
    new_attr->key = attr->key;
    new_attr->value = g_strdup(attr->value);

    return new_attr;
}

static SPReprAttr *
sp_attribute_new_from_code(int key, gchar const *value)
{
    SPReprAttr *new_attr = sp_attribute_alloc();
    new_attr->next = NULL;
    new_attr->key = key;
    new_attr->value = g_strdup(value);

    return new_attr;
}

#define SP_REPR_CHUNK_SIZE 32

static SPRepr *
sp_repr_alloc(SPReprClass *type)
{
    SPRepr *repr;
    if (type->pool) {
        repr = type->pool;
        type->pool = repr->next;
    } else {
        char *chunk = (char *) g_malloc( type->size * SP_REPR_CHUNK_SIZE );

        size_t max = type->size * ( SP_REPR_CHUNK_SIZE - 1 );
        size_t offset;
        for ( offset = type->size ;
              offset < max ;
              offset += type->size )
        {
            repr = (SPRepr *)(chunk+offset);
            repr->next = (SPRepr *)(chunk+offset+type->size);
        }

        ((SPRepr *) (chunk + offset))->next = NULL;
        type->pool = (SPRepr *) (chunk+type->size);

        repr = (SPRepr *) chunk;
    }
    repr->type = type;

    return repr;
}

static void
sp_repr_free(SPRepr *repr)
{
    repr->next = repr->type->pool;
    repr->type->pool = repr;
}

#define SP_ATTRIBUTE_ALLOC_SIZE 32
static SPReprAttr *free_attribute = NULL;

static SPReprAttr *
sp_attribute_alloc(void)
{
    SPReprAttr *attribute;
    if (free_attribute) {
        attribute = free_attribute;
        free_attribute = free_attribute->next;
    } else {
        attribute = g_new(SPReprAttr, SP_ATTRIBUTE_ALLOC_SIZE);
        for (int i = 1; i < SP_ATTRIBUTE_ALLOC_SIZE - 1; i++) {
            attribute[i].next = attribute + i + 1;
        }
        attribute[SP_ATTRIBUTE_ALLOC_SIZE - 1].next = NULL;
        free_attribute = attribute + 1;
    }

    return attribute;
}

static void
sp_attribute_free(SPReprAttr *attribute)
{
    if (attribute->value) {
        g_free(attribute->value);
        attribute->value = NULL;
    }
    attribute->next = free_attribute;
    free_attribute = attribute;
}

#define SP_LISTENER_ALLOC_SIZE 32
static SPListener *free_listener = NULL;

static SPListener *
sp_listener_alloc()
{
    SPListener *listener;
    if (free_listener) {
        listener = free_listener;
        free_listener = free_listener->next;
    } else {
        listener = g_new(SPListener, SP_LISTENER_ALLOC_SIZE);
        for (int i = 1; i < SP_LISTENER_ALLOC_SIZE - 1; i++) {
            listener[i].next = listener + i + 1;
        }
        listener[SP_LISTENER_ALLOC_SIZE - 1].next = NULL;
        free_listener = listener + 1;
    }

    return listener;
}

static void
sp_listener_free(SPListener *listener)
{
    listener->next = free_listener;
    free_listener = listener;
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
