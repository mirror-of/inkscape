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

#include "util/shared-c-string-ptr.h"

#include "xml/repr.h"
#include "xml/repr-get-children.h"
#include "xml/sp-repr-listener.h"
#include "xml/sp-repr-event-vector.h"
#include "xml/sp-repr-attr.h"
#include "xml/sp-repr-action.h"
#include "xml/sp-repr-action-fns.h"
#include "xml/simple-session.h"
#include "xml/text-node.h"
#include "xml/element-node.h"
#include "xml/comment-node.h"
#include "xml/simple-document.h"

using Inkscape::Util::SharedCStringPtr;

SPRepr *
sp_repr_new(gchar const *name)
{
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(*name != '\0', NULL);

    return new Inkscape::XML::ElementNode(g_quark_from_string(name));
}

SPRepr *
sp_repr_new_text(gchar const *content)
{
    g_return_val_if_fail(content != NULL, NULL);
    return new Inkscape::XML::TextNode(SharedCStringPtr::copy(content));
}

SPRepr *
sp_repr_new_comment(gchar const *comment)
{
    g_return_val_if_fail(comment != NULL, NULL);
    return new Inkscape::XML::CommentNode(SharedCStringPtr::copy(comment));
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

gchar const *
sp_repr_name(SPRepr const *repr)
{
    g_return_val_if_fail(repr != NULL, NULL);
    return repr->name();
}

gchar const *
sp_repr_content(SPRepr const *repr)
{
    g_assert(repr != NULL);
    return repr->content();
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

unsigned
sp_repr_set_content(SPRepr *repr, gchar const *newcontent) {
    g_return_val_if_fail(repr != NULL, FALSE);
    repr->setContent(newcontent);
    return true;
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

unsigned
sp_repr_remove_child(SPRepr *repr, SPRepr *child)
{
    g_assert(repr != NULL);
    repr->removeChild(child);
    return true;
}

unsigned
sp_repr_change_order(SPRepr *const repr, SPRepr *const child, SPRepr *const ref)
{
    g_assert( repr != NULL );
    repr->changeOrder(child, ref);
    return true;
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
sp_repr_synthesize_events(SPRepr *repr, SPReprEventVector const *vector, void *data)
{
    repr->synthesizeEvents(vector, data);
}

void
sp_repr_add_listener(SPRepr *repr, SPReprEventVector const *vector, void *data)
{
    g_assert(repr != NULL);
    repr->addListener(vector, data);
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
    SPReprDoc *doc = new Inkscape::XML::SimpleDocument(g_quark_from_static_string("xml"));
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
