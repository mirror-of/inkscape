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

Inkscape::XML::Node *
sp_repr_new(gchar const *name)
{
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(*name != '\0', NULL);

    return new Inkscape::XML::ElementNode(g_quark_from_string(name));
}

Inkscape::XML::Node *
sp_repr_new_text(gchar const *content)
{
    g_return_val_if_fail(content != NULL, NULL);
    return new Inkscape::XML::TextNode(SharedCStringPtr::copy(content));
}

Inkscape::XML::Node *
sp_repr_new_comment(gchar const *comment)
{
    g_return_val_if_fail(comment != NULL, NULL);
    return new Inkscape::XML::CommentNode(SharedCStringPtr::copy(comment));
}

Inkscape::XML::Document *
sp_repr_document_new(char const *rootname)
{
    Inkscape::XML::Document *doc = new Inkscape::XML::SimpleDocument(g_quark_from_static_string("xml"));
    if (!strcmp(rootname, "svg:svg")) {
        sp_repr_set_attr(doc, "version", "1.0");
        sp_repr_set_attr(doc, "standalone", "no");
        Inkscape::XML::Node *comment = sp_repr_new_comment(" Created with Inkscape (http://www.inkscape.org/) ");
        doc->appendChild(comment);
        sp_repr_unref(comment);
    }

    Inkscape::XML::Node *root = sp_repr_new(rootname);
    doc->appendChild(root);
    sp_repr_unref(root);

    return doc;
}

Inkscape::XML::Document *
sp_repr_document_new_list(GSList *reprs)
{
    g_assert(reprs != NULL);

    Inkscape::XML::Document *doc = sp_repr_document_new("void");
    doc->removeChild(doc->firstChild());

    for ( GSList *iter = reprs ; iter ; iter = iter->next ) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) iter->data;
        doc->appendChild(repr);
    }

    g_assert(sp_repr_document_root(doc) != NULL);

    return doc;
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
